#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <ESPmDNS.h>
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
        // Get the configured hostname
        Preferences prefs;
        String hostname = DEFAULT_MDNS_HOSTNAME;
        if (prefs.begin(PREFS_NAMESPACE, true)) {
            hostname = prefs.getString("mdns_hostname", DEFAULT_MDNS_HOSTNAME);
            prefs.end();
        }

        Serial.printf("Web interface available at:\n");
        Serial.printf("  http://%s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  http://%s.local\n", hostname.c_str());
    }

    Serial.println("\n=================================");
    Serial.println("System Ready!");
    Serial.println("=================================\n");
    Serial.flush();
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

    // Minimal delay - let tasks run smoothly
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

    // Use double buffering with larger buffers to reduce partial updates
    // Buffer size: 1/5 of screen to minimize number of flushes
    size_t buf_size = SCREEN_WIDTH * (SCREEN_HEIGHT / 4) * sizeof(lv_color_t);
    buf1 = (lv_color_t *)heap_caps_aligned_alloc(64, buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    buf2 = (lv_color_t *)heap_caps_aligned_alloc(64, buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);


    if (buf1 != NULL && buf2 != NULL) {
        Serial.printf("Display buffers allocated in PSRAM: 2x%d bytes\n", buf_size);
        // Double buffering - one buffer for drawing, one for DMA
        lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * (SCREEN_HEIGHT / 4));
    } else if (buf1 != NULL) {
        Serial.printf("Single display buffer allocated in PSRAM: %d bytes\n", buf_size);
        if (buf2 != NULL) free(buf2);
        buf2 = NULL;
        lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * (SCREEN_HEIGHT / 4));
    } else {
        Serial.println("PSRAM buffer failed, trying SRAM");
        buf1 = (lv_color_t *)malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t));

        if (buf1 == NULL) {
            Serial.println("ERROR: Failed to allocate display buffer!");
            return;
        }

        lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 40);
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

    // Try to load saved credentials and hostname
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true)) {
        String ssid = prefs.getString("wifi_ssid", "");
        String password = prefs.getString("wifi_pass", "");
        String hostname = prefs.getString("mdns_hostname", DEFAULT_MDNS_HOSTNAME);
        prefs.end();

        // Ensure hostname is valid (alphanumeric and hyphens only, max 63 chars)
        if (hostname.length() == 0 || hostname.length() > 63) {
            hostname = DEFAULT_MDNS_HOSTNAME;
        }

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

                // Start mDNS service with configured hostname
                if (MDNS.begin(hostname.c_str())) {
                    Serial.printf("mDNS responder started: %s.local\n", hostname.c_str());
                    MDNS.addService("http", "tcp", 80);
                } else {
                    Serial.println("Error starting mDNS");
                }
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
    // Cast color buffer directly - LVGL uses RGB565 which matches our display
    void *color_buffer = (void *)color_p;

    // Call esp_lcd_panel_draw_bitmap with proper coordinates
    // Note: x_end and y_end are EXCLUSIVE (one past the last pixel)
    esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle,
                                               area->x1, area->y1,
                                               area->x2 + 1, area->y2 + 1,
                                               color_buffer);

    if (ret != ESP_OK) {
        Serial.printf("Draw error: %d, area: x1=%d y1=%d x2=%d y2=%d\n",
                      ret, area->x1, area->y1, area->x2, area->y2);
    }

    lv_disp_flush_ready(disp);
}

// LVGL touchpad read callback
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    TouchPoint point = touch.readTouch();

    if (point.touched) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = point.x;
        data->point.y = point.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}
