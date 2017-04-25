#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef PLUSPLUSCHAT_CHATWINDOW_
#define PLUSPLUSCHAT_CHATWINDOW_

#include <windows.h>
#include <commctrl.h>
#include <ctime>
#include "Communicator.h"
#include "ContextSingleton.h"
#include "Enumerations.h"

// ChatWindow uses codes 200 - 299
#define IDC_CH_HISTORY			201
#define IDC_CH_MESSAGE_TB		202
#define IDC_CH_SEND_BUTTON		203

namespace PlusPlusChat {

	HWND CreateChatWindow(HINSTANCE hInstance, std::wstring & roomName, std::wstring & userName);
	ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance);
	LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	bool ActivateChatWindow(std::wstring & roomName, std::wstring & userName);
	void ReceiveMessage(std::wstring & sender, std::wstring & message);
	std::wstring GetTime();
	void CreateChatWindowLayout(HWND hWnd);
	void Send(HWND hWnd);
	HWND CreateCustomEdit(
		_In_     DWORD     dwStyle,
		_In_     int       x,
		_In_     int       y,
		_In_     int       nWidth,
		_In_     int       nHeight,
		_In_opt_ HWND      hWndParent,
		_In_opt_ HMENU     hMenu,
		_In_opt_ HINSTANCE hInstance);
}
#endif