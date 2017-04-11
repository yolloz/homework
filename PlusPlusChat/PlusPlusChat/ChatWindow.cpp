#include "ChatWindow.h"


HWND hEditIn = NULL;
HWND hEditOut = NULL;
wchar_t szHistory[10000];

namespace PlusPlusChat {
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
				send(ContextSingleton::GetInstance().Socket, (char *)szBuffer, wcslen(szBuffer) * 2, 0);
				SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)L"");
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
				wchar_t szIncoming[1024];
				ZeroMemory(szIncoming, sizeof(szIncoming));

				int inDataLength = recv(ContextSingleton::GetInstance().Socket,
					(char*)szIncoming,
					sizeof(szIncoming) / sizeof(szIncoming[0]),
					0);

				wcsncat_s(szHistory, szIncoming, inDataLength);
				wcscat_s(szHistory, L"\r\n");

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
				closesocket(ContextSingleton::GetInstance().Socket);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			}
			break;
			}
		}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
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

	HWND CreateChatWindow(HINSTANCE hInstance) {
		HWND hWnd = CreateWindowEx(NULL,
			L"Chat Window",
			L"PlusPlusChat",
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

	bool ActivateChatWindow() {
		if (!ContextSingleton::GetInstance().chatWindow) {
			HINSTANCE hInst = GetModuleHandle(NULL);
			if (!RegisterChatWindow(hInst)) {
				//fail
				return false;
			}
			ContextSingleton::GetInstance().chatWindow = CreateChatWindow(hInst);
			if (!ContextSingleton::GetInstance().chatWindow) {
				//fail
				return false;
			}
		}

		
		ShowWindow(ContextSingleton::GetInstance().chatWindow, SW_SHOWDEFAULT);
		WSAAsyncSelect(ContextSingleton::GetInstance().Socket, ContextSingleton::GetInstance().chatWindow, WM_SOCKET, (FD_READ | FD_CLOSE));
		return true;
	}
}