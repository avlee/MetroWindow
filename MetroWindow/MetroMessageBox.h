#pragma once

#include "MetroFrame.h"

namespace MetroWindow
{

typedef struct
{
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
    DWORD ctype;
} DialogItemTemplate;
    
class METROWINDOW_DECL CMetroMessageBox : public CMetroFrame
{
public:
    CMetroMessageBox(HINSTANCE hInstance);
    virtual ~CMetroMessageBox(void);

    INT_PTR Show(HWND hWndParent, LPCTSTR lpszMessage, LPCTSTR lpszCaption = NULL, UINT uType = MB_OK);

    virtual LRESULT OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    int PixelToDluX(int dluX);
    int PixelToDluY(int dluY);
    int AddButton(UINT nID, int x, int y);
    void AddItem(DWORD cType, UINT nID, RECT *pRect);
    static INT_PTR CALLBACK MsgBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HICON _hIcon;
    HFONT _hFont;
    bool _rightJustifyButtons;
    bool _disableClose;
    int _buttonWidth;
    int _buttonHeight;
    int _buttonCount;
    int _defaultButton;
    UINT _defaultButtonId;
    int _baseUnitX;
    int _baseUnitY;
    DLGTEMPLATE _dlgTempl;
    DialogItemTemplate _items[5];
};

} // namespace MetroWindow
