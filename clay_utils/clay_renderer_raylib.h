#ifndef CLAY_RENDERER_RAYLIB_H
#define CLAY_RENDERER_RAYLIB_H

#include "raylib.h"
#include "clay.h"

// Camera for 3D models
extern Camera Raylib_camera;

// Function to get a ray from screen position
Ray GetScreenToWorldPointWithZDistance(Vector2 position, Camera camera,
                                     int screenWidth, int screenHeight,
                                     float zDistance);

// Text measurement function for Clay
Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text,
                                  Clay_TextElementConfig *config,
                                  void *userData);

// Initialize the Raylib window and settings
void Clay_Raylib_Initialize(int width, int height, const char *title,
                          unsigned int flags);

// Clean up resources
void Clay_Raylib_Close(void);

// Render Clay commands using Raylib
void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font *fonts);

#endif // CLAY_RENDERER_RAYLIB_H