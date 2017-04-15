#ifndef PLUSPLUSCHAT_CONTEXTSINGLETON_
#define PLUSPLUSCHAT_CONTEXTSINGLETON_

#include <winsock2.h>
#include "Enumerations.h"
#include "resource.h"

#define WM_SOCKET		104

namespace PlusPlusChat {

	class ContextSingleton
	{
	public:
		ContextSingleton(ContextSingleton const&) = delete;
		void operator=(ContextSingleton const&) = delete;

#define WM_SOCKET		104	

		AppState state = AppState::DISCONNECTED;
		USHORT nPort = 5464;
		HWND connectionWindow = NULL;
		HWND chatWindow = NULL;
		HWND roomWindow = NULL;
		SOCKET Socket = NULL;
		HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
		HICON iconSmall = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));


		static ContextSingleton& GetInstance();
	private:
		ContextSingleton();
	};
}

#endif
