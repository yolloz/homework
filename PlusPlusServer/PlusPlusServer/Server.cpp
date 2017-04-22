#include "Server.h"

Server & Server::GeInstance() {
	static Server instance;
	return instance;
}

AppState& Server::State() {
	return _state;
}

const std::map<SOCKET, std::uint64_t> & Server::Sockets() const{
	return _sockets;
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
}

Action Server::ResolveAction(std::wstring & msg) {
	auto && context = GeInstance();
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

bool Server::CanAddClient() const{
	return _activeClients < _maxActiveClients;
}

bool Server::AddClient(SOCKET newSocket) {
	if (_activeClients < _maxActiveClients) {
		ClientDetails c;
		auto id = GetNewID();
		c.state = ClientState::NEW;
		c.socket = newSocket;
		_clients[id] = c;
		_sockets[newSocket] = id;
		_activeClients++;
		return true;
	}
	return false;;
}

bool Server::RemoveClient(SOCKET client) {
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
		_activeClients--;
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
	RoomDetails r;
	r.name = name;
	r.privateRoom = privateRoom;
	auto id = GetNewID();
	_chatRooms[id] = r;
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

Server::Server() {}


