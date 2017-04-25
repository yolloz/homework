#include "Main.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	if (!RegisterMainWindow(hInst)) {
		int nResult = GetLastError();
		MessageBox(NULL, L"Main Window class creation failed\r\nError code: " + nResult, L"Main Window Class Failed", MB_ICONERROR);
		return 1;
	}

	HWND hWnd = CreateMainWindow(hInst);
	if (!hWnd) {
		int nResult = GetLastError();
		MessageBox(NULL, L"Window creation failed\r\nError code:" + nResult, L"Window Creation Failed", MB_ICONERROR);
		return 1;
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