#ifndef CONFIG_H
#define CONFIG_H

// ========================================
// PIN DEFINITIONS - Easy to modify
// ========================================

// RGB LCD Display pins (ESP32-8048S043C-I)
#define LCD_DE          40
#define LCD_VSYNC       41
#define LCD_HSYNC       39
#define LCD_PCLK        42
#define LCD_BL          2    // Backlight control

// RGB Data pins
#define LCD_R0          45
#define LCD_R1          48
#define LCD_R2          47
#define LCD_R3          21
#define LCD_R4          14

#define LCD_G0          5
#define LCD_G1          6
#define LCD_G2          7
#define LCD_G3          15
#define LCD_G4          16
#define LCD_G5          4

#define LCD_B0          8
#define LCD_B1          3
#define LCD_B2          46
#define LCD_B3          9
#define LCD_B4          1

// Touch controller pins (GT911)
#define TOUCH_SDA       19  
#define TOUCH_SCL       20
#define TOUCH_INT       -1   // Not connected by default (can use GPIO 18 with R17 mod)
#define TOUCH_RST       38
#define TOUCH_WIDTH     480
#define TOUCH_HEIGHT    272

// Hardware control pins
// Note: GPIOs 0-48 are used by display/touch, avoid those
#define VALVE_PIN       10   // Pin to control the valve (HIGH = open) - Available GPIO
#define FLOW_SENSOR_PIN 11   // Flow counter pulse input - Available GPIO

// Display settings
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   480

// ========================================
// FLOW SENSOR CONFIGURATION
// ========================================

// Calibration factor: pulses per liter
// Adjust this based on your flow sensor specifications
#define DEFAULT_PULSES_PER_LITER  450.0

// Minimum flow rate to detect (pulses per second)
// Used to detect if flow has stopped
#define MIN_FLOW_RATE   2

// ========================================
// DISPENSING SETTINGS
// ========================================

// Timeout for dispensing (milliseconds)
// If no flow detected for this duration, stop dispensing
#define FLOW_TIMEOUT    5000

// Overshoot compensation (ml)
// Account for valve closing delay
#define OVERSHOOT_COMPENSATION  5.0

// Preset button amounts (in ml)
#define PRESET_1_ML     100
#define PRESET_2_ML     250
#define PRESET_3_ML     500
#define PRESET_4_ML     1000

// ========================================
// WIFI SETTINGS
// ========================================

// Default WiFi credentials (can be changed via config screen)
#define DEFAULT_WIFI_SSID     ""
#define DEFAULT_WIFI_PASSWORD ""

// Default mDNS hostname (can be changed via config screen)
// Device will be accessible at <hostname>.local
#define DEFAULT_MDNS_HOSTNAME "waterdispenser"

// WiFi connection timeout (milliseconds)
#define WIFI_TIMEOUT    15000

// ========================================
// SYSTEM SETTINGS
// ========================================

// Display brightness (0-255)
#define DEFAULT_BRIGHTNESS  200

// Preferences namespace for storing settings
#define PREFS_NAMESPACE "waterdisp"

// Debounce time for buttons (milliseconds)
#define BUTTON_DEBOUNCE 50

// LVGL tick period (milliseconds)
#define LVGL_TICK_PERIOD 5

#endif // CONFIG_H
