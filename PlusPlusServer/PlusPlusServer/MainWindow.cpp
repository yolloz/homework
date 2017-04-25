#include "MainWindow.h"

ATOM WINAPI RegisterMainWindow(HINSTANCE hInst) {
	WNDCLASSEX wClass;
	ZeroMemory(&wClass, sizeof(WNDCLASSEX));

	HICON icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	HICON iconSmall = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON3));

	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = icon;
	wClass.hIconSm = iconSmall;
	wClass.hInstance = hInst;
	wClass.lpfnWndProc = (WNDPROC)MainWindowProc;
	wClass.lpszClassName = L"MainWindow Class";
	wClass.lpszMenuName = NULL;
	wClass.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClassEx(&wClass);
}

void CreateMainWindowLayout(HWND hWnd) {
	ZeroMemory(Server::GetInstance().szHistory, sizeof(Server::GetInstance().szHistory));

	// Create incoming message box
	HWND hEditIn = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20, 20, 580, 240,
		hWnd, (HMENU)IDC_EDIT_IN,
		GetModuleHandle(NULL), NULL);

	// Create outgoing message box
	HWND hEditOut = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20, 280, 580, 60,
		hWnd, (HMENU)IDC_EDIT_OUT,
		GetModuleHandle(NULL), NULL);

	// Create port message box
	HWND hPortNumber = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | ES_NUMBER,
		50, 360, 60, 23,
		hWnd, (HMENU)IDC_PORTNUMBER,
		GetModuleHandle(NULL), NULL);

	// Create port label
	HWND hPortLbl = CreateWindowEx(
		0,
		L"STATIC", L"Port:",
		WS_CHILD | WS_VISIBLE,
		20, 360, 25, 23,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	// Create start button
	HWND hStartBtn = CreateWindowEx(
		0,
		L"BUTTON", L"Start",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		120, 360, 75, 23,
		hWnd, (HMENU)IDC_MAIN_BUTTON,
		GetModuleHandle(NULL), NULL);


	HWND hSendBtn = CreateWindowEx(
		0,
		L"BUTTON", L"Send",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		205, 360, 75, 23,
		hWnd, (HMENU)IDC_SEND_BUTTON,
		GetModuleHandle(NULL), NULL);

	if (!hEditIn || !hEditOut || !hPortNumber || !hPortLbl || !hStartBtn || !hSendBtn)
	{
		MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
	}
	// Initialize controls
	HGDIOBJ font = GetStockObject(DEFAULT_GUI_FONT);

	SendMessage(hEditIn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hEditIn, WM_SETTEXT, NULL, (LPARAM)L"Waiting for client to connect...");

	SendMessage(hEditOut, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)L"Type message here...");

	SendMessage(hPortNumber, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPortNumber, EM_LIMITTEXT, 5, NULL);

	SendMessage(hPortLbl, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));

	SendMessage(hStartBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));

	SendMessage(hSendBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MAIN_BUTTON:
		{
			if (Server::State() == AppState::STOPPED) {
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
					port = Server::defaultPort;
				}
				if (port > 65535 || port < 1024) {
					MessageBox(hWnd, L"Port number is invalid. Valid numbers are 1024 - 65535", L"Invalid port number", MB_ICONERROR);
					break;
				}
				SendMessage(GetDlgItem(hWnd, IDC_MAIN_BUTTON), WM_SETTEXT, NULL, (LPARAM)L"Stop");
				EnableWindow(hPortNumber, false);
				Server::StartListening(hWnd, port);
				Server::State() = AppState::LISTENING;
				// start timer
				SetTimer(hWnd, IDC_PING_TIMER, 30000, NULL);
			}
			else {
				// stop timer
				KillTimer(hWnd, IDC_PING_TIMER);
				shutdown(Server::ServerSocket(), SD_BOTH);
				closesocket(Server::ServerSocket());
				WSACleanup();
				HWND hPortNumber = GetDlgItem(hWnd, IDC_PORTNUMBER);
				SendMessage(GetDlgItem(hWnd, IDC_MAIN_BUTTON), WM_SETTEXT, NULL, (LPARAM)L"Start");
				EnableWindow(hPortNumber, true);
				Server::State() = AppState::STOPPED;
			}
		}
		break;

		case IDC_SEND_BUTTON:
		{
			/*wchar_t szBuffer[1024];
			ZeroMemory(szBuffer, sizeof(szBuffer));

			SendMessage(hEditOut, WM_GETTEXT, sizeof(szBuffer), reinterpret_cast<LPARAM>(szBuffer));
			for (auto n = Server::GeInstance().cl; n <= nClient; n++)
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
		CreateMainWindowLayout(hWnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		if (Server::State() == AppState::LISTENING) {
			shutdown(Server::ServerSocket(), SD_BOTH);
			closesocket(Server::ServerSocket());
			WSACleanup();
		}
		return 0;
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
				wcsncat_s(Server::GetInstance().szHistory, incoming, inDataLength);
				wcscat_s(Server::GetInstance().szHistory, L"\r\n");

				SendMessage(GetDlgItem(hWnd, IDC_EDIT_IN), WM_SETTEXT, sizeof(incoming) - 1, reinterpret_cast<LPARAM>(&Server::GetInstance().szHistory));

				Server::ProcessMessage(incoming, (SOCKET)wParam);
			}
			break;
		}

		case FD_CLOSE:
		{
			//MessageBox(hWnd, L"Client closed connection", L"Connection closed!", MB_ICONINFORMATION | MB_OK);
			Server::RemoveClient((SOCKET)wParam);
			break;
		}

		case FD_ACCEPT:
		{
			sockaddr sockAddrClient;
			int size = sizeof(sockaddr);
			auto socket = accept(wParam, &sockAddrClient, &size);
			if (socket != INVALID_SOCKET)
			{
				if (Server::CanAddClient())
				{
					Server::AddClient(socket);
					SendMessage(GetDlgItem(hWnd, IDC_EDIT_IN), WM_SETTEXT, NULL, (LPARAM)L"Client connected!");
				}
				else {
					Server::HandleError(ErrorCode::CLIENT_LIMIT_REACHED, socket);
					shutdown(socket, SD_BOTH);
					closesocket(socket);
				}
			}
			else {
				// TODO log maybe
			}
		}
		break;
		}
	}
	case WM_TIMER: {
		if (wParam == IDC_PING_TIMER)
		{
			Server::Ping();
		}
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateMainWindow(HINSTANCE hInstance) {
	HWND hWindow = CreateWindowEx(NULL,
		L"MainWindow Class",
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
	return hWindow;
}
