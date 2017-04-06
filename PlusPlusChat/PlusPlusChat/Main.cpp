#include "Main.h"

#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

	char *szServer = "192.168.0.188";
	USHORT nPort = 5555;

	HWND hEditIn = NULL;
	HWND hEditOut = NULL;
	HWND connectionWindow = NULL;
	HWND chatWindow = NULL;

	SOCKET Socket = NULL;
	wchar_t szHistory[10000];

	int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
	{
		if (!RegisterConnectionWindow(hInst)) {
			int nResult = GetLastError();
			MessageBox(NULL, L"Window class creation failed\r\nError code: " + nResult, L"Window Class Failed", MB_ICONERROR);
			return 1;
		}
		HWND hWnd = CreateConnectionWindow(hInst);
		if (!hWnd) {
			int nResult = GetLastError();
			MessageBox(NULL, L"Window creation failed\r\nError code:" + nResult, L"Window Creation Failed", MB_ICONERROR);
			return 1;
		}
		connectionWindow = hWnd;
		ShowWindow(hWnd, nShowCmd);

		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return 0;
	}

	LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			ZeroMemory(szHistory, sizeof(szHistory));

			// Create incoming message box
			hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE,
				L"EDIT",
				L"",
				WS_CHILD | WS_VISIBLE | ES_MULTILINE |
				ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				50,
				120,
				400,
				200,
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
				(LPARAM)L"Attempting to connect to server...");

			// Create outgoing message box
			hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE,
				L"EDIT",
				L"",
				WS_CHILD | WS_VISIBLE | ES_MULTILINE |
				ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				50,
				50,
				400,
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
				WM_SETFONT, (WPARAM)hfDefault,
				MAKELPARAM(FALSE, 0));
			SendMessage(hEditOut,
				WM_SETTEXT,
				NULL,
				(LPARAM)L"Type message here...");

			// Create a push button
			HWND hWndButton = CreateWindow(
				L"BUTTON",
				L"Send",
				WS_TABSTOP | WS_VISIBLE |
				WS_CHILD | BS_DEFPUSHBUTTON,
				50,
				330,
				75,
				23,
				hWnd,
				(HMENU)IDC_MAIN_BUTTON,
				GetModuleHandle(NULL),
				NULL);

			SendMessage(hWndButton,
				WM_SETFONT,
				(WPARAM)hfDefault,
				MAKELPARAM(FALSE, 0));
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_MAIN_BUTTON:
			{
				wchar_t szBuffer[1024];

				int test = sizeof(szBuffer);
				ZeroMemory(szBuffer, sizeof(szBuffer));

				SendMessage(hEditOut,
					WM_GETTEXT,
					sizeof(szBuffer),
					reinterpret_cast<LPARAM>(szBuffer));
				send(Socket, (char *)szBuffer, wcslen(szBuffer) * 2, 0);
				SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)L"");
			}
			break;
			}
			break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			shutdown(Socket, SD_BOTH);
			closesocket(Socket);
			WSACleanup();
			return 0;
		}
		break;

		case WM_SOCKET:
		{
			if (WSAGETSELECTERROR(lParam))
			{
				MessageBox(hWnd,
					L"Connection to server failed",
					L"Error",
					MB_OK | MB_ICONERROR);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				break;
			}
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
			{
				wchar_t szIncoming[1024];
				ZeroMemory(szIncoming, sizeof(szIncoming));

				int inDataLength = recv(Socket,
					(char*)szIncoming,
					sizeof(szIncoming) / sizeof(szIncoming[0]),
					0);

				wcsncat(szHistory, szIncoming, inDataLength);
				wcscat(szHistory, L"\r\n");

				SendMessage(hEditIn,
					WM_SETTEXT,
					sizeof(szIncoming) - 1,
					reinterpret_cast<LPARAM>(&szHistory));
			}
			break;

			case FD_CLOSE:
			{
				MessageBox(hWnd,
					L"Server closed connection",
					L"Connection closed!",
					MB_ICONINFORMATION | MB_OK);
				closesocket(Socket);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			}
			break;
			}
		}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	ATOM WINAPI RegisterConnectionWindow(HINSTANCE hInstance) {
		WNDCLASSEX wClass;
		ZeroMemory(&wClass, sizeof(WNDCLASSEX));
		wClass.cbClsExtra = NULL;
		wClass.cbSize = sizeof(WNDCLASSEX);
		wClass.cbWndExtra = NULL;
		wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wClass.hIcon = NULL;
		wClass.hIconSm = NULL;
		wClass.hInstance = hInstance;
		wClass.lpfnWndProc = (WNDPROC)ConnectionWindowProc;
		wClass.lpszClassName = L"Connection Window";
		wClass.lpszMenuName = NULL;
		wClass.style = CS_HREDRAW | CS_VREDRAW;

		return RegisterClassEx(&wClass);
	}

	void CreateConnectionWindowLayout(HWND hWnd) {
		// create default font
		HGDIOBJ defaultFont = GetStockObject(DEFAULT_GUI_FONT);

		// create IP text box label
		HWND hIpLbl = CreateWindowEx(0,
			L"STATIC",
			L"IP:",
			WS_CHILD | WS_VISIBLE,
			20,
			20,
			20,
			23,
			hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL);

		// create IP text box
		HWND hIpEdit = CreateWindowEx(WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			40, 20, 100, 23,
			hWnd,
			(HMENU)IDC_CW_IPEDIT,
			GetModuleHandle(NULL),
			NULL);

		// Create port label
		HWND hPortLbl = CreateWindowEx(0,
			L"STATIC",
			L"(Optional) Port:",
			WS_CHILD | WS_VISIBLE,
			150,
			20,
			70,
			23,
			hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL);

		// Create port message box
		HWND hPortNumber = CreateWindowEx(WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | ES_NUMBER,
			230, 20, 60, 23,
			hWnd,
			(HMENU)IDC_CW_PORTEDIT,
			GetModuleHandle(NULL),
			NULL);

		// Create a connect button
		HWND hConnectBtn = CreateWindow(
			L"BUTTON", L"Connect",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			300, 20, 75, 23,
			hWnd,
			(HMENU)IDC_CW_CONNECTBTN,
			GetModuleHandle(NULL),
			NULL);

		if (!hIpEdit || !hIpLbl)
		{
			MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
		}

		// initialize controls
		SendMessage(hIpLbl, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hIpEdit, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		SendMessage(hIpEdit, EM_LIMITTEXT, 15, NULL);

		SendMessage(hPortNumber, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		SendMessage(hPortNumber, EM_LIMITTEXT, 5, NULL);

		SendMessage(hPortLbl, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hConnectBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
	}

	LRESULT CALLBACK ConnectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CW_CONNECTBTN:
			{
				if (!chatWindow) {
					HINSTANCE hInst = GetModuleHandle(NULL);
					if (!RegisterChatWindow(hInst)) {
						//fail
						return 1;
					}
					chatWindow = CreateChatWindow(hInst);
					if (!chatWindow) {
						//fail
						return 1;
					}
				}
				if (state == AppState::DISCONNECTED) {
					// Get port number
					wchar_t portBuffer[6];
					HWND hPortNumber = GetDlgItem(hWnd, IDC_CW_PORTEDIT);
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
						MessageBox(connectionWindow, L"Port number is invalid. Valid numbers are 1024 - 65535", L"Invalid port number", MB_ICONERROR);
						break;
					}

					// Get IP address
					wchar_t ipBuffer[16];
					HWND hIpAddress = GetDlgItem(hWnd, IDC_CW_IPEDIT);
					SendMessage(hIpAddress, WM_GETTEXT, sizeof(ipBuffer), reinterpret_cast<LPARAM>(ipBuffer));
					CStringA cIp(ipBuffer);
					cIp.GetBuffer();

					if (ConnectToServer(chatWindow, port, cIp.GetBuffer())) {
						UnregisterClass(L"Connection Window", GetModuleHandle(NULL));

						DestroyWindow(connectionWindow);
						connectionWindow = NULL;
						ShowWindow(chatWindow, SW_SHOWDEFAULT);
					}
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
			CreateConnectionWindowLayout(hWnd);
		}
		break;

		case WM_DESTROY:
		{
			//PostQuitMessage(0);
			//return 0;
		}
		break;

		/*case WM_SOCKET:
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
		}*/
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND CreateConnectionWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Connection Window",
			L"PlusPlusChat",
			WS_OVERLAPPEDWINDOW,
			200, 200, 400, 400,
			NULL,
			NULL,
			hInstance,
			NULL);
		return hWnd;
	}

	ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance) {
		WNDCLASSEX wClass;
		ZeroMemory(&wClass, sizeof(WNDCLASSEX));
		wClass.cbClsExtra = NULL;
		wClass.cbSize = sizeof(WNDCLASSEX);
		wClass.cbWndExtra = NULL;
		wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wClass.hIcon = NULL;
		wClass.hIconSm = NULL;
		wClass.hInstance = hInstance;
		wClass.lpfnWndProc = (WNDPROC)ChatWindowProc;
		wClass.lpszClassName = L"Chat Window";
		wClass.lpszMenuName = NULL;
		wClass.style = CS_HREDRAW | CS_VREDRAW;

		return RegisterClassEx(&wClass);
	}

	HWND CreateChatWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Chat Window",
			L"PlusPlusChat Server",
			WS_OVERLAPPEDWINDOW,
			200,
			200,
			640,
			480,
			NULL,
			NULL,
			hInstance,
			NULL);
		return hWnd;
	}

	bool ConnectToServer(HWND hWnd, std::int_fast32_t port, char* ipAddress) {
		// Set up Winsock
		WSADATA WsaDat;
		int nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
		if (nResult != 0)
		{
			MessageBox(hWnd,
				L"Winsock initialization failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Socket == INVALID_SOCKET)
		{
			MessageBox(hWnd,
				L"Socket creation failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		nResult = WSAAsyncSelect(Socket, hWnd, WM_SOCKET, (FD_CLOSE | FD_READ));
		if (nResult)
		{
			MessageBox(hWnd,
				L"WSAAsyncSelect failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		// Resolve IP address for hostname
		struct hostent *host;
		if ((host = gethostbyname(ipAddress)) == NULL)
		{
			MessageBox(hWnd,
				L"Unable to resolve host name",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		// Set up our socket address structure
		SOCKADDR_IN SockAddr;
		SockAddr.sin_port = htons(port);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

		connect(Socket, (LPSOCKADDR)(&SockAddr), sizeof(SockAddr));
	}
