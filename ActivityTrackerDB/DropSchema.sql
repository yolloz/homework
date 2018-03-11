/*
	Autor: Michal Jurco
	skript: Zrusenie schemy
*/

use ActivityTracker
go

begin transaction DropSchema

drop table Activity
drop table [Route]
drop table ChallengeParticipation
drop table Challenge
drop table Friendship
drop table ActivityType
drop table [User]

--rollback transaction
commit transaction