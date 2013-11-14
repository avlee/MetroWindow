#pragma once

#include "MetroFrame.h"

namespace MetroWindow
{

class METROWINDOW_DECL CMetroDialog : public CMetroFrame
{
public:
    CMetroDialog(HINSTANCE hInstance);
    virtual ~CMetroDialog(void);

    INT_PTR DoModal(int resId, HWND hWndParent);
    void EndDialog();

    virtual void OnInitDialog();
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    static INT_PTR CALLBACK __DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace MetroWindow

