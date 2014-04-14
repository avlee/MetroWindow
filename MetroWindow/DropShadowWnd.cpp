#include "stdafx.h"
#include "DropShadowWnd.h"
#include "math.h"

static const TCHAR *kDropShadowWndClassName = TEXT("MetroShadowWnd");

DropShadowWnd::DropShadowWnd(void)
    : hWnd_(NULL)
{
}


DropShadowWnd::~DropShadowWnd(void)
{
}

bool DropShadowWnd::RegisterWindowClass(HINSTANCE hInstance)
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

    ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
    return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void DropShadowWnd::Create(HINSTANCE hInstance, HWND hParentWnd)
{
    if (RegisterWindowClass(hInstance))
    {
        shadow_.Initialize();

        hWnd_ = ::CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
            kDropShadowWndClassName,
            NULL,
            WS_POPUP, CW_USEDEFAULT,
            0, 0, 0, hParentWnd, NULL, hInstance, NULL);

        ASSERT(hWnd_ != NULL);
    }
}

LRESULT DropShadowWnd::OnParentWndProc(HWND hParentWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    switch(uMsg) {
    case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS *pwp = (WINDOWPOS *)lParam;
            if (pwp->flags & SWP_SHOWWINDOW || pwp->flags & SWP_HIDEWINDOW ||
                !(pwp->flags & SWP_NOMOVE) || !(pwp->flags & SWP_NOSIZE))
            {
                ShowShadow(hParentWnd);
            }
            break;
        }
    //case WM_NCACTIVATE:
    //    ShowShadow(hParentWnd);
    //    break;
    case WM_DESTROY:
        ::DestroyWindow(hWnd_);
        break;
    }

    bHandled = FALSE;
    return 0;
}

void DropShadowWnd::UpdateShadow(HWND hParentWnd)
{
    RECT rectParent;
	::GetWindowRect(hParentWnd, &rectParent);

    int shadowSize = shadow_.GetShadowSize();
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

	ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);

	HDC hMemDC = ::CreateCompatibleDC(NULL);
	HBITMAP hOriBmp = (HBITMAP)::SelectObject(hMemDC, shadow_bitmap);

    shadow_.MakeShadow(hMemDC, shadowWndWidth, shadowWndHeight);

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

void DropShadowWnd::ShowShadow(HWND hParentWnd)
{
    LONG lParentStyle = ::GetWindowLong(hParentWnd, GWL_STYLE);

    // Show shadow if parent is normal and visiable.
    if (WS_VISIBLE & lParentStyle && !((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle))
    {
        ::ShowWindow(hWnd_, SW_SHOWNA);
        UpdateShadow(hParentWnd);
    }
    else
    {
        ::ShowWindow(hWnd_, SW_HIDE);
    }
}
