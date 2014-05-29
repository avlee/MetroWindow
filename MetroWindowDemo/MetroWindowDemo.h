#pragma once

#include "resource.h"
#include "MetroWindow\MetroWindow.h"
#include "MetroWindow\MetroDialog.h"
#include "MetroWindow\MetroMessageBox.h"

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
        , _modelessWindow(NULL)
    {
    }

    virtual ~CMainWindow(void)
    {
        if (_modelessWindow != NULL)
            delete _modelessWindow;
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

        CreateWindow(L"BUTTON", L"Show Message Box - Styled",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 250, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST5, GetModuleInstance(), 0);

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
            testWidnow.ShowDropShadowOnXP(true);
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
            if (_modelessWindow == NULL)
            {
                _modelessWindow = new CMetroWindow(GetModuleInstance());
                _modelessWindow->ShowDropShadowOnXP(true);
            }

            if (!::IsWindow(_modelessWindow->GetHWnd()))
            {
                std::wstring strTitle = L"ModelLess Window";
                _modelessWindow->Create(GetHWnd(), strTitle.c_str(),
                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0);
            }

            _modelessWindow->ShowWindow();

            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST5)
        {
            CMetroMessageBox msgBox(GetModuleInstance());
            INT_PTR ret = msgBox.Show(*this, L"第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！\r\n\r\n第二行", L"测试", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
            //INT_PTR ret = MessageBox(*this, L"第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！第一行,还是要长一点才能测试出效果来嘛！\r\n\r\n第二行", L"测试", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
            if (ret == IDYES)
            {
                MessageBox(*this, L"你选择了YES", L"结果", MB_OK);
            }
            else
            {
                MessageBox(*this, L"你选择了NO", L"结果", MB_OK);
            }
        }

        return 0;
    }

private:
    CTestDialog _modelessDialog;
    CMetroWindow * _modelessWindow;
};


