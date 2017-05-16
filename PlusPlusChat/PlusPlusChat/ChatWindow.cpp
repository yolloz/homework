#include "ChatWindow.h"

std::vector<wchar_t> history;
bool shiftDown = false;
bool capturedEnter = false;
WNDPROC oldEditProc;
const int SendBtnHeight = 23;
const int SendBtnWidth = 75;
const int MessageTbHeight = 60;
const int sideMargin = 20;
const int innerMargin = 10;

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

		case WM_GETMINMAXINFO: {
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 300;
			lpMMI->ptMinTrackSize.y = 300;
			return 0; // by processing this message I should return zero
		}

		case WM_SIZE: {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			ResizeCH(hWnd, width, height);
			break;
		}

		case WM_SOCKET:
		{
			if (WSAGETSELECTERROR(lParam))
			{
				if (lParam != WSAEMSGSIZE) {
					auto l = WSAGetLastError();
					MessageBox(hWnd,
						L"Connection to server failed",
						L"Error",
						MB_OK | MB_ICONERROR);
					SendMessage(hWnd, WM_DESTROY, NULL, NULL);
					break;
				}
			}
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
			{
				std::vector<std::wstring> messages;

				if (ReceiveData(messages, (SOCKET)wParam)) {					
					for (auto i = messages.begin(); i < messages.end(); i++)
					{
						Communicator::ProcessMessage(*i, (SOCKET)wParam);
					}
				}
			}
			break;

			case FD_CLOSE:
			{
				MessageBox(hWnd, L"Server closed connection", L"Connection closed!", MB_ICONINFORMATION | MB_OK);
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
		wchar_t buffer[1001];

		ZeroMemory(buffer, sizeof(buffer));
		// get the message
		HWND hMessageTb = GetDlgItem(hWnd, IDC_CH_MESSAGE_TB);
		SendMessage(hMessageTb, WM_GETTEXT, sizeof(buffer), reinterpret_cast<LPARAM>(buffer));
		if (wcslen(buffer) > 0)
		{
			Communicator::SendMsg(Action::SEND, buffer, ContextSingleton::GetInstance().Socket);
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

	std::wstring GetTime(time_t t) {
		wchar_t buffer[7];
		if (t == 0) {
			t = time(0);   // get time now 
		}
		struct tm * now = localtime(&t);
		int hour = now->tm_hour;
		int minute = now->tm_min;
		buffer[0] = L'0' + (hour / 10);
		buffer[1] = L'0' + (hour % 10);
		buffer[2] = L':';
		buffer[3] = L'0' + (minute / 10);
		buffer[4] = L'0' + (minute % 10);
		buffer[5] = L' ';
		buffer[6] = 0;
		return std::wstring(buffer);
	}

	void ReceiveMessageCH(std::wstring & sender, time_t time, std::wstring & message) {
		if (history.size() > 0) {
			// remove null char
			history.pop_back();
		}
		std::wstring info = GetTime(time) + sender + L"\r\n"; 
		for (auto i = info.begin(); i < info.end(); i++)
		{
			history.push_back(*i);
		}
		std::wstring text = L">> " + message + L"\r\n";
		for (auto i = text.begin(); i < text.end(); i++)
		{
			history.push_back(*i);
		}
		history.push_back(L'\0');

		SendMessage(GetDlgItem(ContextSingleton::GetInstance().chatWindow, IDC_CH_HISTORY), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(history.data()));
		SendMessage(GetDlgItem(ContextSingleton::GetInstance().chatWindow, IDC_CH_HISTORY), EM_LINESCROLL, 0, 65535);
	}

	void CreateChatWindowLayout(HWND hWnd) {
		// Create chat history
		HWND hChatHistory = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
			sideMargin, sideMargin, 585, 300,
			hWnd, (HMENU)IDC_CH_HISTORY,
			GetModuleHandle(NULL), NULL);		

		// Create outgoing message box
		HWND hMessageTb = CreateCustomEdit(
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
			sideMargin, 330, 585, MessageTbHeight,
			hWnd, (HMENU)IDC_CH_MESSAGE_TB,
			GetModuleHandle(NULL));

		// Create a push button
		HWND hSendButton = CreateWindowEx(
			0,
			L"BUTTON", L"Send",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			530, 400, SendBtnWidth, SendBtnHeight,
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
		SendMessage(hMessageTb, EM_LIMITTEXT, 1000, NULL);
		//SendMessage(hMessageTb, WM_SETTEXT, NULL, (LPARAM)L"Type message here...");

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

	void ResizeCH(HWND hWnd, int width, int height) {
		// compute history size
		int historyHeight = height - (2 * sideMargin + 2 * innerMargin + SendBtnHeight + MessageTbHeight);
		int innerWidth = width - 2 * sideMargin;
		MoveWindow(GetDlgItem(hWnd, IDC_CH_HISTORY), sideMargin, sideMargin, innerWidth, historyHeight, true);

		// compute message textbox position
		int msgTopOffset = sideMargin + historyHeight + innerMargin;
		MoveWindow(GetDlgItem(hWnd, IDC_CH_MESSAGE_TB), sideMargin, msgTopOffset, innerWidth, MessageTbHeight, true);

		// compute send button position
		int sendLeftOffset = width - (sideMargin + SendBtnWidth);
		int sendTopMargin = sideMargin + historyHeight + MessageTbHeight + 2 * innerMargin;
		MoveWindow(GetDlgItem(hWnd, IDC_CH_SEND_BUTTON), sendLeftOffset, sendTopMargin, SendBtnWidth, SendBtnHeight, true);
	}
}