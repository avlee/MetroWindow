#pragma once

#include "resource.h"
#include "MetroWindow\MetroWindow.h"
#include "MetroWindow\MetroDialog.h"

using namespace MetroWindow;

class CTestDialog : public CMetroDialog
{
public:
    CTestDialog(HINSTANCE hInstance) : CMetroDialog(hInstance)
    {
    }

    virtual ~CTestDialog(void)
    {
    }

    virtual LPCTSTR GetWindowClassName() const
    {
        return _T("TestMetroDialog");
    }
};

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

    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT result = CMetroWindow::OnCreate(uMsg, wParam, lParam, bHandled);

        btnTest_ = CreateWindow(L"BUTTON", L"显示模态对话框",
                              WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                              100, 50, 150, 24,
                              GetHWnd(), (HMENU)IDC_BTN_TEST, GetModuleInstance(), 0);

        return result;
    }

    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDC_BTN_TEST)
        {
            CTestDialog testWidnow(GetModuleInstance());
            testWidnow.DoModal(IDD_DIALOG1, GetHWnd());
            bHandled = TRUE;
        }

        return 0;
    }

private:
    HWND btnTest_;
};


