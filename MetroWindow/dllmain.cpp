// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "MetroCaptionTheme.h"
#include "UxThemeApi.h"
#include "DwmApi.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MetroWindow::UxThemeApi::LoadUxThemeApi();
        MetroWindow::DwmApi::LoadDwmApi();
        MetroWindow::CMetroCaptionTheme::LoadBitmapFromResource(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        MetroWindow::CMetroCaptionTheme::FreeResources();
        MetroWindow::DwmApi::UnloadDwmApi();
        MetroWindow::UxThemeApi::UnloadUxThemeApi();
        break;
    }
    return TRUE;
}

