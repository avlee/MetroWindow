#pragma once

#include "resource.h"
#include "MetroWindow\MetroWindow.h"

using namespace MetroWindow;

class CMainWindow : public CMetroWindow
{
public:
    CMainWindow(HINSTANCE hInstance) : CMetroWindow(hInstance)
    {
    }

    virtual ~CMainWindow(void)
    {
    }

    virtual LPCTSTR GetWindowClassName() const
    {
        return _T("MainMetroWindow");
    }

    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        ::PostQuitMessage(0);
        bHandled = TRUE;
        return 0;
    }
};
