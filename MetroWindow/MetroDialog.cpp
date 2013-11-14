#include "MetroDialog.h"

namespace MetroWindow
{

CMetroDialog::CMetroDialog(HINSTANCE hInstance)
    : CMetroFrame(hInstance)
{
}


CMetroDialog::~CMetroDialog(void)
{
}

INT_PTR CMetroDialog::DoModal(int resId, HWND hWndParent)
{
    INT_PTR result = ::DialogBoxParam(GetModuleInstance(), MAKEINTRESOURCE(resId), hWndParent,
                                      &CMetroDialog::__DlgFunc, (LPARAM)this);
    if (::IsWindow(hWndParent))
        ::SetFocus(hWndParent);

    return result;
}

void CMetroDialog::EndDialog()
{
    if (::IsWindow(_hWnd)) {
        ::EndDialog(_hWnd, IDOK);
    }
}

void CMetroDialog::OnInitDialog()
{
    CenterWindow();
}

LRESULT CMetroDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    EndDialog();

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

    if (uMsg == WM_INITDIALOG)
    {
        BOOL bHandled = FALSE;
        LRESULT lRet = CMetroFrame::OnCreate(uMsg, wParam, lParam, bHandled);
        OnInitDialog();
        return lRet;
    }
    else
    {
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
