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
    HICON message_box_icon_;
    HFONT font_;
    bool right_justify_buttons_;
    bool disable_close_;
    int button_width_;
    int button_height_;
    int button_count_;
    int default_button_;
    UINT default_button_id_;
    int base_unit_x_;
    int base_unit_y_;
    DLGTEMPLATE dialog_templ_;
    DialogItemTemplate dialog_items_[5];
};

} // namespace MetroWindow
