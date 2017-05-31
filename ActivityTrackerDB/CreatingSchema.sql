use ActivityTracker
go

begin transaction CreateSchema

create table [User](
	ID bigint identity(1,1) not null,
	Username varchar(30) not null,
	Passwd nvarchar(30) not null,
	FirstName nvarchar(50) null,
	LastName nvarchar(50) null,
	Active bit not null default 1,
	CreateDate datetime not null default getdate(),
	LastLogin datetime null,
	constraint PK_User primary key clustered (ID),
	constraint UQ_User_Username unique (Username),
	constraint CHK_User_Username check (len(Username) >= 3),
	constraint CHK_User_Passwd check (len(Passwd) >= 6)
)

create table ActivityType(
	ID bigint identity(1,1) not null,
	[Description] nvarchar(30) not null,
	constraint PK_ActivityType primary key clustered (ID),
	constraint UQ_ActivityType_Description unique ([Description])
)

create table Friendship(
	User1 bigint not null,
	User2 bigint not null,
	CreateDate datetime not null default getdate(),
	constraint FK_Friendship_User1 foreign key (User1) references [User](ID)
		on delete cascade,
	constraint FK_Friendship_User2 foreign key (User2) references [User](ID)
		on delete cascade,
	constraint PK_Friendship primary key (User1, User2)
)

create table Challenge(
	ID bigint identity(1,1) not null,
	ActivityType bigint not null,
	[Description] nvarchar(max) not null,
	Goal nvarchar(255) null,
	Prize nvarchar(255) null,
	StartDate datetime not null,
	EndDate datetime not null,
	constraint PK_Challenge primary key clustered(ID),
	constraint FK_Challenge_ActivityType foreign key (ActivityType) references ActivityType(ID)
		on delete no action,
	constraint CHK_Challenge_Description check (len([Description]) >= 10),
	constraint CHK_Challenge_DateDifference check (StartDate < EndDate)
)

create table ChallengeParticipation(
	UserID bigint not null,
	ChallengeID bigint not null,
	constraint FK_ChallengeParticipation_UserID foreign key (UserID) references [User](ID)
		on delete cascade,
	constraint FK_ChallengeParticipation_ChallengeID foreign key (ChallengeID) references Challenge(ID)
		on delete cascade,
	constraint PK_ChallengeParticipation primary key (UserID, ChallengeID)
)

create table [Route](
	ID bigint identity(1,1) not null,
	[Description] nvarchar(255) null,
	OwnerID bigint not null,
	[Public] bit not null,
	CreateDate datetime not null default getdate(),
	constraint PK_Route primary key clustered (ID),
	constraint FK_Route_OwnerID foreign key (OwnerID) references [User](ID)
		on delete cascade
)

create table Activity(
	ID bigint identity(1,1) not null,
	UserID bigint not null,
	RouteID bigint null,
	ActivityTypeID bigint not null,
	Duration bigint not null,
	Distance numeric(5,3) null,
	StartDate datetime not null default getdate(),
	constraint PK_Activity primary key clustered (ID),
	constraint FK_Activity_UserID foreign key (UserID) references [User](ID)
		on delete cascade,
	constraint FK_Activity_RouteID foreign key (RouteID) references [Route](ID)
		on delete set null,
	constraint FK_Activity_ActivityTypeID foreign key (ActivityTypeID) references ActivityType(ID)
		on delete no action,
	constraint CHK_Activity_Duration check (Duration >= 0),
	constraint CHK_Activity_Distance check (Distance is null or Distance >= 0)
)

rollback transaction
--commit transaction