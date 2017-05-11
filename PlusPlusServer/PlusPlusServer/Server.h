#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef PLUSPLUSSERVER_SERVER_
#define PLUSPLUSSERVER_SERVER_

#define WM_SOCKET		201

#include <map>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
#include <sstream>
#include <iterator>
#include <WinSock2.h>
#include <unordered_set>
#include "Enums.h"


class Server
{
private:
	class MessageCache {
	public:

		struct Message {
			time_t time;
			std::wstring sender;
			std::wstring text;
		};

		class iterator : public std::iterator<std::input_iterator_tag, Message> {
		private:
			MessageCache * cache;
			size_t idx;
		public:
			iterator(MessageCache * cache, size_t idx = 0) : cache(cache), idx(idx) {}
			inline bool operator==(iterator other) {
				return idx == other.idx;
			}
			inline bool operator!=(iterator other) {
				return idx != other.idx;
			}
			inline iterator& operator++() {
				idx = idx == cache->endIdx ? cache->endIdx : idx + 1;
				idx %= cache->size;
				return *this;
			}
			inline iterator operator++(int) {
				iterator retval = *this;
				++(*this);
				return retval;
			}
			inline reference operator*() const {
				return cache->Get(idx);
			}
			inline Message * operator->() {
				return &cache->Get(idx);
			}
		};

		inline iterator begin() {
			return iterator(this, startIdx);
		}

		inline iterator end() {
			return iterator(this, endIdx);
		}

		inline void Add(std::wstring & sender, std::wstring & text, time_t time) {
			auto && p = _messages[endIdx];
			p.time = time;
			p.text = text;
			p.sender = sender;
			endIdx = (endIdx + 1) % size;
			if (endIdx == startIdx) {
				startIdx = (startIdx + 1) % size;
			}
		}

		inline void SetSize(size_t size) {
			// add one free space as end pointer
			size++;
			this->size = size;
			_messages.resize(size);
			endIdx = 0;
			startIdx = 0;
		}

	private:
		size_t size = -1;
		size_t startIdx = 0;
		size_t endIdx = 0;
		std::vector<Message> _messages;

		inline Message & Get(size_t idx) {
			return _messages[idx];
		}
	};

public:

	struct ClientDetails {
		SOCKET socket;
		ClientState state;
		std::wstring alias;
		std::uint64_t chatroomID;
		time_t timestamp;
	};

	struct RoomDetails {
		std::wstring name;
		bool privateRoom;
		std::unordered_set<std::uint64_t> members;
		MessageCache history;
	};

	struct ServerDetails {
		int allClients = 0;
		int chattingClients = 0;
		int allRooms = 0;
		int privateRooms = 0;
		int publicRooms = 0;
	};

	const static std::int_fast32_t defaultPort = 5464;

	static Server& GetInstance();
	static AppState& State();
	static SOCKET & ServerSocket();
	static Action ResolveAction(std::wstring & msg);
	static inline bool CanAddClient() {
		return GetInstance()._CanAddClient();
	}
	static void StartListening(HWND hWnd, std::int_fast32_t port);
	static inline void ProcessMessage(std::wstring & message, SOCKET s) {
		GetInstance()._ProcessMessage(message, s);
	}
	static inline void HandleError(ErrorCode code, SOCKET s) {
		GetInstance()._HandleError(code, s);
	}
	static inline bool AddClient(SOCKET newSocket) {
		return GetInstance()._AddClient(newSocket);
	}
	static inline bool RemoveClient(SOCKET client) {
		return GetInstance()._RemoveClient(client);
	}
	static inline void Ping() {
		GetInstance()._Ping();
	}
	static inline ServerDetails GetDetails() {
		return GetInstance()._GetDetails();
	}
	
	// disable copying
	Server(Server const&) = delete;
	void operator=(Server const&) = delete;

private:
	Server();
	AppState _state = AppState::STOPPED;
	SOCKET _serverSocket = NULL;
	std::map<std::wstring, Action> _msgLookup;
	bool _lookupInitialized = false;
	std::map<SOCKET, std::uint64_t> _sockets;
	std::map<std::uint64_t, RoomDetails> _chatRooms;
	std::map<std::uint64_t, ClientDetails> _clients;
	const size_t _maxActiveClients = 100;
	std::uint64_t _id = 1;
	const std::wstring UNIQ = L"#PPChat";
	const wchar_t SPACE = L' ';
	size_t cacheSize = 5;

	bool _CanAddClient() const;
	bool _AddClient(SOCKET newSocket);
	bool _RemoveClient(SOCKET client);
	void _ProcessMessage(std::wstring & message, SOCKET s);
	void _HandleError(ErrorCode code, SOCKET s);
	void InitLookup();
	std::uint64_t GetNewID();
	std::uint64_t AddChatRoom(std::wstring & name, bool privateRoom);
	ErrorCode JoinChatRoom(std::uint64_t clientId, std::wstring & alias, std::wstring & roomName, bool privateRoom);
	std::wstring BuildMessage(const std::wstring & action, const std::wstring & payload);
	std::wstring BuildMessage(const std::wstring & action);
	void SendMsg(Action action, const std::wstring & payload, SOCKET s);
	ErrorCode AddChatRoom(std::wstring & name, std::wstring & privateRoom, std::wstring & username, SOCKET s);
	ErrorCode JoinChatRoom(SOCKET client, std::wstring & alias, std::wstring & roomName, std::wstring & privateRoom);
	std::wstring GetPublicRooms() const;
	void ClientConnected(SOCKET client);
	void _Ping();
	ServerDetails _GetDetails();

	// helper functions
	template<typename Out>
	void split(const std::wstring &s, wchar_t delim, Out result) {
		std::wstringstream ss;
		ss.str(s);
		std::wstring item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	inline std::vector<std::wstring> split(const std::wstring & s, wchar_t delim) {
		std::vector<std::wstring> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}	
};

#endif

