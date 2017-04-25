#include "ChatWindow.h"


//HWND hEditIn = NULL;
//HWND hEditOut = NULL;
wchar_t szHistory[10000];
bool shiftDown = false;
bool capturedEnter = false;
WNDPROC oldEditProc;



namespace PlusPlusChat {
	LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			CreateChatWindowLayout(hWnd);
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CH_SEND_BUTTON:
			{
				Send(hWnd);
			}
			break;
			}
			break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			shutdown(ContextSingleton::GetInstance().Socket, SD_BOTH);
			closesocket(ContextSingleton::GetInstance().Socket);
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
				MessageBox(hWnd,
					L"Server closed connection",
					L"Connection closed!",
					MB_ICONINFORMATION | MB_OK);
				closesocket(ContextSingleton::GetInstance().Socket);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			}
			break;
			}
		}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Send(HWND hWnd) {
		wchar_t szBuffer[1024];

		int test = sizeof(szBuffer);
		ZeroMemory(szBuffer, sizeof(szBuffer));
		// get the message
		HWND hMessageTb = GetDlgItem(hWnd, IDC_CH_MESSAGE_TB);
		SendMessage(hMessageTb, WM_GETTEXT, sizeof(szBuffer), reinterpret_cast<LPARAM>(szBuffer));
		if (wcslen(szBuffer) > 0)
		{
			Communicator::SendMsg(Action::SEND, szBuffer, ContextSingleton::GetInstance().Socket);
		}
		SendMessage(hMessageTb, WM_SETTEXT, NULL, (LPARAM)L"");
	}

	ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance) {
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
		wClass.lpfnWndProc = (WNDPROC)ChatWindowProc;
		wClass.lpszClassName = L"Chat Window";
		wClass.lpszMenuName = NULL;
		wClass.style = CS_HREDRAW | CS_VREDRAW;

		return RegisterClassEx(&wClass);
	}

	HWND CreateChatWindow(HINSTANCE hInstance, std::wstring & roomName, std::wstring & userName) {
		auto title = L"*PlusPlusChat*    Chatroom: " + roomName + L", User: " + userName;
		HWND hWnd = CreateWindowEx(
			NULL,
			L"Chat Window", title.c_str(),
			WS_OVERLAPPEDWINDOW,
			200, 200, 640, 480,
			NULL, NULL,
			hInstance, NULL);

		return hWnd;
	}

	bool ActivateChatWindow(std::wstring & roomName, std::wstring & userName) {
		if (!ContextSingleton::GetInstance().chatWindow) {
			HINSTANCE hInst = GetModuleHandle(NULL);
			if (!RegisterChatWindow(hInst)) {
				//fail
				return false;
			}
			ContextSingleton::GetInstance().chatWindow = CreateChatWindow(hInst, roomName, userName);
			if (!ContextSingleton::GetInstance().chatWindow) {
				//fail
				return false;
			}
		}
		
		ShowWindow(ContextSingleton::GetInstance().chatWindow, SW_SHOWDEFAULT);
		WSAAsyncSelect(ContextSingleton::GetInstance().Socket, ContextSingleton::GetInstance().chatWindow, WM_SOCKET, (FD_READ | FD_CLOSE));
		return true;
	}

	std::wstring GetTime() {
		wchar_t buffer[7];
		time_t t = time(0);   // get time now
		struct tm * now = localtime(&t);
		//int day = now->tm_mday;
		//int month = now->tm_mon + 1;
		int hour = now->tm_hour;
		int minute = now->tm_min;
		/*buffer[0] = L'0' + (day / 10);
		buffer[1] = L'0' + (day % 10);
		buffer[2] = L'.';
		buffer[3] = L'0' + (month / 10);
		buffer[4] = L'0' + (month % 10);
		buffer[5] = L'.';
		buffer[6] = L' ';*/
		buffer[0] = L'0' + (hour / 10);
		buffer[1] = L'0' + (hour % 10);
		buffer[2] = L':';
		buffer[3] = L'0' + (minute / 10);
		buffer[4] = L'0' + (minute % 10);
		buffer[5] = L' ';
		buffer[6] = 0;
		return std::wstring(buffer);
	}

	void ReceiveMessage(std::wstring & sender, std::wstring & message) {
		std::wstring info = GetTime() + sender + L"\r\n";
		wcsncat_s(szHistory, info.c_str(), info.length());
		//wcscat_s(szHistory, L"\r\n");
		std::wstring text = L">> " + message + L"\r\n";
		wcsncat_s(szHistory, text.c_str(), text.length());
		//wcscat_s(szHistory, L"\r\n");

		SendMessage(GetDlgItem(ContextSingleton::GetInstance().chatWindow, IDC_CH_HISTORY), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(&szHistory));
	}

	void CreateChatWindowLayout(HWND hWnd) {
		ZeroMemory(szHistory, sizeof(szHistory));

		// Create chat history
		HWND hChatHistory = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
			20, 20, 585, 300,
			hWnd, (HMENU)IDC_CH_HISTORY,
			GetModuleHandle(NULL), NULL);		

		// Create outgoing message box
		HWND hMessageTb = CreateCustomEdit(
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
			20, 330, 585, 60,
			hWnd, (HMENU)IDC_CH_MESSAGE_TB,
			GetModuleHandle(NULL));

		// Create a push button
		HWND hSendButton = CreateWindowEx(
			0,
			L"BUTTON", L"Send",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			530, 400, 75, 23,
			hWnd, (HMENU)IDC_CH_SEND_BUTTON,
			GetModuleHandle(NULL), NULL);

		if (!hChatHistory || !hMessageTb || !hSendButton)
		{
			MessageBox(hWnd, L"Could not create chat window layout.", L"Error", MB_OK | MB_ICONERROR);
		}

		// initialize controls
		HGDIOBJ font = GetStockObject(DEFAULT_GUI_FONT);

		SendMessage(hChatHistory, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));

		SendMessage(hMessageTb, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
		SendMessage(hMessageTb, WM_SETTEXT, NULL, (LPARAM)L"Type message here...");

		SendMessage(hSendButton, WM_SETFONT, (WPARAM)font, MAKELPARAM(FALSE, 0));
	}

	LRESULT CALLBACK subEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYUP: {
			if (wParam == VK_SHIFT) {
				shiftDown = false;
			}
			break;
		}
		case WM_KEYDOWN: {
			if (wParam == VK_SHIFT) {
				shiftDown = true;
				break;
			}
			else if (wParam == VK_RETURN) {
				if (!shiftDown) {
					capturedEnter = true;
					Send(ContextSingleton::GetInstance().chatWindow);
					break;
				}
			}
		}
		case WM_CHAR: {
			if (capturedEnter && wParam == L'\r') {
				capturedEnter = false;
				break;
			}
		}
		default:
			return CallWindowProc(oldEditProc, hWnd, msg, wParam, lParam);

		}
		return 0;
	}

	HWND CreateCustomEdit(
			_In_     DWORD     dwStyle,
			_In_     int       x,
			_In_     int       y,
			_In_     int       nWidth,
			_In_     int       nHeight,
			_In_opt_ HWND      hWndParent,
			_In_opt_ HMENU     hMenu,
			_In_opt_ HINSTANCE hInstance){
		// create control
		HWND hInput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, NULL);
		//set old proc
		oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)subEditProc);
		return hInput;
	}
}