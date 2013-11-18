#pragma once

#include "MetroCaptionTheme.h"
#include "MetroRefPtr.h"

namespace MetroWindow
{

class CCaptionButton
{
public:
	CCaptionButton(LONG hitTest, CMetroCaptionTheme& theme);
	~CCaptionButton(void);

	void Draw(HDC hdc);

	RECT Bounds() const { return _bounds; }
	void Bounds(const RECT& bounds) { ::CopyRect(&_bounds, &bounds); }
    int Width() const { return _bounds.right - _bounds.left; }
    int Height() const { return _bounds.bottom - _bounds.top; }
    HBITMAP Image() const { return _image; }
	void Image(HBITMAP image) { _image = image; }
	bool Visible() const { return _visible; }
	void Visible(bool visible) { _visible = visible; }
	bool Enabled() const { return _enabled; }
	void Enabled(bool enabled) { _enabled = enabled; }
	bool Pressed() const { return _pressed; }
	void Pressed(bool pressed) { _pressed = pressed; }
	bool Hovered() const { return _hovered; }
	void Hovered(bool hovered) { _hovered = hovered; }
	LONG HitTest() const { return _hitTest; }

private:
    void FillSolidRect(HDC hdc, LPCRECT lpRect, COLORREF clr);

	IMPLEMENT_REFCOUNTING(CCaptionButton)
private:
	CMetroCaptionTheme& _theme; //TODO: Using smart ptr like CefRefPtr

	LONG _hitTest;
    HBITMAP _image;
    HBITMAP _hoverImage;
    RECT _bounds;
    bool _pressed;
    bool _hovered;
    bool _enabled;
    bool _visible;
};

} //namespace MetroWindow
