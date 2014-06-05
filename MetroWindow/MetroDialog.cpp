#include "stdafx.h"
#include "MetroDialog.h"

namespace MetroWindow
{

CMetroDialog::CMetroDialog(HINSTANCE hInstance)
    : CMetroFrame(hInstance), is_modal_(false)
{
}


CMetroDialog::~CMetroDialog(void)
{
    if (!is_modal_ && ::IsWindow(hWnd_))
    {
        ::DestroyWindow(hWnd_);
    }
}

INT_PTR CMetroDialog::DoModal(int resId, HWND hWndParent)
{
    is_modal_ = true;

    INT_PTR result = ::DialogBoxParam(GetModuleInstance(), MAKEINTRESOURCE(resId), hWndParent,
                                      &CMetroDialog::__DlgFunc, (LPARAM)this);
    if (::IsWindow(hWndParent))
        ::SetFocus(hWndParent);

    return result;
}

void CMetroDialog::ShowModeless(int resID, HWND hWndParent)
{
    if (!::IsWindow(hWnd_)) {
        is_modal_ = false;
        hWnd_ = ::CreateDialogParam(GetModuleInstance(), MAKEINTRESOURCE(resID), hWndParent,
                                    &CMetroDialog::__DlgFunc, (LPARAM)this);
    } else {
        OnShowModeless();
    }

    ::ShowWindow(hWnd_, SW_SHOW);
    ::SetFocus(hWnd_);
}

BOOL CMetroDialog::EndDialog()
{
    ASSERT(::IsWindow(hWnd_));

    return ::EndDialog(hWnd_, IDOK);
}

BOOL CMetroDialog::DestroyWindow()
{
    ASSERT(::IsWindow(hWnd_));

    return ::DestroyWindow(hWnd_);
}

void CMetroDialog::OnInitDialog()
{
    if (wcslen(title_) > 0)
    {
        ::SetWindowTextW(hWnd_, title_);
    }

    CenterWindow();
}

void CMetroDialog::OnShowModeless()
{
}

LRESULT CMetroDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (is_modal_)
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
        pThis->hWnd_ = hwndDlg;
    } else {
        pThis = (CMetroDialog *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (uMsg == WM_NCDESTROY && pThis != NULL)
        {
			LRESULT lRes = ::DefWindowProc(hwndDlg, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->hWnd_, GWLP_USERDATA, 0L);
			pThis->hWnd_ = NULL;
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
