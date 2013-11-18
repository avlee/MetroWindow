#ifndef METROWINDOW_UXTHEMEAPI_H__
#define METROWINDOW_UXTHEMEAPI_H__

#pragma once

#include <Windows.h>
#include <Uxtheme.h>

namespace MetroWindow
{

namespace UxThemeApi
{
    void LoadUxThemeApi();

    void UnloadUxThemeApi(void);

    bool IsUxThemeSupported();

    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);

    HRESULT DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
        LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect,
        const DTTOPTS *pOptions);

    HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *pFont);

    HRESULT CloseThemeData(HTHEME hTheme);

    HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

    HPAINTBUFFER BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget,
        BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);

    HRESULT EndBufferedPaint(HPAINTBUFFER hBufferedPaint,BOOL fUpdateTarget);

    HRESULT BufferedPaintSetAlpha( HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);

} //namespace UxThemeApi

class CBufferedPaint
{
public:
    CBufferedPaint() : _hPaintBuffer(NULL)
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
        _hPaintBuffer = UxThemeApi::BeginBufferedPaint(hdcTarget, prcTarget,
            BPBF_TOPDOWNDIB, &_paintParams, phdcPaint);

        return (_hPaintBuffer != NULL && phdcPaint != NULL);
    }

    void EndPaint()
    {
        if(_hPaintBuffer != NULL)
        {
            // SetAlpha with specifies entire buffer
            UxThemeApi::BufferedPaintSetAlpha(_hPaintBuffer, NULL, 255);
            UxThemeApi::EndBufferedPaint(_hPaintBuffer, TRUE);
            _hPaintBuffer = NULL;
        }
    }

private:
    HPAINTBUFFER _hPaintBuffer;
    BP_PAINTPARAMS _paintParams;
};

} //namespace MetroWindow

#endif //METROWINDOW_UXTHEMEAPI_H__
