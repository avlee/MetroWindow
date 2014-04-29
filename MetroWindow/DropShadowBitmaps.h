#pragma once

namespace MetroWindow
{

enum ShadowSide
{
    Left,
    Top,
    Right,
    Bottom
};

class DropShadowBitmaps
{
public:
    explicit DropShadowBitmaps(COLORREF color);
    ~DropShadowBitmaps(void);

    void Initialize();
    void MakeShadow(HDC hdc, int width, int height, ShadowSide side) const;

    int GetShadowSize() const;

private:
    void DeleteBitmaps();
    HBITMAP CreateBitmap(int width, int height, void ** ppvBits) const;
    HBITMAP BuildShadowImage(int rotation, int offsetX, int offsetY, bool corner) const;
    BYTE GetAlpha(int x, int y, int width, int height,
        int rotation, int offsetX, int offsetY, bool corner) const;
    void DrawShadowCorner(HDC hdc, HBITMAP image, int x, int y, int width, int height) const;
    void DrawShadowBorder(HDC hdc, HBITMAP image, int x, int y, int width, int height) const;

private:
    COLORREF color_;
    HBITMAP corner_nw_;
    HBITMAP corner_ne_;
    HBITMAP corner_se_;
    HBITMAP corner_sw_;
    HBITMAP border_n_;
    HBITMAP border_e_;
    HBITMAP border_s_;
    HBITMAP border_w_;
    bool initialized_;
};

} //namespace MetroWindow

