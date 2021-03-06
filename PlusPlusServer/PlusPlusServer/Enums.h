#ifndef PLUSPLUSSERVER_ENUMS_
#define PLUSPLUSSERVER_ENUMS_

#include "../../PlusPlusChat_shared/SharedEnums.h"

enum AppState
{
	LISTENING, STOPPED
};

//enum Action {
//	INIT, TINI, /*ACK,*/ ERR, CREATE, JOIN, GETROOMS, ROOMS, JOINED, SEND, RECV, PING, PONG, INVALID_ACTION
//};

enum ClientState {
	NEW, CONNECTED, CHATTING
};

enum ErrorCode : int {
	OK = 0, 
	INVALID_REQUEST = 1,
	UNAUTHORIZED_REQUEST = 2,
	ROOM_NAME_TAKEN = 3,
	USERNAME_TAKEN = 4,
	NO_SUCH_ROOM = 5,
	CLIENT_LIMIT_REACHED = 6
};

#endif
