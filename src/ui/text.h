// This file contains helper functions for drawing text on the screen
#ifndef UI_TEXT_H
#define UI_TEXT_H

#include <raylib.h>

// drawTextf is like printf but instead of printing to the console
// draws text on the screen at given position.
void drawTextf(int x, int y, int font_size, Color color, const char *format,
               ...);

#endif
