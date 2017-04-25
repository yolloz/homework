#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSSERVER_MAINWINDOW_
#define PLUSPLUSSERVER_MAINWINDOW_

#define IDC_EDIT_IN			101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define IDC_PORTNUMBER		110
#define IDC_SEND_BUTTON		153
#define IDC_PING_TIMER		179

#include <windows.h>
#include "Server.h"
#include "resource.h"

HWND CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM WINAPI RegisterMainWindow(HINSTANCE hInstance);
void CreateMainWindowLayout(HWND hWnd);

#endif

