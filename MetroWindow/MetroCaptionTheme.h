#pragma once

#ifdef METROWINDOW_EXPORTS
#   define METROWINDOW_DECL __declspec (dllexport)
#else
#   define METROWINDOW_DECL __declspec (dllimport)
#endif

namespace MetroWindow
{

enum ThemeColorStyle
{
    ThemeColorLight,
    ThemeColorDark
};

class METROWINDOW_DECL CMetroCaptionTheme
{
public:
    CMetroCaptionTheme();
    explicit CMetroCaptionTheme(COLORREF color);
    ~CMetroCaptionTheme(void);

    void Reset(COLORREF captionColor);
    static void LoadBitmapFromResource(HINSTANCE hInstance);
    static void FreeResources();

    COLORREF GetCaptionColor() { return caption_color_; }
    void SetCaptionColor(COLORREF captionColor);

    COLORREF ActiveBorderColor() { return border_active_color_; }

    COLORREF InactiveCaptionColor() { return caption_inactive_color_; }
    COLORREF InactiveBorderColor() { return border_inactive_color_; }
    COLORREF CaptionTextColor() { return caption_text_color_; }
    COLORREF InactiveCaptionTextColor() { return caption_text_inactive_color_; }
    COLORREF ButtonHoverColor() { return button_hover_color_; }
    COLORREF ButtonPressColor() { return button_press_color_; }
    HBITMAP MinimizeButton() { return minimize_button_image_; }
    HBITMAP MaximizeButton() { return maximize_button_image_; }
    HBITMAP RestoreButton() { return restore_button_image_; }
    HBITMAP FullscreenButton() { return fullscreen_button_image_; }
    HBITMAP CloseButton() { return close_button_image_; }
    HBITMAP SizeGrap() { return size_grap_image_; }

private:
    COLORREF ChangeColorBrightness(COLORREF color, float factor);
    COLORREF ChangeColorBrightness(COLORREF color, float factor, int alpha);
    COLORREF BlendColors(COLORREF baseColor, COLORREF overColor, int alpha);

private:
    static HBITMAP background_image_;
    static HBITMAP minimize_button_image_;
    static HBITMAP maximize_button_image_;
    static HBITMAP restore_button_image_;
    static HBITMAP fullscreen_button_image_;
    static HBITMAP close_button_image_;
    static HBITMAP size_grap_image_;

    ThemeColorStyle color_style_;

    COLORREF caption_color_;
    COLORREF caption_inactive_color_;
    COLORREF border_active_color_;
    COLORREF border_inactive_color_;
    COLORREF caption_text_color_;
    COLORREF caption_text_inactive_color_;
    COLORREF button_hover_color_;
    COLORREF button_press_color_;
};

} //namespace MetroWindow
