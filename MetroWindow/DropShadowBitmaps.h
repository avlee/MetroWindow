#pragma once

class DropShadowBitmaps
{
public:
    DropShadowBitmaps(void);
    ~DropShadowBitmaps(void);

    void Initialize();
    void MakeShadow(HDC hdc, int width, int height);

    int GetShadowSize() const;

private:
    void DeleteBitmaps();
    HBITMAP CreateBitmap(int width, int height, void ** ppvBits);
    HBITMAP BuildCorner(int rotation);
    HBITMAP BuildBorder(int rotation);
    void GetAlpha(int x, int y, int width, int height, int rotation, int* xPos, int* yPos);
    void DrawShadowCorner(HDC hdc, HBITMAP image, int x, int y, int width, int height);
    void DrawShadowBorder(HDC hdc, HBITMAP image, int x, int y, int width, int height);

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
};

