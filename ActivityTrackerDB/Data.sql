/*
	Autor: Michal Jurco
	skript: Vlozenie testovacich dat
*/

use ActivityTracker
go

-- fills the database with test data
insert into ActivityType ([Description]) 
	values 
		('Cycling'),
		('Running'),
		('Swimming'),
		('Walking'),
		('Weight training'),
		('Cross-counrty skiing'),
		('Downhill skiing')
-- test creating users
exec CreateUserWithName 'michal', 'heslo123', 'Michal', 'Jurèo'
exec CreateUserWithoutName 'noname', 'heslo123'
-- create army of users
exec CreateUserWithName 'bot1', 'heslo123', 'Peter', 'Novák'
exec CreateUserWithName 'bot2', 'heslo123', 'Ján', 'Novák'
exec CreateUserWithName 'bot3', 'heslo123', 'Jozef', 'Novák'
exec CreateUserWithName 'bot4', 'heslo123', 'Šimon', 'Novák'
exec CreateUserWithName 'bot5', 'heslo123', 'Peter', 'Horváth'
exec CreateUserWithName 'bot6', 'heslo123', 'Ján', 'Horváth'
exec CreateUserWithName 'bot7', 'heslo123', 'Jozef', 'Horváth'
exec CreateUserWithName 'bot8', 'heslo123', 'Šimon', 'Horváth'
exec CreateUserWithName 'bot9', 'heslo123', 'Peter', 'Veselý'
exec CreateUserWithName 'bot10', 'heslo123', 'Ján', 'Veselý'
exec CreateUserWithName 'bot11', 'heslo123', 'Jozef', 'Veselý'
exec CreateUserWithName 'bot12', 'heslo123', 'Šimon', 'Veselý'
-- make friends in families
exec MakeFriend 3, 4
exec MakeFriend 3, 5
exec MakeFriend 3, 6
exec MakeFriend 4, 5
exec MakeFriend 4, 6
exec MakeFriend 5, 6
exec MakeFriend 7, 8
exec MakeFriend 7, 9
exec MakeFriend 7, 10
exec MakeFriend 8, 9
exec MakeFriend 8, 10
exec MakeFriend 9, 10
exec MakeFriend 11, 12
exec MakeFriend 11, 13
exec MakeFriend 11, 14
exec MakeFriend 12, 13
exec MakeFriend 12, 14
exec MakeFriend 13, 14
-- make friends between Peters
exec MakeFriend 3, 7
exec MakeFriend 3, 11
exec MakeFriend 7, 11
--accept friendship in families
exec AcceptRequest 4, 3 
exec AcceptRequest 5, 3
exec AcceptRequest 6, 3
exec AcceptRequest 5, 4
exec AcceptRequest 6, 4
exec AcceptRequest 6, 5
exec AcceptRequest 8, 7
exec AcceptRequest 9, 7
exec AcceptRequest 10, 7
exec AcceptRequest 9, 8
exec AcceptRequest 10, 8
exec AcceptRequest 10, 9
exec AcceptRequest 12, 11
exec AcceptRequest 13, 11
exec AcceptRequest 14, 11
exec AcceptRequest 13, 12
exec AcceptRequest 14, 12
exec AcceptRequest 14, 13


exec CreateChallenge 2, 'Challenge 2017', 'Most km in 2017', 'A chocolate', '2017-01-01', '2018-01-01'

exec JoinChallenge 1, 1
exec JoinChallenge 3, 1

exec CreateRoute 1, 'Beh v parku', 1
exec CreateRoute 1, 'Vela kopcov', 0
exec CreateRoute 3, 'Okolo Prahy', 0
exec CreateRoute 3, 'Atleticky oval', 1
exec CreateRoute 4, 'Beh v horach', 0

exec TrackActivity 1, 1, 2, 3934, 11.68, '2017-06-07 16:23'
exec TrackActivity 1, 2, 1, 7327, 79.55, '2017-06-04 09:17'
exec TrackActivity 1, null, 2, 5178, 14.60, '2017-06-03 08:56'
exec TrackActivity 1, null, 1, 5850, 45.86, '2017-06-02 10:27'
exec TrackActivity 1, null, 3, 3600, 2.00, '2017-06-01 16:41'
exec TrackActivity 1, null, 3, 1800, 1.00, '2017-05-30 17:00'
exec TrackActivity 1, 4, 2, 3218, 8.88, '2017-05-30 08:44'

exec TrackActivity 3, 4, 2, 3934, 10.68, '2017-06-07 16:23'
exec TrackActivity 3, null, 1, 7327, 79.55, '2017-06-04 09:17'
exec TrackActivity 3, 1, 2, 5178, 13.60, '2017-06-03 08:56'
exec TrackActivity 3, 3, 1, 5850, 45.86, '2017-06-02 10:27'
exec TrackActivity 3, null, 3, 3600, 2.00, '2017-06-01 16:41'
exec TrackActivity 3, null, 3, 1800, 1.00, '2017-05-30 17:00'
exec TrackActivity 3, 4, 2, 3218, 7.88, '2017-05-30 08:44'
