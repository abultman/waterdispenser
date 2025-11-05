#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include "config.h"

// Global LCD panel handle
extern esp_lcd_panel_handle_t panel_handle;

// Initialize RGB LCD
bool initRGBDisplay();

// Draw bitmap to display
void drawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);

// Get framebuffer pointer
void* getFramebuffer();

#endif // DISPLAY_DRIVER_H
