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


//SOCKET Socket[maxActiveClients - 1];

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
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
			if (Server::GeInstance().State() == AppState::STOPPED) {
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
				Server::GeInstance().State() = AppState::LISTENING;
			}
			else {
				shutdown(Server::GeInstance().ServerSocket, SD_BOTH);
				closesocket(Server::GeInstance().ServerSocket);
				WSACleanup();
				HWND hPortNumber = GetDlgItem(hWnd, IDC_PORTNUMBER);
				SendMessage(hStartBtn, WM_SETTEXT, NULL, (LPARAM)L"Start");
				EnableWindow(hPortNumber, true);
				Server::GeInstance().State() = AppState::STOPPED;
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
		CreateLayout(hWnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(Server::GeInstance().ServerSocket, SD_BOTH);
		closesocket(Server::GeInstance().ServerSocket);
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
			wchar_t incoming[1024];
			ZeroMemory(incoming, sizeof(incoming));

			int inDataLength = recv((SOCKET)wParam, (char*)incoming, sizeof(incoming) / sizeof((char)incoming[0]), 0);

			if (inDataLength != -1)
			{
				wcsncat(szHistory, incoming, inDataLength);
				wcscat(szHistory, L"\r\n");

				SendMessage(hEditIn, WM_SETTEXT, sizeof(incoming) - 1, reinterpret_cast<LPARAM>(&szHistory));

				ProcessMessage(incoming, (SOCKET)wParam);
			}
			break;
		}

		case FD_CLOSE:
		{
			MessageBox(hWnd, L"Client closed connection", L"Connection closed!", MB_ICONINFORMATION | MB_OK);
			Server::GeInstance().RemoveClient((SOCKET)wParam);
			break;
		}

		case FD_ACCEPT:
		{
			int size = sizeof(sockaddr);
			auto socket = accept(wParam, &sockAddrClient, &size);
			if (socket != INVALID_SOCKET)
			{
				if (Server::GeInstance().CanAddClient())
				{
					Server::GeInstance().AddClient(socket);
					SendMessage(hEditIn, WM_SETTEXT, NULL, (LPARAM)L"Client connected!");
				}
				else {
					SendMsg(Action::ERR, L"Maximum connections limit was reached", socket);
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

	HWND hStartBtn = CreateWindow(
		L"BUTTON", L"Send",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		205, 360, 75, 23,
		hWnd,
		(HMENU)IDC_SEND_BUTTON,
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
		MessageBox(hWnd, L"Winsock initialization failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}
	Server::GeInstance().ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Server::GeInstance().ServerSocket == INVALID_SOCKET)
	{
		MessageBox(hWnd, L"Socket creation failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(Server::GeInstance().ServerSocket, (LPSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hWnd, L"Unable to bind socket", L"Error", MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}

	nResult = WSAAsyncSelect(Server::GeInstance().ServerSocket,
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

	if (listen(Server::GeInstance().ServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		MessageBox(hWnd,
			L"Unable to listen!",
			L"Error",
			MB_OK);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
		return;
	}
}

bool ProcessMessage(wchar_t * message, SOCKET s) {
	std::wstring ws(message);
	// split message to tokens
	std::vector<std::wstring> tokens = split(ws, L' ');
	if (tokens.size() >= 2) {
		// check if it starts with unique code
		if (tokens[0] == UNIQ) {
			auto action = Server::ResolveAction(tokens[1]);
			switch (action) {

			case Action::INIT: {
				Server::GeInstance().ClientConnected(s);
				SendMsg(Action::TINI, L"", s);
				break;
			}

			//case Action::ACK: {
			//	// no need for an action
			//	break;
			//}

			case Action::GETROOMS: {
				SendMsg(Action::ROOMS, Server::GeInstance().GetPublicRooms(), s);
				break;
			}

			case Action::CREATE: {
				if (tokens.size() == 5) {
					auto rtc = Server::GeInstance().AddChatRoom(tokens[3], tokens[2], tokens[4], s);
					if (rtc == ErrorCode::OK) {
						// everything was fine, notify client about succesful join
						SendMsg(Action::JOINED, tokens[3] + SPACE + tokens[4], s);
					}
					else {
						HandleError(rtc, s);
					}
				}
				else {
					HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}

			case Action::JOIN: {
				if (tokens.size() == 5) {
					auto rtc = Server::GeInstance().JoinChatRoom(s, tokens[4], tokens[3], tokens[2]);
					if (rtc == ErrorCode::OK) {
						// everything was fine, notify client about succesful join
						SendMsg(Action::JOINED, tokens[3] + SPACE + tokens[4], s);
					}
					else{
						HandleError(rtc, s);
					}
				}
				else {
					HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}

			case Action::SEND: {
				if (tokens.size() > 2) {
					auto rtc = Server::GeInstance().JoinChatRoom(s, tokens[4], tokens[3], tokens[2]);
					if (rtc == ErrorCode::OK) {
						// everything was fine, notify client about succesful join
						SendMsg(Action::JOINED, tokens[3] + SPACE + tokens[4], s);
					}
					else {
						HandleError(rtc, s);
					}
				}
				else {
					HandleError(ErrorCode::INVALID_REQUEST, s);
				}
				break;
			}
					   
			}
		}
	}
	return false;
}

std::wstring BuildMessage(const std::wstring & action, const std::wstring & payload) {
	return UNIQ + SPACE + action + SPACE + payload;
}

std::wstring BuildMessage(const std::wstring & action) {
	return UNIQ + SPACE + action;
}

void HandleError(ErrorCode code, SOCKET s) {
	std::wstring payload;
	switch (code)
	{
	case OK:
		break;
	case INVALID_REQUEST: {
		payload = L"Invalid request";
		break;
	}
	case UNAUTHORIZED_REQUEST:
		payload = L"Unauthorized request";
		break;
	case ROOM_NAME_TAKEN:
		payload = L"Room name was already taken";
		break;
	case USERNAME_TAKEN:
		payload = L"Username was already taken";
		break;
	case NO_SUCH_ROOM:
		payload = L"No such room was found";
		break;
	default:
		payload = L"Ooops. Something failed :(";
		break;
	}
	if (code != ErrorCode::OK) {
		SendMsg(Action::ERR, payload, s);
	}
}

void SendMsg(Action action, const std::wstring & payload, SOCKET s) {	
	std::wstring p;
	bool valid = true;
	switch (action) {
	case Action::ERR: {
		p = BuildMessage(L"ERR", payload);
		break;
	}

	case Action::TINI: {
		p = BuildMessage(L"TINI");
		break;
	}

	case Action::ROOMS: {
		p = BuildMessage(L"ROOMS", payload);
		break;
	}

	case Action::JOINED: {
		p = BuildMessage(L"JOINED", payload);
		break;
	}

	default: {
		valid = false;
		break;
	}
	}
	if (valid) {
		const wchar_t * msg(p.c_str());
		send(s, (char *)msg, wcslen(msg) * 2, 0);
	}
}



