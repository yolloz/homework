use ActivityTracker
go

--
if object_id('CreateUserWithName', 'P') is not null drop procedure CreateUserWithName
go

create procedure CreateUserWithName
	@username varchar(30),
	@passwd nvarchar(30),
	@firstName nvarchar(50),
	@lastName nvarchar(50)
	as begin
		set nocount on
		-- cut whitespaces from username
		set @username = ltrim(rtrim(@username))
		-- if username is too short, alert user 
		if len(@username) < 3 raiserror('Username is too short', 15,1)
		-- check if username is already present
		if (select count(*) from [User] where Username = @username) > 0 
			raiserror('Username is already taken!', 15,1)
		if len(@passwd) < 6 raiserror('Password is too short', 15,1)
		-- we made it here, so can create user
		begin transaction
		insert into [User] (Username, Passwd, FirstName, LastName, Active, CreateDate, LastLogin)
			values (@username, @passwd, @firstName, @lastName, 1, getdate(), null)
		commit transaction
	end
go

if object_id('CreateUserWithoutName', 'P') is not null drop procedure CreateUserWithoutName
go
	
create procedure CreateUserWithoutName 
	@username varchar(30),
	@passwd nvarchar(30)
	as begin
		exec CreateUserWithName @username, @passwd, null, null
	end
go

if object_id('DeleteUser', 'P') is not null drop procedure DeleteUser
go

create procedure DeleteUser
	@pUser bigint
	as begin
		set nocount on

		begin transaction
		delete from Friendship where User1 = @pUser or User2 = @pUser
		delete from Activity where UserID = @pUser
		delete from ChallengeParticipation where UserID = @pUser
		delete from [Route] where ID in 
		(
			select ID 
			 from [Route] as r
			 where OwnerID = @pUser
			 and not exists
			 ( select * from Activity where RouteID = r.ID)
		)
		update [Route] set OwnerID = null 
			where ID in 
			(
				select ID 
				from [Route] as r
				where OwnerID = @pUser
				and exists
				( select * from Activity where RouteID = r.ID)
			)
		delete from [User] where ID = @pUser
		commit transaction
	end
go

if object_id('LoginUser', 'P') is not null drop procedure LoginUser
go

create procedure LoginUser
	@username varchar(30),
	@passwd nvarchar(30)
	as begin
		set nocount on
		if exists (select * from [User] where Username = @username and Passwd = @passwd)
			begin
				begin transaction
				update [User] set LastLogin = getdate() where Username = @username
				commit transaction
			end
		else
			raiserror('Invalid username or password', 15, 1)
	end
go

if object_id('CreateChallenge', 'P') is not null drop procedure CreateChallenge
go

create procedure CreateChallenge
	@pType bigint,
	@pDesc nvarchar(max),
	@pGoal nvarchar(255),
	@pPrize nvarchar(255),
	@pStart datetime,
	@pEnd datetime
	as begin
		set nocount on

		begin transaction
		insert into Challenge ( ActivityType, [Description], Goal, Prize, StartDate, EndDate)
			values ( @pType, @pDesc, @pGoal, @pPrize, @pStart, @pEnd)
		commit transaction

	end
go

if object_id('JoinChallenge', 'P') is not null drop procedure JoinChallenge
go

create procedure JoinChallenge
	@pUser bigint,
	@pChallenge bigint
	as begin
		set nocount on

		if ((select count(*) from ChallengeParticipation where UserID = @pUser and ChallengeID = @pChallenge) = 0)
			begin
				begin transaction
				insert into ChallengeParticipation (UserID, ChallengeID) values (@pUser, @pChallenge)
				commit transaction
			end
		else
			raiserror('You already take part in this challenge', 15, 1)
	end
go

if object_id('TrackActivity', 'P') is not null drop procedure TrackActivity
go

create procedure TrackActivity
	@pUser bigint,
	@pRoute bigint,
	@pType bigint,
	@pDuration bigint,
	@pDistance numeric(5,3),
	@pStart datetime
	as begin
		set nocount on

		begin transaction
		insert into Activity(UserID, RouteID, ActivityTypeID, Duration, Distance, StartDate)
			values (@pUser, @pRoute, @pType, @pDuration, @pDistance, @pStart)
		commit transaction
	end
go

if object_id('ToTimeString', 'FN') is not null drop function ToTimeString
go

create function ToTimeString(@pSeconds bigint)
	returns varchar(30)
	as begin
		/*declare @hours bigint = @pSeconds / 3600
		set @pSeconds = @pSeconds % 3600
		declare @minutes bigint = @pSeconds / 60
		set @pSeconds = @pSeconds % 60*/
		return(SELECT CONVERT(varchar, DATEADD(ms, @pSeconds * 1000, 0), 108))
	end
go

if object_id('AreFriends', 'FN') is not null drop function AreFriends
go

create function AreFriends(
	@pUser1 bigint,
	@pUser2 bigint
	) 
	returns bit
	as begin
		-- check if users are already friends
		declare @friends bit
		if ((select count(*) from Friendship where (User1 = @pUser1 and User2 = @pUser2) or (User1 = @pUser2 and User2 = @pUser1)) > 0)
			set @friends = 1 
			else set @friends = 0
		return @friends
	end
go

if object_id('MakeFriend', 'P') is not null drop procedure MakeFriend
go

create procedure MakeFriend
	@pUser1 bigint,
	@pUser2 bigint
	as begin
		set nocount on

		-- check if users are already friends
		declare @friends bit = ActivityTracker.dbo.AreFriends(@pUser1, @pUser2)
		if @friends = 1
			raiserror('Users are already friends', 15, 1)
		else
			begin
				begin transaction
				insert into Friendship (User1, User2, Pending, CreateDate)
					values (@pUser1, @pUser2, 1, GETDATE())
				commit transaction
			end		
	end
go

if object_id('AcceptRequest', 'P') is not null drop procedure AcceptRequest
go

create procedure AcceptRequest
	@pUser bigint,
	@pFriend bigint
	as
	begin
		set nocount on

		if exists ( select * from Friendship where User1 = @pFriend and User2 = @pUser and Pending = 1 )
			begin
				begin transaction
				update Friendship set Pending = 0 where User1 = @pFriend and User2 = @pUser and Pending = 1
				commit transaction
			end
			else
				raiserror('Unable to accept friend request', 15, 1)
	end
go 

if object_id('CreateRoute', 'P') is not null drop procedure CreateRoute
go

create procedure CreateRoute
	@pUser bigint,
	@pDescription nvarchar(255),
	@pPublic bit
	as
	begin
		set nocount on
		begin transaction
		insert into [Route]([Description], OwnerID, [Public], CreateDate) values (@pDescription, @pUser, @pPublic, getdate())
		commit transaction
	end
go

if object_id('DeleteRoute', 'P') is not null drop procedure DeleteRoute
go 

create procedure DeleteRoute
	@pRoute bigint,
	@pUser bigint
	as
	begin
		set nocount on
		declare @publ bit
		select @publ = [Public] from [Route] where ID = @pRoute
		if (select OwnerID = [Public] from [Route] where ID = @pRoute) != @pUser
			raiserror('You are not allowed to delete this route!', 15,1)

		begin transaction
		-- set route as null for owner where it was picked
		update [Activity] set RouteID = null where RouteID = @pRoute and UserID = @pUser
		if @publ = 0 or not exists(select * from Activity where RouteID = @pRoute and UserID != @pUser)			
			delete from [Route] where ID = @pRoute
		else
			update [Route] set OwnerID = null where ID = @pRoute
		commit transaction
	end
go 

if object_id('ChallengeLeaderboards', 'V') is not null drop view ChallengeLeaderboards
go

create view ChallengeLeaderboards
	as
		select cp.UserID, cp.ChallengeID, sum(a.Distance) as totalDistance, sum(a.Duration) as totalDuration
			from Challenge as ch
			inner join ChallengeParticipation as cp on (ch.ID = cp.ChallengeID)
			inner join Activity as a on (a.UserID = cp.UserID)
			inner join [User] as u on (cp.UserID = u.ID)
			where a.ActivityTypeID = ch.ActivityType
			group by cp.UserID, cp.ChallengeID
go

if object_id('Friends', 'V') is not null drop view Friends
go

create view Friends
	as
		select u1.ID as UserID,
			   u1.Username as Username,
			   u1.FirstName as [First name],
			   u1.Lastname as [Last name],
			   u2.ID as FriendID,
			   u2.Username as [Friend Username],
			   u2.FirstName as [Friend First name],
			   u2.Lastname as [Friend Last name],
			   f.CreateDate as FriendsFrom
			from Friendship as f
			inner join [User] as u1 on (f.User1 = u1.ID)
			inner join [User] as u2 on (f.User2 = u2.ID)
			where Pending = 0
		union
		select u1.ID as UserID,
			   u1.Username as Username,
			   u1.FirstName as [First name],
			   u1.Lastname as [Last name],
			   u2.ID as FriendID,
			   u2.Username as [Friend Username],
			   u2.FirstName as [Friend First name],
			   u2.Lastname as [Friend Last name],
			   f.CreateDate as FriendsFrom
			from Friendship as f
			inner join [User] as u1 on (f.User2 = u1.ID)
			inner join [User] as u2 on (f.User1 = u2.ID)
			where Pending = 0
go

if object_id('FriendRequests', 'V') is not null drop view FriendRequests
go

create view FriendRequests
	as
		select u1.ID as UserID,
			   u1.Username as Username,
			   u1.FirstName as [First name],
			   u1.Lastname as [Last name],
			   u2.ID as RequesterID,
			   u2.Username as [Requester Username],
			   u2.FirstName as [Requester First name],
			   u2.Lastname as [Requester Last name],
			   f.CreateDate as RequestFrom
			from Friendship as f
			inner join [User] as u1 on (f.User2 = u1.ID)
			inner join [User] as u2 on (f.User1 = u2.ID)
			where Pending = 1
go

if object_id('MonthlyStatistics', 'V') is not null drop view MonthlyStatistics
go

create view MonthlyStatistics
	as
		select u.ID as UserID,
			   u.Username,
			   stat.[Year],
			   stat.[Month],
			   stat.Workouts,
			   ActivityTracker.dbo.ToTimeString(stat.OveralDuration) as OveralDuration,
			   stat.OveralDistance,
			   act.[Description] as Activity
			from
			(
			select UserID,
				   COUNT(*) as Workouts,	
				   sum(Duration) as OveralDuration,
				   sum(Distance) as OveralDistance,
				   MONTH(StartDate) as [Month],
				   YEAR(StartDate) as [Year],
				   ActivityTypeID
				from Activity 
				group by UserID, ActivityTypeID, MONTH(StartDate), YEAR(StartDate)
			) as stat
			inner join [User] as u on (stat.UserID = u.ID)
			inner join ActivityType as act on (stat.ActivityTypeID = act.ID)
go


if object_id('AvailableRoutes', 'V') is not null drop view AvailableRoutes
go

create view AvailableRoutes
as 
	select u.ID as UserID, r.*
	from [User] as u
	inner join [Route] as r on (u.ID = r.OwnerID or (u.id != r.OwnerID and r.OwnerID is not null and r.[Public] = 1) or (r.OwnerID is null and r.[Public] = 1))
	 






