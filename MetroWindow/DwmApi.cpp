#include "stdafx.h"
#include "DwmApi.h"
#include "WindowExtenders.h"

namespace MetroWindow
{

namespace DwmApi
{
    typedef HRESULT (__stdcall *fnDwmIsCompositionEnabled)(BOOL *pfEnabled);
    typedef BOOL (__stdcall *fnDwmDefWindowProc)(__in HWND hwnd,
        UINT msg, WPARAM wParam, LPARAM lParam, __out LRESULT *plResult);
    typedef HRESULT (__stdcall *fnDwmSetWindowAttribute)(HWND hwnd,
        DWORD dwAttribute, __out_bcount(cbAttribute) LPCVOID pvAttribute, DWORD cbAttribute);
    typedef HRESULT (__stdcall *fnDwmExtendFrameIntoClientArea)(HWND hWnd, __in const MARGINS* pMarInset);

    // Window attributes
    typedef enum _DWMWINDOWATTRIBUTE
    {
        DWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
        DWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
        DWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
        DWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
        DWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
        DWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
        DWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
        DWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
        DWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
        DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
        DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
        DWMWA_EXCLUDED_FROM_PEEK,           // [set] LivePreview exclusion information
        DWMWA_LAST
    } DWMWINDOWATTRIBUTE;

    HMODULE dwm_dll_;
    fnDwmIsCompositionEnabled pfnDwmIsCompositionEnabled;
    fnDwmSetWindowAttribute pfnDwmSetWindowAttribute;
    fnDwmDefWindowProc pfnDwmDefWindowProc;
    fnDwmExtendFrameIntoClientArea pfnDwmExtendFrameIntoClientArea;

    void LoadDwmApi()
    {
        dwm_dll_ = NULL;
        pfnDwmIsCompositionEnabled = NULL;
        pfnDwmSetWindowAttribute = NULL;
        pfnDwmDefWindowProc = NULL;
        pfnDwmExtendFrameIntoClientArea = NULL;

        if (WindowExtenders::IsVista())
        {
            dwm_dll_ = ::LoadLibraryW(L"dwmapi.dll");
            if (dwm_dll_ != NULL)
            {
                pfnDwmIsCompositionEnabled = reinterpret_cast<fnDwmIsCompositionEnabled>(::GetProcAddress(
                    dwm_dll_, "DwmIsCompositionEnabled"));

                pfnDwmSetWindowAttribute = reinterpret_cast<fnDwmSetWindowAttribute>(::GetProcAddress(
                    dwm_dll_, "DwmSetWindowAttribute"));

                pfnDwmDefWindowProc = reinterpret_cast<fnDwmDefWindowProc>(::GetProcAddress(
                    dwm_dll_, "DwmDefWindowProc"));

                pfnDwmExtendFrameIntoClientArea = reinterpret_cast<fnDwmExtendFrameIntoClientArea>(::GetProcAddress(
                    dwm_dll_, "DwmExtendFrameIntoClientArea"));


            }
        }
    }

    void UnloadDwmApi(void)
    {
        pfnDwmIsCompositionEnabled = NULL;
        pfnDwmSetWindowAttribute = NULL;
        pfnDwmDefWindowProc = NULL;
        pfnDwmExtendFrameIntoClientArea = NULL;

        if (dwm_dll_ != NULL)
        {
            ::FreeLibrary(dwm_dll_);
            dwm_dll_ = NULL;
        }
    }

    bool IsDwmEnabled()
    {
        if (pfnDwmIsCompositionEnabled != NULL)
        {
            BOOL bEnabled = FALSE;
            HRESULT hr = pfnDwmIsCompositionEnabled(&bEnabled);
            if (SUCCEEDED(hr) && bEnabled)
            {
                return true;
            }
        }

        return false;
    }

    bool DwmAllowNcPaint(HWND hwnd)
    {
        if (pfnDwmSetWindowAttribute != NULL)
        {
            BOOL bEnable = TRUE;

            // Enable non-client area rendering on the window.
            HRESULT hr = pfnDwmSetWindowAttribute(hwnd,
                DWMWA_ALLOW_NCPAINT, &bEnable, sizeof(bEnable));
            if (SUCCEEDED(hr))
            {
                return true;
            }
        }
 
        return false;
    }

    BOOL DwmDefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
    {
        BOOL bRet = FALSE;
        if (pfnDwmDefWindowProc != NULL)
        {
            bRet = pfnDwmDefWindowProc(hwnd, msg, wParam, lParam, plResult);
        }

        return bRet;
    }

    HRESULT DwmExtendFrameIntoClientArea(HWND hWnd)
    {
        if (pfnDwmExtendFrameIntoClientArea != NULL)
        {
            MARGINS margins = { 0 };
            return pfnDwmExtendFrameIntoClientArea(hWnd, &margins);
        }

        return E_NOTIMPL;
    }
    
} //namespace DwmApi

} //namespace MetroWindow
