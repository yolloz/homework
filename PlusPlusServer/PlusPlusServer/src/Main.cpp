#include "Main.h"

#pragma warning(disable: 4996)
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

int nPort = 5555;

HWND hEditIn = NULL;
HWND hEditOut = NULL;
wchar_t szHistory[10000];
sockaddr sockAddrClient;

const int nMaxClients = 3;
int nClient = 0;
SOCKET Socket[nMaxClients - 1];
SOCKET ServerSocket = NULL;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wClass;
	ZeroMemory(&wClass, sizeof(WNDCLASSEX));
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = NULL;
	wClass.hIconSm = NULL;
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

	HWND hWnd = CreateWindowEx(NULL,
		L"Window Class",
		L"Winsock Async Server",
		WS_OVERLAPPEDWINDOW,
		200,
		200,
		640,
		480,
		NULL,
		NULL,
		hInst,
		NULL);

	if (!hWnd)
	{
		int nResult = GetLastError();

		MessageBox(NULL,
			L"Window creation failed\r\nError code:",
			L"Window Creation Failed",
			MB_ICONERROR);
	}

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

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MAIN_BUTTON:
		{
			wchar_t szBuffer[1024];
			ZeroMemory(szBuffer, sizeof(szBuffer));

			SendMessage(hEditOut,
				WM_GETTEXT,
				sizeof(szBuffer),
				reinterpret_cast<LPARAM>(szBuffer));
			for (int n = 0; n <= nClient; n++)
			{
				send(Socket[n], (char*)szBuffer, wcslen(szBuffer)*2, 0);
			}

			SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)"");
		}
		break;
		}
		break;
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
			(LPARAM)L"Waiting for client to connect...");

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
			WM_SETFONT,
			(WPARAM)hfDefault,
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

		WSADATA WsaDat;
		int nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
		if (nResult != 0)
		{
			MessageBox(hWnd,
				L"Winsock initialization failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			break;
		}

		ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ServerSocket == INVALID_SOCKET)
		{
			MessageBox(hWnd,
				L"Socket creation failed",
				L"Critical Error",
				MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			break;
		}

		SOCKADDR_IN SockAddr;
		SockAddr.sin_port = htons(nPort);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		//SockAddr.sin_addr.s_addr = inet_addr(INADDR_ANY);

		if (bind(ServerSocket, (LPSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
		{
			MessageBox(hWnd, L"Unable to bind socket", L"Error", MB_OK);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			break;
		}

		nResult = WSAAsyncSelect(ServerSocket,
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
			break;
		}

		if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			MessageBox(hWnd,
				L"Unable to listen!",
				L"Error",
				MB_OK);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			break;
		}
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(ServerSocket, SD_BOTH);
		closesocket(ServerSocket);
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
			if (nClient<nMaxClients)
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
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable: 4127)  // Conditional expression is a constant

#define DATA_BUFSIZE 4096

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	WSADATA wsd;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	WSAOVERLAPPED RecvOverlapped;
	SOCKET ConnSocket = INVALID_SOCKET;
	WSABUF DataBuf;
	DWORD RecvBytes, Flags;
	char buffer[DATA_BUFSIZE];

	int err = 0;
	int rc;

	// Load Winsock
	rc = WSAStartup(MAKEWORD(2, 2), &wsd);
	if (rc != 0) {
		wprintf(L"Unable to load Winsock: %d\n", rc);
		return 1;
	}
	// Make sure the hints struct is zeroed out
	SecureZeroMemory((PVOID)& hints, sizeof(struct addrinfo));

	// Initialize the hints to retrieve the server address for IPv4
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;



		ConnSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ConnSocket == INVALID_SOCKET) {
			wprintf(L"socket failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			return 1;
		}

		rc = connect(ConnSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (rc == SOCKET_ERROR) {

			if (WSAECONNREFUSED == (err = WSAGetLastError())) {
				closesocket(ConnSocket);
				ConnSocket = INVALID_SOCKET;
				continue;
			}
			wprintf(L"connect failed with error: %d\n", err);
			freeaddrinfo(result);
			closesocket(ConnSocket);
			return 1;
		}
		break;
	
	if (ConnSocket == INVALID_SOCKET) {
		wprintf(L"Unable to establish connection with the server!\n");
		freeaddrinfo(result);
		return 1;
	}

	wprintf(L"Client connected...\n");

	// Make sure the RecvOverlapped struct is zeroed out
	SecureZeroMemory((PVOID)& RecvOverlapped, sizeof(WSAOVERLAPPED));

	// Create an event handle and setup an overlapped structure.
	RecvOverlapped.hEvent = WSACreateEvent();
	if (RecvOverlapped.hEvent == NULL) {
		wprintf(L"WSACreateEvent failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ConnSocket);
		return 1;
	}

	DataBuf.len = DATA_BUFSIZE;
	DataBuf.buf = buffer;

	// Call WSARecv until the peer closes the connection
	// or until an error occurs
	while (1) {

		Flags = 0;
		rc = WSARecv(ConnSocket, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
		if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
			wprintf(L"WSARecv failed with error: %d\n", err);
			break;
		}

		rc = WSAWaitForMultipleEvents(1, &RecvOverlapped.hEvent, TRUE, INFINITE, TRUE);
		if (rc == WSA_WAIT_FAILED) {
			wprintf(L"WSAWaitForMultipleEvents failed with error: %d\n", WSAGetLastError());
			break;
		}

		rc = WSAGetOverlappedResult(ConnSocket, &RecvOverlapped, &RecvBytes, FALSE, &Flags);
		if (rc == FALSE) {
			wprintf(L"WSARecv operation failed with error: %d\n", WSAGetLastError());
			break;
		}

		wprintf(L"Read %d bytes\n", RecvBytes);

		WSAResetEvent(RecvOverlapped.hEvent);

		// If 0 bytes are received, the connection was closed
		if (RecvBytes == 0)
			break;
	}

	WSACloseEvent(RecvOverlapped.hEvent);
	closesocket(ConnSocket);
	freeaddrinfo(result);

	WSACleanup();

	return 0;
}*/