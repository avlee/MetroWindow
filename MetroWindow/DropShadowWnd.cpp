#include "stdafx.h"
#include "DropShadowWnd.h"

namespace MetroWindow
{

static const TCHAR *kDropShadowWndClassName = TEXT("MetroShadowWnd");

namespace
{
    DropShadowBitmaps active_shadow_(RGB(0, 0, 0));
    DropShadowBitmaps inactive_shadow_(RGB(102, 102, 102));

    LRESULT CALLBACK DropShadowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Avoid hide drop shadow before the minimize animation of the owner window.
        if (uMsg == WM_SHOWWINDOW)
            return 0;
        else
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

} // namespace

CDropShadowWnd::CDropShadowWnd(ShadowSide side)
    : side_(side), hWnd_(NULL), shadow_image_(NULL), width_(0), height_(0)
{
}


CDropShadowWnd::~CDropShadowWnd(void)
{
    if (shadow_image_ != NULL)
    {
        ::DeleteObject(shadow_image_);
        shadow_image_ = NULL;
    }

    if (hWnd_ != NULL)
    {
        ::DestroyWindow(hWnd_);
    }
}

bool CDropShadowWnd::RegisterWindowClass(HINSTANCE hInstance)
{
    // Register window class for shadow window
    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DropShadowWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = kDropShadowWndClassName;
    wcex.hIconSm        = NULL;

    ATOM ret = ::RegisterClassEx(&wcex);

    if (ret != NULL)
    {
        // Initialize at here to reduce lock.
        active_shadow_.Initialize();
        inactive_shadow_.Initialize();
    }

    ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void CDropShadowWnd::Create(HINSTANCE hInstance, HWND hParentWnd)
{
    if (RegisterWindowClass(hInstance))
    {
        hWnd_ = ::CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
            kDropShadowWndClassName,
            NULL,
            WS_POPUP, CW_USEDEFAULT,
            0, 0, 0, hParentWnd, NULL, hInstance, NULL);

        ASSERT(hWnd_ != NULL);

        // Disable message handler of drop shadow window,
        // this will avoid break the modal dialog lost modal behavior.
        ::EnableWindow(hWnd_, FALSE);
    }
}

void CDropShadowWnd::Destroy()
{
    ASSERT(hWnd_ != NULL);
    ::DestroyWindow(hWnd_);
    hWnd_ = NULL;
}

void CDropShadowWnd::HideShadow()
{
    ::ShowWindow(hWnd_, SW_HIDE);
}

HBITMAP CDropShadowWnd::CreateBitmap(int width, int height)
{
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * 4;

    BYTE *pvBits;
    return ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);
}

void CDropShadowWnd::UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow, bool force)
{
    int width = bounds_.right - bounds_.left;
    int height = bounds_.bottom - bounds_.top;

    HDC hScreenDC = ::GetDC(NULL);
    HDC hMemDC = ::CreateCompatibleDC(hScreenDC);
    ::ReleaseDC(NULL, hScreenDC);

    HBITMAP hMemBmp = CreateBitmap(width, height);
    ::SelectObject(hMemDC, hMemBmp);

    HDC hPaintDC = ::CreateCompatibleDC(hMemDC);

    if (width != width_ || height != height_ || force || shadow_image_ == NULL)
    {
        width_ = width;
        height_ = height;

        if (shadow_image_ != NULL)
        {
            ::DeleteObject(shadow_image_);
            shadow_image_ = NULL;
        }

        shadow_image_ = CreateBitmap(width, height);
        ::SelectObject(hPaintDC, shadow_image_);

        shadow.MakeShadow(hPaintDC, width, height, side_);
    }
    else
    {
        ::SelectObject(hPaintDC, shadow_image_);
    }

    ::BitBlt(hMemDC, 0, 0, width, height, hPaintDC, 0, 0, SRCCOPY);

    ::DeleteDC(hPaintDC);

    POINT ptDst = {bounds_.left, bounds_.top};
    POINT ptSrc = {0, 0};
    SIZE wndSize = {width, height};
    BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    ::ShowWindow(hWnd_, SW_SHOWNOACTIVATE);

    ::UpdateLayeredWindow(hWnd_, NULL, &ptDst, &wndSize, hMemDC,
        &ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

    // Delete the Memory DC, this will release its hold on the memory bitmap.
    ::DeleteDC(hMemDC);
    ::DeleteObject(hMemBmp);
}

void CDropShadowWnd::CalculateBounds(RECT rectParent, int shadowSize)
{
    switch (side_)
    {
        case Left:
            {
                bounds_.left = rectParent.left - shadowSize;
                bounds_.top = rectParent.top;
                bounds_.right = rectParent.left;
                bounds_.bottom = rectParent.bottom;
            }
            break;

        case Top:
            {
                bounds_.left = rectParent.left - shadowSize;
                bounds_.top = rectParent.top - shadowSize;
                bounds_.right = rectParent.right + shadowSize;
                bounds_.bottom = rectParent.top;
            }
            break;

        case Right:
            {
                bounds_.left = rectParent.right;
                bounds_.top = rectParent.top;
                bounds_.right = rectParent.right + shadowSize;
                bounds_.bottom = rectParent.bottom;
            }
            break;

        case Bottom:
            {
                bounds_.left = rectParent.left - shadowSize;
                bounds_.top = rectParent.bottom;
                bounds_.right = rectParent.right + shadowSize;
                bounds_.bottom = rectParent.bottom + shadowSize;
            }
            break;
        default:
            {
                bounds_.left = 0;
                bounds_.top = 0;
                bounds_.right = 0;
                bounds_.bottom = 0;
            }
            break;
    }
}

CDropShadow::CDropShadow(void)
    : active_(false)
{
    shadow_wnds_[0] = new CDropShadowWnd(Left);
    shadow_wnds_[1] = new CDropShadowWnd(Top);
    shadow_wnds_[2] = new CDropShadowWnd(Right);
    shadow_wnds_[3] = new CDropShadowWnd(Bottom);
}

CDropShadow::~CDropShadow(void)
{
    for (int i = 0; i < 4; ++i)
    {
        delete shadow_wnds_[i];
    }
}

void CDropShadow::Create(HINSTANCE hInstance, HWND hParentWnd)
{
    for (int i = 0; i < 4; ++i)
    {
        shadow_wnds_[i]->Create(hInstance, hParentWnd);
    }
}

void CDropShadow::Destroy()
{
    for (int i = 0; i < 4; ++i)
    {
        shadow_wnds_[i]->Destroy();
    }
}

void CDropShadow::ShowShadow(HWND hParentWnd, bool active)
{
    bool force = false;
    if (active != active_)
    {
        active_ = active;
        force = true;
    }

    LONG lParentStyle = ::GetWindowLong(hParentWnd, GWL_STYLE);

    if (WS_VISIBLE & lParentStyle && !(WS_MAXIMIZE & lParentStyle))
    {
        if (WS_MINIMIZE & lParentStyle)
        {
            HWND hParentOwner = ::GetWindow(hParentWnd, GW_OWNER);
            if (hParentOwner != NULL)
            {
                // Show drop shadow if the parent window has owner and minimized.
                UpdateShadow(hParentWnd, inactive_shadow_, force);
            }
            else
            {
                HideShadow();
            }
        }
        else
        {
            // Show drop shadow if parent is normal and visiable.
            UpdateShadow(hParentWnd, active ? active_shadow_ : inactive_shadow_, force);
        }
    }
    else
    {
        HideShadow();
    }
}

void CDropShadow::UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow, bool force)
{
    RECT rectParent;
    ::GetWindowRect(hParentWnd, &rectParent);

    int shadowSize = shadow.GetShadowSize();

    for (int i = 0; i < 4; ++i)
    {
        shadow_wnds_[i]->CalculateBounds(rectParent, shadowSize);
        shadow_wnds_[i]->UpdateShadow(hParentWnd, shadow, force);
    }
}

void CDropShadow::HideShadow()
{
    for (int i = 0; i < 4; ++i)
    {
        shadow_wnds_[i]->HideShadow();
    }
}

} //namespace MetroWindow
