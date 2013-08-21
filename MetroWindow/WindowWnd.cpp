#include "stdafx.h"
#include "WindowWnd.h"

namespace MetroWindow
{

CWindowWnd::CWindowWnd(HINSTANCE hInstance)
	: _hInst(hInstance)
    , _hWnd(NULL)
    , _hIcon(NULL)
    , _hSmallIcon(NULL)
{
}

CWindowWnd::~CWindowWnd(void)
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

UINT CWindowWnd::GetClassStyle() const
{
	return CS_HREDRAW | CS_VREDRAW;
}

HWND CWindowWnd::Create(
	HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,
	const RECT rc, HMENU hMenu)
{
	return Create(hwndParent, pstrName, dwStyle, dwExStyle,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
}

HWND CWindowWnd::Create(
	HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HMENU hMenu)
{
	if (!RegisterWindowClass()) return NULL;

	_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle,
		x, y, cx, cy, hwndParent, hMenu, GetModuleInstance(), this);

	ASSERT(_hWnd!=NULL);
	return _hWnd;
}

void CWindowWnd::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
	ASSERT(::IsWindow(_hWnd));
	if (!::IsWindow(_hWnd)) return;
	::ShowWindow(_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

UINT CWindowWnd::ShowModal()
{
    ASSERT(::IsWindow(_hWnd));
    UINT nRet = 0;
    HWND hWndParent = GetWindowOwner(_hWnd);

    ::ShowWindow(_hWnd, SW_SHOWNORMAL);
    ::EnableWindow(hWndParent, FALSE);

    MSG msg = { 0 };
    while (::IsWindow(_hWnd) && ::GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_CLOSE && msg.hwnd == _hWnd)
        {
            nRet = msg.wParam;
            ::EnableWindow(hWndParent, TRUE);
            ::SetFocus(hWndParent);
        }

        if (::TranslateMessage(&msg))
        {
            ::DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) break;
    }

    ::EnableWindow(hWndParent, TRUE);
    ::SetFocus(hWndParent);

    if (msg.message == WM_QUIT) ::PostQuitMessage(msg.wParam);

    return nRet;
}

void CWindowWnd::Close(UINT nRet)
{
	ASSERT(::IsWindow(_hWnd));
	if (!::IsWindow(_hWnd)) return;
	::PostMessage(_hWnd, WM_CLOSE, (WPARAM)nRet, 0L);
}

void CWindowWnd::CenterWindow()
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

void CWindowWnd::SetIcon(UINT nIconRes, UINT nSmallIconRes)
{
    if (_hIcon != NULL) ::DestroyIcon(_hIcon);
	_hIcon = (HICON)::LoadImage(
		GetModuleInstance(), MAKEINTRESOURCE(nIconRes), IMAGE_ICON,
		::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON),
		LR_DEFAULTCOLOR);
	ASSERT(_hIcon);
    if (_hWnd != NULL)
    {
	    ::SendMessage(_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) _hIcon);
    }

    if (_hSmallIcon != NULL) ::DestroyIcon(_hSmallIcon);

    UINT nIconSmRes = (nSmallIconRes > 0) ? nSmallIconRes : nIconRes;
	_hSmallIcon = (HICON)::LoadImage(
		GetModuleInstance(), MAKEINTRESOURCE(nIconSmRes), IMAGE_ICON,
		::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	ASSERT(_hSmallIcon);
    if (_hWnd != NULL)
    {
	    ::SendMessage(_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) _hSmallIcon);
    }
}

bool CWindowWnd::RegisterWindowClass()
{
	WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = GetClassStyle();
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hIcon          = _hIcon;
    wcex.hIconSm        = _hSmallIcon;
	wcex.lpfnWndProc    = CWindowWnd::__WndProc;
	wcex.hInstance      = GetModuleInstance();
	wcex.hCursor        = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = GetWindowClassName();

	ATOM ret = ::RegisterClassEx(&wcex);

	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK CWindowWnd::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowWnd* pThis = NULL;

	if (uMsg == WM_NCCREATE)
    {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
		pThis->_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	}
	else
    {
		pThis = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

LRESULT CWindowWnd::OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc(_hWnd, uMsg, wParam, lParam);
}

};
