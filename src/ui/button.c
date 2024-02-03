#include "button.h"

#include "types.h"

internal inline int buttonSpacing(int font_size) {
  // NOTE(nk2ge5k): this spacing is borrowed from raylib source code
  // I do not fully understand why division by 10.
  return font_size / 10;
}

// buttonFromText creates new button from the text.
Button buttonFromText(char *text, int font_size, Vector2 position,
                      float padding) {

  Vector2 size = MeasureTextEx(GetFontDefault(), text, font_size,
                               buttonSpacing(font_size));

  Button button = {
      .text = text,
      .font_size = font_size,
      .padding = padding,
      .rect =
          {
              .x = position.x,
              .y = position.y,
              .width = size.x + (padding * 2),
              .height = size.y + (padding * 2),
          },
  };

  return button;
}

// buttonDraw draws button on the screen.
void buttonDraw(Button *button, Color bg, Color stroke, Color font_color) {
  Vector2 text_pos = {
      .x = button->rect.x + button->padding,
      .y = button->rect.y + button->padding,
  };

  DrawRectangleRec(button->rect, bg);
  DrawTextEx(GetFontDefault(),                 // font
             button->text,                     // text
             text_pos,                         // positon
             button->font_size,                // font size
             buttonSpacing(button->font_size), // spacing
             font_color                        // color
  );

  DrawRectangleLinesEx(button->rect, 2, stroke);
}

// buttonIsCollidedV check if button is collided with vector.
bool buttonIsCollidedV(Button *button, Vector2 vec) {
  return CheckCollisionPointRec(vec, button->rect);
}
