#pragma once

#include <string>

#include "MetroCaptionTheme.h"

namespace MetroWindow
{

class CCaptionButton; // Forward declare
class CCaptionButtonManager;
class CDropShadow;

class METROWINDOW_DECL CMetroFrame
{
public:
    CMetroFrame(HINSTANCE hInstance);
    virtual ~CMetroFrame(void);

    HWND GetHWnd() const { return hWnd_; }
    operator HWND() const { return hWnd_; }

    HINSTANCE GetModuleInstance() const { return hInst_; }

    void SetBackgroundColor(COLORREF bgColor) { background_color_ = bgColor; }
    void SetCaptionColor(COLORREF captionColor) { caption_theme_.SetCaptionColor(captionColor); }
    void SetIcon(UINT nIconRes, UINT nSmallIconRes = 0);

    HICON GetIcon() const { return hIcon_; }
    HICON GetSmallIcon() const { return hIcon_small_; }

    void SetWindowTitle(const wchar_t* title);
    void SetMinSize(int cx, int cy);

    void ShowDropShadowOnXP(bool show);
    void ClientAreaMovable(bool movable) { client_area_movable_ = movable; }
    void UseCustomTitile(bool custom) { use_custom_title_ = custom; }
    void UseThickFrame(bool thick) { use_thick_frame_ = thick; }

    void EnableCloseButton(bool enable);
    void CenterWindow(HWND hWndCenter = NULL);

protected:
    virtual LRESULT OnDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnWmLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnNcMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnDwmCompositionChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    void RemoveWindowBorderStyle();
    BOOL ModifyWindowStyle(LONG removeStyle, LONG addStyle);
    BOOL PaintNonClientArea(HRGN hrgnUpdate);
    void DrawWindowFrame(HDC hdc, const RECT& bounds, const SIZE& borderSize, int captionHeight);
    void DrawCaptionTitle(HDC hdc, LPWSTR title, RECT bounds, COLORREF color);
    void DrawThemeCaptionTitleEx(HDC hdc, LPCWSTR title, const RECT& bounds, COLORREF color, COLORREF bgColor);
    int GetCaptionHeight();
    RECT GetFrameIconBounds(const RECT& windowBounds, SIZE borderSize);
    void FillSolidRect(HDC hdc, LPCRECT lpRect, COLORREF clr);
    void ShowSystemMenu(POINT point);

protected:
    HINSTANCE hInst_;
    HWND hWnd_;
    HICON hIcon_;
    HICON hIcon_small_;
    wchar_t title_[256];

private:
    HFONT caption_font_;
    COLORREF background_color_;
    SIZE min_size_;

    bool show_drop_shadow_on_xp_;
    bool is_dwm_enabled_;
    bool is_uxtheme_supported_;
    bool trace_nc_mouse_;
    bool is_non_client_area_active_;
    bool is_sizing_;
    bool is_sizable_;
    bool prepare_fullscreen_;
    bool is_fullscreen_;
    int caption_height_;

    bool use_custom_title_;
    bool client_area_movable_;
    bool use_thick_frame_;
    bool show_icon_on_caption_;
    bool close_button_enabled_;

    TRACKMOUSEEVENT track_mouse_event_;

    CCaptionButtonManager * caption_button_manager_;
    CMetroCaptionTheme caption_theme_;

    CDropShadow * drop_shadow_;

    CCaptionButton * pressed_button_;
    CCaptionButton * hovered_button_;
};

} //namespace MetroWindow
