#pragma once

#include <Windows.h>

namespace MetroWindow
{

namespace DwmApi
{
    void LoadDwmApi();

    void UnloadDwmApi(void);

    bool IsDwmEnabled();

    bool DwmAllowNcPaint(HWND hwnd);

    BOOL DwmDefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

    HRESULT DwmExtendFrameIntoClientArea(HWND hWnd);
    
} //namespace DwmApi

} //namespace MetroWindow
