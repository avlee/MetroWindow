#ifndef METROWINDOW_WINDOWWND_H__
#define METROWINDOW_WINDOWWND_H__

#pragma once

#include "MetroFrame.h"

namespace MetroWindow
{

class METROWINDOW_DECL CMetroWindow : public CMetroFrame
{
public:
    CMetroWindow(HINSTANCE hInstance);
    virtual ~CMetroWindow(void);

    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT& rc, HMENU hMenu = NULL);
    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = 400, int cy = 300, HMENU hMenu = NULL);
    
    void ShowWindow(bool bShow = true, bool bTakeFocus = true);
    void CenterWindow();

protected:
    virtual LPCTSTR GetWindowClassName() const = 0;
    virtual UINT GetClassStyle() const;

private:
    bool RegisterWindowClass();
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}; //namespace MetroWindow

#endif //METROWINDOW_WINDOWWND_H__

