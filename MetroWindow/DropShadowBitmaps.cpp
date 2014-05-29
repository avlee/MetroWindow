#include "stdafx.h"
#include "DropShadowBitmaps.h"

namespace MetroWindow
{

static const BYTE kShadowCorner[26][26] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 4, 4, 4, 4 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 7, 7, 7, 7, 7 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9, 9, 10, 10, 11, 12, 12 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9, 11, 11, 13, 13, 15, 15, 15, 16, 16 },
    { 0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18, 20, 20, 21, 22, 22, 23 },
    { 0, 0, 0, 1, 1, 1, 2, 3, 4, 6, 7, 9, 11, 13, 15, 16, 19, 20, 22, 24, 26, 27, 27, 29, 29, 29 },
    { 0, 0, 0, 1, 1, 2, 3, 4, 6, 8, 9, 11, 14, 17, 19, 22, 25, 27, 29, 31, 33, 35, 35, 37, 37, 37 },
    { 0, 0, 1, 1, 2, 3, 4, 6, 7, 10, 12, 15, 19, 22, 25, 28, 31, 34, 37, 39, 41, 43, 44, 45, 46, 46 },
    { 0, 0, 1, 1, 3, 4, 6, 8, 10, 13, 16, 19, 23, 27, 32, 37, 42, 46, 50, 53, 57, 59, 60, 61, 62, 63 },
    { 0, 1, 1, 2, 3, 5, 7, 9, 12, 16, 20, 25, 30, 36, 42, 51, 58, 66, 71, 75, 79, 81, 82, 84, 85, 86 },
    { 0, 1, 1, 3, 4, 6, 9, 11, 15, 19, 25, 31, 38, 47, 59, 70, 81, 92, 99, 104, 108, 110, 111, 113, 114, 115 },
    { 0, 1, 1, 3, 5, 8, 11, 14, 19, 23, 30, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 2, 4, 6, 9, 13, 17, 22, 27, 36, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 3, 4, 7, 11, 15, 19, 25, 32, 42, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 3, 5, 8, 12, 16, 22, 28, 37, 51, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 4, 6, 9, 14, 19, 25, 31, 42, 58, 81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 5, 7, 11, 15, 20, 27, 34, 46, 66, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 5, 8, 11, 17, 22, 29, 37, 50, 71, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 5, 9, 13, 18, 24, 31, 39, 53, 75, 104, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 3, 6, 9, 13, 20, 26, 33, 41, 57, 79, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 4, 7, 10, 15, 20, 27, 35, 43, 59, 81, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 4, 7, 10, 15, 21, 27, 35, 44, 60, 82, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 4, 7, 11, 15, 22, 29, 37, 45, 61, 84, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 4, 7, 12, 16, 22, 29, 37, 46, 62, 85, 114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 4, 7, 12, 16, 23, 29, 37, 46, 63, 86, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const BYTE kShadowBorder[12][1] = {
    { 2 },
    { 4 },
    { 7 },
    { 12 },
    { 16 },
    { 23 },
    { 29 },
    { 37 },
    { 46 },
    { 63 },
    { 86 },
    { 115 }
};

DropShadowBitmaps::DropShadowBitmaps(COLORREF color)
    : color_(color),
    corner_nw_(NULL),
    corner_ne_(NULL),
    corner_sw_(NULL),
    corner_se_(NULL),
    border_n_(NULL),
    border_e_(NULL),
    border_s_(NULL),
    border_w_(NULL),
    initialized_(false)
{
}


DropShadowBitmaps::~DropShadowBitmaps(void)
{
    DeleteBitmaps();
}

void DropShadowBitmaps::Initialize()
{
    if (!initialized_)
    {
        corner_nw_ = BuildShadowImage(0, 2, 2, true);
        corner_ne_ = BuildShadowImage(1, 2, 0, true);
        corner_se_ = BuildShadowImage(2, 0, 0, true);
        corner_sw_ = BuildShadowImage(3, 0, 2, true);
        border_n_ = BuildShadowImage(0, 0, 2, false);
        border_e_ = BuildShadowImage(1, 0, 0, false);
        border_s_ = BuildShadowImage(2, 0, 0, false);
        border_w_ = BuildShadowImage(3, 0, 2, false);

        initialized_ = true;
    }
}

void DropShadowBitmaps::DeleteBitmaps()
{
    if (corner_nw_ != NULL) { ::DeleteObject(corner_nw_); corner_nw_ = NULL; }
    if (corner_ne_ != NULL) { ::DeleteObject(corner_ne_); corner_ne_ = NULL; }
    if (corner_se_ != NULL) { ::DeleteObject(corner_se_); corner_se_ = NULL; }
    if (corner_sw_ != NULL) { ::DeleteObject(corner_sw_); corner_sw_ = NULL; }

    if (border_n_ != NULL) { ::DeleteObject(border_n_); border_n_ = NULL; }
    if (border_e_ != NULL) { ::DeleteObject(border_e_); border_e_ = NULL; }
    if (border_s_ != NULL) { ::DeleteObject(border_s_); border_s_ = NULL; }
    if (border_w_ != NULL) { ::DeleteObject(border_w_); border_w_ = NULL; }

     initialized_ = false;
}

void DropShadowBitmaps::MakeShadow(HDC hdc, int width, int height, ShadowSide side) const
{
    int cornerSize = arraysize(kShadowCorner);
    int borderWidth = arraysize(kShadowBorder);

    int top = 0;
    int left = 0;
    int right = width;
    int bottom = height;

    // The top and bottom borders extend over the sides of the window.
    // The left and right borders do no. This means that we need to
    // update the bounds to make it seem like the left and right
    // borders do extend outside of the window.
    if (side == Left || side == Right)
    {
        top -= borderWidth;
        bottom += borderWidth;
        height += borderWidth * 2;
    }

    // Left top corner
    if (side == Left || side == Top)
    {
        DrawShadowCorner(hdc, corner_nw_, left, top, cornerSize, cornerSize);
    }

    // Right top corner
    if (side == Right || side == Top)
    {
        DrawShadowCorner(hdc, corner_ne_, right - cornerSize, top, cornerSize, cornerSize);
    }

    // Left bottom corner
    if (side == Left || side == Bottom)
    {
        DrawShadowCorner(hdc, corner_sw_, left, bottom - cornerSize, cornerSize, cornerSize);
    }

    // Right bottom corner
    if (side == Right || side == Bottom)
    {
        DrawShadowCorner(hdc, corner_se_, right - cornerSize, bottom - cornerSize, cornerSize, cornerSize);
    }

    if (side == Top)
    {
        DrawShadowBorder(hdc, border_n_, left + cornerSize, top,
            width - cornerSize * 2, borderWidth);
    }
    else if (side == Bottom)
    {
        DrawShadowBorder(hdc, border_s_, left + cornerSize,
            bottom - borderWidth, width - cornerSize * 2, borderWidth);
    }
    else if (side == Left)
    {
        DrawShadowBorder(hdc, border_w_, left, top + cornerSize,
            borderWidth, height - cornerSize * 2);
    }
    else if (side == Right)
    {
        DrawShadowBorder(hdc, border_e_, right - borderWidth,
            top + cornerSize, borderWidth, height - cornerSize * 2);
    }
}

int DropShadowBitmaps::GetShadowSize() const
{
    return arraysize(kShadowBorder);
}

HBITMAP DropShadowBitmaps::CreateBitmap(int width, int height, void ** ppvBits) const
{
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * 4;
    
    HBITMAP bitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);

    ZeroMemory(*ppvBits, bmi.bmiHeader.biSizeImage);

    return bitmap;
}

HBITMAP DropShadowBitmaps::BuildShadowImage(int rotation, int offsetX, int offsetY, bool corner) const
{
    int height = corner ? arraysize(kShadowCorner) : arraysize(kShadowBorder);
    int width = corner ? arraysize(kShadowCorner[0]) : arraysize(kShadowBorder[0]);

    if (rotation % 2 == 1)
    {
        int tmp = width;
        width = height;
        height = tmp;
    }

    BYTE *pvBits;
    HBITMAP bitmap = CreateBitmap(width, height, (void **)&pvBits);

    BYTE r = GetRValue(color_);
    BYTE g = GetGValue(color_);
    BYTE b = GetBValue(color_);

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            BYTE alpha = GetAlpha(x, y, width, height, rotation, offsetX, offsetY, corner);

            int pos = y * width * 4  + x * 4;

            pvBits[pos]     = (b * alpha) >> 8;
            pvBits[pos + 1] = (g * alpha) >> 8;
            pvBits[pos + 2] = (r * alpha) >> 8;
            pvBits[pos + 3] = alpha;
        }
    }

    return bitmap;
}

BYTE DropShadowBitmaps::GetAlpha(int x, int y, int width, int height,
    int rotation, int offsetX, int offsetY, bool corner) const
{
    BYTE alpha = 0;
    int borderSize = arraysize(kShadowBorder);

    int xPos = 0;
    int yPos = 0;
    int signX = 1;
    int signY = 1;

    int n = rotation % 4;
    switch (n)
    {
    case 0:
        {
            xPos = x;
            yPos = y;
            signX = -1;
            signY = -1;
        }
        break;
    case 1:
        {
            xPos = y;
            yPos = (width - 1) - x;
            signX = -1;
            signY = 1;
        }
        break;
    case 2:
        {
            xPos = (width - 1) - x;
            yPos = (height - 1) - y;
            signX = 1;
            signY = 1;
        }
        break;
    case 3:
        {
            xPos = (height - 1) - y;
            yPos = x;
            signX = 1;
            signY = -1;
        }
        break;
    default:
        return alpha;
    }

    if (xPos < offsetX || yPos < offsetY)
        alpha = 0;
    else if (xPos >= borderSize && yPos >= borderSize)
        alpha = 0;
    else
    {
        xPos += offsetX * signX;
        yPos += offsetY * signY;

        alpha = corner ? kShadowCorner[yPos][xPos] : kShadowBorder[yPos][xPos];
    }

    return alpha;
}

void DropShadowBitmaps::DrawShadowCorner(HDC hdc, HBITMAP image, int x, int y, int width, int height) const
{
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    HBITMAP hbmOldBmp = (HBITMAP)::SelectObject(hdcMem, image);

    ::BitBlt(hdc, x, y, width, height, hdcMem, 0, 0, SRCCOPY);

    ::SelectObject(hdcMem, hbmOldBmp);
    ::DeleteDC(hdcMem);
}

void DropShadowBitmaps::DrawShadowBorder(HDC hdc, HBITMAP image, int x, int y, int width, int height) const
{
    HBRUSH hBrush = ::CreatePatternBrush(image);
    ::SetBrushOrgEx(hdc, x, y, NULL);

    RECT rect = {x, y, x + width, y + height};

    ::FillRect(hdc, &rect, hBrush);

    ::DeleteObject(hBrush);
}

} //namespace MetroWindow
