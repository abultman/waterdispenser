#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "config.h"
#include "display_driver.h"
#include "GT911.h"
#include "HardwareControl.h"
#include "UIManager.h"
#include "WebServer.h"

// Touch object
GT911 touch(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

// LVGL display buffer - allocated in PSRAM to save DRAM
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;

// LVGL display driver
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// Forward declarations
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void setupDisplay();
void setupTouch();
void setupLVGL();
void setupWiFi();

void setup() {
    Serial.begin(115200);
    delay(4000);  // Give serial/USB CDC time to initialize

    Serial.println();
    Serial.println();
    Serial.println();
    Serial.flush();

    Serial.println("========================================");
    Serial.println("NEW FIRMWARE - Water Dispenser v2.0");
    Serial.println("========================================");
    Serial.flush();

    // Initialize NVS (required for Preferences)
    Serial.println("[1/7] Initializing NVS...");
    Serial.flush();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("NVS partition full or version mismatch, erasing...");
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            Serial.printf("ERROR: NVS erase failed: %d\n", ret);
        }
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        Serial.printf("ERROR: NVS init failed: %d\n", ret);
    } else {
        Serial.println("NVS initialized successfully");
    }

    // Initialize display
    Serial.println("[2/7] Initializing display...");
    Serial.flush();
    setupDisplay();

    // Initialize touch controller
    Serial.println("[3/7] Initializing touch controller...");
    Serial.flush();
    setupTouch();

    // Initialize LVGL
    Serial.println("[4/7] Initializing LVGL...");
    Serial.flush();
    setupLVGL();

    // Initialize hardware control
    Serial.println("[5/7] Initializing hardware control...");
    Serial.flush();
    hardwareControl.begin();

    // Initialize UI
    Serial.println("[6/7] Initializing UI...");
    Serial.flush();
    uiManager.begin();

    // Setup WiFi (non-blocking)
    Serial.println("[7/7] Setting up WiFi...");
    Serial.flush();
    setupWiFi();

    // Initialize web server
    Serial.println("Starting web server...");
    Serial.flush();
    webServer.begin();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Web interface available at: http://%s\n", WiFi.localIP().toString().c_str());
    }

    Serial.println("\n=================================");
    Serial.println("System Ready!");
    Serial.println("=================================\n");
}

void loop() {
    // Update LVGL
    lv_timer_handler();

    // Update hardware control
    hardwareControl.update();

    // Update UI
    uiManager.update();

    // Update web server
    webServer.update();

    // Small delay to prevent watchdog issues
    delay(5);
}

void setupDisplay() {
    if (!initRGBDisplay()) {
        Serial.println("ERROR: Failed to initialize display!");
    }
}

void setupTouch() {
    if (touch.begin()) {
        touch.setRotation(1);  // Match display rotation
        Serial.println("Touch controller initialized");
    } else {
        Serial.println("ERROR: Touch controller initialization failed!");
    }
}

void setupLVGL() {
    lv_init();

    // Use single large buffer with proper alignment for PSRAM
    // Buffer size: full screen height in 10 strips to reduce updates
    size_t buf_size = SCREEN_WIDTH * (SCREEN_HEIGHT / 10) * sizeof(lv_color_t);
    buf1 = (lv_color_t *)heap_caps_aligned_alloc(64, buf_size, MALLOC_CAP_SPIRAM);

    if (buf1 != NULL) {
        Serial.printf("Display buffer allocated in PSRAM: %d bytes\n", buf_size);
        // Single buffer - simpler and may reduce jitter
        lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * (SCREEN_HEIGHT / 10));
    } else {
        Serial.println("PSRAM buffer failed, trying SRAM");
        buf1 = (lv_color_t *)malloc(SCREEN_WIDTH * 20 * sizeof(lv_color_t));

        if (buf1 == NULL) {
            Serial.println("ERROR: Failed to allocate display buffer!");
            return;
        }

        lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 20);
    }

    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize input device driver (touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    Serial.println("LVGL initialized");
}

void setupWiFi() {
    WiFi.mode(WIFI_STA);

    // Try to load saved credentials
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true)) {
        String ssid = prefs.getString("wifi_ssid", "");
        String password = prefs.getString("wifi_pass", "");
        prefs.end();

        if (ssid.length() > 0) {
            Serial.printf("Attempting to connect to WiFi: %s\n", ssid.c_str());
            WiFi.begin(ssid.c_str(), password.c_str());

            // Non-blocking connection attempt
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
                delay(100);
                Serial.print(".");
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
            } else {
                Serial.println("\nWiFi connection failed. Use config screen to setup.");
            }
        } else {
            Serial.println("No saved WiFi credentials. Use config screen to setup.");
        }
    }
}

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    drawBitmap(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);

    lv_disp_flush_ready(disp);
}

// LVGL touchpad read callback
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    TouchPoint point = touch.readTouch();

    
    if (point.touched) {
        Serial.printf("Touched: %d, %d \n", point.x, point.y);
        data->state = LV_INDEV_STATE_PR;
        data->point.x = point.x;
        data->point.y = point.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}
