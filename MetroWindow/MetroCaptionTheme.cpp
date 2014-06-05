#include "stdafx.h"
#include "MetroCaptionTheme.h"
#include "resource.h"
#include "lpng.h"

namespace MetroWindow
{

HBITMAP CMetroCaptionTheme::background_image_ = NULL;
HBITMAP CMetroCaptionTheme::minimize_button_image_ = NULL;
HBITMAP CMetroCaptionTheme::maximize_button_image_ = NULL;
HBITMAP CMetroCaptionTheme::restore_button_image_ = NULL;
HBITMAP CMetroCaptionTheme::fullscreen_button_image_ = NULL;
HBITMAP CMetroCaptionTheme::close_button_image_ = NULL;
HBITMAP CMetroCaptionTheme::size_grap_image_ = NULL;

CMetroCaptionTheme::CMetroCaptionTheme()
{
    Reset(RGB(255,255,255));
}

CMetroCaptionTheme::CMetroCaptionTheme(COLORREF color)
{
    Reset(color);
}

CMetroCaptionTheme::~CMetroCaptionTheme(void)
{
}

void CMetroCaptionTheme::Reset(COLORREF captionColor)
{
    border_active_color_ = RGB(153, 153, 153);
    border_inactive_color_ = RGB(193, 193, 193);
    button_press_color_ = RGB(78, 166, 234);
    background_image_ = NULL;
    SetCaptionColor(captionColor);
}

void CMetroCaptionTheme::SetCaptionColor(COLORREF captionColor)
{
    if (caption_color_ != captionColor)
    {
        caption_color_ = captionColor;
        caption_inactive_color_ = ChangeColorBrightness(captionColor, 0.4f, 102);
        button_hover_color_ = BlendColors(captionColor, RGB(226, 226, 226), 153);

        if (GetGValue(caption_color_) > 9 ||
            (299 * GetRValue(caption_color_) + 587 * GetGValue(caption_color_) + 114 * GetBValue(caption_color_)) / 1000 > 125)
        {
            color_style_ = ThemeColorLight;
        }
        else
        {
            color_style_ = ThemeColorDark;
        }

        caption_text_color_ = (color_style_ == ThemeColorDark) ? RGB(255,255,255) : RGB(0,0,0);
        caption_text_inactive_color_ = BlendColors(caption_text_color_, RGB(255, 255, 255), 102);
        //NotifyThemeChanged("CaptionColor");
    }
}

void CMetroCaptionTheme::LoadBitmapFromResource(HINSTANCE hInstance)
{
    minimize_button_image_ = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_MIN), L"PNG", hInstance, TRUE);
    maximize_button_image_ = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_MAX), L"PNG", hInstance, TRUE);
    restore_button_image_ = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_SHRINK), L"PNG", hInstance, TRUE);
    fullscreen_button_image_ = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_FULLSCREEN), L"PNG", hInstance, TRUE);
    close_button_image_ = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_CLOSE), L"PNG", hInstance, TRUE);
    size_grap_image_ = LoadPng(MAKEINTRESOURCE(IDB_RESIZE), L"PNG", hInstance, TRUE);
}

void CMetroCaptionTheme::FreeResources()
{
    if (minimize_button_image_) { ::DeleteObject(minimize_button_image_); minimize_button_image_ = NULL; }
    if (maximize_button_image_) { ::DeleteObject(maximize_button_image_); maximize_button_image_ = NULL; }
    if (restore_button_image_) { ::DeleteObject(restore_button_image_); restore_button_image_ = NULL; }
    if (fullscreen_button_image_) { ::DeleteObject(fullscreen_button_image_); fullscreen_button_image_ = NULL; }
    if (close_button_image_) { ::DeleteObject(close_button_image_); close_button_image_ = NULL; }
    if (size_grap_image_) { ::DeleteObject(size_grap_image_); size_grap_image_ = NULL; }
}

COLORREF CMetroCaptionTheme::ChangeColorBrightness(COLORREF color, float factor)
{
    return ChangeColorBrightness(color, factor, 0);
}

COLORREF CMetroCaptionTheme::ChangeColorBrightness(COLORREF color, float factor, int alpha)
{
    //TODO: Use HLSColor to make lighter
    float red = GetRValue(color);
    float green = GetGValue(color);
    float blue = GetBValue(color);

    if (factor < 0)
    {
        factor = 1 + factor;
        red *= factor;
        green *= factor;
        blue *= factor;
    }
    else
    {
        red = (255 - red) * factor + red;
        green = (255 - green) * factor + green;
        blue = (255 - blue) * factor + blue;
    }

    COLORREF brightColor = RGB(red,green,blue);

    if (alpha > 0)
    {
        return BlendColors(color, brightColor, alpha);
    }
    else
    {
        return brightColor;
    }
}

COLORREF CMetroCaptionTheme::BlendColors(COLORREF baseColor, COLORREF overColor, int alpha)
{
    BYTE red = GetRValue(baseColor);
    BYTE green = GetGValue(baseColor);
    BYTE blue = GetBValue(baseColor);

    red   = ((GetRValue(overColor) - red) * alpha + (red << 8)) >> 8;
    green = ((GetGValue(overColor) - green) * alpha + (green << 8)) >> 8;
    blue  = ((GetBValue(overColor) - blue) * alpha + (blue << 8)) >> 8;

    return RGB(red,green,blue);
}

}; //namespace MetroWindow
