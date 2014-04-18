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
    HBITMAP CreateBitmap(int width, int height);
    void UpdateShadow(HWND hParentWnd, DropShadowBitmaps& shadow, bool force);

private:
    HWND hWnd_;
    HBITMAP shadow_image_;
    int width_;
    int height_;
    bool active_;
};

} //namespace MetroWindow
