#include "ConnectionWindow.h"

namespace PlusPlusChat {

	ATOM WINAPI RegisterConnectionWindow(HINSTANCE hInstance) {
		WNDCLASSEX wClass;
		ZeroMemory(&wClass, sizeof(WNDCLASSEX));
		wClass.cbClsExtra = NULL;
		wClass.cbSize = sizeof(WNDCLASSEX);
		wClass.cbWndExtra = NULL;
		wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wClass.hIcon = ContextSingleton::GetInstance().icon;
		wClass.hIconSm = ContextSingleton::GetInstance().iconSmall;
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

		// create manual connection groupbox
		HWND hManualConnectGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Manual Connection",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 20, 380, 68,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// create IP text box label
		HWND hIpLbl = CreateWindowEx(
			0,
			L"STATIC", L"IP:",
			WS_CHILD | WS_VISIBLE,
			30, 48, 20, 23,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// create IP text box
		HWND hIpEdit = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"192.168.0.188",//L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			55, 45, 100, 23,
			hWnd, (HMENU)IDC_CW_IPEDIT, GetModuleHandle(NULL), NULL);

		// Create port label
		HWND hPortLbl = CreateWindowEx(
			0,
			L"STATIC", L"(Optional) Port:",
			WS_CHILD | WS_VISIBLE,
			165, 48, 70, 23,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// Create port message box
		HWND hPortNumber = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"4444",//L"",
			WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP,
			245, 45, 60, 23,
			hWnd, (HMENU)IDC_CW_PORTEDIT, GetModuleHandle(NULL), NULL);

		// Create a connect button
		HWND hConnectBtn = CreateWindow(
			L"BUTTON", L"Connect",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			315, 45, 75, 23,
			hWnd, (HMENU)IDC_CW_CONNECTBTN, GetModuleHandle(NULL), NULL);

		// create automatic connection groupbox
		HWND hAutoConnectGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Automatic Connection",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 98, 380, 68,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// Create auto connect button
		HWND hAutoConnectBtn = CreateWindow(
			L"BUTTON", L"Connect to server from CONFIG file",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			80, 123, 260, 23,
			hWnd, (HMENU)IDC_CW_AUTOCONNECTBTN, GetModuleHandle(NULL), NULL);

		if (!hIpEdit || !hIpLbl)
		{
			MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
		}

		// initialize controls
		SendMessage(hManualConnectGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hIpLbl, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hIpEdit, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		SendMessage(hIpEdit, EM_LIMITTEXT, 15, NULL);

		SendMessage(hPortNumber, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		SendMessage(hPortNumber, EM_LIMITTEXT, 5, NULL);

		SendMessage(hPortLbl, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hConnectBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hAutoConnectGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hAutoConnectBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
	}

	LRESULT CALLBACK ConnectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CW_CONNECTBTN:
			{				
				if (ContextSingleton::GetInstance().state == AppState::DISCONNECTED) {
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
						port = ContextSingleton::GetInstance().nPort;
					}
					if (port > 65535 || port < 1024) {
						MessageBox(ContextSingleton::GetInstance().connectionWindow, L"Port number is invalid. Valid numbers are 1024 - 65535", L"Invalid port number", MB_ICONERROR);
						break;
					}

					// Get IP address
					wchar_t ipBuffer[16];
					HWND hIpAddress = GetDlgItem(hWnd, IDC_CW_IPEDIT);
					SendMessage(hIpAddress, WM_GETTEXT, sizeof(ipBuffer), reinterpret_cast<LPARAM>(ipBuffer));

					if (ConnectToServer(ContextSingleton::GetInstance().connectionWindow, port, ipBuffer)) {
						//ContextSingleton::GetInstance().state = AppState::CONNECTING;
					}
				}
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
			auto state = ContextSingleton::GetInstance().state;
			if (state == AppState::DISCONNECTED || state == AppState::CONNECTING) {
				if (state == AppState::CONNECTING) {
					shutdown(ContextSingleton::GetInstance().Socket, SD_BOTH);
					closesocket(ContextSingleton::GetInstance().Socket);
					WSACleanup();
				}
				PostQuitMessage(0);
				return 0;
			}
		}
		break;

		case WM_SOCKET:
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
			{
				wchar_t incoming[1024];
				ZeroMemory(incoming, sizeof(incoming));

				int inDataLength = recv((SOCKET)wParam, (char*)incoming, sizeof(incoming) / sizeof((char)incoming[0]), 0);

				if (inDataLength != -1)
				{
					Communicator::ProcessMessage(incoming, (SOCKET)wParam);
				}
			}
			break;

			case FD_CLOSE:
			{
				MessageBox(hWnd, L"Server closed connection!", L"Connection closed!", MB_ICONINFORMATION | MB_OK);
				DestroyWindow(hWnd);
				break;
			}

			case FD_CONNECT:
			{
				ContextSingleton::GetInstance().state = AppState::CONNECTING;
				Communicator::SendMsg(Action::INIT, L"", (SOCKET)wParam);
				break;
			}
			}
		}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND CreateConnectionWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Connection Window",
			L"PlusPlusChat",
			WS_OVERLAPPEDWINDOW,
			200, 200, 435, 225,
			NULL,
			NULL,
			hInstance,
			NULL);
		return hWnd;
	}

	bool ConnectToServer(HWND hWnd, std::int_fast32_t port, wchar_t* ipAddress) {
		// resolve IP address
		in_addr address;
		int nResult = InetPton(AF_INET, ipAddress, &address);

		if (nResult != 1) {
			if (nResult == 0) {
				// invalid format
				MessageBox(NULL, L"The IP address you entered is not a valid IP address.", L"Connection Failed", MB_ICONERROR);
				return false;
			}
			else {
				MessageBox(NULL, L"An error occured during IP address evaluation.\r\nError code: " + WSAGetLastError(), L"Connection Failed", MB_ICONERROR);
				return false;
			}
		}

		// Set up Winsock
		WSADATA WsaDat;
		nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
		if (nResult != 0)
		{
			MessageBox(hWnd,
				L"Winsock initialization failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		ContextSingleton::GetInstance().Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ContextSingleton::GetInstance().Socket == INVALID_SOCKET)
		{
			MessageBox(hWnd,
				L"Socket creation failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}

		nResult = WSAAsyncSelect(ContextSingleton::GetInstance().Socket, hWnd, WM_SOCKET, (FD_CLOSE | FD_READ | FD_CONNECT));
		if (nResult)
		{
			MessageBox(hWnd,
				L"WSAAsyncSelect failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			return false;
		}


		//// Resolve IP address for hostname
		//struct hostent *host;
		//if ((host = getaddrI(ipAddress)) == NULL)
		//{
		//	MessageBox(hWnd,
		//		L"Unable to resolve host name",
		//		L"Critical Error",
		//		MB_ICONERROR);
		//	SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		//	return false;
		//}

		// Set up our socket address structure
		SOCKADDR_IN SockAddr;
		SockAddr.sin_port = htons(port);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr = address;

		connect(ContextSingleton::GetInstance().Socket, (LPSOCKADDR)(&SockAddr), sizeof(SockAddr));
		return true;
	}

	void DeactivateConnectionWindow() {
		UnregisterClass(L"Connection Window", GetModuleHandle(NULL));
		DestroyWindow(ContextSingleton::GetInstance().connectionWindow);
		ContextSingleton::GetInstance().connectionWindow = NULL;
	}
}