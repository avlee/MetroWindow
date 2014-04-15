#include "stdafx.h"
#include "MetroDialog.h"

namespace MetroWindow
{

CMetroDialog::CMetroDialog(HINSTANCE hInstance)
    : CMetroFrame(hInstance), _bModal(false)
{
}


CMetroDialog::~CMetroDialog(void)
{
    if (!_bModal && ::IsWindow(_hWnd))
    {
        ::DestroyWindow(_hWnd);
    }
}

INT_PTR CMetroDialog::DoModal(int resId, HWND hWndParent)
{
    _bModal = true;

    INT_PTR result = ::DialogBoxParam(GetModuleInstance(), MAKEINTRESOURCE(resId), hWndParent,
                                      &CMetroDialog::__DlgFunc, (LPARAM)this);
    if (::IsWindow(hWndParent))
        ::SetFocus(hWndParent);

    return result;
}

void CMetroDialog::ShowModeless(int resID, HWND hWndParent)
{
    if (!::IsWindow(_hWnd)) {
        _bModal = false;
        _hWnd = ::CreateDialogParam(GetModuleInstance(), MAKEINTRESOURCE(resID), hWndParent,
                                    &CMetroDialog::__DlgFunc, (LPARAM)this);
    } else {
        OnShowModeless();
    }

    ::ShowWindow(_hWnd, SW_SHOW);
    ::SetFocus(_hWnd);
}

BOOL CMetroDialog::EndDialog()
{
    ASSERT(::IsWindow(_hWnd));

    return ::EndDialog(_hWnd, IDOK);
}

BOOL CMetroDialog::DestroyWindow()
{
    ASSERT(::IsWindow(_hWnd));

    return ::DestroyWindow(_hWnd);
}

void CMetroDialog::OnInitDialog()
{
    if (wcslen(_title) > 0)
    {
        ::SetWindowTextW(_hWnd, _title);
    }

    CenterWindow();
}

void CMetroDialog::OnShowModeless()
{
}

LRESULT CMetroDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (_bModal)
        EndDialog();
    else
        DestroyWindow();

    bHandled = TRUE;

    return 1;
}

LRESULT CMetroDialog::OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //Has no meaning for a dialog, but needed for handlers that use it
    return 0;
}

LRESULT CMetroDialog::OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            BOOL bHandled = FALSE;
            LRESULT lRet = CMetroFrame::OnCreate(uMsg, wParam, lParam, bHandled);
            OnInitDialog();
            
            return lRet;
        }
    case WM_CTLCOLORDLG:
        return (LONG_PTR)::GetStockObject(WHITE_BRUSH);
    default:
        return CMetroFrame::OnWndProc(uMsg, wParam, lParam);
    }
}

INT_PTR CALLBACK CMetroDialog::__DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMetroDialog* pThis = NULL;

    if (uMsg == WM_INITDIALOG) {
        // get the pointer to the window from lpCreateParams
        ::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
        pThis = (CMetroDialog*)lParam;
        pThis->_hWnd = hwndDlg;
    } else {
        pThis = (CMetroDialog *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (uMsg == WM_NCDESTROY && pThis != NULL)
        {
			LRESULT lRes = ::DefWindowProc(hwndDlg, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->_hWnd, GWLP_USERDATA, 0L);
			pThis->_hWnd = NULL;
			return lRes;
		}
    }

    // if we have the pointer, go to the message handler of the window
    if (pThis) {
        LRESULT lRes = pThis->OnWndProc(uMsg, wParam, lParam);
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, lRes);
        return lRes;
    } else {
        return 0;
    }
}

} // namespace MetroWindow
