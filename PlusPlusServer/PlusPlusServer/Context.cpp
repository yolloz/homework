#include "Context.h"

Context & Context::GetContext() {
	static Context instance;
	return instance;
}

AppState& Context::State() {
	return _state;
}

const std::map<SOCKET, std::uint64_t> & Context::Sockets() const{
	return _sockets;
}


void Context::InitLookup() {
	_msgLookup[L"INIT"] = Action::INIT;
	_msgLookup[L"ACK"] = Action::ACK;
	_msgLookup[L"ERR"] = Action::ERR;
	_msgLookup[L"CREATE"] = Action::CREATE;
	_msgLookup[L"JOIN"] = Action::JOIN;
	_msgLookup[L"GETROOMS"] = Action::GETROOMS;
	_msgLookup[L"INVALID_ACTION"] = Action::INVALID_ACTION;
}

Action Context::ResolveAction(std::wstring & msg) {
	auto && context = GetContext();
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

std::uint64_t Context::GetNewID() {
	return _id++;
}

bool Context::CanAddClient() const{
	return _activeClients < _maxActiveClients;
}

bool Context::AddClient(SOCKET newSocket) {
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

bool Context::RemoveClient(SOCKET client) {
	auto i = _sockets.find(client);
	if (i != _sockets.end()) {
		auto id = i->second;
		auto details = _clients[id];
		if (details.state == ClientState::CHATTING) {
			// remove from chatroom
			_chatRooms[details.chatroomID].members.erase(id);
		}
		_clients.erase(id);
		_sockets.erase(i);
		_activeClients--;
		return true;
	}
	return false;
}

std::uint64_t Context::AddChatRoom(std::wstring & name, bool privateRoom) {
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

bool Context::JoinChatRoom(std::uint64_t clientId, std::wstring & alias, std::wstring & roomName, bool privateRoom) {
	for (auto i = _chatRooms.begin(); i != _chatRooms.end(); i++)
	{
		if (i->second.name == roomName) {
			if (i->second.privateRoom == privateRoom) {
				i->second.members.insert(clientId);
				auto && c = _clients[clientId];
				c.chatroomID = i->first;
				c.alias = alias;
				c.state = ClientState::CHATTING;
				return true;
			}
			break;
		}
	}
	return false;
}

std::wstring Context::GetPublicRooms() const {
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

void Context::ClientConnected(SOCKET client)
{
	auto id = _sockets[client];
	_clients[id].state = ClientState::CONNECTED;
}

Context::Context() {}


