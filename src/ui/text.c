#include "ui/text.h"

#include <stdio.h>

#define MAX_TEXT_LENGTH 1024

void drawTextf(int x, int y, int font_size, Color color, const char *format,
               ...) {
  char buf[MAX_TEXT_LENGTH + 1];

  va_list argptr;
  va_start(argptr, format);
  vsnprintf(buf, MAX_TEXT_LENGTH, format, argptr);
  va_end(argptr);

  DrawText(buf, x, y, font_size, color);
}
