#ifndef PLUSPLUSSERVER_CONTEXT_
#define PLUSPLUSSERVER_CONTEXT_

#include <map>
#include <string>
#include <WinSock2.h>
#include <unordered_set>
#include "Enums.h"


class Context
{
public:

	struct ClientDetails {
		SOCKET socket;
		ClientState state;
		std::wstring alias;
		std::int_least32_t chatroomID;
	};

	struct RoomDetails {
		std::wstring name;
		bool privateRoom;
		std::unordered_set<std::uint64_t> members;
	};

	static Context& GetContext();
	AppState& State();
	static Action ResolveAction(std::wstring & msg);
	const std::map<SOCKET, std::uint64_t> & Sockets() const;
	bool CanAddClient() const;
	bool AddClient(SOCKET newSocket);
	bool RemoveClient(SOCKET client);
	std::uint64_t AddChatRoom(std::wstring & name, bool privateRoom);
	bool JoinChatRoom(std::uint64_t clientId, std::wstring & alias, std::wstring & roomName, bool privateRoom);
	std::wstring GetPublicRooms() const;
	void ClientConnected(SOCKET client);
	SOCKET ServerSocket = NULL;
	Context(Context const&) = delete;
	void operator=(Context const&) = delete;

private:
	Context();
	AppState _state = AppState::STOPPED;
	std::map<std::wstring, Action> _msgLookup;
	bool _lookupInitialized = false;
	std::map<SOCKET, std::uint64_t> _sockets;
	std::map<std::uint64_t, RoomDetails> _chatRooms;
	std::map<std::uint64_t, ClientDetails> _clients;
	const std::int_fast32_t _maxActiveClients = 100;
	std::uint64_t _activeClients = 0;
	std::uint64_t _id = 1;

	void InitLookup();
	std::uint64_t GetNewID();
	

};

#endif

