#ifndef PLUSPLUSCHAT_ENUMS_
#define PLUSPLUSCHAT_ENUMS_

namespace PlusPlusChat {

#include "../../PlusPlusChat_shared/SharedEnums.h"

	// DISCONNECTED -> CONNECTING -> CONNECTED -> JOINING -> CHATTING
	enum AppState {
		DISCONNECTED, CONNECTED, CHATTING, CONNECTING, JOINING
	};

	//enum Action {
	//	INIT, TINI, /*ACK,*/ ERR, CREATE, JOIN, GETROOMS, ROOMS, JOINED, SEND, RECV, PING, PONG, INVALID_ACTION
	//};

}

#endif
