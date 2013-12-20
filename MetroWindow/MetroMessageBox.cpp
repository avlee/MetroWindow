#include "stdafx.h"
#include "MetroMessageBox.h"

namespace MetroWindow
{

struct MetroMessageBoxValues
{
   HHOOK hHook;          // hook handle
   HWND  hWnd;           // message box handle
   WNDPROC lpMsgBoxProc; // window procedure
   LPVOID lpUserData;
} static __declspec(thread) cmbv;

CMetroMessageBox::CMetroMessageBox(HINSTANCE hInstance)
    : CMetroFrame(hInstance)
{
    SetCaptionColor(RGB(255, 106, 0));
    UseThickFrame(true);
}


CMetroMessageBox::~CMetroMessageBox(void)
{
}

int CMetroMessageBox::Show(HWND hWndParent, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    cmbv.hHook = NULL;
    cmbv.hWnd = NULL;
    cmbv.lpMsgBoxProc = NULL;
    cmbv.lpUserData = this;

    HINSTANCE hInstance = ::GetModuleHandle(NULL);

    cmbv.hHook = ::SetWindowsHookEx(WH_CBT, CMetroMessageBox::CbtHookProc, hInstance, 
                                    GetCurrentThreadId());

    int nRet = ::MessageBox(hWndParent, lpText, lpCaption, uType);

    ::UnhookWindowsHookEx(cmbv.hHook);

    return nRet;
}

LRESULT CMetroMessageBox::OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::CallWindowProc(cmbv.lpMsgBoxProc, _hWnd, uMsg, wParam, lParam);
}

LRESULT CMetroMessageBox::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Despite MSDN's documentation of lParam not being used,
    // calling DefWindowProc with lParam set to -1 causes Windows not to draw over the caption.
    ::CallWindowProc(cmbv.lpMsgBoxProc, _hWnd, uMsg, wParam, -1);

    return CMetroFrame::OnNcActivate(uMsg, wParam, lParam, bHandled);
}

LRESULT CALLBACK CMetroMessageBox::CbtHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   if (nCode < 0)
   {
      return ::CallNextHookEx(cmbv.hHook, nCode, wParam, lParam); 
   }

   switch(nCode)
   {
   case HCBT_CREATEWND: // a window is about to be created
      {
         LPCBT_CREATEWND lpCbtCreate = (LPCBT_CREATEWND)lParam;
         if(WC_DIALOG == lpCbtCreate->lpcs->lpszClass)
         {
            // WC_DIALOG is the class name of message box
            // but it has not yet a window procedure set.
            // So keep in mind the handle to sublass it later
            // when its first child is about to be created
            cmbv.hWnd = (HWND)wParam;
         }
         else
         {
            if((NULL == cmbv.lpMsgBoxProc) && (NULL != cmbv.hWnd))
            {
               // subclass the dialog 
               cmbv.lpMsgBoxProc = (WNDPROC)::SetWindowLongPtr(cmbv.hWnd, GWL_WNDPROC, 
                   (LONG)CMetroMessageBox::MsgBoxProc);
            }
         }
      }
      break;
   case HCBT_DESTROYWND: // a window is about to be destroyed
      {
         if(cmbv.hWnd == (HWND)wParam) // it's our messge box
         {
            // so set back its default procedure
            ::SetWindowLongPtr(cmbv.hWnd, GWL_WNDPROC, 
                          (LONG)cmbv.lpMsgBoxProc);
         }
      }
   }
   return 0;
}

INT_PTR CALLBACK CMetroMessageBox::MsgBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMetroMessageBox* pThis = (CMetroMessageBox*)cmbv.lpUserData;

    // if we have the pointer, go to the message handler of the window
    if (pThis) {
        if (uMsg == WM_INITDIALOG) {
            pThis->_hWnd = hWnd;
        } else if (uMsg == WM_NCDESTROY) {
            LRESULT lRes = ::CallWindowProc(cmbv.lpMsgBoxProc, hWnd, uMsg, wParam, lParam);
			pThis->_hWnd = NULL;
			return lRes;
        } else if (uMsg == WM_CTLCOLORDLG || uMsg == WM_CTLCOLORSTATIC) {
            return (LONG_PTR)::GetStockObject(WHITE_BRUSH);;
        }

        LRESULT lRes = pThis->OnWndProc(uMsg, wParam, lParam);
        ::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, lRes);
        return lRes;
    } else {
        return ::CallWindowProc(cmbv.lpMsgBoxProc, hWnd, uMsg, wParam, lParam);
    }
}

} // namespace MetroWindow
