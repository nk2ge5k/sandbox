/// File contains some helpers for creating UI buttons.
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <raylib.h>

#include "types.h"

typedef struct Button {
  char *text;
  i32 font_size;
  f32 padding;
  Rectangle rect;
} Button;

// buttonFromText creates new button from the text.
Button buttonFromText(char *text, i32 font_size, Vector2 position, f32 padding);

// buttonDraw draws button on the screen.
void buttonDraw(Button *button, Color bg, Color stroke, Color font_color);

// buttonIsCollidedV check if button is collided with vector.
bool buttonIsCollidedV(Button *button, Vector2 vec);

#endif
