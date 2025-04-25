#include "raylib.h"

int main(void)
{
  const int screenW = 800, screenH = 450;
  InitWindow(screenW, screenH, "Donu");
  SetTargetFPS(60);

  const char *msg = "Hello, highlighted world!";
  const int fontSize = 32;
  const Vector2 pos = { 10, 10 };     // top-left corner for the text
  const int padding = 6;               // space around letters inside the highlight

  while (!WindowShouldClose())
  {
    // 1. Compute text dimensions every frame (or cache once if text ≠ dynamic)
    int textW = MeasureText(msg, fontSize);
    int textH = fontSize;            // for default font, height ≈ fontSize

    // 2. Build a rectangle big enough to cover the text plus padding
    Rectangle highlight = {
      pos.x - padding,
      pos.y - padding,
      textW + 2*padding,
      textH + 2*padding
    };

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw highlight first (50 % opacity yellow)
    DrawRectangleRec(highlight, Fade(YELLOW, 0.5f));

    // 3. Draw the text on top
    DrawText(msg, (int)pos.x, (int)pos.y, fontSize, BLACK);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

