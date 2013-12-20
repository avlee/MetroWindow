#pragma once

#include "MetroFrame.h"

namespace MetroWindow
{
    
class METROWINDOW_DECL CMetroMessageBox : public CMetroFrame
{
public:
    CMetroMessageBox(HINSTANCE hInstance);
    virtual ~CMetroMessageBox(void);

    int Show(HWND hWndParent, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

    virtual LRESULT OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    static LRESULT CALLBACK CbtHookProc(int, WPARAM, LPARAM);
    static INT_PTR CALLBACK MsgBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace MetroWindow
