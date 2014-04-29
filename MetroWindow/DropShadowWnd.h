#pragma once

#include "MetroCaptionTheme.h"
#include "DropShadowBitmaps.h"

namespace MetroWindow
{

class CDropShadowWnd
{
public:
    CDropShadowWnd(ShadowSide side);
    ~CDropShadowWnd(void);

    void Create(HINSTANCE hInstance, HWND hParentWnd);
    void Destroy();
    void HideShadow();
    void UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow, bool force);
    void CalculateBounds(RECT rectParent, int shadowSize);

private:
    bool RegisterWindowClass(HINSTANCE hInstance);
    HBITMAP CreateBitmap(int width, int height);

private:
    ShadowSide side_;
    HWND hWnd_;
    HBITMAP shadow_image_;
    int width_;
    int height_;
    RECT bounds_;
};

class CDropShadow
{
public:
    CDropShadow(void);
    ~CDropShadow(void);

    void Create(HINSTANCE hInstance, HWND hParentWnd);
    void Destroy();
    void ShowShadow(HWND hParentWnd, bool active);

private:
    void UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow, bool force);
    void HideShadow();

private:
    bool active_;
    CDropShadowWnd* shadow_wnds_[4];
};

} //namespace MetroWindow
