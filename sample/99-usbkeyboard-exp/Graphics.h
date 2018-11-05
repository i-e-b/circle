//
// Created by iain on 05/11/18.
//

#ifndef USBKEYBOARD_GRAPHICS_H
#define USBKEYBOARD_GRAPHICS_H

#include <circle/screen.h>


class Graphics {
public:

    static void Line(CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color);
    static void Circle (CScreenDevice &dev, int xc, int yc, int radius, TScreenColor color);
    static void Ellipse (CScreenDevice &dev, int xc, int yc, int width, int height, TScreenColor color);
    static void Rectangle (CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color);

    static void FloodFill(CScreenDevice &dev, int x, int y, TScreenColor newColor);
};


#endif //USBKEYBOARD_GRAPHICS_H
