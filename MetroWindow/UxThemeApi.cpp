#include "UxThemeApi.h"

#include "WindowExtenders.h"

namespace MetroWindow
{

namespace UxThemeApi
{
    typedef HTHEME  (__stdcall *fnOpenThemeData)(HWND, LPCWSTR);
    typedef HRESULT (__stdcall *fnDrawThemeTextEx)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const DTTOPTS *);
    typedef HRESULT (__stdcall *fnGetThemeSysFont)(HTHEME, int, LOGFONT *);
    typedef HRESULT (__stdcall *fnCloseThemeData)(HTHEME);
    typedef HRESULT (__stdcall *fnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
    typedef HPAINTBUFFER (__stdcall *fnBeginBufferedPaint)(HDC, const RECT*, BP_BUFFERFORMAT, __in_opt BP_PAINTPARAMS *, __out HDC *);
    typedef HRESULT (__stdcall *fnBufferedPaintSetAlpha)(HPAINTBUFFER, __in_opt const RECT *, BYTE);
    typedef HRESULT (__stdcall *fnEndBufferedPaint)(HPAINTBUFFER, BOOL);
    typedef HRESULT (__stdcall *fnBufferedPaintInit)(VOID);
    typedef HRESULT (__stdcall *fnBufferedPaintUnInit)(VOID);

    HMODULE _hUxThemeDLL;
    fnOpenThemeData _pfnOpenThemeData;
    fnDrawThemeTextEx _pfnDrawThemeTextEx;
    fnGetThemeSysFont _pfnGetThemeSysFont;
    fnCloseThemeData _pfnCloseThemeData;
    fnSetWindowTheme _pfnSetWindowTheme;
    fnBeginBufferedPaint _pfnBeginBufferedPaint;
    fnBufferedPaintSetAlpha _pfnBufferedPaintSetAlpha;
    fnEndBufferedPaint _pfnEndBufferedPaint;
    fnBufferedPaintInit _pfnBufferedPaintInit;
    fnBufferedPaintUnInit _pfnBufferedPaintUnInit;

    void LoadUxThemeApi()
    {
        HMODULE _hUxThemeDLL = NULL;
        _pfnOpenThemeData = NULL;
        _pfnDrawThemeTextEx = NULL;
        _pfnGetThemeSysFont = NULL;
        _pfnCloseThemeData = NULL;
        _pfnSetWindowTheme = NULL;
        _pfnBeginBufferedPaint = NULL;
        _pfnBufferedPaintSetAlpha = NULL;
        _pfnEndBufferedPaint = NULL;
        _pfnBufferedPaintInit = NULL;
        _pfnBufferedPaintUnInit = NULL;

        if (WindowExtenders::IsVista())
        {
            _hUxThemeDLL = ::LoadLibraryW(L"uxtheme.dll");
            if (_hUxThemeDLL != NULL)
            {
                _pfnOpenThemeData = reinterpret_cast<fnOpenThemeData>(::GetProcAddress(_hUxThemeDLL, "OpenThemeData"));
                _pfnDrawThemeTextEx = reinterpret_cast<fnDrawThemeTextEx>(::GetProcAddress(_hUxThemeDLL, "DrawThemeTextEx"));
                _pfnGetThemeSysFont = reinterpret_cast<fnGetThemeSysFont>(::GetProcAddress(_hUxThemeDLL, "GetThemeSysFont"));
                _pfnCloseThemeData = reinterpret_cast<fnCloseThemeData>(::GetProcAddress(_hUxThemeDLL, "CloseThemeData"));
                _pfnSetWindowTheme = reinterpret_cast<fnSetWindowTheme>(::GetProcAddress(_hUxThemeDLL, "SetWindowTheme"));
                _pfnBeginBufferedPaint = reinterpret_cast<fnBeginBufferedPaint>(::GetProcAddress(_hUxThemeDLL, "BeginBufferedPaint"));
                _pfnBufferedPaintSetAlpha = reinterpret_cast<fnBufferedPaintSetAlpha>(::GetProcAddress(_hUxThemeDLL, "BufferedPaintSetAlpha"));
                _pfnEndBufferedPaint = reinterpret_cast<fnEndBufferedPaint>(::GetProcAddress(_hUxThemeDLL, "EndBufferedPaint"));
                _pfnBufferedPaintInit = reinterpret_cast<fnBufferedPaintInit>(::GetProcAddress(_hUxThemeDLL, "BufferedPaintInit"));
                _pfnBufferedPaintUnInit = reinterpret_cast<fnBufferedPaintUnInit>(::GetProcAddress(_hUxThemeDLL, "BufferedPaintUnInit"));

                if (_pfnBufferedPaintInit != NULL)
                {
                    _pfnBufferedPaintInit();
                }
            }
        }
    }

    void UnloadUxThemeApi(void)
    {
        if (_pfnBufferedPaintUnInit != NULL)
        {
            _pfnBufferedPaintUnInit();
        }

        _pfnOpenThemeData = NULL;
        _pfnDrawThemeTextEx = NULL;
        _pfnGetThemeSysFont = NULL;
        _pfnCloseThemeData = NULL;
        _pfnSetWindowTheme = NULL;
        _pfnBeginBufferedPaint = NULL;
        _pfnBufferedPaintSetAlpha = NULL;
        _pfnEndBufferedPaint = NULL;
        _pfnBufferedPaintInit = NULL;
        _pfnBufferedPaintUnInit = NULL;

        if (_hUxThemeDLL != NULL)
        {
            ::FreeLibrary(_hUxThemeDLL);
            _hUxThemeDLL = NULL;
        }
    }

    bool IsUxThemeSupported()
    {
        return (_hUxThemeDLL != NULL) && (_pfnDrawThemeTextEx != NULL);
    }

    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
    {
        if (_pfnOpenThemeData != NULL)
        {
            return _pfnOpenThemeData(hwnd, pszClassList);
        }
        return NULL;
    }

    HRESULT DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
        LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect,
        const DTTOPTS *pOptions)
    {
        if (_pfnDrawThemeTextEx != NULL)
        {
            return _pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
        }
        return E_NOTIMPL;
    }

    HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *pFont)
    {
        if (_pfnGetThemeSysFont != NULL)
        {
            return _pfnGetThemeSysFont(hTheme, iFontId, pFont);
        }

        return E_NOTIMPL;
    }

    HRESULT CloseThemeData(HTHEME hTheme)
    {
        if (_pfnCloseThemeData != NULL)
        {
            return _pfnCloseThemeData(hTheme);
        }

        return S_FALSE;
    }

    HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
    {
        if (_pfnSetWindowTheme != NULL)
        {
            return _pfnSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
        }

        return E_NOTIMPL;
    }

    HPAINTBUFFER BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget,
        BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc)
    {
        if (_pfnBeginBufferedPaint != NULL)
        {
            return _pfnBeginBufferedPaint(hdcTarget, prcTarget, dwFormat, pPaintParams, phdc);
        }

        return NULL;
    }

    HRESULT EndBufferedPaint(HPAINTBUFFER hBufferedPaint,BOOL fUpdateTarget)
    {
        if (_pfnEndBufferedPaint != NULL)
        {
            return _pfnEndBufferedPaint(hBufferedPaint, fUpdateTarget);
        }

        return E_NOTIMPL;
    }

    HRESULT BufferedPaintSetAlpha( HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha)
    {
        if (_pfnBufferedPaintSetAlpha != NULL)
        {
            return _pfnBufferedPaintSetAlpha(hBufferedPaint, prc, alpha);
        }

        return E_NOTIMPL;
    }

} //namespace UxThemeApi

} //namespace MetroWindow
