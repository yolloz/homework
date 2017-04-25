#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(disable: 4996)

#ifndef PLUSPLUSCHAT_CONNECTIONWINDOW_
#define PLUSPLUSCHAT_CONNECTIONWINDOW_

#include <windows.h>
#include <commctrl.h>
#include "ContextSingleton.h"
#include "ChatWindow.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <Ws2tcpip.h>
#include "Communicator.h"
#include "Enumerations.h"


// ConnectionWindow uses codes 300 - 399
#define IDC_CW_WINDOW			301
#define IDC_CW_CONNECTBTN		302
#define IDC_CW_IPEDIT			303
#define IDC_CW_PORTEDIT			304
#define IDC_CW_AUTOCONNECTBTN	305
#define IDC_CW_PROGRESS			306


namespace PlusPlusChat {
	struct configRecord {
		bool valid;
		std::wstring ipAddress;
		std::int_fast32_t port;
	};

	HWND CreateConnectionWindow(HINSTANCE hInstance);
	LRESULT CALLBACK ConnectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	ATOM WINAPI RegisterConnectionWindow(HINSTANCE hInstance);
	void CreateConnectionWindowLayout(HWND hWnd);
	bool ConnectToServer(HWND hWnd, std::int_fast32_t port, const wchar_t* ipAddress);
	void DeactivateConnectionWindow();
	void SetWaitingCW(HWND hWnd, bool waiting);
	void AutoConnect(HWND hWnd);
	std::vector<configRecord> ParseConfig(std::ifstream & ifs);
}

#endif