#include "stdafx.h"
#include "CaptionButton.h"
#include "MiscWapppers.h"
#include "WindowExtenders.h"

namespace MetroWindow
{

CCaptionButton::CCaptionButton(LONG hitTest, CMetroCaptionTheme& theme)
    : theme_(theme), hit_test_(hitTest), image_(NULL)
{
    pressed_ = false;
    hovered_ = false;
    enabled_ = true;
    visible_ = true;
}


CCaptionButton::~CCaptionButton(void)
{
}

void CCaptionButton::Draw(HDC hdc)
{
    if (!Visible()) return;

    HBITMAP image = image_;
    CRect srcRect(0, 0, 14, 14);

    if (pressed_)
    {
        FillSolidRect(hdc, &bounds_, theme_.ButtonPressColor());
        srcRect.OffsetRect(28, 0);
    }
    else if (hovered_)
    {
        FillSolidRect(hdc, &bounds_, theme_.ButtonHoverColor());
        srcRect.OffsetRect(14, 0);
    }

    if (image != NULL)
    {
        CRect destRect;
        destRect.top = bounds_.top + (Height() - srcRect.Height()) / 2;
        destRect.left = bounds_.left + (Width() - srcRect.Width()) / 2 + 1;
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
    : min_button_(NULL), max_button_(NULL)
{
}

CCaptionButtonManager::~CCaptionButtonManager()
{
    min_button_ = NULL;
    max_button_ = NULL;

    std::vector<CCaptionButton *>::const_iterator iter = caption_buttons_.begin();
    std::vector<CCaptionButton *>::const_iterator end = caption_buttons_.end();
    for (; iter != end; ++iter)
        delete *iter;
}

void CCaptionButtonManager::UpdateCaptionButtons(HWND hWnd, CMetroCaptionTheme& captionTheme, bool dwmEnabled)
{
    // create buttons
    if (caption_buttons_.size() == 0)
    {
        CCaptionButton * closeButton = new CCaptionButton(HTCLOSE, captionTheme);
        caption_buttons_.push_back(closeButton);

        closeButton->Image(captionTheme.CloseButton());

        if (WindowExtenders::IsDrawMaximizeBox(hWnd))
        {
            max_button_ = new CCaptionButton(HTMAXBUTTON, captionTheme);
            caption_buttons_.push_back(max_button_);

            max_button_->Image(::IsZoomed(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MaximizeButton());
        }

        if (WindowExtenders::IsDrawMinimizeBox(hWnd))
        {
            min_button_ = new CCaptionButton(HTMINBUTTON, captionTheme);
            caption_buttons_.push_back(min_button_);

            min_button_->Image(::IsIconic(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MinimizeButton());
        }

        // add command handlers
        //foreach (CaptionButton button in caption_buttons_)
        //    button.PropertyChanged += OnCommandButtonPropertyChanged;
    }
    else
    {
        if (min_button_ != NULL)
        {
            min_button_->Image(::IsIconic(hWnd) ?
                captionTheme.RestoreButton() : captionTheme.MinimizeButton());
        }

        if (max_button_ != NULL)
        {
            max_button_->Image(::IsZoomed(hWnd) ?
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
    for (btnIter = caption_buttons_.begin(); btnIter != caption_buttons_.end(); btnIter++)
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
    for (btnIter = caption_buttons_.begin(); btnIter != caption_buttons_.end(); btnIter++)
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

void CCaptionButtonManager::EnableButton(LONG hitTest, bool enable)
{
    std::vector<CCaptionButton *>::iterator btnIter;
    for (btnIter = caption_buttons_.begin(); btnIter != caption_buttons_.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button->HitTest() == hitTest)
        {
            button->Enabled(enable);
            break;
        }
    }
}

CCaptionButton * CCaptionButtonManager::CommandButtonFromPoint(POINT point)
{
    CCaptionButton * foundButton = NULL;

    std::vector<CCaptionButton *>::const_iterator btnIter;
    for (btnIter = caption_buttons_.begin(); btnIter != caption_buttons_.end(); btnIter++)
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

    std::vector<CCaptionButton *>::const_iterator btnIter;
    for (btnIter = caption_buttons_.begin(); btnIter != caption_buttons_.end(); btnIter++)
    {
        CCaptionButton* button = *btnIter;
        if (button != NULL && button->Visible() && button->Enabled() && button->HitTest() == hitTest)
        {
            foundButton = *btnIter;
        }
    }

    return foundButton;
}

} //namespace MetroWindow
