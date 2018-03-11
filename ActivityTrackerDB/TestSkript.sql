/*
	Autor: Michal Jurco
	skript: Demonstracny skript
*/

use ActivityTracker
go

--najprv si vytvorime uzivatela, s ktorym sa budeme hrat
exec ActivityTracker.dbo.CreateUserWithName 'testUser', 'hesloheslo', 'Testovaci', 'Uzivatel';

--skusime si aj vytvorit uzivatela bez mena
exec ActivityTracker.dbo.CreateUserWithoutName 'uzivatelBezMena', 'heslo123';

--zmazeme uzivatela bez mena
declare @userID bigint = (select ID from [User] where Username = 'uzivatelBezMena');
exec ActivityTracker.dbo.DeleteUser @userID;

--prihlasime sa
exec LoginUser 'testUser', 'hesloheslo123' --nespravne heslo
exec LoginUser 'testUser', 'hesloheslo' --spravne heslo

--posleme ziadost o priatelstvo uzivatelovi 1
declare @userID bigint = (select ID from [User] where Username = 'testUser');
exec MakeFriend @userID, 1
select * from FriendRequests where UserID = 1

--vytvorime privatnu trasu
declare @userID bigint = (select ID from [User] where Username = 'testUser');
exec CreateRoute @userID, 'Testovacia trasa', 0
select * from AvailableRoutes where userID = @userID

--pridame sa do vyzvy
declare @userID bigint = (select ID from [User] where Username = 'testUser');
exec JoinChallenge @userID, 1

--pridame nejake aktivity
declare @userID bigint = (select ID from [User] where Username = 'testUser');
declare @routeID bigint = (select top 1 ID from AvailableRoutes where userID = @userID and [Description] = 'Testovacia trasa')
exec TrackActivity @userID, null, 3, 3600, 1.5, '2017-05-05 14:00:00' --plavanie 1H
exec TrackActivity @userID, @routeID, 2, 3000, 8, '2017-05-06 14:00:00' --beh 50min
select * from ChallengeLeaderboards where challengeID = 1 --priebezne vysledky

--pozrieme sa na svoje statistiky
declare @userID bigint = (select ID from [User] where Username = 'testUser');
select * from MonthlyStatistics where UserID = @userID

--pozrieme sa na nas news feed
declare @userID bigint = (select ID from [User] where Username = 'testUser');
select * from NewsFeed where UserID = @userID order by [Date] desc -- radime zostupne ako byva na news feede

--uzivatel 1 prijal nasu pozvanku, pozrieme sa na news feed teraz
declare @userID bigint = (select ID from [User] where Username = 'testUser');
exec AcceptRequest 1, @userID
select * from NewsFeed where UserID = @userID order by [Date] desc -- radime zostupne ako byva na news feede