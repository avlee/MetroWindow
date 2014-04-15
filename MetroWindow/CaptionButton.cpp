#include "stdafx.h"
#include "CaptionButton.h"
#include "MiscWapppers.h"
#include "WindowExtenders.h"

namespace MetroWindow
{

CCaptionButton::CCaptionButton(LONG hitTest, CMetroCaptionTheme& theme)
	: _theme(theme), _hitTest(hitTest), _image(NULL)
{
    _pressed = false;
    _hovered = false;
    _enabled = true;
    _visible = true;
}


CCaptionButton::~CCaptionButton(void)
{
}

void CCaptionButton::Draw(HDC hdc)
{
    if (!Visible()) return;

    HBITMAP image = _image;
    CRect srcRect(0, 0, 14, 14);

    if (_pressed)
    {
        FillSolidRect(hdc, &_bounds, _theme.ButtonPressColor());
        srcRect.OffsetRect(28, 0);
    }
    else if (_hovered)
    {
        FillSolidRect(hdc, &_bounds, _theme.ButtonHoverColor());
        srcRect.OffsetRect(14, 0);
    }

    if (image != NULL)
    {
        CRect destRect;
        destRect.top = _bounds.top + (Height() - srcRect.Height()) / 2;
        destRect.left = _bounds.left + (Width() - srcRect.Width()) / 2 + 1;
        destRect.Width(srcRect.Width());
        destRect.Height(srcRect.Height());

        HDC hdcBmpMem = ::CreateCompatibleDC(hdc);
        HBITMAP hbmOldBmp = (HBITMAP)::SelectObject(hdcBmpMem, image);

        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
        ::GdiAlphaBlend(hdc, destRect.left, destRect.top, destRect.Width(), destRect.Height(), 
            hdcBmpMem, srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(), bf);

        ::SelectObject(hdcBmpMem, hbmOldBmp);
	    ::DeleteDC(hdcBmpMem);
    }
}

void CCaptionButton::FillSolidRect(HDC hdc, LPCRECT lpRect, COLORREF clr)
{
    COLORREF clrOld = ::SetBkColor(hdc, clr);
    ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
    ::SetBkColor(hdc, clrOld);

    //HBRUSH hBrush = ::CreateSolidBrush(clr);
    //::FillRect(hdc, lpRect, hBrush);
    //::DeleteObject(hBrush);
}

CCaptionButtonManager::CCaptionButtonManager()
    : _minButton(NULL), _maxButton(NULL)
{
}

CCaptionButtonManager::~CCaptionButtonManager()
{
    _minButton = NULL;
    _maxButton = NULL;

    std::vector<CCaptionButton *>::const_iterator iter = _captionButtons.begin();
    std::vector<CCaptionButton *>::const_iterator end = _captionButtons.end();
    for (; iter != end; ++iter)
        delete *iter;
}

void CCaptionButtonManager::UpdateCaptionButtons(HWND hWnd, CMetroCaptionTheme& captionTheme, bool dwmEnabled)
{
    // create buttons
    if (_captionButtons.size() == 0)
    {
        CCaptionButton * closeButton = new CCaptionButton(HTCLOSE, captionTheme);
        _captionButtons.push_back(closeButton);

        closeButton->Image(captionTheme.CloseButton());

        if (WindowExtenders::IsDrawMaximizeBox(hWnd))
        {
            _maxButton = new CCaptionButton(HTMAXBUTTON, captionTheme);
            _captionButtons.push_back(_maxButton);

            _maxButton->Image(::IsZoomed(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MaximizeButton());
        }

        if (WindowExtenders::IsDrawMinimizeBox(hWnd))
        {
            _minButton = new CCaptionButton(HTMINBUTTON, captionTheme);
            _captionButtons.push_back(_minButton);

            _minButton->Image(::IsIconic(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MinimizeButton());
        }

        // add command handlers
        //foreach (CaptionButton button in _captionButtons)
        //    button.PropertyChanged += OnCommandButtonPropertyChanged;
    }
    else
    {
        if (_minButton != NULL)
        {
            _minButton->Image(::IsIconic(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MinimizeButton());
        }

        if (_maxButton != NULL)
        {
            _maxButton->Image(::IsZoomed(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MaximizeButton());
        }
    }

    // Calculate Caption Button Bounds
    CRect rect;
    ::GetWindowRect(hWnd, &rect);
    rect.OffsetRect(-rect.left, -rect.top);

    CSize borderSize = WindowExtenders::GetBorderSize(hWnd, dwmEnabled);
    CSize captionButtonSize = WindowExtenders::GetCaptionButtonSize(hWnd);

    CRect buttonRect;
    buttonRect.left = rect.right - borderSize.cx - captionButtonSize.cx;
    buttonRect.top = rect.top;
    buttonRect.Width(captionButtonSize.cx);
    buttonRect.Height(captionButtonSize.cy);

    // Do not overlap the frame border
    if (!dwmEnabled/* && captionTheme.BackgroundImage == null*/)
    {
        buttonRect.InflateRect(0, -1);
    }

    std::vector<CCaptionButton *>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button != NULL && button->Visible())
        {
            button->Bounds(buttonRect);
            buttonRect.MoveToX(buttonRect.left - captionButtonSize.cx);
        }
    }
}

int CCaptionButtonManager::Draw(HDC hdc)
{
    int width = 0;

    std::vector<CCaptionButton *>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button != NULL)
        {
            button->Draw(hdc);
            width += button->Width();
        }
    }

    return width;
}

CCaptionButton * CCaptionButtonManager::CommandButtonFromPoint(POINT point)
{
    CCaptionButton * foundButton = NULL;

    std::vector<CCaptionButton *>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button != NULL && button->Visible() && ::PtInRect(&button->Bounds(), point))
        {
            foundButton = *btnIter;
        }
    }

    return foundButton;
}

CCaptionButton * CCaptionButtonManager::CommandButtonByHitTest(LONG hitTest)
{
    CCaptionButton * foundButton = NULL;

    std::vector<CCaptionButton *>::iterator btnIter;
    for (btnIter = _captionButtons.begin(); btnIter != _captionButtons.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button != NULL && button->Visible() && button->HitTest() == hitTest)
        {
            foundButton = *btnIter;
        }
    }

    return foundButton;
}

} //namespace MetroWindow
