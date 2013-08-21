// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "MetroCaptionTheme.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        MetroWindow::CMetroCaptionTheme::LoadBitmapFromResource(hModule);
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        MetroWindow::CMetroCaptionTheme::FreeResources();
		break;
	}
	return TRUE;
}

