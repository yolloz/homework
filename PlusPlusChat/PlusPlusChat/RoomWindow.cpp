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
			30, 35, 280, 23,
			hWnd, (HMENU)IDC_RW_USERNAME_TB, GetModuleHandle(NULL), NULL);

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
			WS_CHILD | WS_VISIBLE | WS_GROUP | WS_VSCROLL | LBS_SORT,
			30, 93, 280, 100,
			hWnd, (HMENU)IDC_RW_PUBLICROOM_LIST, GetModuleHandle(NULL), NULL);

		// Create refresh public rooms button
		HWND hJoinPublicRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Refresh",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			150, 198, 75, 23,
			hWnd, (HMENU)IDC_RW_REFRESHPUBLICROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create Join public room button
		HWND hRefreshPublicRoomsBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Join Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			235, 198, 75, 23,
			hWnd, (HMENU)IDC_RW_JOINPUBLICROOM_BTN, GetModuleHandle(NULL), NULL);

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
			30, 256, 195, 23,
			hWnd, (HMENU)IDC_RW_PRIVATEROOM_TB, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hJoinPrivateRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Join Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			235, 256, 75, 23,
			hWnd, (HMENU)IDC_RW_JOINPRIVATEROOM_BTN, GetModuleHandle(NULL), NULL);

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
			30, 314, 280, 23,
			hWnd, (HMENU)IDC_RW_NEWROOM_TB, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hNewPrivateRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Create Private Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			30, 342, 135, 23,
			hWnd, (HMENU)IDC_RW_NEWPRIVATEROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create Join private room button
		HWND hNewPublicRoomBtn = CreateWindowEx(
			0,
			L"BUTTON", L"Create Public Room",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			175, 342, 135, 23,
			hWnd, (HMENU)IDC_RW_NEWPUBLICROOM_BTN, GetModuleHandle(NULL), NULL);

		// Create progress bar
		HWND hProgress = CreateWindowEx(
			0,
			PROGRESS_CLASS, L"My progress",
			WS_VISIBLE | WS_CHILD | PBS_MARQUEE ,
			20, 385, 300, 18,
			hWnd, (HMENU)IDC_RW_PROGRESS, GetModuleHandle(NULL), NULL);

		// Hide progress bar
		ShowWindow(hProgress, SW_HIDE);

		if (!hUsernameGrp || !hUsernameTb || !hPublicRoomGrp || !hPublicRoomList || !hRefreshPublicRoomsBtn 
			|| !hJoinPublicRoomBtn || !hPrivateRoomGrp || !hPrivateRoomTb || !hJoinPrivateRoomBtn || !hNewRoomGrp 
			|| !hNewRoomTb || !hNewPrivateRoomBtn || !hNewPublicRoomBtn || !hProgress)
		{
			MessageBox(hWnd, L"Could not create window layout.", L"Error", MB_OK | MB_ICONERROR);
		}
		else {
			// initialize controls
			SendMessage(hUsernameGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hUsernameTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
			SendMessage(hUsernameTb, EM_LIMITTEXT, 30, NULL);

			SendMessage(hPublicRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hPublicRoomList, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hJoinPublicRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hRefreshPublicRoomsBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hPrivateRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hPrivateRoomTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
			SendMessage(hPrivateRoomTb, EM_LIMITTEXT, 30, NULL);

			SendMessage(hJoinPrivateRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hNewRoomGrp, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hNewRoomTb, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
			SendMessage(hNewRoomTb, EM_LIMITTEXT, 30, NULL);

			SendMessage(hNewPrivateRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));

			SendMessage(hNewPublicRoomBtn, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(FALSE, 0));
		}		
	}

	LRESULT CALLBACK RoomWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_COMMAND: {
			switch (LOWORD(wParam))
			{
			case IDC_RW_NEWPUBLICROOM_BTN:
			{
				if (ContextSingleton::GetInstance().state == AppState::CONNECTED && CreateNewRoom(hWnd, false)) {
					SetWaitingRW(hWnd, true);
					ContextSingleton::GetInstance().state = AppState::JOINING;
				}				
				break;
			}
			case IDC_RW_NEWPRIVATEROOM_BTN: {
				if (ContextSingleton::GetInstance().state == AppState::CONNECTED && CreateNewRoom(hWnd, true)) {
					SetWaitingRW(hWnd, true);
					ContextSingleton::GetInstance().state = AppState::JOINING;
				}
				break;
			}
			case IDC_RW_JOINPRIVATEROOM_BTN: {
				if (ContextSingleton::GetInstance().state == AppState::CONNECTED && JoinRoom(hWnd, true)) {
					SetWaitingRW(hWnd, true);
					ContextSingleton::GetInstance().state = AppState::JOINING;
				}
				break;
			}
			case IDC_RW_JOINPUBLICROOM_BTN: {
				if (ContextSingleton::GetInstance().state == AppState::CONNECTED && JoinRoom(hWnd, false)) {
					SetWaitingRW(hWnd, true);
					ContextSingleton::GetInstance().state = AppState::JOINING;
				}
				break;
			}
			case IDC_RW_REFRESHPUBLICROOM_BTN: {
				Communicator::SendMsg(Action::GETROOMS, L"", ContextSingleton::GetInstance().Socket);
				break;
			}
											
			}
			break;
		}

		case WM_CREATE:
		{
			CreateRoomWindowLayout(hWnd);
			break;
		}		

		case WM_DESTROY:
		{
			auto state = ContextSingleton::GetInstance().state;
			if (state == AppState::CONNECTED || state == AppState::JOINING) {
				PostQuitMessage(0);
				shutdown(ContextSingleton::GetInstance().Socket, SD_BOTH);
				closesocket(ContextSingleton::GetInstance().Socket);
				WSACleanup();
				return 0;
			}
			break;
		}

		case WM_SOCKET:
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
			{
				std::vector<wchar_t> incoming(1100, 0);
				auto q = incoming.size() * sizeof(incoming[0]);
				int inDataLength = recv((SOCKET)wParam, (char*)incoming.data(), incoming.size() * sizeof(incoming[0]), 0);
				while (inDataLength == q) {
					incoming.resize(q, 0);
					inDataLength = recv((SOCKET)wParam, (char*)incoming.data(), incoming.size() * sizeof(incoming[0]), 0);
				}
				bool wasNull = true;
				auto start = incoming.begin();
				std::vector<std::wstring> messages;
				for (auto  i = incoming.begin(); i < incoming.end(); i++)
				{
					if (*i == 0) {
						if (wasNull) {
							// two nulls mean nothing is behind this
							break;
						}
						else {
							wasNull = true;
							// cut message
							messages.push_back(std::wstring(start, i));
						}
					}
					else {
						if (wasNull) {
							wasNull = false;
							start = i;
						}
					}
				}

				for (auto m : messages)
				{
					Communicator::ProcessMessage(m.c_str(), (SOCKET)wParam);
				}
			}
			break;

			case FD_CLOSE:
			{
				MessageBox(hWnd, L"Server closed connection!", L"Connection closed!", MB_ICONINFORMATION | MB_OK);
				DestroyWindow(hWnd);
			}
			break;

			case FD_CONNECT:
			{
				ContextSingleton::GetInstance().state = AppState::CONNECTING;
				Communicator::SendMsg(Action::INIT, L"", (SOCKET)wParam);
			}
			break;


			}
			break;
		}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND CreateRoomWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Room Window",
			L"PlusPlusChat",
			WS_OVERLAPPEDWINDOW,
			200, 200, 355, 460,
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

	void SetWaitingRW(HWND hWnd, bool waiting) {
		if (waiting) {
			// disable interactive components
			EnableWindow(GetDlgItem(hWnd, IDC_RW_USERNAME_TB), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_JOINPUBLICROOM_BTN), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_JOINPRIVATEROOM_BTN), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_PRIVATEROOM_TB), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWROOM_TB), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWPRIVATEROOM_BTN), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWPUBLICROOM_BTN), false);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_PUBLICROOM_LIST), false);
			// set waiting cursor
			SetCursor(ContextSingleton::GetInstance().waitCursor);
			// show progress
			auto progress = GetDlgItem(hWnd, IDC_RW_PROGRESS);
			ShowWindow(progress, SW_SHOW);
			SendMessage(progress, PBM_SETMARQUEE, 1, (LPARAM)NULL);
		}
		else {
			// enable interactive components
			EnableWindow(GetDlgItem(hWnd, IDC_RW_USERNAME_TB), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_JOINPUBLICROOM_BTN), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_JOINPRIVATEROOM_BTN), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_PRIVATEROOM_TB), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWROOM_TB), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWPRIVATEROOM_BTN), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_NEWPUBLICROOM_BTN), true);
			EnableWindow(GetDlgItem(hWnd, IDC_RW_PUBLICROOM_LIST), true);
			// set default cursor
			SetCursor(ContextSingleton::GetInstance().defaultCursor);
			// hide progress
			auto progress = GetDlgItem(hWnd, IDC_RW_PROGRESS);
			ShowWindow(progress, SW_HIDE);
			SendMessage(progress, PBM_SETMARQUEE, 0, (LPARAM)NULL);
		}
	}

	bool JoinRoom(HWND hWnd, bool privateRoom) {
		// extract username
		wchar_t usernameBuff[31];
		HWND hUsername = GetDlgItem(hWnd, IDC_RW_USERNAME_TB);
		SendMessage(hUsername, WM_GETTEXT, sizeof(usernameBuff), reinterpret_cast<LPARAM>(usernameBuff));
		auto usernameLength = wcslen(usernameBuff);
		if (usernameLength == 0) {
			MessageBox(hWnd, L"Username is empty. Please, enter username.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		wchar_t roomNameBuff[31];
		ZeroMemory(roomNameBuff, sizeof(roomNameBuff));	
		size_t roomNameLength;
		if (privateRoom) {
			//extract room name
			HWND hRoomName = GetDlgItem(hWnd, IDC_RW_PRIVATEROOM_TB);
			SendMessage(hRoomName, WM_GETTEXT, sizeof(roomNameBuff), reinterpret_cast<LPARAM>(roomNameBuff));	
			roomNameLength = wcslen(roomNameBuff);
			if (roomNameLength == 0) {
				MessageBox(hWnd, L"Chat room name is empty. Please, enter a name.", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
		}
		else {
			// extract selected item from listbox
			HWND listBox = GetDlgItem(hWnd, IDC_RW_PUBLICROOM_LIST);

			// Get selected index.
			int lbItem = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);

			if (lbItem != LB_ERR)
			{
				// Get item data.
				size_t i = (size_t)SendMessage(listBox, LB_GETITEMDATA, lbItem, 0);
				wcscpy_s(roomNameBuff, ContextSingleton::GetInstance().roomsList[i].c_str());
				roomNameLength = wcslen(roomNameBuff);
			}
			else {
				MessageBox(hWnd, L"No chat room was slected", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
		}		
		// Construct payload
		wchar_t payload[64];

		// set room visibility - S:secret / V:visible
		if (privateRoom)
		{
			payload[0] = L'S';
		}
		else {
			payload[0] = L'V';
		}
		payload[1] = L' ';
		for (size_t i = 0; i < roomNameLength; i++)
		{
			if (roomNameBuff[i] == L' ') {
				MessageBox(hWnd, L"Chat room name cannot contain whitespace.", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
			payload[i + 2] = roomNameBuff[i];
		}
		payload[roomNameLength + 2] = L' ';
		for (size_t i = 0; i < usernameLength; i++)
		{
			if (usernameBuff[i] == L' ') {
				MessageBox(hWnd, L"Username cannot contain whitespace.", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
			payload[i + roomNameLength + 3] = usernameBuff[i];
		}
		payload[3 + roomNameLength + usernameLength] = 0;
		Communicator::SendMsg(Action::JOIN, payload, ContextSingleton::GetInstance().Socket);
		return true;
	}

	bool CreateNewRoom(HWND hWnd, bool privateRoom) {
		// extract username
		wchar_t usernameBuff[31];
		HWND hUsername = GetDlgItem(hWnd, IDC_RW_USERNAME_TB);
		SendMessage(hUsername, WM_GETTEXT, sizeof(usernameBuff), reinterpret_cast<LPARAM>(usernameBuff));
		auto usernameLength = wcslen(usernameBuff);
		if (usernameLength == 0) {
			MessageBox(hWnd, L"Username is empty. Please, enter username.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		//extract room name
		wchar_t roomNameBuff[31];
		HWND hRoomName = GetDlgItem(hWnd, IDC_RW_NEWROOM_TB);
		SendMessage(hRoomName, WM_GETTEXT, sizeof(roomNameBuff), reinterpret_cast<LPARAM>(roomNameBuff));
		auto roomNameLength = wcslen(roomNameBuff);
		if (roomNameLength == 0) {
			MessageBox(hWnd, L"Chat room name is empty. Please, enter a name.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Construct payload
		wchar_t payload[64];

		// set room visibility - S:secret / V:visible
		if (privateRoom)
		{
			payload[0] = L'S';
		}
		else {
			payload[0] = L'V';
		}
		payload[1] = L' ';
		for (size_t i = 0; i < roomNameLength; i++)
		{
			if (roomNameBuff[i] == L' ') {
				MessageBox(hWnd, L"Chat room name cannot contain whitespace.", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
			payload[i + 2] = roomNameBuff[i];
		}
		payload[roomNameLength + 2] = L' ';
		for (size_t i = 0; i < usernameLength; i++)
		{
			if (usernameBuff[i] == L' ') {
				MessageBox(hWnd, L"Username cannot contain whitespace.", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
			payload[i + roomNameLength + 3] = usernameBuff[i];
		}
		payload[3 + roomNameLength + usernameLength] = 0;
		Communicator::SendMsg(Action::CREATE, payload, ContextSingleton::GetInstance().Socket);
		return true;
	}

	void ReloadPublicRoomsList() {
		auto hWnd = ContextSingleton::GetInstance().roomWindow;
		if (hWnd != NULL)
		{
			auto && vct = ContextSingleton::GetInstance().roomsList;
			auto listBox = GetDlgItem(hWnd, IDC_RW_PUBLICROOM_LIST);
			// delete all current items
			SendMessage(listBox, LB_RESETCONTENT, 0, 0);
			for (size_t i = 0; i < vct.size(); i++)
			{
				// insert item
				int pos = (int)SendMessage(listBox, LB_ADDSTRING, 0,(LPARAM)vct[i].c_str());
				// set item index to match array index
				SendMessage(listBox, LB_SETITEMDATA, pos, (LPARAM)i);
			}
		}
	}
}