#include "Communicator.h"

namespace PlusPlusChat {
	const std::wstring Communicator::UNIQ = L"#PPChat";
	const std::wstring Communicator::SPACE = L" ";
	std::map<std::wstring, Action> Communicator::_actionLookup;

	bool Communicator::ProcessMessage(std::wstring & message, SOCKET s) {
		std::vector<std::wstring> tokens = Communicator::split(message, L' ');
		if (tokens.size() >= 2) {
			if (tokens[0] == UNIQ) {
				auto action = Communicator::ResolveAction(tokens[1]);
				switch (action) {
				case TINI: {
					ContextSingleton::GetInstance().state = AppState::CONNECTED;
					// switch windows
					DeactivateConnectionWindow();
					ActivateRoomWindow();

					// ask server to fill list box
					Communicator::SendMsg(Action::GETROOMS, L"", s);
					break;
				}

				case Action::ROOMS: {
					if (ContextSingleton::GetInstance().roomWindow != NULL) {
						auto && vct = ContextSingleton::GetInstance().roomsList;
						vct.clear();
						for (size_t i = 2; i < tokens.size(); i++)
						{
							vct.push_back(tokens[i]);
						}
						ReloadPublicRoomsList();
					}
					break;
				}

				case Action::JOINED: {
					if (ContextSingleton::GetInstance().state == AppState::JOINING) {
						ContextSingleton::GetInstance().state = AppState::CHATTING;
						DeactivateRoomWindow();
						ActivateChatWindow(tokens[2], tokens[3]);
						SendMsg(Action::GETHISTORY, L"", s);
					}
					break;
				}

				case Action::RECV: {
					if (ContextSingleton::GetInstance().state == AppState::CHATTING) {
						// cut out message
						std::wstring text(message, tokens[0].length() + tokens[1].length() + tokens[2].length() + 3);
						ReceiveMessageCH(tokens[2], 0, text);
					}
					break;
				}

				case Action::HISTORY: {
					if (ContextSingleton::GetInstance().state == AppState::CHATTING) {
						if (tokens.size() > 4) {
							// cut out message
							std::wstring text(message, tokens[0].length() + tokens[1].length() + tokens[2].length() + tokens[3].length() + 4);
							time_t t = 0;
							try {
								t = std::stoull(tokens[2]);
							}
							catch (...){
								t = 0;
							}
							ReceiveMessageCH(tokens[3], t, text);
						}
					}
					break;
				}

				case Action::PING: {
					SendMsg(Action::PONG, L"", s);
					break;
				}

				case Action::ERR: {
					// get active window
					HWND hWnd = NULL;
					auto state = ContextSingleton::GetInstance().state;
					switch (state)
					{
					case PlusPlusChat::DISCONNECTED:
						hWnd = ContextSingleton::GetInstance().connectionWindow;
						break;
					case PlusPlusChat::CONNECTED:
						hWnd = ContextSingleton::GetInstance().roomWindow;
						break;
					case PlusPlusChat::CHATTING:
						hWnd = ContextSingleton::GetInstance().chatWindow;
						break;
					case PlusPlusChat::CONNECTING:
						hWnd = ContextSingleton::GetInstance().connectionWindow;
						break;
					case PlusPlusChat::JOINING:
						hWnd = ContextSingleton::GetInstance().roomWindow;
						break;
					}
					std::wstring error(message, tokens[0].length() + tokens[1].length() + 2);
					if (state == AppState::JOINING) {
						ContextSingleton::GetInstance().state = AppState::CONNECTED;
						SetWaitingRW(hWnd, false);
					}
					MessageBox(hWnd, error.c_str(), L"Error", MB_ICONINFORMATION | MB_OK);
				}

				}
			}
		}
		return false;
	}

	std::wstring Communicator::BuildMessage(const std::wstring & action, const std::wstring & payload) {
		return UNIQ + SPACE + action + SPACE + payload;
	}

	std::wstring Communicator::BuildMessage(const std::wstring & action) {
		return UNIQ + SPACE + action;
	}

	void Communicator::SendMsg(Action action, const std::wstring & payload, SOCKET s) {
		bool valid = true;
		std::wstring p;
		switch (action) {
		case Action::ERR: {
			p = BuildMessage(L"ERR", payload);
			break;
		}

		case Action::INIT: {
			p = BuildMessage(L"INIT");			
			break;
		}

		case Action::CREATE: {
			p = BuildMessage(L"CREATE", payload);
			break;
		}

		case Action::JOIN: {
			p = BuildMessage(L"JOIN", payload);
			break;
		}

		case Action::GETROOMS: {
			p = BuildMessage(L"GETROOMS");
			break;
		}

		case Action::SEND: {
			p = BuildMessage(L"SEND", payload);
			break;
		}

		case Action::PONG:
			p = BuildMessage(L"PONG");
			break;

		case Action::GETHISTORY:
			p = BuildMessage(L"GETHISTORY");
			break;

		default: {
			valid = false;
			break;
		}
		}
		// send message if action is valid
		if (valid) {
			const wchar_t * msg(p.c_str());
			send(s, (char *)p.c_str(), (p.length() + 1) * 2, 0);
		}
	}

	Action Communicator::ResolveAction(std::wstring & msg) {
		if (_actionLookup.size() == 0) {
			InitLookup();
		}
		auto i = _actionLookup.find(msg);
		if (i == _actionLookup.end()) {
			return Action::INVALID_ACTION;
		}
		else {
			return i->second;
		}
	}

	void Communicator::InitLookup() {
		_actionLookup[L"TINI"] = Action::TINI;
		_actionLookup[L"INIT"] = Action::INIT;
		_actionLookup[L"ERR"] = Action::ERR;
		_actionLookup[L"ROOMS"] = Action::ROOMS;
		_actionLookup[L"INVALID_ACTION"] = Action::INVALID_ACTION;
		_actionLookup[L"JOINED"] = Action::JOINED;
		_actionLookup[L"SEND"] = Action::SEND;
		_actionLookup[L"RECV"] = Action::RECV;
		_actionLookup[L"PING"] = Action::PING;
		_actionLookup[L"PONG"] = Action::PONG;
		_actionLookup[L"HISTORY"] = Action::HISTORY;
		_actionLookup[L"GETHISTORY"] = Action::GETHISTORY;
	}
}