#include "RoomWindow.h"

namespace PlusPlusChat {

	ATOM WINAPI RegisterRoomWindow(HINSTANCE hInstance) {
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
		wClass.lpfnWndProc = (WNDPROC)RoomWindowProc;
		wClass.lpszClassName = L"Room Window";
		wClass.lpszMenuName = NULL;
		wClass.style = CS_HREDRAW | CS_VREDRAW;

		return RegisterClassEx(&wClass);
	}

	void CreateRoomWindowLayout(HWND hWnd) {
		// create default font
		HGDIOBJ defaultFont = GetStockObject(DEFAULT_GUI_FONT);

		// create Username groupbox
		HWND hUsernameGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Username",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 20, 300, 48,
			hWnd,NULL, GetModuleHandle(NULL), NULL);

		// create Username text box
		HWND hUsernameTb = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 15, 280, 23,
			hUsernameGrp, (HMENU)IDC_RW_USERNAME_TB, GetModuleHandle(NULL), NULL);

		// Create public room groupbox
		HWND hPublicRoomGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Join Public Room",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 78, 300, 153,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// Create Public rooms listbox
		HWND hPublicRoomList = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				L"LISTBOX", L"",
				WS_CHILD | WS_VISIBLE | WS_GROUP | WS_VSCROLL,
				10, 15, 280, 100,
				hPublicRoomGrp, (HMENU)IDC_RW_PUBLICROOM_LIST, GetModuleHandle(NULL), NULL);

		// Create Join public room button
		HWND hJoinPublicRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Join Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			215, 120, 75, 23,
			hPublicRoomGrp, (HMENU)IDC_RW_JOINPUBLICROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create private room groupbox
		HWND hPrivateRoomGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Join Private Room",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 241, 300, 48,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// create Private room text box
		HWND hPrivateRoomTb = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 15, 195, 23,
			hPrivateRoomGrp, (HMENU)IDC_RW_PRIVATEROOM_TB, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hJoinPrivateRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Join Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			215, 15, 75, 23,
			hPrivateRoomGrp, (HMENU)IDC_RW_JOINPRIVATEROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create new room groupbox
		HWND hNewRoomGrp = CreateWindowEx(
			0,
			L"BUTTON", L"Create New Room",
			WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
			20, 299, 300, 76,
			hWnd, NULL, GetModuleHandle(NULL), NULL);

		// create new room text box
		HWND hNewRoomTb = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 15, 280, 23,
			hNewRoomGrp, (HMENU)IDC_RW_NEWROOM_TB, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hNewPrivateRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Create Private Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 43, 135, 23,
			hNewRoomGrp, (HMENU)IDC_RW_NEWPRIVATEROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hNewPublicRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Create Public Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			155, 43, 135, 23,
			hNewRoomGrp, (HMENU)IDC_RW_NEWPUBLICROOM_BTN, GetModuleHandle(NULL), NULL);

		/*if (!hIpEdit || !hIpLbl)
		{
			MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
		}*/

		// initialize controls
		SendMessage(hUsernameGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hUsernameTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		SendMessage(hUsernameTb, EM_LIMITTEXT, 30, NULL);

		SendMessage(hPublicRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hJoinPublicRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hPrivateRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hPrivateRoomTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		
		SendMessage(hJoinPrivateRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hNewRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hNewRoomTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hNewPrivateRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

		SendMessage(hNewPublicRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
	}

	LRESULT CALLBACK RoomWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CW_CONNECTBTN:
			{
				//if (ContextSingleton::GetInstance().state == AppState::DISCONNECTED) {
				//	// Get port number
				//	wchar_t portBuffer[6];
				//	HWND hPortNumber = GetDlgItem(hWnd, IDC_CW_PORTEDIT);
				//	SendMessage(hPortNumber, WM_GETTEXT, sizeof(portBuffer), reinterpret_cast<LPARAM>(portBuffer));
				//	std::int_fast32_t port = 0;
				//	for (size_t i = 0; i < wcslen(portBuffer); i++)
				//	{
				//		port *= 10;
				//		port += portBuffer[i] - L'0';
				//	}
				//	if (port == 0) {
				//		port = ContextSingleton::GetInstance().nPort;
				//	}
				//	if (port > 65535 || port < 1024) {
				//		MessageBox(ContextSingleton::GetInstance().connectionWindow, L"Port number is invalid. Valid numbers are 1024 - 65535", L"Invalid port number", MB_ICONERROR);
				//		break;
				//	}

				//	// Get IP address
				//	wchar_t ipBuffer[16];
				//	HWND hIpAddress = GetDlgItem(hWnd, IDC_CW_IPEDIT);
				//	SendMessage(hIpAddress, WM_GETTEXT, sizeof(ipBuffer), reinterpret_cast<LPARAM>(ipBuffer));

				//	if (ConnectToServer(ContextSingleton::GetInstance().connectionWindow, port, ipBuffer)) {
				//		ContextSingleton::GetInstance().state = AppState::CONNECTING;
				//	}
				//}
			}
			break;
			}
			break;
		case WM_CREATE:
		{
			CreateRoomWindowLayout(hWnd);
		}
		break;

		case WM_DESTROY:
		{
			auto state = ContextSingleton::GetInstance().state;
			if (state == AppState::CONNECTED) {
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


				/*UnregisterClass(L"Connection Window", GetModuleHandle(NULL));

				DestroyWindow(ContextSingleton::GetInstance().connectionWindow);
				ContextSingleton::GetInstance().connectionWindow = NULL;
				ShowWindow(ContextSingleton::GetInstance().chatWindow, SW_SHOWDEFAULT);*/

				/*for (int n = 0; n <= nMaxClients; n++)
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
				}*/
			}
			break;

			case FD_CLOSE:
			{
				/*MessageBox(hWnd,
				L"Client closed connection",
				L"Connection closed!",
				MB_ICONINFORMATION | MB_OK);*/
			}
			break;

			case FD_CONNECT:
			{
				ContextSingleton::GetInstance().state = AppState::CONNECTING;
				Communicator::SendMsg(Action::INIT, L"", (SOCKET)wParam);
			}
			break;


			}
		}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND CreateRoomWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Room Window",
			L"PlusPlusChat",
			WS_OVERLAPPEDWINDOW,
			200, 200, 355, 435,
			NULL,
			NULL,
			hInstance,
			NULL);
		return hWnd;
	}

	bool ActivateRoomWindow(){
		if (!ContextSingleton::GetInstance().roomWindow) {
			HINSTANCE hInst = GetModuleHandle(NULL);
			if (!RegisterRoomWindow(hInst)) {
				//fail
				return false;
			}
			ContextSingleton::GetInstance().roomWindow = CreateRoomWindow(hInst);
			if (!ContextSingleton::GetInstance().roomWindow) {
				//fail
				return false;
			}
		}

		ShowWindow(ContextSingleton::GetInstance().roomWindow, SW_SHOWDEFAULT);
		WSAAsyncSelect(ContextSingleton::GetInstance().Socket, ContextSingleton::GetInstance().roomWindow, WM_SOCKET, (FD_READ | FD_CLOSE));
		return true;
	}

	void DeactivateRoomWindow() {
		UnregisterClass(L"Room Window", GetModuleHandle(NULL));
		DestroyWindow(ContextSingleton::GetInstance().roomWindow);
		ContextSingleton::GetInstance().roomWindow = NULL;
	}
}