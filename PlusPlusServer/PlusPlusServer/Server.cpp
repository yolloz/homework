#include "Server.h"

Server & Server::GetInstance() {
	static Server instance;
	return instance;
}

AppState& Server::State() {
	return GetInstance()._state;
}

SOCKET & Server::ServerSocket() {
	return GetInstance()._serverSocket;
}

void Server::InitLookup() {
	_msgLookup[L"INIT"] = Action::INIT;
	//_msgLookup[L"ACK"] = Action::ACK;
	_msgLookup[L"ERR"] = Action::ERR;
	_msgLookup[L"CREATE"] = Action::CREATE;
	_msgLookup[L"JOIN"] = Action::JOIN;
	_msgLookup[L"GETROOMS"] = Action::GETROOMS;
	_msgLookup[L"INVALID_ACTION"] = Action::INVALID_ACTION;
	_msgLookup[L"JOINED"] = Action::JOINED;
	_msgLookup[L"SEND"] = Action::SEND;
	_msgLookup[L"RECV"] = Action::RECV;
	_msgLookup[L"PING"] = Action::PING;
	_msgLookup[L"PONG"] = Action::PONG;
	_msgLookup[L"HISTORY"] = Action::HISTORY;
	_msgLookup[L"GETHISTORY"] = Action::GETHISTORY;
}

Action Server::ResolveAction(std::wstring & msg) {
	auto && context = GetInstance();
	if (!context._lookupInitialized) {
		context.InitLookup();
		context._lookupInitialized = true;
	}
	auto i = context._msgLookup.find(msg);
	if (i == context._msgLookup.end()) {
		return Action::INVALID_ACTION;
	}
	else {
		return i->second;
	}
}

std::uint64_t Server::GetNewID() {
	return _id++;
}

bool Server::_CanAddClient() const{
	return _clients.size() < _maxActiveClients;
}

bool Server::_AddClient(SOCKET newSocket) {
	if (_clients.size() < _maxActiveClients) {
		ClientDetails c;
		auto id = GetNewID();
		c.state = ClientState::NEW;
		c.socket = newSocket;
		_clients[id] = c;
		_sockets[newSocket] = id;
		return true;
	}
	return false;;
}

bool Server::_RemoveClient(SOCKET client) {
	auto i = _sockets.find(client);
	if (i != _sockets.end()) {
		auto id = i->second;
		auto details = _clients[id];
		if (details.state == ClientState::CHATTING) {
			// remove from chatroom
			auto && room = _chatRooms[details.chatroomID];
			room.members.erase(id);
			// if last chatroom member left room, remove room
			if (room.members.size() == 0) {
				_chatRooms.erase(details.chatroomID);
			}
		}
		_clients.erase(id);
		_sockets.erase(i);
		return true;
	}
	return false;
}

std::uint64_t Server::AddChatRoom(std::wstring & name, bool privateRoom) {
	for (auto i = _chatRooms.begin(); i != _chatRooms.end(); i++)
	{
		if (i->second.name == name) {
			return 0;
		}
	}
	auto id = GetNewID();
	RoomDetails & r = _chatRooms[id];
	r.name = name;
	r.privateRoom = privateRoom;
	r.history.SetSize(cacheSize);
	return id;
}

ErrorCode Server::JoinChatRoom(std::uint64_t clientId, std::wstring & alias, std::wstring & roomName, bool privateRoom) {
	for (auto i = _chatRooms.begin(); i != _chatRooms.end(); i++)
	{
		if (i->second.name == roomName) {
			if (i->second.privateRoom == privateRoom) {
				for (auto u = i->second.members.begin(); u != i->second.members.end(); u++)
				{
					if (_clients[*u].alias == alias) {
						return ErrorCode::USERNAME_TAKEN;
					}
				}
				i->second.members.insert(clientId);
				auto && c = _clients[clientId];
				c.chatroomID = i->first;
				c.alias = alias;
				c.state = ClientState::CHATTING;
				return ErrorCode::OK;
			}
			break;
		}
	}
	return ErrorCode::NO_SUCH_ROOM;
}

std::wstring Server::GetPublicRooms() const {
	bool addSpace = false;
	std::wstring rtc;
	for (auto i = _chatRooms.begin(); i != _chatRooms.end(); i++)
	{
		if (i->second.privateRoom == false) {
			if (addSpace) {
				rtc += L" ";
			}
			else {
				addSpace = true;
			}
			rtc += i->second.name;
		}
	}
	return rtc;
}

void Server::ClientConnected(SOCKET client)
{
	auto id = _sockets[client];
	_clients[id].state = ClientState::CONNECTED;
}

ErrorCode Server::AddChatRoom(std::wstring & name, std::wstring & privateRoom, std::wstring & username, SOCKET s) {
	if (privateRoom == L"S" || privateRoom == L"V") {
		bool secret = privateRoom == L"S";
		auto clientID = _sockets[s];
		auto client = _clients[clientID];
		if (client.state == ClientState::CONNECTED) {
			// can try to add room now
			auto roomID = AddChatRoom(name, secret);
			if (roomID > 0) {
				// chatroom was created, insert user
				return JoinChatRoom(clientID, username, name, secret);				
			}
			else {
				// room name is taken
				return ErrorCode::ROOM_NAME_TAKEN;
			}
		}
		else {
			// client shouldn't send this request
			return ErrorCode::UNAUTHORIZED_REQUEST;
		}
	}
	else {
		// invalid query structure
		return ErrorCode::INVALID_REQUEST;
	}
}

ErrorCode Server::JoinChatRoom(SOCKET client, std::wstring & alias, std::wstring & roomName, std::wstring & privateRoom)
{
	if (privateRoom == L"S" || privateRoom == L"V") {
		bool secret = privateRoom == L"S";
		auto clientID = _sockets[client];
		auto client = _clients[clientID];
		if (client.state == ClientState::CONNECTED) {
			// can try to join room now			
				return JoinChatRoom(clientID, alias, roomName, secret);
		}
		else {
			// client shouldn't send this request
			return ErrorCode::UNAUTHORIZED_REQUEST;
		}
	}
	else {
		// invalid query structure
		return ErrorCode::INVALID_REQUEST;
	}
}

std::wstring Server::BuildMessage(const std::wstring & action, const std::wstring & payload) {
	return UNIQ + SPACE + action + SPACE + payload;
}

std::wstring Server::BuildMessage(const std::wstring & action) {
	return UNIQ + SPACE + action;
}

void Server::SendMsg(Action action, const std::wstring & payload, SOCKET s) {
	std::wstring p;
	bool valid = true;
	switch (action) {
	case Action::ERR: 
		p = BuildMessage(L"ERR", payload);
		break;	

	case Action::TINI: 
		p = BuildMessage(L"TINI");
		break;	

	case Action::ROOMS: 
		p = BuildMessage(L"ROOMS", payload);
		break;	

	case Action::JOINED: 
		p = BuildMessage(L"JOINED", payload);
		break;	

	case Action::RECV:
		p = BuildMessage(L"RECV", payload);
		break;

	case Action::PING:
		p = BuildMessage(L"PING");
		break;

	case Action::HISTORY:
		p = BuildMessage(L"HISTORY", payload);
		break;

	default: 
		valid = false;
		break;	
	}
	if (valid) {
		const wchar_t * msg(p.c_str());
		send(s, (char *)msg, (wcslen(msg) + 1) * 2, 0);
	}
}

void Server::_HandleError(ErrorCode code, SOCKET s) {
	std::wstring payload;
	switch (code)
	{
	case OK:
		break;
	case INVALID_REQUEST:
		payload = L"Invalid request";
		break;
	case UNAUTHORIZED_REQUEST:
		payload = L"Unauthorized request";
		break;
	case ROOM_NAME_TAKEN:
		payload = L"Room name was already taken";
		break;
	case USERNAME_TAKEN:
		payload = L"Username was already taken";
		break;
	case NO_SUCH_ROOM:
		payload = L"No such room was found";
		break;
	case CLIENT_LIMIT_REACHED:
		payload = L"Maximum connections limit was reached";
		break;
	default:
		payload = L"Ooops. Something failed :(";
		break;
	}
	if (code != ErrorCode::OK) {
		SendMsg(Action::ERR, payload, s);
	}
}

void Server::_ProcessMessage(std::wstring & message, SOCKET s) {
	// split message to tokens
	std::vector<std::wstring> tokens = split(message, L' ');
	if (tokens.size() >= 2) {
		// check if it starts with unique code
		if (tokens[0] == UNIQ) {
			bool validReq = true;
			// resolve action from 2nd token
			auto action = ResolveAction(tokens[1]);
			switch (action) {

			case Action::INIT: {
				ClientConnected(s);
				SendMsg(Action::TINI, L"", s);
				break;
			}

			case Action::PONG:
				// good, client answered to ping request, timestamp was already recorded
				break;

			case Action::GETROOMS: {
				SendMsg(Action::ROOMS, GetPublicRooms(), s);
				break;
			}

			case Action::CREATE: {
				if (tokens.size() == 5) {
					auto rtc = AddChatRoom(tokens[3], tokens[2], tokens[4], s);
					if (rtc == ErrorCode::OK) {
						// everything was fine, notify client about succesful join
						SendMsg(Action::JOINED, tokens[3] + SPACE + tokens[4], s);
					}
					else {
						_HandleError(rtc, s);
					}
				}
				else {
					_HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}

			case Action::JOIN: {
				if (tokens.size() == 5) {
					auto rtc = JoinChatRoom(s, tokens[4], tokens[3], tokens[2]);
					if (rtc == ErrorCode::OK) {
						// everything was fine, notify client about succesful join
						SendMsg(Action::JOINED, tokens[3] + SPACE + tokens[4], s);
					}
					else {
						_HandleError(rtc, s);
					}
				}
				else {
					_HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}

			case Action::SEND: {
				if (tokens.size() > 2) {
					std::wstring text(message, tokens[0].length() + tokens[1].length() + 2);
					if (text.length() > 0) {
						// resend message to all clients in the room
						auto clientID = _sockets[s];
						auto && client = _clients[clientID];
						auto && room = _chatRooms[client.chatroomID];
						std::wstring payload = client.alias + SPACE + text;
						room.history.Add(client.alias, text, time(0));
						for each (auto id in room.members)
						{
							SendMsg(Action::RECV, payload, _clients[id].socket);
						}
					}
				}
				else {
					_HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}

			case Action::GETHISTORY: {
				auto clientId = _sockets[s];
				auto && client = _clients[clientId];
				if (client.state == ClientState::CHATTING) {
					auto && room = _chatRooms[client.chatroomID];
					for (auto i = room.history.begin(); i != room.history.end(); i++)
					{						
						SendMsg(Action::HISTORY, std::to_wstring(i->time) + SPACE + i->sender + SPACE + i->text, s);
					}
				}
				break;
			}
			default:
				validReq = false;
				_HandleError(ErrorCode::INVALID_REQUEST, s);
				break;
			}
			if (validReq) {
				// set timestamp for client message
				_clients[_sockets[s]].timestamp = time(0);
			}
		}
	}
	
}

void Server::StartListening(HWND hWnd, std::int_fast32_t port, size_t cacheSize) {
	auto && s = GetInstance();
	WSADATA WsaDat;
	int nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
	if (nResult != 0)
	{
		MessageBox(hWnd, L"Winsock initialization failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}
	s._serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s._serverSocket == INVALID_SOCKET)
	{
		MessageBox(hWnd, L"Socket creation failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s._serverSocket, (LPSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hWnd, L"Unable to bind socket", L"Error", MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	nResult = WSAAsyncSelect(s._serverSocket, hWnd, WM_SOCKET, (FD_CLOSE | FD_ACCEPT | FD_READ));
	if (nResult)
	{
		MessageBox(hWnd, L"WSAAsyncSelect failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	if (listen(s._serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		MessageBox(hWnd, L"Unable to listen!", L"Error", MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}
	s.cacheSize = cacheSize;
}

void Server::_Ping() {
	time_t currTime = time(0);
	for (auto && i : _clients)
	{
		if (currTime - i.second.timestamp > 600) {
			// disconnect client
			_RemoveClient(i.second.socket);
		}
		else {
			SendMsg(Action::PING, L"", i.second.socket);
		}
	}
}

Server::ServerDetails Server::_GetDetails() {
	ServerDetails rtc;
	rtc.allClients = _clients.size();
	for (auto i = _clients.begin(); i != _clients.end(); i++)
	{
		if (i->second.state == ClientState::CHATTING) {
			rtc.chattingClients++;
		}
	}
	rtc.allRooms = _chatRooms.size();
	for (auto i = _chatRooms.begin(); i != _chatRooms.end(); i++)
	{
		if (i->second.privateRoom) {
			rtc.privateRooms++;
		}
		else {
			rtc.publicRooms++;
		}
	}
	return rtc;
}

Server::Server() {}


