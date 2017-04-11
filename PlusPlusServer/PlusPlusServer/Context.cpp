#include "Context.h"

Context & Context::GetContext() {
	static Context instance;
	return instance;
}

AppState& Context::State() {
	return _state;
}

std::map<SOCKET, Context::ClientDetails> & Context::Clients() {
	return _clients;
}

void Context::InitLookup() {
	_msgLookup[L"INIT"] = Action::INIT;
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

std::uint_fast32_t Context::GetNewID() {
	return _clientID++;
}

bool Context::CanAddClient() {
	return _activeClients < _maxActiveClients;
}

bool Context::AddClient(SOCKET newClient) {
	if (_activeClients < _maxActiveClients) {
		ClientDetails c;
		c.state = ClientState::NEW;
		_clients[newClient] = c;
		_activeClients++;
		return true;
	}
	return false;;
}

void Context::RemoveClient(SOCKET client) {
	auto i = _clients.find(client);
	if (i != _clients.end()) {
		_clients.erase(i);
		_activeClients--;
	}
}

Context::Context()
{
}


