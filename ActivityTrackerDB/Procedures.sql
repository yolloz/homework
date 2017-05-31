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
		insert into [User] (Username, Passwd, FirstName, LastName, Active, CreateDate, LastLogin)
			values (@username, @passwd, @firstName, @lastName, 1, getdate(), null)
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
	@username varchar(30)
	as begin
		set nocount on
		delete from [User] where Username = @username
	end
go