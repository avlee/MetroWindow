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

    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int ctrlId = LOWORD(wParam);

        switch (ctrlId) {
        case IDOK:
        case IDCANCEL:
            EndDialog();
            break;
        }
        return 1;
    }
};

class CTestModalWindow : public CMetroWindow
{
public:
    CTestModalWindow(HINSTANCE hInstance) : CMetroWindow(hInstance)
    {
    }

    virtual ~CTestModalWindow(void)
    {
    }

    virtual LPCTSTR GetWindowClassName() const
    {
        return _T("TestModalMetroWindow");
    }

    bool ShowDialog(HWND hWndParent)
    {
        ::EnableWindow(hWndParent, FALSE);

        ShowWindow();

        bool loop = true;

        MSG msg;

        while (loop && GetMessage(&msg, NULL, 0, 0))
	    {
            if (msg.message == WM_CLOSE) loop = false;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	    }

        EnableWindow(hWndParent, TRUE);
        SetForegroundWindow(hWndParent);

        return true;
    }
};

class CTestWindow : public CMetroWindow
{
public:
    CTestWindow(HINSTANCE hInstance) : CMetroWindow(hInstance)
    {
    }

    virtual ~CTestWindow(void)
    {
    }

    virtual LPCTSTR GetWindowClassName() const
    {
        return _T("TestMetroWindow");
    }
};

class CMainWindow : public CMetroWindow
{
public:
    CMainWindow(HINSTANCE hInstance) : CMetroWindow(hInstance), _modellessWindow(NULL)
    {
    }

    virtual ~CMainWindow(void)
    {
        delete _modellessWindow;
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

        CreateWindow(L"BUTTON", L"显示模态对话框 - 原生",
                              WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                              100, 50, 200, 24,
                              GetHWnd(), (HMENU)IDC_BTN_TEST1, GetModuleInstance(), 0);

        CreateWindow(L"BUTTON", L"显示模态窗口 - 模拟",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    100, 100, 200, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST2, GetModuleInstance(), 0);

        CreateWindow(L"BUTTON", L"显示非模态窗口 - 模拟",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    100, 150, 200, 24,
                    GetHWnd(), (HMENU)IDC_BTN_TEST3, GetModuleInstance(), 0);

        return result;
    }

    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDC_BTN_TEST1)
        {
            CTestDialog testWidnow(GetModuleInstance());
            testWidnow.DoModal(IDD_DIALOG1, GetHWnd());
            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST2)
        {
            CTestModalWindow testWindow(GetModuleInstance());
            testWindow.Create(*this, L"ModalWindow", WS_OVERLAPPEDWINDOW, 0);
            testWindow.ShowDialog(*this);
            bHandled = TRUE;
        }
        else if (wmId == IDC_BTN_TEST3)
        {
            if (_modellessWindow == NULL)
            {
                _modellessWindow = new CTestWindow(GetModuleInstance());
                _modellessWindow->Create(*this, L"ModalLessWindow", WS_OVERLAPPEDWINDOW, 0);
            }
            
            _modellessWindow->ShowWindow();

            bHandled = TRUE;
        }

        return 0;
    }

private:
    CTestWindow* _modellessWindow;
};


