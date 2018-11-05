//
// Created by iain on 05/11/18.
//

#include "Graphics.h"

void Graphics::Circle(CScreenDevice &dev, int xc, int yc, int radius, TScreenColor color) {
    int x = 0;
    int y = radius;
    int delta = 2 - 2 * radius;
    int error = 0;

    while (y >= 0) {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);

        error = 2 * (delta + y) - 1;
        if (delta < 0 && error <= 0) {
            ++x;
            delta += 2 * x + 1;
            continue;
        }
        error = 2 * (delta - x) - 1;
        if (delta > 0 && error > 0) {
            --y;
            delta += 1 - 2 * y;
            continue;
        }
        ++x;
        delta += 2 * (x - y);
        --y;
    }
}

void Graphics::Line(CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color) {
    int dx = x1-x0, sx = x0<x1 ? 1 : -1;
    int dy = y1-y0, sy = y0<y1 ? 1 : -1;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int err = (dx>dy ? dx : -dy) >> 1, e2;

    for(;;){
        dev.SetPixel (x0, y0, color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void Graphics::Ellipse(CScreenDevice &dev, int xc, int yc, int width, int height, TScreenColor color) {
    int a2 = width * width;
    int b2 = height * height;
    int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;

    // Top and bottom
    for (x = 0, y = height, sigma = 2*b2+a2*(1-2*height); b2*x <= a2*y; x++) {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);
        if (sigma >= 0)
        {
            sigma += fa2 * (1 - y);
            y--;
        }
        sigma += b2 * ((4 * x) + 6);
    }

    // Left and right
    for (x = width, y = 0, sigma = 2*a2+b2*(1-2*width); a2*y <= b2*x; y++) {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);
        if (sigma >= 0)
        {
            sigma += fb2 * (1 - x);
            x--;
        }
        sigma += a2 * ((4 * y) + 6);
    }
}

void Graphics::Rectangle(CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color) {
    for (int nPosX = x0; nPosX < x1; nPosX++)
    {
        dev.SetPixel (nPosX, y0, color);
        dev.SetPixel (nPosX, y1, color);
    }
    for (int nPosY = y0; nPosY <= y1; nPosY++)
    {
        dev.SetPixel (x0, nPosY, color);
        dev.SetPixel (x1, nPosY, color);
    }
}


void push(int* x_stk, int* y_stk, int &slen, int nx, int ny) {
    if (slen > 100) return;
    x_stk[slen] = nx;
    y_stk[slen] = ny;
    slen++;
}

int pop(int* x_stk, int* y_stk, int &slen, int &ox, int &oy) {
    if (slen < 1) return 0;
    slen--;
    ox = x_stk[slen];
    oy = y_stk[slen];
    return 1;
}

void Graphics::FloodFill(CScreenDevice &dev, int x, int y, TScreenColor newColor) {
    TScreenColor oldColor = dev.GetPixel(x,y);
    if (newColor == oldColor) return;

    int x1;
    bool spanAbove, spanBelow;

    int x_stk[101];
    int y_stk[101];
    int slen = 0;

    int w = dev.GetWidth();
    int h = dev.GetHeight();
    if (w < 100) w = 640;
    if (h < 100) h = 360;

    push(x_stk, y_stk, slen, x, y);
    while(pop(x_stk, y_stk, slen, x, y))
    {
        x1 = x;
        while(x1 > 0 && dev.GetPixel(x1,y) == oldColor) x1--;
        x1++;
        spanAbove = spanBelow = 0;
        while(x1 < w && dev.GetPixel(x1,y) == oldColor)
        {
            dev.SetPixel(x1,y,newColor);
            if(!spanAbove && y > 0 && dev.GetPixel(x1,y-1) == oldColor){
                push(x_stk, y_stk, slen, x1, y - 1);
                spanAbove = 1;
            }
            else if(spanAbove && y > 0 && dev.GetPixel(x1,y-1) != oldColor) {
                spanAbove = 0;
            }
            if(!spanBelow && y < h - 1 && dev.GetPixel(x1,y+1) == oldColor){

                push(x_stk, y_stk, slen, x1, y + 1);
                spanBelow = 1;
            }
            else if(spanBelow && y < h - 1 && dev.GetPixel(x1,y+1) != oldColor) {
                spanBelow = 0;
            }
            x1++;
        }
    }
}
