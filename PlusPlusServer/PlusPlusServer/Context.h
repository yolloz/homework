#ifndef PLUSPLUSSERVER_CONTEXT_
#define PLUSPLUSSERVER_CONTEXT_

#include <map>
#include <string>
#include <WinSock2.h>
#include "Enums.h"


class Context
{
public:

	struct ClientDetails {
		ClientState state;
		std::string alias;
		std::int_least32_t chatroomID;
	};

	static Context& GetContext();
	AppState& State();
	static Action ResolveAction(std::wstring & msg);
	std::map<SOCKET, ClientDetails> & Clients();
	bool CanAddClient();
	bool AddClient(SOCKET newClient);
	void RemoveClient(SOCKET client);
	SOCKET ServerSocket = NULL;
	Context(Context const&) = delete;
	void operator=(Context const&) = delete;

private:
	Context();
	AppState _state = AppState::STOPPED;
	std::map<std::wstring, Action> _msgLookup;
	bool _lookupInitialized = false;
	std::map<SOCKET, ClientDetails> _clients;
	const std::int_fast32_t _maxActiveClients = 100;
	std::int_fast32_t _activeClients = 0;
	std::uint_fast32_t _clientID = 0;

	void InitLookup();
	std::uint_fast32_t GetNewID();
	

};

#endif

