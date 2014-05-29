#pragma once

#include "resource.h"
#include "MetroWindow\MetroWindow.h"
#include "MetroWindow\MetroDialog.h"
#include "MetroWindow\MetroMessageBox.h"

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

        CreateWindow(L"BUTTON", L"Show Message Box - Styled",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    80, 250, 250, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST5, GetModuleInstance(), 0);

        _modelessDialog.SetWindowTitle(L"Modeless Dialog");
        _modelessDialog.SetCaptionColor(RGB(61, 140, 206));
        _modelessDialog.UseThickFrame(true);

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
            testWindow.Create(*this, L"Modal Window", WS_OVERLAPPEDWINDOW, 0);
            testWindow.ShowDialog(*this);
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

            testWindow->Create(*this, strTitle.c_str(), WS_OVERLAPPEDWINDOW, 0);
            testWindow->ShowWindow();

            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST5)
        {
            CMetroMessageBox msgBox(GetModuleInstance());
            INT_PTR ret = msgBox.Show(*this, L"��һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�����\r\n\r\n�ڶ���", L"����", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
            //INT_PTR ret = MessageBox(*this, L"��һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�������һ��,����Ҫ��һ����ܲ��Գ�Ч�����\r\n\r\n�ڶ���", L"����", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
            if (ret == IDYES)
            {
                MessageBox(*this, L"��ѡ����YES", L"���", MB_OK);
            }
            else
            {
                MessageBox(*this, L"��ѡ����NO", L"���", MB_OK);
            }
        }

        return 0;
    }

private:
    CTestDialog _modelessDialog;
    std::vector<CMetroWindow *> _modelessWindows;
};


