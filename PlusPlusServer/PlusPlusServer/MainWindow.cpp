#include "MainWindow.h"

time_t startTime;

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
	/***  DETAILS SECTION  **/

	// Create details groupbox
	HWND hDetailsGrp = CreateWindowEx(
		0,
		L"BUTTON", L"Details",
		WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
		20, 20, 290, 220,
		hWnd, NULL, GetModuleHandle(NULL), NULL);

	// Connected clients
	HWND hConnectedL = CreateWindowEx(
		0,
		L"STATIC", L"Connected clients:",
		WS_CHILD | WS_VISIBLE,
		40, 50, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hConnectedR = CreateWindowEx(
		0,
		L"STATIC", L"0",
		WS_CHILD | WS_VISIBLE,
		170, 50, 120, 20,
		hWnd, (HMENU)IDC_CONNECTED_LBL,
		GetModuleHandle(NULL), NULL);

	// Chatting clients
	HWND hChattingL = CreateWindowEx(
		0,
		L"STATIC", L"Chatting clients:",
		WS_CHILD | WS_VISIBLE,
		40, 80, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hChattingR = CreateWindowEx(
		0,
		L"STATIC", L"0",
		WS_CHILD | WS_VISIBLE,
		170, 80, 120, 20,
		hWnd, (HMENU)IDC_CHATTING_LBL,
		GetModuleHandle(NULL), NULL);

	// All rooms
	HWND hRoomsL = CreateWindowEx(
		0,
		L"STATIC", L"Active rooms:",
		WS_CHILD | WS_VISIBLE,
		40, 110, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hRoomsR = CreateWindowEx(
		0,
		L"STATIC", L"0",
		WS_CHILD | WS_VISIBLE,
		170, 110, 120, 20,
		hWnd, (HMENU)IDC_ROOMS_LBL,
		GetModuleHandle(NULL), NULL);
	
	// Public rooms
	HWND hPublicRoomsL = CreateWindowEx(
		0,
		L"STATIC", L"Public rooms:",
		WS_CHILD | WS_VISIBLE,
		40, 140, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hPublicRoomsR = CreateWindowEx(
		0,
		L"STATIC", L"0",
		WS_CHILD | WS_VISIBLE,
		170, 140, 120, 20,
		hWnd, (HMENU)IDC_PUBLIC_LBL,
		GetModuleHandle(NULL), NULL);

	// private rooms
	HWND hPrivateRoomsL = CreateWindowEx(
		0,
		L"STATIC", L"Private rooms:",
		WS_CHILD | WS_VISIBLE,
		40, 170, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hPrivateRoomsR = CreateWindowEx(
		0,
		L"STATIC", L"0",
		WS_CHILD | WS_VISIBLE,
		170, 170, 120, 20,
		hWnd, (HMENU)IDC_PRIVATE_LBL,
		GetModuleHandle(NULL), NULL);

	// Server up time
	HWND hUpTimeL = CreateWindowEx(
		0,
		L"STATIC", L"Server up time (hh:mm):",
		WS_CHILD | WS_VISIBLE,
		40, 200, 120, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	HWND hUpTimeR = CreateWindowEx(
		0,
		L"STATIC", L"--:--",
		WS_CHILD | WS_VISIBLE,
		170, 200, 120, 20,
		hWnd, (HMENU)IDC_UPTIME_LBL,
		GetModuleHandle(NULL), NULL);

	/***  MANAGE SECTION  ***/
	/*
	// Create manage groupbox
	HWND hManageGrp = CreateWindowEx(
		0,
		L"BUTTON", L"Manage",
		WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
		315, 20, 290, 400,
		hWnd, NULL, GetModuleHandle(NULL), NULL);

	HWND hManageRoomsBtn = CreateWindowEx(
		0,
		L"BUTTON", L"Manage Chatrooms",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		335, 55, 120, 23,
		hWnd, NULL,//(HMENU)IDC_MAIN_BUTTON,
		GetModuleHandle(NULL), NULL);

	HWND hManageClientsBtn = CreateWindowEx(
		0,
		L"BUTTON", L"Manage Clients",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		465, 55, 120, 23,
		hWnd, NULL,//(HMENU)IDC_MAIN_BUTTON,
		GetModuleHandle(NULL), NULL);
*/
	/***  CONTOL SECTION  ***/

	// Create control groupbox
	HWND hControlGrp = CreateWindowEx(
		0,
		L"BUTTON", L"Control",
		WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
		20, 250, 290, 150,
		hWnd, NULL, GetModuleHandle(NULL), NULL);

	// Create port message box
	HWND hPortNumber = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | ES_NUMBER,
		185, 285, 60, 23,
		hWnd, (HMENU)IDC_PORTNUMBER,
		GetModuleHandle(NULL), NULL);

	// Create port label
	HWND hPortLbl = CreateWindowEx(
		0,
		L"STATIC", L"Port (optional):",
		WS_CHILD | WS_VISIBLE,
		85, 288, 90, 20,
		hWnd, NULL,
		GetModuleHandle(NULL), NULL);

	// Create start button
	HWND hStartBtn = CreateWindowEx(
		0,
		L"BUTTON", L"Start",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		40, 330, 250, 50,
		hWnd, (HMENU)IDC_MAIN_BUTTON,
		GetModuleHandle(NULL), NULL);

	if (!hControlGrp || !hDetailsGrp || !hPortNumber || !hPortLbl || !hStartBtn )
	{
		MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
	}
	// Initialize controls
	HGDIOBJ font = GetStockObject(DEFAULT_GUI_FONT);

	SendMessage(hDetailsGrp, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hConnectedL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hConnectedR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hChattingL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hChattingR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hRoomsL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hRoomsR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPublicRoomsL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPublicRoomsR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPrivateRoomsL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPrivateRoomsR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hUpTimeL, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hUpTimeR, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	/*
	SendMessage(hManageGrp, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hManageRoomsBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hManageClientsBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	*/
	SendMessage(hControlGrp, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPortNumber, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	SendMessage(hPortNumber, EM_LIMITTEXT, 5, NULL);

	SendMessage(hPortLbl, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));

	SendMessage(hStartBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));

	//SendMessage(hSendBtn, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_COMMAND: {
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
				SetTimer(hWnd, IDC_PING_TIMER, 300000, NULL);
				SetTimer(hWnd, IDC_UPTIME_TIMER, 31000, NULL);
				startTime = time(0);
				std::wstring zeroTime = L"00:00";
				UpdateUI(hWnd, Server::ServerDetails(), zeroTime);
				MessageBox(NULL, (L"Server started listening at port " + std::to_wstring(port)).c_str(), L"Information", MB_ICONINFORMATION);
			}
			else {
				// stop timer
				KillTimer(hWnd, IDC_PING_TIMER);
				KillTimer(hWnd, IDC_UPTIME_TIMER);
				shutdown(Server::ServerSocket(), SD_BOTH);
				closesocket(Server::ServerSocket());
				WSACleanup();
				HWND hPortNumber = GetDlgItem(hWnd, IDC_PORTNUMBER);
				std::wstring noTime = L"--:--";
				UpdateUI(hWnd, Server::ServerDetails(), noTime);
				SendMessage(GetDlgItem(hWnd, IDC_MAIN_BUTTON), WM_SETTEXT, NULL, (LPARAM)L"Start");
				EnableWindow(hPortNumber, true);
				Server::State() = AppState::STOPPED;
			}
			break;
		}		
		}
		break;
	}
		
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
			std::vector<std::wstring> messages;

			if (ReceiveData(messages, (SOCKET)wParam) != SOCKET_ERROR) {
				for (auto && m : messages)
				{
					Server::ProcessMessage(m, (SOCKET)wParam);
				}
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
		else if (wParam == IDC_UPTIME_TIMER)
		{
			// get details
			auto details = Server::GetDetails();
			// get uptime
			std::int64_t elapsed = time(0) - startTime;
			auto hrs = elapsed / 3600;
			auto mins = elapsed % 3600;
			mins /= 60;
			std::wstring hStr;
			std::wstring mStr;
			if (hrs < 10) {
				hStr = L'0' + std::to_wstring(hrs);
			}
			else {
				hStr = std::to_wstring(hrs);
			}
			if (mins < 10) {
				mStr = L'0' + std::to_wstring(mins);
			}
			else {
				mStr = std::to_wstring(mins);
			}	
			// send messages
			UpdateUI(hWnd, details, hStr + L':' + mStr);
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
		345,
		460,
		NULL,
		NULL,
		hInstance,
		NULL);
	return hWindow;
}

void UpdateUI(HWND hWnd, Server::ServerDetails & details, std::wstring & uptime) {
	SendMessage(GetDlgItem(hWnd, IDC_CONNECTED_LBL), WM_SETTEXT, 0, (LPARAM)std::to_wstring(details.allClients).c_str());
	SendMessage(GetDlgItem(hWnd, IDC_CHATTING_LBL), WM_SETTEXT, 0, (LPARAM)std::to_wstring(details.chattingClients).c_str());
	SendMessage(GetDlgItem(hWnd, IDC_ROOMS_LBL), WM_SETTEXT, 0, (LPARAM)std::to_wstring(details.allRooms).c_str());
	SendMessage(GetDlgItem(hWnd, IDC_PUBLIC_LBL), WM_SETTEXT, 0, (LPARAM)std::to_wstring(details.publicRooms).c_str());
	SendMessage(GetDlgItem(hWnd, IDC_PRIVATE_LBL), WM_SETTEXT, 0, (LPARAM)std::to_wstring(details.privateRooms).c_str());
	SendMessage(GetDlgItem(hWnd, IDC_UPTIME_LBL), WM_SETTEXT, 0, (LPARAM)uptime.c_str());
}
