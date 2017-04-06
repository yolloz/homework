#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(disable: 4996)

#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <atlstr.h>
#include <string>
#include <memory>
#include <stdlib.h>
//#include "ConnectionWindow.h"

#pragma comment(lib,"ws2_32.lib")

//using namespace PlusPlusChat;
#define IDC_CW_WINDOW			301
#define IDC_CW_CONNECTBTN		302
#define IDC_CW_IPEDIT			303
#define IDC_CW_PORTEDIT			304

enum AppState {
	DISCONNECTED, CONNECTED, CHATTING
};

AppState state = AppState::DISCONNECTED;

HWND CreateConnectionWindow(HINSTANCE hInstance);
LRESULT CALLBACK ConnectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM WINAPI RegisterConnectionWindow(HINSTANCE hInstance);
void CreateConnectionWindowLayout(HWND hWnd);

HWND CreateChatWindow(HINSTANCE hInstance);
ATOM WINAPI RegisterChatWindow(HINSTANCE hInstance);
LRESULT CALLBACK ChatWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool ConnectToServer(HWND hWnd, std::int_fast32_t port, char* ipAddress);
