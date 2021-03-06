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
#include "../../PlusPlusChat_shared/PlusPlusHelpers.h"

// ChatWindow uses codes 200 - 299
#define IDC_CH_HISTORY			201
#define IDC_CH_MESSAGE_TB		202
#define IDC_CH_SEND_BUTTON		203

namespace PlusPlusChat {

	HWND CreateChatWindow(HINSTANCE hInstance, std::wstring & roomName, std::wstring & userName);
	ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance);
	LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	bool ActivateChatWindow(std::wstring & roomName, std::wstring & userName);
	void ReceiveMessageCH(std::wstring & sender, time_t time, std::wstring & message);
	std::wstring GetTime(time_t t);
	void CreateChatWindowLayout(HWND hWnd);
	void Send(HWND hWnd);
	void ResizeCH(HWND hWnd, int width, int height);
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