/*
 * @file il3820_drawLine.c
 *
 * @author Matthew Matz & Roy Eltham
 *
 * @version 0.5
 *
 * @copyright Copyright (C) Parallax, Inc. 2018.  See end of file for
 * terms of use (MIT License).
 *
 * @brief Waveshare ePaper display driver component, see il3820_h. for documentation.
 *
 * @detail Please submit bug reports, suggestions, and improvements to
 * this code to editor@parallax.com.
 */


#include "il3820.h"


void il3820_drawFastHLine(screen *dev, int x, int y, int w, int color) {
    for (int h = 0; h < w; h++) {
        il3820_drawPixel(dev, x + h, y, color);
    }
}

void il3820_drawFastVLine(screen *dev, int x, int y, int w, int color) {
    for (int h = 0; h < w; h++) {
        il3820_drawPixel(dev, x, y + h, color);
    }
}


/**
 * TERMS OF USE: MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

