#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSSERVER_MAINWINDOW_
#define PLUSPLUSSERVER_MAINWINDOW_

#define IDC_EDIT_IN			101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define IDC_PORTNUMBER		110
#define IDC_UPTIME_LBL		120
#define IDC_CONNECTED_LBL	121
#define IDC_CHATTING_LBL	122
#define IDC_PRIVATE_LBL		123
#define IDC_PUBLIC_LBL		124
#define IDC_ROOMS_LBL		125
#define IDC_SEND_BUTTON		153
#define IDC_PING_TIMER		179
#define IDC_UPTIME_TIMER	180
#define IDC_CACHESIZE_TB	181

#include <windows.h>
#include <ctime>
#include "Server.h"
#include "resource.h"
#include "../../PlusPlusChat_shared/PlusPlusHelpers.h"

HWND CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM WINAPI RegisterMainWindow(HINSTANCE hInstance);
void CreateMainWindowLayout(HWND hWnd);
void UpdateUI(HWND hWnd, Server::ServerDetails & details, std::wstring & uptime);
void StartServer(HWND hWnd);
void StopServer(HWND hWnd);

#endif

