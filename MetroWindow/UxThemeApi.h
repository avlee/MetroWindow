#ifndef METROWINDOW_UXTHEMEAPI_H__
#define METROWINDOW_UXTHEMEAPI_H__

#pragma once

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
    CBufferedPaint() : paint_buffer_(NULL)
    {
        memset(&paint_params_, 0, sizeof(BP_PAINTPARAMS));
        paint_params_.cbSize = sizeof(BP_PAINTPARAMS);
        paint_params_.dwFlags = BPPF_NONCLIENT;
    }

    ~CBufferedPaint()
    {
        EndPaint();
    }

    bool BeginPaint(HDC hdcTarget, const RECT* prcTarget, HDC* phdcPaint)
    {
        paint_buffer_ = UxThemeApi::BeginBufferedPaint(hdcTarget, prcTarget,
            BPBF_TOPDOWNDIB, &paint_params_, phdcPaint);

        return (paint_buffer_ != NULL && phdcPaint != NULL);
    }

    void EndPaint()
    {
        if(paint_buffer_ != NULL)
        {
            // SetAlpha with specifies entire buffer
            UxThemeApi::BufferedPaintSetAlpha(paint_buffer_, NULL, 255);
            UxThemeApi::EndBufferedPaint(paint_buffer_, TRUE);
            paint_buffer_ = NULL;
        }
    }

private:
    HPAINTBUFFER paint_buffer_;
    BP_PAINTPARAMS paint_params_;
};

} //namespace MetroWindow

#endif //METROWINDOW_UXTHEMEAPI_H__
