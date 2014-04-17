#pragma once

#include "resource.h"
#include "MetroWindow\MetroWindow.h"
#include "MetroWindow\MetroDialog.h"

#include <vector>
#include <sstream>

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

    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int ctrlId = LOWORD(wParam);

        switch (ctrlId) {
        case IDOK:
        case IDCANCEL:
            EndDialog();
            return 1;
        }
        return 0;
    }
};

class CMainWindow : public CMetroWindow
{
public:
    CMainWindow(HINSTANCE hInstance)
        : CMetroWindow(hInstance)
        , _modelessDialog(hInstance)
    {
    }

    virtual ~CMainWindow(void)
    {
        std::vector<CMetroWindow *>::const_iterator iter = _modelessWindows.begin();
        std::vector<CMetroWindow *>::const_iterator end = _modelessWindows.end();
        for (; iter != end; iter++)
            delete *iter;
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

        CreateWindow(L"BUTTON", L"Show Modal Dialog - Native",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 50, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST1, GetModuleInstance(), 0);

        CreateWindow(L"BUTTON", L"Show Modal Dialog - Simulate",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 100, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST2, GetModuleInstance(), 0);

        CreateWindow(L"BUTTON", L"Show Modeless Dialog - Native",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 150, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST3, GetModuleInstance(), 0);

        CreateWindow(L"BUTTON", L"Show Modeless Dialog - Simulate",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 200, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST4, GetModuleInstance(), 0);

        _modelessDialog.SetWindowTitle(L"Modeless Dialog");
        _modelessDialog.SetCaptionColor(RGB(61, 140, 206));
        _modelessDialog.UseThickFrame(true);
        _modelessDialog.ShowDropShadowOnXP(true);

        return result;
    }

    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDC_BTN_TEST1)
        {
            CTestDialog testWidnow(GetModuleInstance());
            testWidnow.SetWindowTitle(L"Modal Dialog");
            testWidnow.DoModal(IDD_DIALOG1, GetHWnd());
            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST2)
        {
            CMetroWindow testWindow(GetModuleInstance());
            testWindow.ShowDropShadowOnXP(true);
            testWindow.Create(GetHWnd(), L"Modal Window", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0);
            testWindow.ShowDialog(GetHWnd());
            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST3)
        {
            _modelessDialog.ShowModeless(IDD_DIALOG1, GetHWnd());
        }
        else if (wmId == IDC_BTN_TEST4)
        {
            CMetroWindow * testWindow = new CMetroWindow(GetModuleInstance());
            _modelessWindows.push_back(testWindow);

            std::wstringstream ss;
            ss << L"ModelLess Window " << _modelessWindows.size();

            std::wstring strTitle = ss.str();

            testWindow->ShowDropShadowOnXP(true);
            testWindow->Create(GetHWnd(), strTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0);
            testWindow->ShowWindow();

            bHandled = TRUE;
        }

        return 0;
    }

private:
    CTestDialog _modelessDialog;
    std::vector<CMetroWindow *> _modelessWindows;
};


