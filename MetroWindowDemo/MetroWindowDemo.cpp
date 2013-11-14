// MetroWindowDemo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MetroWindowDemo.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_METROWINDOWDEMO, szWindowClass, MAX_LOADSTRING);

	CMainWindow mainWindow(hInstance);
    mainWindow.SetIcon(IDI_METROWINDOWDEMO, IDI_SMALL);
    mainWindow.ClientAreaMovable(true);
    mainWindow.Create(NULL, szTitle, WS_OVERLAPPEDWINDOW, NULL);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_METROWINDOWDEMO));

    mainWindow.ShowWindow();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

