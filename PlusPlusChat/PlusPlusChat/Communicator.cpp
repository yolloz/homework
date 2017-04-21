#include "Communicator.h"

namespace PlusPlusChat {
	const std::wstring Communicator::UNIQ = L"#PPChat";
	const std::wstring Communicator::SPACE = L" ";
	std::map<std::wstring, Action> Communicator::_actionLookup;

	bool Communicator::ProcessMessage(wchar_t * message, SOCKET s) {
		std::wstring ws(message);
		std::vector<std::wstring> tokens = Communicator::split(ws, L' ');
		if (tokens.size() >= 2) {
			if (tokens[0] == UNIQ) {
				auto action = Communicator::ResolveAction(tokens[1]);
				switch (action) {
				case TINI: {
					ContextSingleton::GetInstance().state = AppState::CONNECTED;
					// switch windows
					DeactivateConnectionWindow();
					ActivateRoomWindow();

					Communicator::SendMsg(Action::ACK, L"", s);	
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

		case Action::ACK: {
			p = BuildMessage(L"ACK");
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

		default: {
			valid = false;
			break;
		}
		}
		// send message if action is valid
		if (valid) {
			const wchar_t * msg(p.c_str());
			send(s, (char *)msg, wcslen(msg) * 2, 0);
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
		_actionLookup[L"ACK"] = Action::ACK;
		_actionLookup[L"ERR"] = Action::ERR;
		_actionLookup[L"ROOMS"] = Action::ROOMS;
		_actionLookup[L"INVALID_ACTION"] = Action::INVALID_ACTION;
	}
}