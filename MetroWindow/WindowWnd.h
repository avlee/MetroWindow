#ifndef METROWINDOW_WINDOWWND_H__
#define METROWINDOW_WINDOWWND_H__

#pragma once

#include "stdafx.h"

namespace MetroWindow
{

class METROWINDOW_DECL CWindowWnd
{
public:
    CWindowWnd(HINSTANCE hInstance);
    virtual ~CWindowWnd(void);

    HWND GetHWnd() const { return _hWnd; }
	operator HWND() const { return _hWnd; }

	HINSTANCE GetModuleInstance() const { return _hInst; }

    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu = NULL);
    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = 400, int cy = 300, HMENU hMenu = NULL);
    
    void ShowWindow(bool bShow = true, bool bTakeFocus = true);
    UINT ShowModal();
    void Close(UINT nRet = IDOK);
    void CenterWindow();
    
    void SetIcon(UINT nIconRes, UINT nSmallIconRes = 0);

    HICON GetIcon() const { return _hIcon; }
    HICON GetSmallIcon() const { return _hSmallIcon; }

protected:
    virtual LPCTSTR GetWindowClassName() const = 0;
    virtual UINT GetClassStyle() const;
    virtual LRESULT OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass();
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE _hInst;
    HWND _hWnd;
    HICON _hIcon;
    HICON _hSmallIcon;
};

}; //namespace MetroWindow

#endif //METROWINDOW_WINDOWWND_H__

