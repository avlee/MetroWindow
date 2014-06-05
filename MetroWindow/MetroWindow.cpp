#include "stdafx.h"
#include "MetroWindow.h"

namespace MetroWindow
{

CMetroWindow::CMetroWindow(HINSTANCE hInstance)
    : CMetroFrame(hInstance)
{
}

CMetroWindow::~CMetroWindow(void)
{
}

LPCTSTR CMetroWindow::GetWindowClassName() const
{
    return TEXT("WC_MetroWindow");
}

UINT CMetroWindow::GetClassStyle() const
{
    return CS_HREDRAW | CS_VREDRAW;
}

HWND CMetroWindow::Create(
    HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,
    const RECT& rc, HMENU hMenu)
{
    return Create(hwndParent, pstrName, dwStyle, dwExStyle,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
}

HWND CMetroWindow::Create(
    HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,
    int x, int y, int cx, int cy, HMENU hMenu)
{
    if (!RegisterWindowClass()) return NULL;

    hWnd_ = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle,
        x, y, cx, cy, hwndParent, hMenu, GetModuleInstance(), this);

    ASSERT(hWnd_!=NULL);
    return hWnd_;
}

void CMetroWindow::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= true*/)
{
    ASSERT(::IsWindow(hWnd_));
    if (::IsWindow(hWnd_))
    {
        ::ShowWindow(hWnd_, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
        if (bTakeFocus)
        {
            ::SetFocus(hWnd_);
        }
    }
}

void CMetroWindow::ShowDialog(HWND hWndParent)
{
    ::EnableWindow(hWndParent, FALSE);

    ShowWindow();

    bool loop = true;

    MSG msg;

    while (loop && ::GetMessage(&msg, NULL, 0, 0))
	{
        if (msg.message == WM_CLOSE) loop = false;
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

    ::EnableWindow(hWndParent, TRUE);
    ::SetForegroundWindow(hWndParent);
}

bool CMetroWindow::RegisterWindowClass()
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = GetClassStyle();
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hIcon          = hIcon_;
    wcex.hIconSm        = hIcon_small_;
    wcex.lpfnWndProc    = CMetroWindow::__WndProc;
    wcex.hInstance      = GetModuleInstance();
    wcex.hCursor        = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = GetWindowClassName();

    ATOM ret = ::RegisterClassEx(&wcex);

    ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
    return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK CMetroWindow::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMetroWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<CMetroWindow*>(lpcs->lpCreateParams);
        pThis->hWnd_ = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CMetroWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (uMsg == WM_NCDESTROY && pThis != NULL)
        {
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->hWnd_, GWLP_USERDATA, 0L);
            pThis->hWnd_ = NULL;
            return lRes;
        }
    }

    if (pThis != NULL)
    {
        return pThis->OnWndProc(uMsg, wParam, lParam);
    }
    else
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

} // namespace MetroWindow
