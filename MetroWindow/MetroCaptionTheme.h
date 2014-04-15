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

	COLORREF GetCaptionColor() { return _captionColor; }
	void SetCaptionColor(COLORREF captionColor);

    COLORREF ActiveBorderColor() { return _activeBorder; }

    COLORREF InactiveCaptionColor() { return _inactiveCaptionColor; }
    COLORREF InactiveBorderColor() { return _inactiveBorder; }
    COLORREF CaptionTextColor() { return _captionTextColor; }
    COLORREF InactiveCaptionTextColor() { return _inactiveCaptionTextColor; }
    COLORREF ButtonHoverColor() { return _buttonHoverColor; }
    COLORREF ButtonPressColor() { return _buttonPressColor; }
    HBITMAP MinimizeButton() { return _minimizeButtonImage; }
    HBITMAP MaximizeButton() { return _maximizeButtonImage; }
    HBITMAP RestoreButton() { return _restoreButtonImage; }
    HBITMAP FullscreenButton() { return _fullscreenButtonImage; }
    HBITMAP CloseButton() { return _closeButtonImage; }
    HBITMAP SizeGrap() { return _sizeGrapImage; }

private:
	COLORREF ChangeColorBrightness(COLORREF color, float factor);
	COLORREF ChangeColorBrightness(COLORREF color, float factor, int alpha);
	COLORREF BlendColors(COLORREF baseColor, COLORREF overColor, int alpha);

private:
    static HBITMAP _backgroundImage;
    static HBITMAP _minimizeButtonImage;
    static HBITMAP _maximizeButtonImage;
    static HBITMAP _restoreButtonImage;
    static HBITMAP _fullscreenButtonImage;
    static HBITMAP _closeButtonImage;
    static HBITMAP _sizeGrapImage;

	ThemeColorStyle _colorStyle;

    COLORREF _captionColor;
    COLORREF _inactiveCaptionColor;
    COLORREF _activeBorder;
    COLORREF _inactiveBorder;
    COLORREF _captionTextColor;
    COLORREF _inactiveCaptionTextColor;
    COLORREF _buttonHoverColor;
    COLORREF _buttonPressColor;
};

} //namespace MetroWindow
