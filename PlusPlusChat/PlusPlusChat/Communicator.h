#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSCHAT_COMMUNICATOR_
#define PLUSPLUSCHAT_COMMUNICATOR_

#include <WinSock2.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>
#include "Enumerations.h"
#include "RoomWindow.h"
#include "ContextSingleton.h"
#include "ConnectionWindow.h"
#include "ChatWindow.h"

namespace PlusPlusChat {
	class Communicator
	{
	public:
		template<typename Out>
		static void split(const std::wstring &s, wchar_t delim, Out result) {
			std::wstringstream ss;
			ss.str(s);
			std::wstring item;
			while (std::getline(ss, item, delim)) {
				*(result++) = item;
			}
		}

		inline static std::vector<std::wstring> split(const std::wstring & s, wchar_t delim) {
			std::vector<std::wstring> elems;
			split(s, delim, std::back_inserter(elems));
			return elems;
		}

		static bool ProcessMessage(const wchar_t * message, SOCKET s);
		static std::wstring BuildMessage(const std::wstring & action, const std::wstring & payload);
		static std::wstring BuildMessage(const std::wstring & action);
		static void SendMsg(Action action, const std::wstring & payload, SOCKET s);
		static Action Communicator::ResolveAction(std::wstring & msg);

	private:
		static std::map<std::wstring, Action> _actionLookup;
		static void InitLookup();
		static const std::wstring UNIQ;
		static const std::wstring SPACE;

	};
}
#endif

