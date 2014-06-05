#pragma once

#include "MetroCaptionTheme.h"

#include <vector>

namespace MetroWindow
{

class CCaptionButton
{
public:
    CCaptionButton(LONG hitTest, CMetroCaptionTheme& theme);
    ~CCaptionButton(void);

    void Draw(HDC hdc);

    RECT Bounds() const { return bounds_; }
    void Bounds(const RECT& bounds) { ::CopyRect(&bounds_, &bounds); }
    int Width() const { return bounds_.right - bounds_.left; }
    int Height() const { return bounds_.bottom - bounds_.top; }
    HBITMAP Image() const { return image_; }
    void Image(HBITMAP image) { image_ = image; }
    bool Visible() const { return visible_; }
    void Visible(bool visible) { visible_ = visible; }
    bool Enabled() const { return enabled_; }
    void Enabled(bool enabled) { enabled_ = enabled; }
    bool Pressed() const { return pressed_; }
    void Pressed(bool pressed) { pressed_ = pressed; }
    bool Hovered() const { return hovered_; }
    void Hovered(bool hovered) { hovered_ = hovered; }
    LONG HitTest() const { return hit_test_; }

private:
    void FillSolidRect(HDC hdc, LPCRECT lpRect, COLORREF clr);

private:
    CMetroCaptionTheme& theme_; //TODO: Using smart ptr like std::shared_ptr

    LONG hit_test_;
    HBITMAP image_;
    HBITMAP hover_image_;
    RECT bounds_;
    bool pressed_;
    bool hovered_;
    bool enabled_;
    bool visible_;
};

class CCaptionButtonManager
{
public:
    CCaptionButtonManager();
    ~CCaptionButtonManager();

    void UpdateCaptionButtons(HWND hWnd, CMetroCaptionTheme& captionTheme, bool dwmEnabled);

    int Draw(HDC hdc);

    CCaptionButton * CommandButtonFromPoint(POINT point);
    CCaptionButton * CommandButtonByHitTest(LONG hitTest);

private:
    CCaptionButton * min_button_;
    CCaptionButton * max_button_;
    std::vector<CCaptionButton *> caption_buttons_;
};

} //namespace MetroWindow
