#ifndef PLUSPLUSCHAT_MAIN_
#define PLUSPLUSCHAT_MAIN_

#pragma warning(disable: 4996)
#include <winsock2.h>
#include <windows.h>
#include <cstdint>

#pragma comment(lib,"ws2_32.lib")

#define IDC_PORTNUMBER 110

enum AppState
{
	LISTENING, STOPPED
};

AppState state = AppState::STOPPED;



void CreateLayout(HWND hWnd);
void StartListening(HWND hWnd, std::int_fast32_t port);

#endif