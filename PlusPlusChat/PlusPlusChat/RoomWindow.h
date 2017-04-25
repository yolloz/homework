#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSCHAT_ROOMWINDOW_
#define PLUSPLUSCHAT_ROOMWINDOW_

#include <windows.h>
#include <commctrl.h>
#include "ContextSingleton.h"
#include "ChatWindow.h"
#include <vector>
#include <string>
#include <cstdint>
#include <Ws2tcpip.h>
#include "Communicator.h"
#include "Enumerations.h"

// ConnectionWindow uses codes 400 - 499
#define IDC_RW_USERNAME_TB			401
#define IDC_RW_PUBLICROOM_LIST		402
#define IDC_RW_JOINPUBLICROOM_BTN	403
#define IDC_RW_JOINPRIVATEROOM_BTN	404
#define IDC_RW_PRIVATEROOM_TB		405
#define IDC_RW_NEWROOM_TB			406
#define IDC_RW_NEWPRIVATEROOM_BTN	407
#define IDC_RW_NEWPUBLICROOM_BTN	408
#define IDC_RW_PROGRESS				409
#define IDC_RW_REFRESHPUBLICROOM_BTN	410


namespace PlusPlusChat {
	HWND CreateRoomWindow(HINSTANCE hInstance);
	LRESULT CALLBACK RoomWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	ATOM WINAPI RegisterRoomWindow(HINSTANCE hInstance);
	void CreateRoomWindowLayout(HWND hWnd);
	bool ActivateRoomWindow();
	void DeactivateRoomWindow();
	void SetWaitingRW(HWND hWnd, bool waiting);
	bool CreateNewRoom(HWND hWnd, bool privateRoom);
	bool JoinRoom(HWND hWnd, bool privateRoom);
	void ReloadPublicRoomsList();
}

#endif
