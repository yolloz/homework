#include "Main.h"

	int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
	{
		if (!RegisterConnectionWindow(hInst)) {
			int nResult = GetLastError();
			MessageBox(NULL, L"Window class creation failed\r\nError code: " + nResult, L"Window Class Failed", MB_ICONERROR);
			return 1;
		}
		HWND hWnd = CreateConnectionWindow(hInst);
		if (!hWnd) {
			int nResult = GetLastError();
			MessageBox(NULL, L"Window creation failed\r\nError code:" + nResult, L"Window Creation Failed", MB_ICONERROR);
			return 1;
		}
		ContextSingleton::GetInstance().connectionWindow = hWnd;
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
