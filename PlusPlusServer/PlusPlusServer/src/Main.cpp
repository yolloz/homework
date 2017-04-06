#include "Main.h"

#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

// default port
std::int_fast32_t nPort = 5464;

// Component handles
HWND hEditIn = NULL;
HWND hEditOut = NULL;
HWND hPortNumber = NULL;
HWND hStartBtn = NULL;
HWND windowHandle = NULL;



wchar_t szHistory[10000];
sockaddr sockAddrClient;

const int nMaxClients = 3;
int nClient = 0;
SOCKET Socket[nMaxClients - 1];
SOCKET ServerSocket = NULL;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wClass;
	ZeroMemory(&wClass, sizeof(WNDCLASSEX));
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = NULL;
	wClass.hIconSm = NULL;
	wClass.hInstance = hInst;
	wClass.lpfnWndProc = (WNDPROC)WinProc;
	wClass.lpszClassName = L"Window Class";
	wClass.lpszMenuName = NULL;
	wClass.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wClass))
	{
		int nResult = GetLastError();
		MessageBox(NULL,
			L"Window class creation failed\r\nError code:",
			L"Window Class Failed",
			MB_ICONERROR);
	}

	windowHandle = CreateWindowEx(NULL,
		L"Window Class",
		L"PlusPlusChat Server",
		WS_OVERLAPPEDWINDOW,
		200,
		200,
		640,
		480,
		NULL,
		NULL,
		hInst,
		NULL);

	if (!windowHandle)
	{
		int nResult = GetLastError();

		MessageBox(NULL,
			L"Window creation failed\r\nError code:",
			L"Window Creation Failed",
			MB_ICONERROR);
	}

	ShowWindow(windowHandle, nShowCmd);

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MAIN_BUTTON:
		{		
			if (state == AppState::STOPPED) {
				// Get port number
				wchar_t portBuffer[6];
				HWND hPortNumber = GetDlgItem(hWnd, IDC_PORTNUMBER);
				SendMessage(hPortNumber, WM_GETTEXT, sizeof(portBuffer), reinterpret_cast<LPARAM>(portBuffer));
				std::int_fast32_t port = 0;
				for (size_t i = 0; i < wcslen(portBuffer); i++)
				{
					port *= 10;
					port += portBuffer[i] - L'0';
				}
				if (port == 0) {
					port = nPort;
				}
				if (port > 65535 || port < 1024) {
					MessageBox(windowHandle, L"Port number is invalid. Valid numbers are 1024 - 65535", L"Invalid port number",MB_ICONERROR);
					break;
				}
				SendMessage(hStartBtn, WM_SETTEXT, NULL, (LPARAM)L"Stop");
				EnableWindow(hPortNumber, false);
				StartListening(windowHandle, port);
				state = AppState::LISTENING;
			}
			else {
				shutdown(ServerSocket, SD_BOTH);
				closesocket(ServerSocket);
				WSACleanup();
				HWND hPortNumber = GetDlgItem(hWnd, IDC_PORTNUMBER);
				SendMessage(hStartBtn, WM_SETTEXT, NULL, (LPARAM)L"Start");
				EnableWindow(hPortNumber, true);
				state = AppState::STOPPED;
			}
			

			/*wchar_t szBuffer[1024];
			ZeroMemory(szBuffer, sizeof(szBuffer));

			SendMessage(hEditOut,
				WM_GETTEXT,
				sizeof(szBuffer),
				reinterpret_cast<LPARAM>(szBuffer));
			for (int n = 0; n <= nClient; n++)
			{
				send(Socket[n], (char*)szBuffer, wcslen(szBuffer) * 2, 0);
			}

			SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)"");*/
		}
		break;
		}
		break;
	case WM_CREATE:
	{
		CreateLayout(hWnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(ServerSocket, SD_BOTH);
		closesocket(ServerSocket);
		WSACleanup();
		return 0;
	}
	break;

	case WM_SOCKET:
	{
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
		{
			for (int n = 0; n <= nMaxClients; n++)
			{
				wchar_t szIncoming[1024];
				ZeroMemory(szIncoming, sizeof(szIncoming));

				int inDataLength = recv(Socket[n],
					(char*)szIncoming,
					sizeof(szIncoming) / sizeof(szIncoming[0]),
					0);

				if (inDataLength != -1)
				{
					wcsncat(szHistory, szIncoming, inDataLength);
					wcscat(szHistory, L"\r\n");

					SendMessage(hEditIn,
						WM_SETTEXT,
						sizeof(szIncoming) - 1,
						reinterpret_cast<LPARAM>(&szHistory));
				}
			}
		}
		break;

		case FD_CLOSE:
		{
			MessageBox(hWnd,
				L"Client closed connection",
				L"Connection closed!",
				MB_ICONINFORMATION | MB_OK);
		}
		break;

		case FD_ACCEPT:
		{
			if (nClient < nMaxClients)
			{
				int size = sizeof(sockaddr);
				Socket[nClient] = accept(wParam, &sockAddrClient, &size);
				if (Socket[nClient] == INVALID_SOCKET)
				{
					int nret = WSAGetLastError();
					WSACleanup();
				}
				SendMessage(hEditIn,
					WM_SETTEXT,
					NULL,
					(LPARAM)L"Client connected!");
			}
			nClient++;
		}
		break;
		}
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CreateLayout(HWND hWnd) {

	ZeroMemory(szHistory, sizeof(szHistory));

	// Create incoming message box
	hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20,
		20,
		580,
		240,
		hWnd,
		(HMENU)IDC_EDIT_IN,
		GetModuleHandle(NULL),
		NULL);
	if (!hEditIn)
	{
		MessageBox(hWnd,
			L"Could not create incoming edit box.",
			L"Error",
			MB_OK | MB_ICONERROR);
	}
	HGDIOBJ hfDefault = GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hEditIn,
		WM_SETFONT,
		(WPARAM)hfDefault,
		MAKELPARAM(FALSE, 0));
	SendMessage(hEditIn,
		WM_SETTEXT,
		NULL,
		(LPARAM)L"Waiting for client to connect...");

	// Create outgoing message box
	hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20,
		280,
		580,
		60,
		hWnd,
		(HMENU)IDC_EDIT_IN,
		GetModuleHandle(NULL),
		NULL);
	if (!hEditOut)
	{
		MessageBox(hWnd,
			L"Could not create outgoing edit box.",
			L"Error",
			MB_OK | MB_ICONERROR);
	}

	SendMessage(hEditOut,
		WM_SETFONT,
		(WPARAM)hfDefault,
		MAKELPARAM(FALSE, 0));
	SendMessage(hEditOut,
		WM_SETTEXT,
		NULL,
		(LPARAM)L"Type message here...");

	// Create port message box
	hPortNumber = CreateWindowEx(WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE |
		ES_NUMBER,
		50,
		360,
		60,
		23,
		hWnd,
		(HMENU)IDC_PORTNUMBER,
		GetModuleHandle(NULL),
		NULL);
	if (!hPortNumber)
	{
		MessageBox(hWnd,
			L"Could not create port edit box.",
			L"Error",
			MB_OK | MB_ICONERROR);
	}

	SendMessage(hPortNumber,
		WM_SETFONT,
		(WPARAM)hfDefault,
		MAKELPARAM(FALSE, 0));
	SendMessage(hPortNumber, EM_LIMITTEXT, 5, NULL);

	// Create port label
	HWND hPortLbl = CreateWindowEx(0,
		L"STATIC",
		L"Port:",
		WS_CHILD | WS_VISIBLE,
		20,
		360,
		25,
		23,
		hWnd,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	SendMessage(hPortLbl, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	// Create a push button
	hStartBtn = CreateWindow(
		L"BUTTON",
		L"Start",
		WS_TABSTOP | WS_VISIBLE |
		WS_CHILD | BS_DEFPUSHBUTTON,
		120,
		360,
		75,
		23,
		hWnd,
		(HMENU)IDC_MAIN_BUTTON,
		GetModuleHandle(NULL),
		NULL);

	SendMessage(hStartBtn,
		WM_SETFONT,
		(WPARAM)hfDefault,
		MAKELPARAM(FALSE, 0));
}

void StartListening(HWND hWnd, std::int_fast32_t port) {
	WSADATA WsaDat;
	int nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
	if (nResult != 0)
	{
		MessageBox(hWnd,
			L"Winsock initialization failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		MessageBox(hWnd,
			L"Socket creation failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ServerSocket, (LPSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hWnd, L"Unable to bind socket", L"Error", MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	nResult = WSAAsyncSelect(ServerSocket,
		hWnd,
		WM_SOCKET,
		(FD_CLOSE | FD_ACCEPT | FD_READ));
	if (nResult)
	{
		MessageBox(hWnd,
			L"WSAAsyncSelect failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		MessageBox(hWnd,
			L"Unable to listen!",
			L"Error",
			MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}
}
