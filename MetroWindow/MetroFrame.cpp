#include "stdafx.h"
#include "MetroFrame.h"
#include "WindowExtenders.h"
#include "DwmApi.h"
#include "CaptionButton.h"
#include "DropShadowWnd.h"
#include "UxThemeApi.h"
#include <Vssym32.h>

namespace
{
enum OSVersion {
  VERSION_PRE_XP = 0,  // Not supported.
  VERSION_XP,
  VERSION_SERVER_2003, // Also includes XP Pro x64 and Server 2003 R2.
  VERSION_VISTA,       // Also includes Windows Server 2008.
  VERSION_WIN7,        // Also includes Windows Server 2008 R2.
  VERSION_WIN8,        // Also includes Windows Server 2012.
  VERSION_WIN8_1,      // Code named Windows Blue
  VERSION_WIN_LAST,    // Indicates error condition.
};

OSVersion os_version_;

OSVersion GetOSVersion()
{
    if (os_version_ == VERSION_PRE_XP)
    {
        OSVERSIONINFO version_info = { sizeof version_info };
        ::GetVersionEx(&version_info);

        if ((version_info.dwMajorVersion == 5) && (version_info.dwMinorVersion > 0))
        {
            // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
            os_version_ = (version_info.dwMinorVersion == 1) ? VERSION_XP : VERSION_SERVER_2003;
        }
        else if (version_info.dwMajorVersion == 6)
        {
            switch (version_info.dwMinorVersion)
            {
            case 0:
                // Treat Windows Server 2008 the same as Windows Vista.
                os_version_ = VERSION_VISTA;
                break;
            case 1:
                // Treat Windows Server 2008 R2 the same as Windows 7.
                os_version_ = VERSION_WIN7;
                break;
            case 2:
                // Treat Windows Server 2012 the same as Windows 8.
                os_version_ = VERSION_WIN8;
                break;
            default:
                os_version_ = VERSION_WIN8_1;
                break;
            }
        }
        else if (version_info.dwMajorVersion > 6)
        {
            os_version_ = VERSION_WIN_LAST;
        }
    }

    return os_version_;
}

} // namespace

namespace MetroWindow
{

CMetroFrame::CMetroFrame(HINSTANCE hInstance)
    : _hInst(hInstance)
    , _hWnd(NULL)
    , _hIcon(NULL)
    , _hSmallIcon(NULL)
    , _hCaptionFont(NULL)
    , _bgColor(RGB(255,255,255))
{
    _title[0] = L'\0';

    _minSize.cx = 0;
    _minSize.cy = 0;

    _showDropShadowOnXP = false;
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

    _dropShadow = NULL;

    _captionButtonMgr = new CCaptionButtonManager();

    _pressedButton = NULL;
    _hoveredButton = NULL;
}


CMetroFrame::~CMetroFrame(void)
{
    delete _captionButtonMgr;

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

    if (_hCaptionFont) ::DeleteObject(_hCaptionFont);

    if (_dropShadow != NULL)
    {
        delete _dropShadow;
        _dropShadow = NULL;
    }
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

void CMetroFrame::SetWindowTitle(const wchar_t* title)
{
    if (_hWnd != NULL)
    {
        ::SetWindowTextW(_hWnd, title);
    }
    else
    {
        wcscpy_s(_title, 255, title);
    }
}

void CMetroFrame::SetMinSize(int cx, int cy)
{
    _minSize.cx = cx;
    _minSize.cy = cy;
}

void CMetroFrame::ShowDropShadowOnXP(bool show)
{
    if (GetOSVersion() == VERSION_XP)
    {
        _showDropShadowOnXP = show;
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
        bHandled = DwmApi::DwmDefWindowProc(_hWnd, uMsg, wParam, lParam, &lRes);
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
    case WM_WINDOWPOSCHANGED: lRes = OnWindowPosChanged(uMsg, wParam, lParam, bHandled); break;
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
    lRes = ::DefWindowProc(_hWnd, uMsg, wParam, lParam);

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
    _trackMouseEvent.hwndTrack = _hWnd;

    _isDwmEnabled = DwmApi::IsDwmEnabled();
    _isUxThemeSupported = UxThemeApi::IsUxThemeSupported();

    if (_isDwmEnabled)
    {
        _isDwmEnabled = DwmApi::DwmAllowNcPaint(_hWnd);
    }

    if (!_isDwmEnabled)
    {
        // Disable theming on current window so we don't get 
        // any funny artifacts (round corners, etc.)
        if (_isUxThemeSupported)
        {
            UxThemeApi::SetWindowTheme(_hWnd, L"", L"");
        }
    }

    RemoveWindowBorderStyle();
    _captionButtonMgr->UpdateCaptionButtons(_hWnd, _captionTheme, _isDwmEnabled);

    ::DisableProcessWindowsGhosting();

    if (!_isDwmEnabled && _showDropShadowOnXP)
    {
        if (_dropShadow == NULL)
        {
            _dropShadow = new CDropShadow();
        }

        _dropShadow->Create(_hInst, _hWnd);
    }

    return 0;
}

LRESULT CMetroFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    if (_dropShadow != NULL)
    {
        _dropShadow->Destroy();
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Despite MSDN's documentation of lParam not being used,
    // calling DefWindowProc with lParam set to -1 causes Windows not to draw over the caption.
    ::DefWindowProc(_hWnd, uMsg, wParam, -1);

    bool ncactive = (wParam != 0) ? true : false;
    if (ncactive != _isNonClientAreaActive)
    {
        if (ncactive)
        {
            ::SetWindowPos(_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            ::SetForegroundWindow(_hWnd);
        }
        _isNonClientAreaActive = ncactive;
        PaintNonClientArea(NULL);

        if (_dropShadow != NULL)
        {
            _dropShadow->ShowShadow(_hWnd, ncactive);
        }
    }

    bHandled = TRUE;
    return 1;
}

LRESULT CMetroFrame::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CSize borderSize = WindowExtenders::GetBorderSize(_hWnd, _isDwmEnabled);

    if ( wParam == TRUE)
    {
        LPNCCALCSIZE_PARAMS pncsp = (LPNCCALCSIZE_PARAMS)lParam;

        ::CopyRect(&pncsp->rgrc[1], &pncsp->rgrc[0]);

        pncsp->rgrc[0].top = pncsp->rgrc[0].top + GetCaptionHeight() + borderSize.cy;
        pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - borderSize.cy;
        pncsp->rgrc[0].left = pncsp->rgrc[0].left + borderSize.cx;
        pncsp->rgrc[0].right = pncsp->rgrc[0].right - borderSize.cx;
    }
    else
    {
        LPRECT pRect = (LPRECT)lParam;

        pRect->top += GetCaptionHeight() + borderSize.cy;
        pRect->bottom -= borderSize.cy;
        pRect->left += borderSize.cx;
        pRect->right -= borderSize.cx;
    }

    bHandled = TRUE;
    _isSizing = true;

    return (_isDwmEnabled ? 1 : 0);
}

LRESULT CMetroFrame::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (_prepareFullScreen || _isSizing || PaintNonClientArea((HRGN)wParam))
    {
        bHandled = TRUE;
    }
    return 1; // For native dialog, must return 1 to avoid default dialog procedure
}

LRESULT CMetroFrame::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT point;
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);

    CRect rectScreen;
    ::GetWindowRect(_hWnd, &rectScreen);

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
                if (!_isDwmEnabled || WindowExtenders::IsWindowSizable(_hWnd))
                {
                    bHandled = FALSE;
                    return 0;
                }
                else
                {
                    bHandled = TRUE;
                    return HTCLIENT;
                }
            }
        }

        // on Button?
        CPoint pt(point.x - rectScreen.left, point.y - rectScreen.top);
        CCaptionButton * sysButton = _captionButtonMgr->CommandButtonFromPoint(pt);
        if (sysButton != NULL && sysButton->Enabled())
        {
            bHandled = TRUE;
            return sysButton->HitTest();
        }

        CSize borderSize = WindowExtenders::GetBorderSize(_hWnd, _isDwmEnabled);
        if (!_isFullScreen)
        {
            // on border?
            rect.InflateRect(-borderSize.cx + 1, -borderSize.cy + 1);

            // let form handle hittest itself if we are on borders
            if (!rect.PtInRect(point))
            {
                if (!_isDwmEnabled || WindowExtenders::IsWindowSizable(_hWnd))
                {
                    bHandled = FALSE;
                    return 0;
                }
                else
                {
                    bHandled = TRUE;
                    return HTCLIENT;
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
        if (WindowExtenders::HasSysMenu(_hWnd)/* && ShowIcon && Icon != null && ShowIconOnCaption*/)
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
    CCaptionButton * button = _captionButtonMgr->CommandButtonByHitTest(wParam);
    if (_pressedButton != button && _pressedButton != NULL)
        _pressedButton->Pressed(false);
    if (button != NULL)
        button->Pressed(true);
    _pressedButton = button;
    if (_pressedButton != NULL)
    {
        PaintNonClientArea(NULL);
    }

    if (_pressedButton != NULL || _isFullScreen)
    {
        bHandled = TRUE;
    }

    // For native dialog, must return 1 to avoid default dialog procedure
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
        if (_pressedButton != NULL)
        {
            if (_pressedButton->Pressed())
            {
                _pressedButton->Pressed(false);
                buttonStateChanged = true;
            }
            _pressedButton = NULL;
        }
    }

    CCaptionButton * button = _captionButtonMgr->CommandButtonByHitTest(wParam);
    if (_hoveredButton != button && _hoveredButton != NULL)
    {
        if (_hoveredButton->Hovered())
        {
            _hoveredButton->Hovered(false);
            buttonStateChanged = true;
        }
    }

    if (_pressedButton == NULL)
    {
        if (button != NULL)
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
        bool pressed = (button == _pressedButton);
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
    if (_pressedButton != NULL)
    {
        // get button at wparam
        CCaptionButton * button = _captionButtonMgr->CommandButtonByHitTest(wParam);
        if (button == NULL)
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
                    UINT cmd = ::IsZoomed(_hWnd) ? SC_RESTORE : SC_MAXIMIZE;
                    ::SendMessage(_hWnd, WM_SYSCOMMAND, cmd, 0);
                    break;
                }
            case HTMINBUTTON:
                {
                    UINT cmd = ::IsIconic(_hWnd) ? SC_RESTORE : SC_MINIMIZE;
                    ::SendMessage(_hWnd, WM_SYSCOMMAND, cmd, 0);
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
    CCaptionButton * button = _captionButtonMgr->CommandButtonByHitTest(wParam);
    if (button != NULL && (button->HitTest() == HTCLOSE || button->HitTest() == HTMAXBUTTON))
        return 0;

    if (WindowExtenders::HasSysMenu(_hWnd))
    {
        POINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);
        CRect rectScreen;
        ::GetWindowRect(_hWnd, &rectScreen);
        CRect rectCaption = rectScreen;
        rectCaption.Height(GetCaptionHeight());

        // right click in caption
        if (rectCaption.PtInRect(point))
        {
            if (button != NULL)
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
    if (_clientAreaMovable && !::IsZoomed(_hWnd))
    {
        ::ReleaseCapture();
        ::SendMessage(_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        bHandled = TRUE;
    }

    return 0;
}

LRESULT CMetroFrame::OnNcMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _traceNCMouse = false;

    bool buttonStateChanged = false;

    if (_pressedButton != NULL)
    {
        _pressedButton->Pressed(false);
        _pressedButton = NULL;

        buttonStateChanged = true;
    }
    if (_hoveredButton != NULL)
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
    ::GetMonitorInfo(::MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
    CRect rcWork = oMonitor.rcWork;
    CRect rcMonitor = oMonitor.rcMonitor;
    rcWork.OffsetRect(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

    // 计算最大化时，正确的原点坐标
    lpMMI->ptMaxPosition.x	= rcWork.left;
    lpMMI->ptMaxPosition.y	= rcWork.top;

    lpMMI->ptMaxTrackSize.x =rcWork.Width();
    lpMMI->ptMaxTrackSize.y =rcWork.Height();

    if (_minSize.cx > 0 && _minSize.cy > 0)
    {
        lpMMI->ptMinTrackSize.x = _minSize.cx;
        lpMMI->ptMinTrackSize.y = _minSize.cy;
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CMetroFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //if (::IsIconic(_hWnd)) bHandled = FALSE;

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
    _captionButtonMgr->UpdateCaptionButtons(_hWnd, _captionTheme, _isDwmEnabled);
    PaintNonClientArea(NULL);

    return 0;
}

LRESULT CMetroFrame::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (_dropShadow != NULL)
    {
        WINDOWPOS *pwp = (WINDOWPOS *)lParam;
        if (pwp->flags & SWP_SHOWWINDOW || pwp->flags & SWP_HIDEWINDOW ||
            !(pwp->flags & SWP_NOMOVE) || !(pwp->flags & SWP_NOSIZE))
        {
            _dropShadow->ShowShadow(_hWnd, _isNonClientAreaActive);
        }
    }

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
    _isDwmEnabled = DwmApi::IsDwmEnabled();
    PaintNonClientArea(NULL);

    return 0;
}

void CMetroFrame::RemoveWindowBorderStyle()
{
    // remove the border style
    DWORD dwStyle = ::GetWindowLong(_hWnd, GWL_STYLE);
    if ((dwStyle & WS_BORDER) != 0)
    {
        DWORD dwNewStyle = dwStyle & ~WS_BORDER;
        if (_isDwmEnabled)
        {
            dwNewStyle |= WS_THICKFRAME;
        }
        SetWindowLong(_hWnd, GWL_STYLE, dwNewStyle);

        SetWindowPos(_hWnd, NULL, 0, 0, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOREDRAW | SWP_NOACTIVATE);
    }

    _isSizable = WindowExtenders::IsWindowSizable(_hWnd);
}

BOOL CMetroFrame::ModifyWindowStyle(LONG removeStyle, LONG addStyle)
{
    DWORD dwStyle = ::GetWindowLong(_hWnd, GWL_STYLE);
    DWORD dwNewStyle = (dwStyle & ~removeStyle) | addStyle;
    if (dwStyle == dwNewStyle)
    {
        return FALSE;
    }

    ::SetWindowLong(_hWnd, GWL_STYLE, dwNewStyle);
    return TRUE;
}

//TODO: Add HRGN paramter to improve draw speed.
BOOL CMetroFrame::PaintNonClientArea(HRGN hrgnUpdate)
{
    BOOL result = FALSE;

    // prepare paint bounds
    CSize borderSize = WindowExtenders::GetBorderSize(_hWnd, _isDwmEnabled);
    int captionHeight = this->GetCaptionHeight();

    CRect rectWindow;
    ::GetWindowRect(_hWnd, &rectWindow);

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
    HDC hdc = ::GetDCEx(_hWnd, NULL, DCX_CACHE | DCX_CLIPSIBLINGS | DCX_WINDOW);
    //HDC hdc = ::GetWindowDC(_hWnd);
    if (hdc == NULL) return result;

    HRGN hrgn = NULL;

    if (!::IsIconic(_hWnd))
    {
        // prepare clipping
        CRect rectClip = rectWindow;

        int cx = borderSize.cx;
        int cy = borderSize.cy;

        if (!_isDwmEnabled)
        {
            DWORD dwExStyle = ::GetWindowLong(_hWnd, GWL_EXSTYLE);
            if (((dwExStyle & WS_EX_DLGMODALFRAME) != 0))
            {
                // The window has a double border
                cx *= 2;
                cy *= 2;
            }
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
        CBufferedPaint bufferedPaint;
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

    ::ReleaseDC(_hWnd, hdc);

    return result;
}

void CMetroFrame::DrawWindowFrame(HDC hdc, const RECT& bounds, const SIZE& borderSize, int captionHeight)
{
    BOOL isMaxisized = ::IsZoomed(_hWnd);

    // prepare bounds
    CRect windowBounds = bounds;

    CRect captionBounds = windowBounds;
    captionBounds.Height(borderSize.cy + captionHeight);

    CRect textBounds = captionBounds;
    
    COLORREF captionColor = (!_isNonClientAreaActive && !_isFullScreen) ?
        _captionTheme.InactiveCaptionColor() : _captionTheme.GetCaptionColor();

    // clear frame area
    COLORREF backColor = (_useThickFrame || isMaxisized)
        ? captionColor : _bgColor;

    FillSolidRect(hdc, &windowBounds, backColor);

    int frameBorderWidth = 1;

    // draw frame border
    if (!_isDwmEnabled && !isMaxisized && !_useThickFrame && _dropShadow == NULL)
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
    if (WindowExtenders::HasSysMenu(_hWnd) && GetSmallIcon() != NULL && _showIconOnCaption)
    {
        iconBounds = GetFrameIconBounds(bounds, borderSize);

        textBounds.left = iconBounds.right + 1;
    }

    // Paint caption buttons
    int buttonWidths = _captionButtonMgr->Draw(hdc);
    textBounds.right -= buttonWidths;

    textBounds.right -= borderSize.cx + 1;

    // draw the default caption title text
    if (!_useCustomTitle)
    {
        WCHAR title[256];
        int titleLen = ::GetWindowTextW(_hWnd, title, 255);
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
    if (!::IsIconic(_hWnd))
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
        wcscpy_s(lf.lfFaceName, L"Segoe UI");

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
    HTHEME hTheme = UxThemeApi::OpenThemeData(NULL, L"CompositedWindow::Window");
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
                    if (SUCCEEDED(UxThemeApi::GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
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

                UxThemeApi::DrawThemeTextEx(hTheme, hdcPaint, 0, 0, title, -1,
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

        UxThemeApi::CloseThemeData(hTheme);
    }
}

int CMetroFrame::GetCaptionHeight()
{
    if (_captionHeight == 0)
    {
        _captionHeight = WindowExtenders::GetCaptionHeight(_hWnd);

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
    int captionHeight = WindowExtenders::GetCaptionHeight(_hWnd);
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

void CMetroFrame::ShowSystemMenu(POINT point)
{
    HWND hWnd = _hWnd;
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