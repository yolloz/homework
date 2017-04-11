#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef PLUSPLUSCHAT_CHATWINDOW_
#define PLUSPLUSCHAT_CHATWINDOW_

#include <windows.h>
#include "ContextSingleton.h"
#include "Enumerations.h"

// ChatWindow uses codes 200 - 299
#define IDC_EDIT_IN			201
#define IDC_EDIT_OUT		202
#define IDC_MAIN_BUTTON		203

namespace PlusPlusChat {

	HWND CreateChatWindow(HINSTANCE hInstance);
	ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance);
	LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	bool ActivateChatWindow();
}
#endif