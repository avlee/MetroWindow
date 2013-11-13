#include "MetroWindow.h"

namespace MetroWindow
{

CMetroWindow::CMetroWindow(HINSTANCE hInstance)
	: CMetroFrame(hInstance)
{
}

CMetroWindow::~CMetroWindow(void)
{
	if (_hSmallIcon != NULL)
    {
        ::DestroyIcon(_hSmallIcon);
        _hSmallIcon = NULL;
    }

    if (_hIcon != NULL)
    {
        ::DestroyIcon(_hIcon);
        _hIcon = NULL;
    }
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

	_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle,
		x, y, cx, cy, hwndParent, hMenu, GetModuleInstance(), this);

	ASSERT(_hWnd!=NULL);
	return _hWnd;
}

void CMetroWindow::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
	ASSERT(::IsWindow(_hWnd));
	if (::IsWindow(_hWnd))
    {
	    ::ShowWindow(_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
    }
}

void CMetroWindow::CenterWindow()
{
	ASSERT(::IsWindow(_hWnd));
	ASSERT((GetWindowStyle(_hWnd)&WS_CHILD)==0);
	RECT rcDlg = { 0 };
	::GetWindowRect(_hWnd, &rcDlg);
	RECT rcArea = { 0 };
	RECT rcCenter = { 0 };
	HWND hWnd=*this;
	HWND hWndParent = ::GetParent(_hWnd);
	HWND hWndCenter = ::GetWindowOwner(_hWnd);
	if (hWndCenter != NULL)
		hWnd = hWndCenter;

	// 处理多显示器模式下屏幕居中
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
	rcArea = oMonitor.rcWork;

	if (hWndCenter == NULL)
		rcCenter = rcArea;
	else
		::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// Find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// The dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;

	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;

	::SetWindowPos(_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

bool CMetroWindow::RegisterWindowClass()
{
	WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = GetClassStyle();
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hIcon          = _hIcon;
    wcex.hIconSm        = _hSmallIcon;
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
		pThis->_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	}
	else
    {
		pThis = reinterpret_cast<CMetroWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (uMsg == WM_NCDESTROY && pThis != NULL)
        {
			LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->_hWnd, GWLP_USERDATA, 0L);
			pThis->_hWnd = NULL;
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

};
