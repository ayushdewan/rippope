#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAY_IMPLEMENTATION
#include "../include/clay_utils/clay.h"
#include "../include/clay_utils/clay_renderer_raylib.h"
#include "../include/piece_table.h"

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;
#define COLOR_ORANGE                                                           \
  (Clay_Color) { 225, 138, 50, 255 }
#define COLOR_BLUE                                                             \
  (Clay_Color) { 111, 173, 162, 255 }

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector)                                 \
  (Clay_Vector2) { .x = vector.x, .y = vector.y }

// TODO: Move these to separate file
// Render Settings
#define FPS 100

// Cursor Settings
#define CURSOR_WIDTH 2 // TODO: move to function/memory location
#define CURSOR_BLINK_RATE 0.5
#define CURSOR_BLINK_CYCLE 1.2

typedef struct {
  double cycle_start;
  bool should_render;
} cursor_state;

typedef struct {
  cursor_state curs;
  piece_table ptbl;
  Font *fonts;
  Clay_TextElementConfig text_config;
} editor_state;

void update_cursor_state(cursor_state *cs_p) {
  double curr_time = GetTime();
  double dt = curr_time - cs_p->cycle_start;
  if (dt > CURSOR_BLINK_CYCLE) {
    cs_p->cycle_start = curr_time;
  }
  cs_p->should_render = dt < CURSOR_BLINK_RATE * CURSOR_BLINK_CYCLE;
}

Clay_RenderCommandArray CreateLayout(editor_state *editor,
                                     render_buffers *render_bufs_p) {

  Clay_BeginLayout();

  Clay_Context *context = Clay_GetCurrentContext();
  Clay_TextElementConfig *config = CLAY_TEXT_CONFIG({
      .fontSize = 30, .textColor = {200, 200, 200, 255},
      //.textAlignment = CLAY_TEXT_ALIGN_CENTER,
  });
  float spaceWidth =
      Clay__MeasureText(
          CLAY__INIT(Clay_StringSlice){.length = 1,
                                       .chars = CLAY__SPACECHAR.chars,
                                       .baseChars = CLAY__SPACECHAR.chars},
          config, context->measureTextUserData)
          .width;

  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .padding = {16, 16, 16, 16},
                   /* .childGap = 16 */},
        .backgroundColor = {50, 50, 50, 255}}) {
    for (size_t line_number = 1;
         line_number <= render_bufs_p->num_line_breaks + 1; line_number++) {
      CLAY({.id = CLAY_ID("LineContainer"),
            .layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(40)},
                       .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                       .childGap = 10},
            .backgroundColor = {50, 50, 50, 50}}) {
        CLAY({.id = CLAY_ID("LineNumberContainer"),
              .layout = {.sizing = {.width = CLAY_SIZING_PERCENT(0.05f),
                                    .height = CLAY_SIZING_GROW(0)},
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = {0, 0, 0, 255}}) {
          Clay_String curr_line_number_str = (Clay_String){
              .isStaticallyAllocated = false,
              .length = strlen(render_bufs_p->line_numbers[line_number]),
              .chars = render_bufs_p->line_numbers[line_number],
          };
          CLAY_TEXT(curr_line_number_str,
                    CLAY_TEXT_CONFIG({
                        .fontSize = 30,
                        .textColor = COLOR_BLUE,
                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                    }));
        }
        CLAY({
            .id = CLAY_ID("LineTextCursorContainer"),
            .layout =
                {
                    .sizing = {.width = CLAY_SIZING_GROW(0),
                               .height = CLAY_SIZING_GROW(0)},
                    .childGap = 2,
                    .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                },
        }) {
          size_t line_end = (line_number == render_bufs_p->num_line_breaks + 1)
                                ? render_bufs_p->edit_text_len
                                : render_bufs_p->line_break_pos[line_number];
          size_t line_start =
              render_bufs_p->line_break_pos[line_number - 1] + 1;
          size_t line_len = line_end - line_start;
          Clay_String curr_line = (Clay_String){
              .isStaticallyAllocated = false,
              .length = line_len,
              .chars = render_bufs_p->edit_text_buf + line_start,
          };
          CLAY_TEXT(curr_line,
                    CLAY_TEXT_CONFIG({
                        .fontSize = 30, .textColor = {200, 200, 200, 255},
                        //.textAlignment = CLAY_TEXT_ALIGN_CENTER,
                    }));

          // TODO: LOOOL FIX THIS
          if (line_number == render_bufs_p->cursor_line &&
              editor->curs.should_render) {
            CLAY(
                {.id = CLAY_ID("CursorHack"),
                 .layout = {.sizing = {.width = CLAY_SIZING_FIXED(CURSOR_WIDTH),
                                       .height = CLAY_SIZING_FIXED(30)}},
                 .floating =
                     {
                         .attachTo = CLAY_ATTACH_TO_PARENT,
                         .offset = {.x = render_bufs_p->cursor_offset *
                                         spaceWidth,
                                    .y = 5}, // magic number = (parent container
                                             // height - fontsize) / 2
                         .attachPoints = CLAY_ATTACH_POINT_LEFT_TOP,
                         .zIndex = 1,
                     },
                 .backgroundColor = {200, 200, 200, 255}});
          }
        }
      }
    }
  }
  return Clay_EndLayout();
}

bool debugEnabled = false;

void UpdateEditorState(editor_state *editor, render_buffers *render_bufs_p) {
  update_cursor_state(&editor->curs);

  char c;
  int reload_data = 1; // consider removing
  while ((c = GetCharPressed()) > 0) {
    ptbl_insert_char(&editor->ptbl, c);
    reload_data = 1;
  }

  int keycode;
  while ((keycode = GetKeyPressed()) > 0) {
    printf("%d\n", keycode);
    switch (keycode) {
    case KEY_LEFT:
      ptbl_update_global_cursor_pos(&editor->ptbl,
                                    editor->ptbl.global_cursor_pos - 1);
      break;
    case KEY_RIGHT:
      ptbl_update_global_cursor_pos(&editor->ptbl,
                                    editor->ptbl.global_cursor_pos + 1);
      break;
    case KEY_BACKSPACE:
      if (editor->ptbl.global_cursor_pos > 0) {
        ptbl_delete_char(&editor->ptbl);
        reload_data = 1;
      }
      break;
    case KEY_ENTER:
      ptbl_insert_char(&editor->ptbl, '\n');
      reload_data = 1;
      break;
    }
  }

  if (reload_data) {
    load_ptbl_data(&editor->ptbl, render_bufs_p);
  }
}

void UpdateDrawFrame(editor_state *editor, render_buffers *render_bufs_p) {
  Vector2 mouseWheelDelta = GetMouseWheelMoveV();
  float mouseWheelX = mouseWheelDelta.x;
  float mouseWheelY = mouseWheelDelta.y;

  if (IsKeyPressed(KEY_D)) {
    debugEnabled = !debugEnabled;
    Clay_SetDebugModeEnabled(debugEnabled);
  }
  //----------------------------------------------------------------------------------
  // Handle scroll containers
  Clay_Vector2 mousePosition =
      RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
  Clay_SetPointerState(mousePosition, IsMouseButtonDown(0));
  Clay_SetLayoutDimensions(
      (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()});

  // Generate the auto layout for rendering
  double currentTime = GetTime();
  UpdateEditorState(editor, render_bufs_p);
  Clay_RenderCommandArray renderCommands = CreateLayout(editor, render_bufs_p);

  // RENDERING ---------------------------------
  BeginDrawing();
  ClearBackground(BLACK);
  currentTime = GetTime();
  Clay_Raylib_Render(renderCommands, editor->fonts);
  EndDrawing();

  //----------------------------------------------------------------------------------
}

bool reinitializeClay = false;

void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
  if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
    reinitializeClay = true;
    Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
  } else if (errorData.errorType ==
             CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
    reinitializeClay = true;
    Clay_SetMaxMeasureTextCacheWordCount(
        Clay_GetMaxMeasureTextCacheWordCount() * 2);
  }
}

char *textbuf = "hi";

int main(void) {
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
      totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(
      clayMemory,
      (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors, 0});
  Clay_Raylib_Initialize(1024, 768, "Clay - Raylib Renderer Example",
                         FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE |
                             FLAG_MSAA_4X_HINT);

  // Get font path based on platform
  const char *fontPath = NULL;
#if defined(_WIN32)
  fontPath = "resources/fonts/FiraCode-Regular.ttf"; // Windows
#elif defined(__APPLE__)
  fontPath = "resources/fonts/FiraCode-Regular.ttf"; // macOS
#else
  // Try Linux system font first, then fall back to relative path
  if (FileExists("/usr/share/fonts/truetype/firacode/FiraCode-Regular.ttf")) {
    fontPath = "/usr/share/fonts/truetype/firacode/FiraCode-Regular.ttf";
  } else {
    fontPath = "resources/fonts/FiraCode-Regular.ttf";
  }
#endif

  // Load fonts
  Font fonts[2];
  fonts[FONT_ID_BODY_24] = LoadFontEx(fontPath, 48, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
  fonts[FONT_ID_BODY_16] = LoadFontEx(fontPath, 32, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  // initialize piece table from file
  editor_state es = (editor_state){
      .curs = {.cycle_start = GetTime(), .should_render = true},
      .ptbl = create_piece_table(textbuf, 2),
      .fonts = fonts,
      .text_config =
          {
              .fontId = FONT_ID_BODY_16,
              .fontSize = 32,
              .textColor = {200, 200, 200, 255},
          },
  };
  ptbl_update_global_cursor_pos(&es.ptbl, 2);

  // initialize render buffers to 0
  render_buffers render_bufs = (render_buffers){
      .edit_text_buf = {0},
      .edit_text_len = 0,
      .line_break_pos = {0},
      .num_line_breaks = 0,
      .line_numbers = {0},
      .cursor_line = 0,
      .cursor_offset = 0,
  };
  memset(render_bufs.edit_text_buf, 0, EDIT_TEXT_BUFFER_MAX_SIZE);
  memset(render_bufs.line_break_pos, 0, MAX_LINE_BREAKS);
  for (int i = 0; i < MAX_LINE_BREAKS; i++) {
    memset(render_bufs.line_numbers[i], 0, 8);
  }
  load_ptbl_data(&es.ptbl, &render_bufs);

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    if (reinitializeClay) {
      Clay_SetMaxElementCount(8192);
      totalMemorySize = Clay_MinMemorySize();
      clayMemory = Clay_CreateArenaWithCapacityAndMemory(
          totalMemorySize, malloc(totalMemorySize));
      Clay_Initialize(
          clayMemory,
          (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()},
          (Clay_ErrorHandler){HandleClayErrors, 0});
      reinitializeClay = false;
    }
    UpdateDrawFrame(&es, &render_bufs);
  }
  free_piece_table(&es.ptbl);
  Clay_Raylib_Close();
  return 0;
}
