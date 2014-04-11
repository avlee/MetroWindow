#pragma once

#include "MetroCaptionTheme.h"
#include "DropShadowBitmaps.h"

class METROWINDOW_DECL DropShadowWnd
{
public:
    DropShadowWnd(void);
    ~DropShadowWnd(void);

    void Create(HINSTANCE hInstance, HWND hParentWnd);

    LRESULT OnParentWndProc(HWND hParentWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    bool RegisterWindowClass(HINSTANCE hInstance);
    void UpdateShadow(HWND hParentWnd);
    void ShowShadow(HWND hParentWnd);

private:
    HWND hWnd_;
    DropShadowBitmaps shadow_;
};

