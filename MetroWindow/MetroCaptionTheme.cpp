#include "stdafx.h"
#include "MetroCaptionTheme.h"
#include "resource.h"
#include "lpng.h"

namespace MetroWindow
{

HBITMAP CMetroCaptionTheme::_backgroundImage = NULL;
HBITMAP CMetroCaptionTheme::_minimizeButtonImage = NULL;
HBITMAP CMetroCaptionTheme::_maximizeButtonImage = NULL;
HBITMAP CMetroCaptionTheme::_restoreButtonImage = NULL;
HBITMAP CMetroCaptionTheme::_fullscreenButtonImage = NULL;
HBITMAP CMetroCaptionTheme::_closeButtonImage = NULL;
HBITMAP CMetroCaptionTheme::_sizeGrapImage = NULL;

CMetroCaptionTheme::CMetroCaptionTheme(COLORREF color)
{
	Reset(color);
}


CMetroCaptionTheme::~CMetroCaptionTheme(void)
{
}

void CMetroCaptionTheme::Reset(COLORREF captionColor)
{
    _activeBorder = RGB(153, 153, 153);
	_inactiveBorder = RGB(193, 193, 193);
    _buttonHoverColor = BlendColors(captionColor, RGB(226, 226, 226), 153);
    _buttonPressColor = RGB(78, 166, 234);
    _backgroundImage = NULL;
    SetCaptionColor(captionColor);
}

void CMetroCaptionTheme::SetCaptionColor(COLORREF captionColor)
{
	if (_captionColor != captionColor)
	{
		_captionColor = captionColor;

		if (GetGValue(_captionColor) > 9 ||
			(299 * GetRValue(_captionColor) + 587 * GetGValue(_captionColor) + 114 * GetBValue(_captionColor)) / 1000 > 125)
		{
			_colorStyle = ThemeColorStyle::Light;
		}
		else
		{
			_colorStyle = ThemeColorStyle::Dark;
		}

		_captionTextColor = (_colorStyle == ThemeColorStyle::Dark) ? RGB(255,255,255) : RGB(0,0,0);
		//NotifyThemeChanged("CaptionColor");
	}
}

void CMetroCaptionTheme::LoadBitmapFromResource(HINSTANCE hInstance)
{
    _minimizeButtonImage = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_MIN), L"PNG", hInstance, TRUE);
    _maximizeButtonImage = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_MAX), L"PNG", hInstance, TRUE);
    _restoreButtonImage = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_SHRINK), L"PNG", hInstance, TRUE);
    _fullscreenButtonImage = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_FULLSCREEN), L"PNG", hInstance, TRUE);
    _closeButtonImage = LoadPng(MAKEINTRESOURCE(IDB_CAPTION_CLOSE), L"PNG", hInstance, TRUE);
    _sizeGrapImage = LoadPng(MAKEINTRESOURCE(IDB_RESIZE), L"PNG", hInstance, TRUE);
}

void CMetroCaptionTheme::FreeResources()
{
    if (_minimizeButtonImage) { ::DeleteObject(_minimizeButtonImage); _minimizeButtonImage = NULL; }
    if (_maximizeButtonImage) { ::DeleteObject(_maximizeButtonImage); _maximizeButtonImage = NULL; }
    if (_restoreButtonImage) { ::DeleteObject(_restoreButtonImage); _restoreButtonImage = NULL; }
    if (_fullscreenButtonImage) { ::DeleteObject(_fullscreenButtonImage); _fullscreenButtonImage = NULL; }
    if (_closeButtonImage) { ::DeleteObject(_closeButtonImage); _closeButtonImage = NULL; }
    if (_sizeGrapImage) { ::DeleteObject(_sizeGrapImage); _sizeGrapImage = NULL; }
}

COLORREF CMetroCaptionTheme::ChangeColorBrightness(COLORREF color, float factor)
{
	return ChangeColorBrightness(color, factor, 0);
}

COLORREF CMetroCaptionTheme::ChangeColorBrightness(COLORREF color, float factor, int alpha)
{
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);

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
