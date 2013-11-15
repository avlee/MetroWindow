#include "stdafx.h"
#include "MetroFrame.h"
#include "WindowExtenders.h"
#include <Vssym32.h>

namespace MetroWindow
{

#ifndef TRACE
#ifdef _DEBUG
#include <stdio.h>
inline void TRACE(WCHAR const * const format, ...)
{
    va_list args;
    va_start(args, format);
    WCHAR output[512];
    vswprintf_s(output, format, args);
    OutputDebugStringW(output);
    va_end(args);
}
#else
#define TRACE __noop
#endif
#endif

CMetroFrame::CMetroFrame(HINSTANCE hInstance)
    : _hInst(hInstance)
    , _hWnd(NULL)
    , _hIcon(NULL)
    , _hSmallIcon(NULL)
    , _hCaptionFont(NULL)
{
    _isDwmEnabled = false;
    _isUxThemeSupported = false;
    _traceNCMouse = false;
    _isNonClientAreaActive = false;
    _isSizing = false;
    _prepareFullScreen = false;
    _isFullScreen = false;
    _captionHeight = 0;
    _useCustomTitle = false;
    _clientAreaMovable = false;
    _useThickFrame = false;
    _showIconOnCaption = true;
}


CMetroFrame::~CMetroFrame(void)
{
    _captionButtons.clear();

    if (_hCaptionFont) ::DeleteObject(_hCaptionFont);
}

void CMetroFrame::SetIcon(UINT nIconRes, UINT nSmallIconRes)
{
    if (_hIcon != NULL) ::DestroyIcon(_hIcon);
	_hIcon = (HICON)::LoadImage(
		_hInst, MAKEINTRESOURCE(nIconRes), IMAGE_ICON,
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
		_hInst, MAKEINTRESOURCE(nIconSmRes), IMAGE_ICON,
		::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	ASSERT(_hSmallIcon);
    if (_hWnd != NULL)
    {
	    ::SendMessage(_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) _hSmallIcon);
    }
}

void CMetroFrame::CenterWindow(HWND hWndCenter/* = NULL*/)
{
    ASSERT(::IsWindow(_hWnd));

    // determine owner window to center against
    DWORD dwStyle = ::GetWindowLong(_hWnd, GWL_STYLE);
    if(hWndCenter == NULL)
    {
        if(dwStyle & WS_CHILD)
            hWndCenter = ::GetParent(_hWnd);
        else
            hWndCenter = ::GetWindow(_hWnd, GW_OWNER);
    }

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    ::GetWindowRect(_hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if (!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if (hWndCenter != NULL)
        {
            DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
            if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = NULL;
        }

        // center within screen coordinates
        //::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

        // Support multi-monitor
        MONITORINFO oMonitor = {};
        oMonitor.cbSize = sizeof(oMonitor);
        ::GetMonitorInfo(::MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
        rcArea = oMonitor.rcWork;

        if (hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
        // center within parent client coordinates
        hWndParent = ::GetParent(_hWnd);
        ASSERT(::IsWindow(hWndParent));

        ::GetClientRect(hWndParent, &rcArea);
        ASSERT(::IsWindow(hWndCenter));
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

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

    ::SetWindowPos(_hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CMetroFrame::OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(_hWnd, uMsg, wParam, lParam);
}

LRESULT CMetroFrame::OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    BOOL bHandled = FALSE;

    if (_isDwmEnabled)
    {
        bHandled = _dwmApi.DwmDefWindowProc(GetHWnd(), uMsg, wParam, lParam, &lRes);
    }

    switch (uMsg)
    {
    case WM_STYLECHANGED:   this->RemoveWindowBorderStyle(); break;
    case WM_SETICON:
    case WM_SETTEXT:		lRes = OnSetText(uMsg, wParam, lParam, bHandled); break;
    case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
    case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
    case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
    case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
    case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
    case WM_NCLBUTTONDOWN:	lRes = OnNcLButtonDown(uMsg, wParam, lParam, bHandled); break;
    case WM_NCMOUSEMOVE:	lRes = OnNcMouseMove(uMsg, wParam, lParam, bHandled); break;
    case WM_NCLBUTTONUP:	lRes = OnNcLButtonUp(uMsg, wParam, lParam, bHandled); break;
    case WM_NCRBUTTONUP:	lRes = OnNcRButtonUp(uMsg, wParam, lParam, bHandled); break;
    case WM_LBUTTONDOWN:	lRes = OnWmLButtonDown(uMsg, wParam, lParam, bHandled); break;
    case WM_NCMOUSELEAVE:	lRes = OnNcMouseLeave(uMsg, wParam, lParam, bHandled); break;
    case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
    case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:        lRes = OnCommand(uMsg, wParam, lParam, bHandled); break;
    case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
    case WM_DWMCOMPOSITIONCHANGED: lRes = OnDwmCompositionChanged(uMsg, wParam, lParam, bHandled); break;
    default:				break;
    }
    if (bHandled) return lRes;

    return OnDefWndProc(uMsg, wParam, lParam);
}

LRESULT CMetroFrame::OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = 0;

    BOOL modified = ModifyWindowStyle(WS_VISIBLE, 0);

    // Setting the caption text and icon cause Windows to redraw the caption.
    // Letting the default WndProc handle the message without the WS_VISIBLE
    // style applied bypasses the redraw.
    lRes = OnDefWndProc(uMsg, wParam, lParam);

    // Put back the style we removed.
    if (modified)
    {
        ModifyWindowStyle(0, WS_VISIBLE);
    }

    PaintNonClientArea(NULL);

    bHandled = TRUE;
    return lRes;
}

LRESULT CMetroFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = TRUE;

    memset(&_trackMouseEvent, 0, sizeof(TRACKMOUSEEVENT));
    _trackMouseEvent.cbSize = sizeof( TRACKMOUSEEVENT );
    _trackMouseEvent.dwFlags = TME_NONCLIENT | TME_LEAVE;
    _trackMouseEvent.hwndTrack = GetHWnd();

    _isDwmEnabled = _dwmApi.IsDwmEnabled();
    _isUxThemeSupported = _uxThemeApi.IsUxThemeSupported();

    if (_isDwmEnabled)
    {
        _isDwmEnabled = _dwmApi.DwmAllowNcPaint(GetHWnd());
    }

    if (!_isDwmEnabled)
    {
        // Disable theming on current window so we don't get 
        // any funny artifacts (round corners, etc.)
        if (_isUxThemeSupported)
        {
            _uxThemeApi.SetWindowTheme(GetHWnd(), L"", L"");
        }
    }

    RemoveWindowBorderStyle();
    UpdateCaptionButtons();

    ::DisableProcessWindowsGhosting();

    return 0;
}

LRESULT CMetroFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Despite MSDN's documentation of lParam not being used,
    // calling DefWindowProc with lParam set to -1 causes Windows not to draw over the caption.
    OnDefWndProc(uMsg, wParam, -1);

    bool ncactive = (wParam != 0) ? true : false;
    if (ncactive != _isNonClientAreaActive)
    {
        if (ncactive)
        {
            ::SetWindowPos(GetHWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            ::SetForegroundWindow(GetHWnd());
        }
        _isNonClientAreaActive = ncactive;
        PaintNonClientArea(NULL);
    }

    bHandled = TRUE;
    return 1;
}

LRESULT CMetroFrame::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if ( wParam == TRUE)
    {
        CSize borderSize = WindowExtenders::GetBorderSize(GetHWnd(), _isDwmEnabled);

        LPNCCALCSIZE_PARAMS pncsp = (LPNCCALCSIZE_PARAMS)lParam;

        pncsp->rgrc[0].top = pncsp->rgrc[0].top + GetCaptionHeight() + borderSize.cy;
        pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - borderSize.cy;
        pncsp->rgrc[0].left = pncsp->rgrc[0].left + borderSize.cx;
        pncsp->rgrc[0].right = pncsp->rgrc[0].right - borderSize.cx;

        pncsp->rgrc[1] = pncsp->rgrc[0];

        bHandled = TRUE;
    }
    else if ( wParam == FALSE)
    {
        LPRECT pRect=(LPRECT)lParam;

        CSize borderSize = WindowExtenders::GetBorderSize(GetHWnd(), _isDwmEnabled);

        pRect->top += GetCaptionHeight() + borderSize.cy;
        pRect->bottom -= borderSize.cy;
        pRect->left += borderSize.cx;
        pRect->right -= borderSize.cx;

        bHandled = TRUE;
    }

    //if ( ::IsZoomed(GetHWnd()))
    //{	// 最大化时，计算当前显示器最适合宽高度
    //	MONITORINFO oMonitor = {};
    //	oMonitor.cbSize = sizeof(oMonitor);
    //	::GetMonitorInfo(::MonitorFromWindow(GetHWnd(), MONITOR_DEFAULTTONEAREST), &oMonitor);
    //	CRect rcWork = oMonitor.rcWork;
    //	CRect rcMonitor = oMonitor.rcMonitor;
    //	rcWork.OffsetRect(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

    //	pRect->right = pRect->left + rcWork.Width();
    //	pRect->bottom = pRect->top + rcWork.Height();
    //	return WVR_REDRAW;
    //}

    _isSizing = true;

    return 0;
}

LRESULT CMetroFrame::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (_prepareFullScreen || _isSizing || PaintNonClientArea((HRGN)wParam))
    {
        bHandled = TRUE;
    }
    return 1;
}

LRESULT CMetroFrame::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT point;
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);

    CRect rectScreen;
    ::GetWindowRect(GetHWnd(), &rectScreen);

    // custom processing
    if (rectScreen.PtInRect(point))
    {
        CRect rect = rectScreen;

        if (!_isFullScreen)
        {
            // let form handle hittest itself if we are on 1px borders
            rect.InflateRect(-1, -1);
            if (!rect.PtInRect(point))
            {
                if (!_isDwmEnabled || WindowExtenders::IsWindowSizable(GetHWnd()))
                {
                    bHandled = FALSE;
                    return 0;
                }
                else
                {
                    bHandled = TRUE;
                    return HTNOWHERE;
                }
            }
        }

        // on Button?
        CPoint pt(point.x - rectScreen.left, point.y - rectScreen.top);
        MetroRefPtr<CCaptionButton> sysButton = CommandButtonFromPoint(pt);
        if (sysButton.get() != NULL && sysButton->Enabled())
        {
            bHandled = TRUE;
            return sysButton->HitTest();
        }

        CSize borderSize = WindowExtenders::GetBorderSize(GetHWnd(), _isDwmEnabled);
        if (!_isFullScreen)
        {
            // on border?
            rect.InflateRect(-borderSize.cx + 1, -borderSize.cy + 1);

            // let form handle hittest itself if we are on borders
            if (!rect.PtInRect(point))
            {
                if (!_isDwmEnabled || WindowExtenders::IsWindowSizable(GetHWnd()))
                {
                    bHandled = FALSE;
                    return 0;
                }
                else
                {
                    bHandled = TRUE;
                    return HTNOWHERE;
                }
            }
        }

        CRect rectCaption = rect;
        rectCaption.bottom = rectCaption.top + GetCaptionHeight();

        // not in caption -> client
        if (!rectCaption.PtInRect(point))
        {
            bHandled = TRUE;
            return HTCLIENT;
        }

        // on icon?
        if (WindowExtenders::HasSysMenu(GetHWnd())/* && ShowIcon && Icon != null && ShowIconOnCaption*/)
        {
            CRect rectSysMenu = GetFrameIconBounds(rectScreen, borderSize);
            if (rectSysMenu.PtInRect(point))
            {
                bHandled = TRUE;
                return HTSYSMENU;
            }
        }

        // on Caption?
        bHandled = TRUE;
        return HTCAPTION;
    }

    bHandled = TRUE;
    return HTNOWHERE;
}

LRESULT CMetroFrame::OnNcLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    MetroRefPtr<CCaptionButton> button = CommandButtonByHitTest(wParam);
    if (_pressedButton.get() != button.get() && _pressedButton.get() != NULL)
        _pressedButton->Pressed(false);
    if (button.get() != NULL)
        button->Pressed(true);
    _pressedButton = button;
    if (_pressedButton.get() != NULL)
    {
        PaintNonClientArea(NULL);
    }

    if (_pressedButton.get() != NULL || _isFullScreen)
    {
        bHandled = TRUE;
    }
    return bHandled;
}

LRESULT CMetroFrame::OnNcMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!_traceNCMouse)
    {
        if (::TrackMouseEvent(&_trackMouseEvent))
        {
            _traceNCMouse = true;
        }
    }

    bool buttonStateChanged = false;

    // Check for hovered and pressed buttons
    if ((::GetKeyState(VK_LBUTTON) & 0x80) != 0x80)
    {
        if (_pressedButton.get() != NULL)
        {
            if (_pressedButton->Pressed())
            {
                _pressedButton->Pressed(false);
                buttonStateChanged = true;
            }
            _pressedButton = NULL;
        }
    }

    MetroRefPtr<CCaptionButton> button = CommandButtonByHitTest(wParam);
    if (_hoveredButton.get() != button.get() && _hoveredButton.get() != NULL)
    {
        if (_hoveredButton->Hovered())
        {
            _hoveredButton->Hovered(false);
            buttonStateChanged = true;
        }
    }

    if (_pressedButton.get() == NULL)
    {
        if (button.get() != NULL)
        {
            if (!button->Hovered())
            {
                button->Hovered(true);
                buttonStateChanged = true;
            }
        }
        _hoveredButton = button;
    }
    else
    {
        bool pressed = (button.get() == _pressedButton.get());
        if (pressed != _pressedButton->Pressed())
        {
            _pressedButton->Pressed(pressed);
            buttonStateChanged = true;
        }
    }

    if (buttonStateChanged)
    {
        PaintNonClientArea(NULL);
    }

    return 0;
}

LRESULT CMetroFrame::OnNcLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // do we have a pressed button?
    if (_pressedButton.get() != NULL)
    {
        // get button at wparam
        MetroRefPtr<CCaptionButton> button = CommandButtonByHitTest(wParam);
        if (button.get() == NULL)
            return 0;

        if (button->Pressed())
        {
            bHandled = TRUE;

            LONG hitTest = button->HitTest();
            switch (hitTest)
            {
            case HTCLOSE:
                ::PostMessage(_hWnd, WM_CLOSE, IDCANCEL, 0L);
                break;
            case HTMAXBUTTON:
                {
                    UINT cmd = ::IsZoomed(GetHWnd()) ? SC_RESTORE : SC_MAXIMIZE;
                    ::SendMessage(GetHWnd(), WM_SYSCOMMAND, cmd, 0);
                    break;
                }
            case HTMINBUTTON:
                {
                    UINT cmd = ::IsIconic(GetHWnd()) ? SC_RESTORE : SC_MINIMIZE;
                    ::SendMessage(GetHWnd(), WM_SYSCOMMAND, cmd, 0);
                    break;
                }
            default:
                break;
            }
        }

        _pressedButton->Pressed(false);
        _pressedButton->Hovered(false);
        _pressedButton = NULL;

        PaintNonClientArea(NULL);
    }

    return 0;
}

LRESULT CMetroFrame::OnNcRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    MetroRefPtr<CCaptionButton> button = CommandButtonByHitTest(wParam);
    if (button.get() != NULL && (button->HitTest() == HTCLOSE || button->HitTest() == HTMAXBUTTON))
        return 0;

    if (WindowExtenders::HasSysMenu(GetHWnd()))
    {
        POINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);
        CRect rectScreen;
        ::GetWindowRect(GetHWnd(), &rectScreen);
        CRect rectCaption = rectScreen;
        rectCaption.Height(GetCaptionHeight());

        // right click in caption
        if (rectCaption.PtInRect(point))
        {
            if (button.get() != NULL)
            {
                button->Hovered(false);

                PaintNonClientArea(NULL);
            }

            ShowSystemMenu(point);

            bHandled = TRUE;
            return HTSYSMENU;
        }
    }
    return 0;
}

LRESULT CMetroFrame::OnWmLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (_clientAreaMovable && !::IsZoomed(GetHWnd()))
    {
        ::ReleaseCapture();
        ::SendMessage(GetHWnd(), WM_NCLBUTTONDOWN, HTCAPTION, 0);

        bHandled = TRUE;
    }

    return 0;
}

LRESULT CMetroFrame::OnNcMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _traceNCMouse = false;

    bool buttonStateChanged = false;

    if (_pressedButton.get() != NULL)
    {
        _pressedButton->Pressed(false);
        _pressedButton = NULL;

        buttonStateChanged = true;
    }
    if (_hoveredButton.get() != NULL)
    {
        _hoveredButton->Hovered(false);
        _hoveredButton = NULL;

        buttonStateChanged = true;
    }

    if (buttonStateChanged)
    {
        PaintNonClientArea(NULL);
    }

    return 0;
}

LRESULT CMetroFrame::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(GetHWnd(), MONITOR_DEFAULTTONEAREST), &oMonitor);
    CRect rcWork = oMonitor.rcWork;
    CRect rcMonitor = oMonitor.rcMonitor;
    rcWork.OffsetRect(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

    // 计算最大化时，正确的原点坐标
    lpMMI->ptMaxPosition.x	= rcWork.left;
    lpMMI->ptMaxPosition.y	= rcWork.top;

    lpMMI->ptMaxTrackSize.x =rcWork.Width();
    lpMMI->ptMaxTrackSize.y =rcWork.Height();

    //lpMMI->ptMinTrackSize.x =m_PaintManager.GetMinInfo().cx;
    //lpMMI->ptMinTrackSize.y =m_PaintManager.GetMinInfo().cy;

    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //if (::IsIconic(GetHWnd())) bHandled = FALSE;

    if (wParam == SIZE_RESTORED)
    {
        _isFullScreen = false;
    }
    //else if (FullscreenMaximized && wParam == SIZE_MAXIMIZED)
    //{
    //    _isFullScreen = true;
    //}

    _prepareFullScreen = false;
    _isSizing = false;
    UpdateCaptionButtons();
    PaintNonClientArea(NULL);

    return 0;
}

LRESULT CMetroFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int sysCommand = wParam & 0xFFF0;
    if (sysCommand == SC_RESTORE)
    {
        // if the caption is not the standard, we must do this!
        //Height = _storeHeight;
        if (_isFullScreen)
        {
            _prepareFullScreen = true;
        }
    }
    else if (sysCommand == SC_MAXIMIZE)
    {
        _prepareFullScreen = true;
    }
    return 0;
}

LRESULT CMetroFrame::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CMetroFrame::OnDwmCompositionChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _isDwmEnabled = _dwmApi.IsDwmEnabled();
    PaintNonClientArea(NULL);

    return 0;
}

void CMetroFrame::RemoveWindowBorderStyle()
{
    // remove the border style
    DWORD dwStyle = ::GetWindowLong(GetHWnd(), GWL_STYLE);
    if ((dwStyle & WS_BORDER) != 0)
    {
        DWORD dwNewStyle = dwStyle & ~WS_BORDER;
        if (_isDwmEnabled)
        {
            dwNewStyle |= WS_THICKFRAME;
        }
        SetWindowLong(GetHWnd(), GWL_STYLE, dwNewStyle);

        SetWindowPos(GetHWnd(), NULL, 0, 0, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOREDRAW | SWP_NOACTIVATE);
    }
}

BOOL CMetroFrame::ModifyWindowStyle(LONG removeStyle, LONG addStyle)
{
    DWORD dwStyle = ::GetWindowLong(GetHWnd(), GWL_STYLE);
    DWORD dwNewStyle = (dwStyle & ~removeStyle) | addStyle;
    if (dwStyle == dwNewStyle)
    {
        return FALSE;
    }

    ::SetWindowLong(GetHWnd(), GWL_STYLE, dwNewStyle);
    return TRUE;
}

//TODO: Add HRGN paramter to improve draw speed.
BOOL CMetroFrame::PaintNonClientArea(HRGN hrgnUpdate)
{
    BOOL result = FALSE;

    // prepare paint bounds
    CSize borderSize = WindowExtenders::GetBorderSize(GetHWnd(), _isDwmEnabled);
    int captionHeight = this->GetCaptionHeight();

    CRect rectWindow;
    ::GetWindowRect(GetHWnd(), &rectWindow);

    //CRect dirty_region;
    //// A value of 1 indicates paint all.
    //if (!hrgnUpdate || hrgnUpdate == reinterpret_cast<HRGN>(1))
    //{
    //    dirty_region = CRect(0, 0, rectWindow.Width(), rectWindow.Height());
    //}
    //else
    //{
    //    RECT rgn_bounding_box;
    //    ::GetRgnBox(hrgnUpdate, &rgn_bounding_box);
    //    if (!::IntersectRect(&dirty_region, &rgn_bounding_box, &rectWindow))
    //        return TRUE;  // Dirty region doesn't intersect window bounds, bale.

    //    // rgn_bounding_box is in screen coordinates. Map it to window coordinates.
    //    dirty_region.OffsetRect(-rectWindow.left, -rectWindow.top);
    //}

    // Map it to window coordinates.
    rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);

    // create graphics handle
    HDC hdc = ::GetDCEx(GetHWnd(), NULL, DCX_CACHE | DCX_CLIPSIBLINGS | DCX_WINDOW);
    //HDC hdc = ::GetWindowDC(GetHWnd());
    if (hdc == NULL) return result;

    HRGN hrgn = NULL;

    if (!::IsIconic(GetHWnd()))
    {
        // prepare clipping
        CRect rectClip = rectWindow;

        int cx = borderSize.cx;
        int cy = borderSize.cy;
        DWORD dwExStyle = ::GetWindowLong(_hWnd, GWL_EXSTYLE);
        if (((dwExStyle & WS_EX_DLGMODALFRAME) != 0))
        {
            // The window has a double border
            cx *= 2;
            cy *= 2;
        }
        rectClip.InflateRect(-cx, -cy);
        rectClip.top += captionHeight;

        RECT rectBounds = rectWindow;

        //TODO: Apply clipping with the udpate region
        //If no need to update just return

        // Apply clipping
        hrgn = ::CreateRectRgnIndirect(&rectBounds);
        //HRGN hrgnClip = CreateRectRgnIndirect(&rectClip);
        //::CombineRgn(hrgn, hrgn, hrgnClip, RGN_XOR);
        ::SelectClipRgn(hdc, hrgn);
        ::ExcludeClipRect(hdc, rectClip.left, rectClip.top, rectClip.right, rectClip.bottom);
    }
    
    if (_isDwmEnabled && _isUxThemeSupported)
    {
        CBufferedPaint bufferedPaint(_uxThemeApi);
        HDC hdcPaint = NULL;
        if (bufferedPaint.BeginPaint(hdc, &rectWindow, &hdcPaint))
        {
            DrawWindowFrame(hdcPaint, rectWindow, borderSize, captionHeight);
            bufferedPaint.EndPaint();

            result = TRUE;
        }
    }
    else
    {
        HDC hdcPaint = ::CreateCompatibleDC(hdc);
        if (hdcPaint)
        {
            // TODO: Using bounds size to cache the buffer
            HBITMAP hbmPaint = ::CreateCompatibleBitmap(hdc, rectWindow.Width(), rectWindow.Height());
            if (hbmPaint)
            {
                HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcPaint, hbmPaint);
                ::SetViewportOrgEx(hdcPaint, -rectWindow.left, -rectWindow.top, NULL);

                // paint
                DrawWindowFrame(hdcPaint, rectWindow, borderSize, captionHeight);

                ::BitBlt(hdc, rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(),
                    hdcPaint, 0, 0, SRCCOPY);

                ::SelectObject(hdcPaint, hbmOld);
                ::DeleteObject(hbmPaint);

                result = TRUE;
            }

            ::DeleteDC(hdcPaint);
        }
    }

    // cleanup data
    if (hrgn != NULL)
    {
        ::SelectClipRgn(hdc, NULL);
        ::DeleteObject(hrgn);
    }

    ::ReleaseDC(GetHWnd(), hdc);

    return result;
}

void CMetroFrame::DrawWindowFrame(HDC hdc, const RECT& bounds, const SIZE& borderSize, int captionHeight)
{
    BOOL isMaxisized = ::IsZoomed(GetHWnd());

    // prepare bounds
    CRect windowBounds = bounds;

    CRect captionBounds = windowBounds;
    captionBounds.Height(borderSize.cy + captionHeight);

    CRect textBounds = captionBounds;
    
    COLORREF captionColor = (!_isNonClientAreaActive && !_isFullScreen) ?
        _captionTheme.InactiveCaptionColor() : _captionTheme.GetCaptionColor();

    // clear frame area
    COLORREF backColor = (_useThickFrame || isMaxisized)
        ? captionColor : RGB(255,255,255);

    FillSolidRect(hdc, &windowBounds, backColor);

    int frameBorderWidth = 1;

    // draw frame border
    if (!_isDwmEnabled && !isMaxisized && !_useThickFrame)
    {
        COLORREF borderColor = _isNonClientAreaActive ?
            _captionTheme.ActiveBorderColor() : _captionTheme.InactiveBorderColor();

        HPEN hPen = ::CreatePen(PS_SOLID | PS_INSIDEFRAME, frameBorderWidth, borderColor);
        HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, GetStockObject(NULL_BRUSH));
        ::Rectangle(hdc, windowBounds.left, windowBounds.top, windowBounds.right, windowBounds.bottom);
        ::SelectObject(hdc, hOldBrush);
        ::SelectObject(hdc, hOldPen);
        ::DeleteObject(hPen);
    }


    // Draw caption background
    //if (CaptionTheme.BackgroundImage != null)
    //{
    //    using (TextureBrush backgroundBrush = new TextureBrush(
    //        CaptionTheme.BackgroundImage, System.Drawing.Drawing2D.WrapMode.Tile))
    //    {
    //        g.FillRectangle(backgroundBrush, captionBounds);
    //    }
    //}
    //else
    {
        CRect fillRect = captionBounds;
        if (!_isDwmEnabled && !isMaxisized)
        {
            fillRect.InflateRect(-frameBorderWidth, -frameBorderWidth);
        }

        FillSolidRect(hdc, &fillRect, captionColor);
    }

    // Caculate caption icons size
    CRect iconBounds;
    if (WindowExtenders::HasSysMenu(GetHWnd()) && GetSmallIcon() != NULL && _showIconOnCaption)
    {
        iconBounds = GetFrameIconBounds(bounds, borderSize);

        textBounds.left = iconBounds.right + 1;
    }

    // Paint caption buttons
    std::vector<MetroRefPtr<CCaptionButton>>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = btnIter->get();
        if (button != NULL)
        {
            button->Draw(hdc);
            textBounds.right -= button->Bounds().Width();
        }
    }

    textBounds.right -= borderSize.cx + 1;

    // draw the default caption title text
    if (!_useCustomTitle)
    {
        WCHAR title[256];
        int titleLen = ::GetWindowTextW(GetHWnd(), title, 255);
        if (titleLen > 0 && !textBounds.IsRectNull() && !textBounds.IsRectEmpty())
        {
            COLORREF captionTextColor = (!_isNonClientAreaActive && !_isFullScreen) ?
                _captionTheme.InactiveCaptionTextColor() : _captionTheme.CaptionTextColor();
            if (_isDwmEnabled && _isUxThemeSupported)
            {
                DrawThemeCaptionTitleEx(hdc, title, textBounds, captionTextColor, captionColor);
            }
            else
            {
                // Draw text using GDI (Whidbey feature).
                DrawCaptionTitle(hdc, title, textBounds, captionTextColor);
            }
        }
    }

    CRect innerCaptionBounds = captionBounds;
    if (!_isDwmEnabled)
    {
        innerCaptionBounds.InflateRect(-frameBorderWidth, -frameBorderWidth);
    }

    // draw the custom caption elements.
    if (!::IsIconic(GetHWnd()))
    {
        /*foreach (ICaptionElement captionElement in _captionElements)
        {
        if (captionElement.Align == CaptionElementAlign.BottomCenter ||
        captionElement.Align == CaptionElementAlign.MiddleCenter ||
        captionElement.Align == CaptionElementAlign.TopCenter)
        {
        g.Clip = new Region(textBounds);

        captionElement.Draw(g, innerCaptionBounds);

        g.ResetClip();
        }
        else
        {
        captionElement.Draw(g, innerCaptionBounds);
        }
        }*/
    }

    // delay draw caption icon
    if (!iconBounds.IsRectNull())
    {
        ::DrawIconEx(hdc, iconBounds.left, iconBounds.top,
            GetSmallIcon(), iconBounds.Width(), iconBounds.Height(), 0, 0, DI_NORMAL);
    }

    // draw resize
    //if (WindowState == FormWindowState.Normal && 
    //    SizeGripStyle != SizeGripStyle.Hide &&
    //    IsFormSizable())
    //{
    //    g.DrawImage(CaptionTheme.SizeGrap, bounds.Width - 10, bounds.Height - 10);
    //}
}

void CMetroFrame::DrawCaptionTitle(HDC hdc, LPWSTR title, RECT bounds, COLORREF color)
{
    if (_hCaptionFont == NULL)
    {
        LOGFONT lf;
        ZeroMemory(&lf, sizeof(lf));
        lf.lfHeight         = 16;
        lf.lfWeight         = FW_BOLD;
        lf.lfCharSet        = DEFAULT_CHARSET;
        lf.lfQuality        = DEFAULT_QUALITY;
        lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wcscpy( lf.lfFaceName, L"Segoe UI");

		_hCaptionFont = ::CreateFontIndirect(&lf);
	}

    HFONT hOldFont = NULL;
    if (_hCaptionFont)
    {
        hOldFont = (HFONT)::SelectObject(hdc, _hCaptionFont);
    }

    int oldBkMode = ::SetBkMode(hdc, TRANSPARENT);
    COLORREF oldColor = ::SetTextColor(hdc, color);
    
    ::DrawText(hdc, title, -1, &bounds,
        DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOCLIP);

    ::SetTextColor(hdc, oldColor);
    ::SetBkMode(hdc, oldBkMode);

    if (hOldFont) ::SelectObject(hdc, hOldFont);
}

void CMetroFrame::DrawThemeCaptionTitleEx(HDC hdc, LPCWSTR title, const RECT& bounds, COLORREF color, COLORREF bgColor)
{
    //TODO: Cache the theme?
    HTHEME hTheme = _uxThemeApi.OpenThemeData(NULL, L"CompositedWindow::Window");
    if (hTheme)
    {
        HDC hdcPaint = ::CreateCompatibleDC(hdc);
        if (hdcPaint)
        {
            int width = bounds.right - bounds.left;
            int height = bounds.bottom - bounds.top;

            // Define the BITMAPINFO structure used to draw text.
            // Note that biHeight is negative. This is done because
            // DrawThemeTextEx() needs the bitmap to be in top-to-bottom
            // order.
            BITMAPINFO dib = { 0 };
            dib.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
            dib.bmiHeader.biWidth           = width;
            dib.bmiHeader.biHeight          = -height;
            dib.bmiHeader.biPlanes          = 1;
            dib.bmiHeader.biBitCount        = 32;
            dib.bmiHeader.biCompression     = BI_RGB;

            HBITMAP hbm = ::CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
            if (hbm)
            {
                HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcPaint, hbm);

                // Create font
                if (_hCaptionFont == NULL)
                {
                    LOGFONT lgFont;
                    if (SUCCEEDED(_uxThemeApi.GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                    {
                        _hCaptionFont = ::CreateFontIndirect(&lgFont);
                    }
                }

                // Select a font.
                HFONT hFontOld = NULL;
                if (_hCaptionFont)
                {
                    hFontOld = (HFONT) SelectObject(hdcPaint, _hCaptionFont);
                }

                // Draw the title.
                CRect rcPaint(0, 0, width, height);

                FillSolidRect(hdcPaint, &rcPaint, bgColor);

                // Setup the theme drawing options.
                DTTOPTS dttOpts = {sizeof(DTTOPTS)};
                dttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR;
                dttOpts.crText = color;
                dttOpts.iGlowSize = 12;

                _uxThemeApi.DrawThemeTextEx(hTheme, hdcPaint, 0, 0, title, -1,
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX,
                    &rcPaint, &dttOpts);

                // Blit text to the frame.
                ::BitBlt(hdc, bounds.left, bounds.top, width, height, hdcPaint, 0, 0, SRCCOPY);

                ::SelectObject(hdcPaint, hbmOld);
                if (hFontOld)
                {
                    ::SelectObject(hdcPaint, hFontOld);
                }
                ::DeleteObject(hbm);
            }
            ::DeleteDC(hdcPaint);
        }

        _uxThemeApi.CloseThemeData(hTheme);
    }
}

void CMetroFrame::UpdateCaptionButtons()
{
    // create buttons
    if (_captionButtons.size() == 0)
    {
        MetroRefPtr<CCaptionButton> closeButton = new CCaptionButton(HTCLOSE, _captionTheme);
        closeButton->Image(_captionTheme.CloseButton());
        _captionButtons.push_back(closeButton);

        if (WindowExtenders::IsDrawMaximizeBox(GetHWnd()))
        {
            _maxButton = new CCaptionButton(HTMAXBUTTON, _captionTheme);
            _maxButton->Image(::IsZoomed(GetHWnd()) ? 
                _captionTheme.RestoreButton() : _captionTheme.MaximizeButton());
            _captionButtons.push_back(_maxButton);
        }

        if (WindowExtenders::IsDrawMinimizeBox(GetHWnd()))
        {
            _minButton = new CCaptionButton(HTMINBUTTON, _captionTheme);
            _minButton->Image(::IsIconic(GetHWnd()) ? 
                _captionTheme.RestoreButton() : _captionTheme.MinimizeButton());
            _captionButtons.push_back(_minButton);
        }

        // add command handlers
        //foreach (CaptionButton button in _captionButtons)
        //    button.PropertyChanged += OnCommandButtonPropertyChanged;
    }
    else
    {
        if (_minButton.get() != NULL)
        {
            _minButton->Image(::IsIconic(GetHWnd()) ? 
                _captionTheme.RestoreButton() : _captionTheme.MinimizeButton());
        }

        if (_maxButton.get() != NULL)
        {
            _maxButton->Image(::IsZoomed(GetHWnd()) ? 
                _captionTheme.RestoreButton() : _captionTheme.MaximizeButton());
        }
    }

    // Calculate Caption Button Bounds
    CRect rect;
    ::GetWindowRect(GetHWnd(), &rect);
    rect.OffsetRect(-rect.left, -rect.top);

    CSize borderSize = WindowExtenders::GetBorderSize(GetHWnd(), _isDwmEnabled);
    CSize captionButtonSize = WindowExtenders::GetCaptionButtonSize(GetHWnd());

    CRect buttonRect;
    buttonRect.left = rect.right - borderSize.cx - captionButtonSize.cx;
    buttonRect.top = rect.top;
    buttonRect.Width(captionButtonSize.cx);
    buttonRect.Height(captionButtonSize.cy);

    // Do not overlap the frame border
    if (!_isDwmEnabled/* && CaptionTheme.BackgroundImage == null*/)
    {
        buttonRect.InflateRect(0, -1);
    }

    std::vector<MetroRefPtr<CCaptionButton>>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = btnIter->get();
        if (button != NULL && button->Visible())
        {
            button->Bounds(buttonRect);
            buttonRect.MoveToX(buttonRect.left - captionButtonSize.cx);
        }
    }
}

int CMetroFrame::GetCaptionHeight()
{
    if (_captionHeight == 0)
    {
        _captionHeight = WindowExtenders::GetCaptionHeight(GetHWnd());

        /*if (CaptionElements.Count > 0)
        {
        int defaultHeight = _captionHeight;
        int height = _captionHeight;

        foreach (ICaptionElement captionElement in CaptionElements)
        {
        if (captionElement.Align == CaptionElementAlign.BottomLeft ||
        captionElement.Align == CaptionElementAlign.BottomRight)
        {
        height = Math.Max(height, defaultHeight + captionElement.Height);
        }
        else
        {
        height = Math.Max(height, captionElement.Height);
        }
        }

        _captionHeight = Math.Max(height, defaultHeight);
        }*/
    }

    return _captionHeight;
}

RECT CMetroFrame::GetFrameIconBounds(const RECT& windowBounds, SIZE borderSize)
{
    int captionHeight = WindowExtenders::GetCaptionHeight(GetHWnd());
    CSize iconSize = WindowExtenders::GetSmallIconSize();

    CRect iconBounds;
    iconBounds.left = windowBounds.left + borderSize.cx;
    iconBounds.top = windowBounds.top + (borderSize.cy + captionHeight - iconSize.cy) / 2;
    iconBounds.Width(iconSize.cx);
    iconBounds.Height(iconSize.cy);

    return iconBounds;
}

void CMetroFrame::FillSolidRect(HDC hdc, LPCRECT lpRect, COLORREF clr)
{
    ASSERT(hdc != NULL);

    COLORREF clrOld = ::SetBkColor(hdc, clr);
    ASSERT(clrOld != CLR_INVALID);
    if(clrOld != CLR_INVALID)
    {
        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
        ::SetBkColor(hdc, clrOld);
    }

    //HBRUSH hBrush = ::CreateSolidBrush(clr);
    //::FillRect(hdc, lpRect, hBrush);
    //::DeleteObject(hBrush);
}

MetroRefPtr<CCaptionButton> CMetroFrame::CommandButtonFromPoint(POINT point)
{
    MetroRefPtr<CCaptionButton> foundButton;

    std::vector<MetroRefPtr<CCaptionButton>>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = btnIter->get();
        if (button != NULL && button->Visible() && button->Bounds().PtInRect(point))
        {
            foundButton = *btnIter;
        }
    }

    return foundButton;
}

MetroRefPtr<CCaptionButton> CMetroFrame::CommandButtonByHitTest(LONG hitTest)
{
    MetroRefPtr<CCaptionButton> foundButton;

    std::vector<MetroRefPtr<CCaptionButton>>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = btnIter->get();
        if (button != NULL && button->Visible() && button->HitTest() == hitTest)
        {
            foundButton = *btnIter;
        }
    }

    return foundButton;
}

void CMetroFrame::ShowSystemMenu(POINT point)
{
    HWND hWnd = GetHWnd();
    HMENU hmenu = ::GetSystemMenu(hWnd, FALSE);
    if (hmenu != NULL)
    {
        UINT cmd = ::TrackPopupMenuEx(hmenu, TPM_LEFTBUTTON | TPM_RETURNCMD, (int)point.x, (int)point.y, hWnd, 0);
        if (0 != cmd)
        {
            ::PostMessage(hWnd, WM_SYSCOMMAND, cmd, 0);
        }
    }
}

} //namespace MetroWindow