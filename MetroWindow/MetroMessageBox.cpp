#include "stdafx.h"
#include "MetroMessageBox.h"
#include <vector>
#include <tchar.h>

namespace MetroWindow
{

class DialogTemplateT
{
public:
    LPCDLGTEMPLATE Template() {
        return (LPCDLGTEMPLATE)&v[0];
    }

    void AlignToDword() {
        if (v.size() % 4) {
            Write(NULL, 4 - (v.size() % 4));
        }
    }

    void Write(LPCVOID pvWrite, DWORD cbWrite) {
        v.insert(v.end(), cbWrite, 0);
        if (pvWrite) {
            CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
        }
    }

    template<typename T> void Write(T t) {
        Write(&t, sizeof(T));
    }

    void WriteString(LPCWSTR psz) {
        Write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR));
    }

private:
    std::vector<BYTE> v;
};

// Get the size of a struct up to and including the specified member.
// This is necessary to set compatible struct sizes for different versions
// of certain Windows APIs (e.g. SystemParametersInfo).
#define SIZEOF_STRUCT_WITH_SPECIFIED_LAST_MEMBER(struct_name, member) \
    offsetof(struct_name, member) + \
    (sizeof static_cast<struct_name*>(NULL)->member)

#define NONCLIENTMETRICS_SIZE_PRE_VISTA \
    SIZEOF_STRUCT_WITH_SPECIFIED_LAST_MEMBER(NONCLIENTMETRICS, lfMessageFont)

bool IsWindowsVistaOrLater()
{
    OSVERSIONINFO version_info = { sizeof version_info };
    ::GetVersionEx(&version_info);
    return (version_info.dwMajorVersion > 5);
}

bool GetNonClientMetrics(NONCLIENTMETRICS* metrics)
{

    static const UINT SIZEOF_NONCLIENTMETRICS =
        IsWindowsVistaOrLater() ?
        sizeof(NONCLIENTMETRICS) : NONCLIENTMETRICS_SIZE_PRE_VISTA;
    metrics->cbSize = SIZEOF_NONCLIENTMETRICS;
    return !!SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                                  SIZEOF_NONCLIENTMETRICS, metrics,
                                  0);
}

int GetDluBaseX(HDC hdc)
{
    // This is how Microsoft recommends calculating the dialog unit
    // conversions. See: http://support.microsoft.com/kb/125681
    SIZE size;
    GetTextExtentPoint32(hdc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
    int dlux = (size.cx/26 + 1) / 2;
    return dlux;
}

int GetDluBaseY(HDC hdc)
{
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    int dluy = std::max<int>(1, tm.tmHeight);;
    return dluy;
}

inline wchar_t* WriteInto(std::wstring* str,
                          size_t length_with_null)
{
    str->reserve(length_with_null);
    str->resize(length_with_null - 1);
    return &((*str)[0]);
}

bool ReplaceChars(const std::wstring& input,
                  const wchar_t replace_chars[],
                  const std::wstring& replace_with,
                  std::wstring* output)
{
    bool removed = false;
    size_t replace_length = replace_with.length();

    *output = input;

    size_t found = output->find_first_of(replace_chars);
    while (found != std::wstring::npos) {
        removed = true;
        output->replace(found, 1, replace_with);
        found = output->find_first_of(replace_chars, found + replace_length);
    }

    return removed;
}

enum {
    ButtonWidth             = 50,
    ButtonHeight            = 14,
    ButtonSpacing           = 6,
    BottomMargin            = 12,
    MinimalHeight           = 70,
    SpacingWidth            = 8,
    SpacingHeight           = 5,
};

#define ICON_CONTROL_ID 1000
#define MESSAGE_CONTROL_ID 1001

#define DIALOG_ITEM_BUTTON 0x0080FFFF
#define DIALOG_ITEM_STATIC 0x0082FFFF

struct ButtonInfo {
    UINT Id;
    WCHAR *Text;
};

//static const ButtonInfo kMessageBoxButtons[] =
//{
//    0,                  L"",
//    IDOK,               L"OK",
//    IDCANCEL,           L"Cancel",
//    IDABORT,            L"&Abort",
//    IDRETRY,            L"&Retry",
//    IDIGNORE,           L"&Ignore",
//    IDYES,              L"&Yes",
//    IDNO,               L"&No",
//    IDCLOSE,            L"Close",
//    IDHELP,             L"&Help",
//    IDTRYAGAIN,         L"&Try Again",
//    IDCONTINUE,         L"&Continue",
//};

static const ButtonInfo kMessageBoxButtons[] = {
    0,                  L"",
    IDOK,               L"确定",
    IDCANCEL,           L"取消",
    IDABORT,            L"终止(&A)",
    IDRETRY,            L"重试(&R)",
    IDIGNORE,           L"忽略(&I)",
    IDYES,              L"是(&Y)",
    IDNO,               L"否(&N)",
    IDCLOSE,            L"关闭",
    IDHELP,             L"帮助",
    IDTRYAGAIN,         L"重试(&T)",
    IDCONTINUE,         L"继续(&C)",
};

const WCHAR * GetMessageBoxButtonText(UINT buttonId)
{
    // This is the safe version.
    //for (int i = 0; i < arraysize(kMessageBoxButtons); ++i)
    //{
    //    if (buttonId == kMessageBoxButtons[i].Id)
    //        return kMessageBoxButtons[i].Text;
    //}

    //return kMessageBoxButtons[0].Text;

    if (buttonId > 0 && buttonId < arraysize(kMessageBoxButtons)) {
        return kMessageBoxButtons[buttonId].Text;
    } else {
        return kMessageBoxButtons[0].Text;
    }
}

BOOL SetClipboardText(LPCTSTR lpszBuffer)
{
    BOOL bSuccess = FALSE;

    // First, open the clipboard. OpenClipboard() takes one
    // parameter, the handle of the window that will temporarily
    // be it's owner. If NULL is passed, the current process
    // is assumed. After opening, empty the clipboard so we
    // can put our text on it.
    if (::OpenClipboard(NULL)) {
        ::EmptyClipboard();

        // Get the size of the string in the buffer that was
        // passed into the function, so we know how much global
        // memory to allocate for the string.
        size_t nSize = _tcslen(lpszBuffer);

        // Allocate the memory for the string.
        HGLOBAL hGlobal = ::GlobalAlloc(GMEM_ZEROINIT, (nSize+1)*sizeof(TCHAR));

        // If we got any error during the memory allocation,
        // we have been returned a NULL handle.
        if (hGlobal) {
            // Now we have a global memory handle to the text
            // stored on the clipboard. We have to lock this global
            // handle so that we have access to it.
            LPTSTR lpszData = (LPTSTR) ::GlobalLock(hGlobal);

            if (lpszData) {
                // Now, copy the text from the buffer into the allocated
                // global memory pointer
                _tcscpy(lpszData, lpszBuffer);

                // Now, simply unlock the global memory pointer,
                // set the clipboard data type and pointer,
                // and close the clipboard.
                ::GlobalUnlock(hGlobal);
#ifdef _UNICODE
                ::SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
                ::SetClipboardData(CF_TEXT, hGlobal);
#endif
                bSuccess = TRUE;
            }
        }
        ::CloseClipboard();
    }

    return bSuccess;
}

LONG CALLBACK IconProc(HWND hwnd, UINT message, WPARAM, LPARAM)
{
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc;
        hdc = BeginPaint(hwnd, &ps);
        ::DrawIcon(hdc, 0, 0, (HICON)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)));
        EndPaint(hwnd, &ps);
    }

    return FALSE;
}

CMetroMessageBox::CMetroMessageBox(HINSTANCE hInstance)
    : CMetroFrame(hInstance), _hIcon(NULL), _hFont(NULL),
      _rightJustifyButtons(false), _disableClose(false), _buttonWidth(0), _buttonHeight(0),
      _buttonCount(0), _defaultButton(1), _defaultButtonId(0), _baseUnitX(1), _baseUnitY(1),
      _returnValue(IDCANCEL), _ended(false)
{
    for (int i = 0; i < 5; ++i) {
        DialogItemTemplate item = { 0 };
        _items[i] = item;
    }

    SetCaptionColor(RGB(255, 106, 0));
    UseThickFrame(true);
}


CMetroMessageBox::~CMetroMessageBox(void)
{
    if (_hIcon)
        DestroyIcon(_hIcon);
    _hIcon = 0;

    if (_hFont)
        ::DeleteObject(_hFont);
}

INT_PTR CMetroMessageBox::Show(HWND hWndParent, LPCTSTR lpszMessage, LPCTSTR lpszCaption, UINT uType)
{
    if (lpszMessage == NULL) lpszMessage = L"";
    if (lpszCaption == NULL) lpszCaption = L"Error";

    _buttonCount = 0;
    _disableClose = false;

    // set default button
    switch (uType & MB_DEFMASK) {
    case MB_DEFBUTTON1 :
        _defaultButton = 1;
        break;
    case MB_DEFBUTTON2 :
        _defaultButton = 2;
        break;
    case MB_DEFBUTTON3 :
        _defaultButton = 3;
        break;
    case MB_DEFBUTTON4 :
        _defaultButton = 4;
        break;
    default:
        _defaultButton = 1;
        break;
    }

    NONCLIENTMETRICSW ncm = { sizeof(ncm) };
    if (!GetNonClientMetrics(&ncm))
        return IDCANCEL;

    ::MessageBeep(uType & MB_ICONMASK);

    int nMaxWidth = (::GetSystemMetrics(SM_CXSCREEN) / 2) + 80;
    if (uType & MB_ICONMASK)
        nMaxWidth -= GetSystemMetrics(SM_CXICON) + 2 * SpacingWidth;

    RECT msgRect;
    SetRect(&msgRect, 0, 0, nMaxWidth, nMaxWidth);

    {
        HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

        _hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
        HFONT hOldFont = (HFONT)::SelectObject(hdc, _hFont);

        _baseUnitX = GetDluBaseX(hdc);
        _baseUnitY = GetDluBaseY(hdc);

        // translate dialog units to pixels
        _buttonWidth  = MulDiv(ButtonWidth, _baseUnitX, 4);
        _buttonHeight = MulDiv(ButtonHeight, _baseUnitY, 8);

        // Get bounds of message text
        ::DrawText(hdc, lpszMessage, -1, &msgRect,
                   DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT | DT_EXPANDTABS);

        // See text for discussion of fancy formula.
        if (ncm.lfMessageFont.lfHeight < 0) {
            ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight,
                                                 72, GetDeviceCaps(hdc, LOGPIXELSY));
        }

        ::SelectObject(hdc, hOldFont);
        ::DeleteDC(hdc);
    }

    msgRect.left    = SpacingWidth;
    msgRect.top     = SpacingWidth;
    msgRect.right  += 2 * SpacingWidth;
    msgRect.bottom += 2 * SpacingWidth;
    if (uType & MB_ICONMASK) {
        msgRect.left  += SpacingWidth;
        msgRect.right += SpacingWidth;
    }

    int width = msgRect.right - msgRect.left;
    int height = msgRect.bottom - msgRect.top;

    // client rect
    RECT mbrect;
    SetRect(&mbrect, 0, 0, width + (2 * SpacingWidth), height + (2 * SpacingWidth));
    if ((mbrect.bottom - mbrect.top) < MinimalHeight)
        mbrect.bottom = MinimalHeight;

    _dlgTempl.x = 0;
    _dlgTempl.y = 0;
    _dlgTempl.cdit = 0;

    _dlgTempl.style = WS_CAPTION | WS_VISIBLE | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME | DS_CENTER;
    if (uType & MB_SYSTEMMODAL) {
        _dlgTempl.style |= DS_SYSMODAL;
    }

    _dlgTempl.dwExtendedStyle = 0;

    RECT iconrect = { 0 };
    if (uType & MB_ICONMASK) {
        LPTSTR lpIcon = NULL;

        switch (uType & MB_ICONMASK) {
        case MB_ICONEXCLAMATION:
            lpIcon = (LPTSTR)IDI_EXCLAMATION;
            break;
        case MB_ICONHAND:
            lpIcon = (LPTSTR)IDI_HAND;
            break;
        case MB_ICONQUESTION:
            lpIcon = (LPTSTR)IDI_QUESTION;
            break;
        case MB_ICONASTERISK:
            lpIcon = (LPTSTR)IDI_ASTERISK;
            break;
        default:
            lpIcon = (LPTSTR)IDI_EXCLAMATION;
            break;
        }

        if (lpIcon)
            _hIcon = ::LoadIcon(NULL, lpIcon);
    }

    if (_hIcon) {
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);

        int icon_x = SpacingWidth;
        int icon_y = SpacingWidth;

        msgRect.left  += cxIcon + icon_x;
        msgRect.right += cxIcon + icon_x;

        mbrect.right = msgRect.right + SpacingWidth;

        SetRect(&iconrect, icon_x, icon_y, icon_x + cxIcon + 2, icon_y + cyIcon + 2);
        AddItem(DIALOG_ITEM_STATIC, ICON_CONTROL_ID, &iconrect);
    }

    // add message text
    AddItem(DIALOG_ITEM_STATIC, MESSAGE_CONTROL_ID, &msgRect);

    int cItems = 0;
    int nWidthStdButtons = 0;

    switch (uType & MB_TYPEMASK) {
    case MB_OK:
        cItems = 1;
        break;
    case MB_OKCANCEL:
    case MB_RETRYCANCEL:
    case MB_YESNO:
        cItems = 2;
        break;
    case MB_YESNOCANCEL:
    case MB_ABORTRETRYIGNORE:
    case MB_CANCELTRYCONTINUE:
        cItems = 3;
        break;
    }

    int x = 0;
    int y = 0;

    y = (msgRect.bottom > iconrect.bottom) ? msgRect.bottom : iconrect.bottom;
    y += SpacingHeight;

    if (_hIcon)
        y += 9;

    int nTotalButtonWidth = _buttonWidth * cItems + (ButtonSpacing * (cItems-1));

    RECT buttonRow;
    SetRect(&buttonRow, 0, y, nTotalButtonWidth, y + _buttonHeight);

    mbrect.bottom = buttonRow.bottom + BottomMargin;

    int bw = buttonRow.right - buttonRow.left;
    int bleft = 2 * SpacingWidth;
    int bright = bleft + bw;

    if (mbrect.right <= (bright + (2 * SpacingWidth)))
        mbrect.right = bright + (2 * SpacingWidth);

    x = ((mbrect.right - mbrect.left) - bw) / 2;
    y = buttonRow.top;

    if (_rightJustifyButtons) {
        x = mbrect.right - nTotalButtonWidth - 2 * SpacingWidth;
    }

    switch (uType & MB_TYPEMASK) {
    case MB_OK:
        x += AddButton(IDOK, x, y);
        break;

    case MB_OKCANCEL:
        x += AddButton(IDOK, x, y);
        x += AddButton(IDCANCEL, x, y);
        break;

    case MB_RETRYCANCEL:
        x += AddButton(IDRETRY, x, y);
        x += AddButton(IDCANCEL, x, y);
        break;

    case MB_YESNO:
        x += AddButton(IDYES, x, y);
        x += AddButton(IDNO, x, y);
        _disableClose = true;
        break;

    case MB_YESNOCANCEL:
        x += AddButton(IDYES, x, y);
        x += AddButton(IDNO, x, y);
        x += AddButton(IDCANCEL, x, y);
        break;

    case MB_ABORTRETRYIGNORE:
        x += AddButton(IDABORT, x, y);
        x += AddButton(IDRETRY, x, y);
        x += AddButton(IDIGNORE, x, y);
        _disableClose = true;
        break;

    case MB_CANCELTRYCONTINUE:
        x += AddButton(IDCANCEL, x, y);
        x += AddButton(IDTRYAGAIN, x, y);
        x += AddButton(IDCONTINUE, x, y);
        break;

    default:
        x += AddButton(IDOK, x, y);
        break;
    }

    if (buttonRow.bottom >= mbrect.bottom)
        mbrect.bottom = buttonRow.bottom + (2 * SpacingWidth);

    if (mbrect.right < (buttonRow.right + (2 * SpacingWidth)))
        mbrect.right = buttonRow.right + (2 * SpacingWidth);

    _dlgTempl.x = 0;
    _dlgTempl.y = 0;
    _dlgTempl.cx = (short)PixelToDluX(mbrect.right - mbrect.left);
    _dlgTempl.cy = (short)PixelToDluY(mbrect.bottom - mbrect.top);

    DialogTemplateT tmp;
    tmp.Write<WORD>(1); // dialog version
    tmp.Write<WORD>(0xFFFF); // extended dialog template
    tmp.Write<DWORD>(0); // help ID
    tmp.Write<DWORD>(_dlgTempl.dwExtendedStyle); // extended style
    tmp.Write<DWORD>(_dlgTempl.style);
    tmp.Write<WORD>(_dlgTempl.cdit); // number of controls
    tmp.Write<WORD>(_dlgTempl.x); // X
    tmp.Write<WORD>(_dlgTempl.y); // Y
    tmp.Write<WORD>(_dlgTempl.cx); // width
    tmp.Write<WORD>(_dlgTempl.cy); // height
    tmp.WriteString(L""); // no menu
    tmp.WriteString(L""); // default dialog class
    tmp.WriteString(lpszCaption); // title

    // Next comes the font description.
    tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfHeight); // point
    tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfWeight); // weight
    tmp.Write<BYTE>(ncm.lfMessageFont.lfItalic); // Italic
    tmp.Write<BYTE>(ncm.lfMessageFont.lfCharSet); // CharSet
    tmp.WriteString(ncm.lfMessageFont.lfFaceName);

    for (int i = 0; i < _dlgTempl.cdit; ++i) {
        DialogItemTemplate * dlgItem = &_items[i];

        tmp.AlignToDword();
        tmp.Write<DWORD>(0); // help id
        tmp.Write<DWORD>(dlgItem->dwExtendedStyle); // window extended style
        tmp.Write<DWORD>(dlgItem->style); // style
        tmp.Write<WORD>(dlgItem->x); // x
        tmp.Write<WORD>(dlgItem->y); // y
        tmp.Write<WORD>(dlgItem->cx); // width
        tmp.Write<WORD>(dlgItem->cy); // height
        tmp.Write<DWORD>(dlgItem->id); // control ID
        tmp.Write<DWORD>(dlgItem->ctype); // static

        if (dlgItem->id == MESSAGE_CONTROL_ID)
            tmp.WriteString(lpszMessage);
        else if (dlgItem->id == ICON_CONTROL_ID)
            tmp.WriteString(L"");
        else
            tmp.WriteString(GetMessageBoxButtonText(dlgItem->id));

        tmp.Write<WORD>(0); // no extra data
    }

    // Show the message dialog.
    HWND hDlg = ::CreateDialogIndirectParam(GetModuleInstance(), tmp.Template(),
        hWndParent, &CMetroMessageBox::MsgBoxProc, (LPARAM)this);
	_ASSERTE(hDlg);

	if (hDlg)
	{
		// disable owner - this is a modal dialog
		::EnableWindow(hWndParent, FALSE);

		MSG msg;
		memset(&msg, 0, sizeof(msg));

		// message loop for dialog
        // TODO: Using global message loop to filter the message?

		while (!_ended)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					break;
				}
				if (msg.message == WM_KEYDOWN)
				{
					// returns TRUE if Ctrl-C processed
					if (OnKeyDown(hDlg, msg.wParam, msg.lParam))
						continue;
				}
				if (!::IsDialogMessage(hDlg, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
			else if (!_ended)
			{
				::WaitMessage();	// suspend thread until new message arrives
			}
		}

		if (msg.message == WM_QUIT)
		{
			PostQuitMessage((int)msg.wParam);
		}

		// re-enable owner
		if (::IsWindow(hWndParent))
			::EnableWindow(hWndParent, TRUE);
		::DestroyWindow(hDlg);
	}

    return _returnValue;
}

// Dialog units to pixels conversion.
// See http://support.microsoft.com/kb/145994 for details.
int CMetroMessageBox::PixelToDluX(int dluX)
{
    return dluX * 4 / _baseUnitX;
}

int CMetroMessageBox::PixelToDluY(int dluY)
{
    return dluY * 8 / _baseUnitY;
}

int CMetroMessageBox::AddButton(UINT nID, int x, int y)
{
    RECT rect;
    SetRect(&rect, x, y, x + _buttonWidth, y + _buttonHeight);

    AddItem(DIALOG_ITEM_BUTTON, nID, &rect);
    return _buttonWidth + ButtonSpacing;
}

void CMetroMessageBox::AddItem(DWORD cType, UINT nID, RECT *pRect)
{
    if (_dlgTempl.cdit < 5) {

        DialogItemTemplate * dlgItem = &_items[_dlgTempl.cdit];

        dlgItem->x = (short)PixelToDluX(pRect->left);
        dlgItem->y = (short)PixelToDluY(pRect->top);
        dlgItem->cx = (short)PixelToDluX(pRect->right - pRect->left);
        dlgItem->cy = (short)PixelToDluY(pRect->bottom - pRect->top);

        dlgItem->dwExtendedStyle = 0;
        dlgItem->id = (WORD)nID;
        dlgItem->ctype = cType;

        switch (cType) {
        case DIALOG_ITEM_BUTTON:
            _buttonCount++;
            dlgItem->style = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
            if (_buttonCount == _defaultButton) {
                dlgItem->style |= BS_DEFPUSHBUTTON;
                _defaultButtonId = nID;
            } else {
                dlgItem->style |= BS_PUSHBUTTON;
            }
            break;

        case DIALOG_ITEM_STATIC:
            dlgItem->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
            break;

        default:
            _ASSERTE(FALSE); // should never get here
        }

        _dlgTempl.cdit++;
    }
}

LRESULT CMetroMessageBox::OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CMetroMessageBox::OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg) {
    case WM_INITDIALOG: {
        BOOL bHandled = FALSE;
        LRESULT lRet = CMetroFrame::OnCreate(uMsg, wParam, lParam, bHandled);

        // Disable close button just like real MessageBox
        if (_disableClose) {
            ::RemoveMenu(GetSystemMenu(GetHWnd(), FALSE), SC_CLOSE, MF_BYCOMMAND);

            // TODO disable the close button.
        }

        if (_hIcon) {
            HWND hwndIcon;

            hwndIcon = ::GetDlgItem(GetHWnd(), ICON_CONTROL_ID);
            if (hwndIcon && ::IsWindow(hwndIcon)) {
                ::SetWindowLongPtr(hwndIcon, GWLP_WNDPROC, (LONG_PTR) IconProc);
                ::SetWindowLongPtr(hwndIcon, GWLP_USERDATA, (LONG_PTR) _hIcon);
            }
        }

        HWND hwndChild = ::GetDlgItem(GetHWnd(), _defaultButtonId);
        if (hwndChild && ::IsWindow(hwndChild))
            ::SetFocus(hwndChild);

        ::SetForegroundWindow(GetHWnd());

        return lRet;
    }
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        return (LONG_PTR)::GetStockObject(WHITE_BRUSH);
    case WM_COMMAND:
        if (wParam == IDCLOSE) {
            return TRUE;
        } else {
            _ended = true;
            _returnValue = wParam;
            return FALSE;
        }
    default:
        return CMetroFrame::OnWndProc(uMsg, wParam, lParam);
    }
}

BOOL CMetroMessageBox::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    BOOL rc = FALSE;

    // convert virtual key code
    TCHAR ch = (TCHAR) wParam;

    // if the most significant bit is set, the key is down
    BOOL bCtrlIsDown = 	GetAsyncKeyState(VK_CONTROL) < 0;

    if (bCtrlIsDown && (ch == _T('C'))) {
        rc = TRUE;

        // user hit Ctrl-C -- copy contents to clipboard
        if (::IsWindow(hWnd)) {
            WCHAR *pszDivider = L"---------------------------\r\n";
            std::wstring text;
            text.append(pszDivider);

            std::wstring title;
            size_t len_with_null = GetWindowTextLength(hWnd) + 1;
            GetWindowText(hWnd, WriteInto(&title, len_with_null), len_with_null);

            text.append(title);
            text.append(L"\r\n");
            text.append(pszDivider);

            HWND hMsgHwnd = ::GetDlgItem(hWnd, MESSAGE_CONTROL_ID);
            if (::IsWindow(hMsgHwnd)) {
                std::wstring message;
                len_with_null = GetWindowTextLength(hMsgHwnd) + 1;
                GetWindowText(hMsgHwnd, WriteInto(&message, len_with_null), len_with_null);
                text.append(message);
                text.append(L"\r\n");
                text.append(pszDivider);
            }

            WCHAR szClassName[MAX_PATH];
            HWND hwndChild = ::GetWindow(hWnd, GW_CHILD);
            while (hwndChild) {
                if (::IsWindow(hwndChild)) {
                    ::GetClassName(hwndChild, szClassName, sizeof(szClassName) - 2);
                    if (_tcsicmp(szClassName, _T("Button")) == 0) {
                        std::wstring buttonText;
                        len_with_null = GetWindowTextLength(hwndChild) + 1;
                        GetWindowText(hwndChild, WriteInto(&buttonText, len_with_null), len_with_null);

                        ReplaceChars(buttonText, L"&", std::wstring(), &buttonText);

                        text.append(buttonText);
                        text.append(L"   ");
                    }
                }
                hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
            }

            text.append(L"\r\n");
            text.append(pszDivider);

            SetClipboardText(text.c_str());
        }
    }
    return rc;
}

INT_PTR CALLBACK CMetroMessageBox::MsgBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMetroMessageBox* pThis = NULL;

    if (uMsg == WM_INITDIALOG) {
        // get the pointer to the window from lpCreateParams
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
        pThis = (CMetroMessageBox*)lParam;
        pThis->_hWnd = hWnd;
    } else {
        pThis = (CMetroMessageBox *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (uMsg == WM_NCDESTROY && pThis != NULL) {
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->_hWnd, GWLP_USERDATA, 0L);
            pThis->_hWnd = NULL;
            return lRes;
        }
    }

    // if we have the pointer, go to the message handler of the window
    if (pThis) {
        LRESULT lRes = pThis->OnWndProc(uMsg, wParam, lParam);
        SetWindowLongPtr(hWnd, DWLP_MSGRESULT, lRes);
        return lRes;
    } else {
        return 0;
    }
}

} // namespace MetroWindow
