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
    void Destroy();
    void ShowShadow(HWND hParentWnd, bool active);

private:
    bool RegisterWindowClass(HINSTANCE hInstance);
    void UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow);

private:
    HWND hWnd_;
    DropShadowBitmaps active_shadow_;
    DropShadowBitmaps inactive_shadow_;
};

} //namespace MetroWindow
