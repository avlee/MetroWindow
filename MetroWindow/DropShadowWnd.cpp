#include "stdafx.h"
#include "DropShadowWnd.h"

namespace MetroWindow
{

static const TCHAR *kDropShadowWndClassName = TEXT("MetroShadowWnd");

namespace
{
    DropShadowBitmaps active_shadow_(RGB(0, 0, 0));
    DropShadowBitmaps inactive_shadow_(RGB(102, 102, 102));
}

CDropShadowWnd::CDropShadowWnd(void)
    : hWnd_(NULL)
{
}


CDropShadowWnd::~CDropShadowWnd(void)
{
}

bool CDropShadowWnd::RegisterWindowClass(HINSTANCE hInstance)
{
    // Register window class for shadow window
    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
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
    }
}

void CDropShadowWnd::Destroy()
{
    ASSERT(hWnd_ != NULL);
    ::DestroyWindow(hWnd_);
}

void CDropShadowWnd::ShowShadow(HWND hParentWnd, bool active)
{
    LONG lParentStyle = ::GetWindowLong(hParentWnd, GWL_STYLE);

    // Show shadow if parent is normal and visiable.
    if (WS_VISIBLE & lParentStyle && !((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle))
    {
        //TODO: Draw drop shadow if the parent window is child window and minimized
        ::ShowWindow(hWnd_, SW_SHOWNA);
        UpdateShadow(hParentWnd, active ? active_shadow_ : inactive_shadow_);
    }
    else
    {
        ::ShowWindow(hWnd_, SW_HIDE);
    }
}

void CDropShadowWnd::UpdateShadow(HWND hParentWnd, const DropShadowBitmaps& shadow)
{
    RECT rectParent;
    ::GetWindowRect(hParentWnd, &rectParent);

    int shadowSize = shadow.GetShadowSize();
    int shadowWndWidth = rectParent.right - rectParent.left + shadowSize * 2;
    int shadowWndHeight = rectParent.bottom - rectParent.top + shadowSize * 2;

    // Create the alpha blending bitmap
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = shadowWndWidth;
    bmi.bmiHeader.biHeight = shadowWndHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = shadowWndWidth * shadowWndHeight * 4;

    BYTE *pvBits;
    HBITMAP shadow_bitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

    HDC hMemDC = ::CreateCompatibleDC(NULL);
    HBITMAP hOriBmp = (HBITMAP)::SelectObject(hMemDC, shadow_bitmap);

    shadow.MakeShadow(hMemDC, shadowWndWidth, shadowWndHeight);

    POINT ptDst = {rectParent.left - shadowSize, rectParent.top - shadowSize};
    POINT ptSrc = {0, 0};
    SIZE WndSize = {shadowWndWidth, shadowWndHeight};
    BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    BOOL bRet= ::UpdateLayeredWindow(hWnd_, NULL, &ptDst, &WndSize, hMemDC,
        &ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

    ASSERT(bRet);

    ::SelectObject(hMemDC, hOriBmp);
    ::DeleteObject(shadow_bitmap);
    ::DeleteDC(hMemDC);
}

} //namespace MetroWindow
