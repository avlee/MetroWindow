#ifndef METROWINDOW_UXTHEMEAPI_H__
#define METROWINDOW_UXTHEMEAPI_H__

#pragma once

#include <Windows.h>
#include <Uxtheme.h>
#include "WindowExtenders.h"

namespace MetroWindow
{

class CUxThemeApi
{
    typedef HTHEME  (__stdcall *fnOpenThemeData)(HWND, LPCWSTR);
    typedef HRESULT (__stdcall *fnDrawThemeText)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
    typedef HRESULT (__stdcall *fnDrawThemeTextEx)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const DTTOPTS *);
    typedef HRESULT (__stdcall *fnGetThemeSysFont)(HTHEME, int, LOGFONT *);
    typedef HRESULT (__stdcall *fnCloseThemeData)(HTHEME);
    typedef HRESULT (__stdcall *fnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
    typedef HPAINTBUFFER (__stdcall *fnBeginBufferedPaint)(HDC, const RECT*, BP_BUFFERFORMAT, __in_opt BP_PAINTPARAMS *, __out HDC *);
    typedef HRESULT (__stdcall *fnBufferedPaintSetAlpha)(HPAINTBUFFER, __in_opt const RECT *, BYTE);
    typedef HRESULT (__stdcall *fnEndBufferedPaint)(HPAINTBUFFER, BOOL);
    typedef HRESULT (__stdcall *fnBufferedPaintInit)(VOID);
    typedef HRESULT (__stdcall *fnBufferedPaintUnInit)(VOID);

public:

    CUxThemeApi(void)
        : _bDllLoaded(false)
        , _hUxThemeDLL(NULL)
        , _pfnOpenThemeData(NULL)
        , _pfnDrawThemeText(NULL)
        , _pfnDrawThemeTextEx(NULL)
        , _pfnGetThemeSysFont(NULL)
        , _pfnCloseThemeData(NULL)
        , _pfnSetWindowTheme(NULL)
        , _pfnBeginBufferedPaint(NULL)
        , _pfnBufferedPaintSetAlpha(NULL)
        , _pfnEndBufferedPaint(NULL)
        , _pfnBufferedPaintInit(NULL)
        , _pfnBufferedPaintUnInit(NULL)
    {
    }

    ~CUxThemeApi(void)
    {
        if (_pfnBufferedPaintUnInit != NULL)
        {
            _pfnBufferedPaintUnInit();
        }

        _pfnOpenThemeData = NULL;
        _pfnDrawThemeText = NULL;
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
        LoadUxThemeApis();

        return (_hUxThemeDLL != NULL) && (_pfnDrawThemeText != NULL);
    }

    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
    {
        LoadUxThemeApis();

        if (_pfnOpenThemeData != NULL)
        {
            return _pfnOpenThemeData(hwnd, pszClassList);
        }
        return NULL;
    }

    HRESULT DrawThemeText(HTHEME hTheme,
        HDC hdc,
        int iPartId,
        int iStateId,
        LPCWSTR pszText,
        int cchText,
        DWORD dwTextFlags,
        DWORD dwTextFlags2,
        LPCRECT pRect)
    {
        if (_pfnDrawThemeText != NULL)
        {
            return _pfnDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
        }
        return E_NOTIMPL;
    }

    HRESULT DrawThemeTextEx(HTHEME hTheme,
        HDC hdc,
        int iPartId,
        int iStateId,
        LPCWSTR pszText,
        int cchText,
        DWORD dwTextFlags,
        LPRECT pRect,
        const DTTOPTS *pOptions)
    {
        if (_pfnDrawThemeTextEx != NULL)
        {
            return _pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
        }
        return E_NOTIMPL;
    }

    HRESULT GetThemeSysFont(HTHEME hTheme,
        int iFontId,
        LOGFONTW *pFont)
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

private:
    void LoadUxThemeApis()
    {
        if (!_bDllLoaded)
        {
            if (WindowExtenders::IsVista())
            {
                _hUxThemeDLL = ::LoadLibraryA("uxtheme.dll");
                if (_hUxThemeDLL != NULL)
                {
                    _pfnOpenThemeData = (fnOpenThemeData)::GetProcAddress(_hUxThemeDLL, "OpenThemeData");
                    _pfnDrawThemeText = (fnDrawThemeText)::GetProcAddress(_hUxThemeDLL, "DrawThemeText");
			        _pfnDrawThemeTextEx = (fnDrawThemeTextEx)::GetProcAddress(_hUxThemeDLL, "DrawThemeTextEx");
			        _pfnGetThemeSysFont = (fnGetThemeSysFont)::GetProcAddress(_hUxThemeDLL, "GetThemeSysFont");
			        _pfnCloseThemeData = (fnCloseThemeData)::GetProcAddress(_hUxThemeDLL, "CloseThemeData");
                    _pfnSetWindowTheme = (fnSetWindowTheme)::GetProcAddress(_hUxThemeDLL, "SetWindowTheme");
                    _pfnBeginBufferedPaint = (fnBeginBufferedPaint)::GetProcAddress(_hUxThemeDLL, "BeginBufferedPaint");
                    _pfnBufferedPaintSetAlpha = (fnBufferedPaintSetAlpha)::GetProcAddress(_hUxThemeDLL, "BufferedPaintSetAlpha");
                    _pfnEndBufferedPaint = (fnEndBufferedPaint)::GetProcAddress(_hUxThemeDLL, "EndBufferedPaint");
                    _pfnBufferedPaintInit = (fnBufferedPaintInit)::GetProcAddress(_hUxThemeDLL, "BufferedPaintInit");
                    _pfnBufferedPaintUnInit = (fnBufferedPaintUnInit)::GetProcAddress(_hUxThemeDLL, "BufferedPaintUnInit");

                    if (_pfnBufferedPaintInit != NULL)
                    {
                        _pfnBufferedPaintInit();
                    }
                }
            }

            _bDllLoaded = true;
        }

    }

private:
    bool _bDllLoaded;
    HMODULE _hUxThemeDLL;
    fnOpenThemeData _pfnOpenThemeData;
    fnDrawThemeText _pfnDrawThemeText;
    fnDrawThemeTextEx _pfnDrawThemeTextEx;
    fnGetThemeSysFont _pfnGetThemeSysFont;
    fnCloseThemeData _pfnCloseThemeData;
    fnSetWindowTheme _pfnSetWindowTheme;
    fnBeginBufferedPaint _pfnBeginBufferedPaint;
    fnBufferedPaintSetAlpha _pfnBufferedPaintSetAlpha;
    fnEndBufferedPaint _pfnEndBufferedPaint;
    fnBufferedPaintInit _pfnBufferedPaintInit;
    fnBufferedPaintUnInit _pfnBufferedPaintUnInit;
};

class CBufferedPaint
{
public:
    CBufferedPaint(CUxThemeApi& uxThemeApi) 
        : _uxThemeApi(uxThemeApi), _hPaintBuffer(NULL)
    {
        memset(&_paintParams, 0, sizeof(BP_PAINTPARAMS));
		_paintParams.cbSize = sizeof(BP_PAINTPARAMS);
        _paintParams.dwFlags = BPPF_NONCLIENT;
    }

    ~CBufferedPaint()
    {
        EndPaint();
    }

    bool BeginPaint(HDC hdcTarget, const RECT* prcTarget, HDC* phdcPaint)
    {
        _hPaintBuffer = _uxThemeApi.BeginBufferedPaint(hdcTarget, prcTarget,
            BPBF_TOPDOWNDIB, &_paintParams, phdcPaint);

        return (_hPaintBuffer != NULL && phdcPaint != NULL);
    }

    void EndPaint()
    {
        if(_hPaintBuffer != NULL)
		{
            // SetAlpha with specifies entire buffer
            _uxThemeApi.BufferedPaintSetAlpha(_hPaintBuffer, NULL, 255);
			_uxThemeApi.EndBufferedPaint(_hPaintBuffer, TRUE);
			_hPaintBuffer = NULL;
		}
    }

private:
    CUxThemeApi& _uxThemeApi;
    HPAINTBUFFER _hPaintBuffer;
    BP_PAINTPARAMS _paintParams;
};

}; //namespace MetroWindow

#endif //METROWINDOW_UXTHEMEAPI_H__
