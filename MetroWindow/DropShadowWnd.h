#pragma once

#include "MetroCaptionTheme.h"
#include "DropShadowBitmaps.h"

namespace MetroWindow
{

class CDropShadowWnd
{
public:
    CDropShadowWnd(void);
    ~CDropShadowWnd(void);

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

} //namespace MetroWindow
