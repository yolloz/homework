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
		switch (action) {
		case Action::ERR: {			
			const wchar_t * msg(BuildMessage(L"ERR", payload).c_str());
			send(s, (char *)msg, wcslen(msg) * 2, 0);
			break;
		}

		case Action::INIT: {
			auto p = BuildMessage(L"INIT");
			const wchar_t * msg(p.c_str());
			send(s, (char *)msg, wcslen(msg) * 2, 0);
			break;
		}

		case Action::ACK: {
			auto p = BuildMessage(L"ACK");
			const wchar_t * msg(p.c_str());
			send(s, (char *)msg, wcslen(msg) * 2, 0);
			break;
		}

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
		_actionLookup[L"INVALID_ACTION"] = Action::INVALID_ACTION;
	}
}