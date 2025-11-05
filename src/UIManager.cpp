#include "UIManager.h"
#include "HardwareControl.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// Global instance
UIManager uiManager;

UIManager::UIManager() {
    _currentScreen = SCREEN_MAIN;
    _screen_main = nullptr;
    _screen_keypad = nullptr;
    _screen_dispensing = nullptr;
    _screen_config = nullptr;
    _screen_calibration = nullptr;
}

void UIManager::begin() {
    // Create all screens
    Serial.println("Main screen");
    Serial.flush();
    createMainScreen();
    Serial.println("keypad screen");
    Serial.flush();
    createKeypadScreen();
    Serial.println("Dispensing screen");
    Serial.flush();
    createDispensingScreen();
    Serial.println("Config screen");
    Serial.flush();
    createConfigScreen();
    Serial.println("Calibration screen");
    Serial.flush();
    createCalibrationScreen();

    Serial.println("Showing main screen");
    Serial.flush();
    // Show main screen
    showScreen(SCREEN_MAIN);
}

void UIManager::update() {
    // Update dispensing screen if active
    if (_currentScreen == SCREEN_DISPENSING) {
        updateDispensingScreen();
    }

    // Update main screen status
    if (_currentScreen == SCREEN_MAIN) {
        updateMainStatus();
    }
}

void UIManager::showScreen(UIScreen screen) {
    _currentScreen = screen;

    switch (screen) {
        case SCREEN_MAIN:
            lv_scr_load(_screen_main);
            break;
        case SCREEN_KEYPAD:
            lv_scr_load(_screen_keypad);
            lv_textarea_set_text(_textarea_amount, "");
            break;
        case SCREEN_DISPENSING:
            lv_scr_load(_screen_dispensing);
            break;
        case SCREEN_CONFIG:
            lv_scr_load(_screen_config);
            break;
        case SCREEN_CALIBRATION:
            lv_scr_load(_screen_calibration);
            break;
    }
}

UIScreen UIManager::getCurrentScreen() {
    return _currentScreen;
}

// ============================================================
// MAIN SCREEN
// ============================================================

void UIManager::createMainScreen() {
    _screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen_main, lv_color_hex(0x2C3E50), 0);

    // Title
    lv_obj_t* title = lv_label_create(_screen_main);
    lv_label_set_text(title, "Water Dispenser");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Status label
    _label_status = lv_label_create(_screen_main);
    lv_label_set_text(_label_status, "Ready");
    lv_obj_set_style_text_font(_label_status, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(_label_status, lv_color_hex(0x95A5A6), 0);
    lv_obj_align(_label_status, LV_ALIGN_TOP_MID, 0, 70);

    // Preset buttons
    int btn_width = 160;
    int btn_height = 100;
    int spacing = 20;
    int start_y = 130;

    _btn_preset1 = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_preset1, btn_width, btn_height);
    lv_obj_align(_btn_preset1, LV_ALIGN_TOP_LEFT, 20, start_y);
    lv_obj_add_event_cb(_btn_preset1, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)PRESET_1_ML);
    lv_obj_t* label1 = lv_label_create(_btn_preset1);
    lv_label_set_text_fmt(label1, "%d ml", PRESET_1_ML);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_24, 0);
    lv_obj_center(label1);

    _btn_preset2 = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_preset2, btn_width, btn_height);
    lv_obj_align(_btn_preset2, LV_ALIGN_TOP_LEFT, 20 + btn_width + spacing, start_y);
    lv_obj_add_event_cb(_btn_preset2, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)PRESET_2_ML);
    lv_obj_t* label2 = lv_label_create(_btn_preset2);
    lv_label_set_text_fmt(label2, "%d ml", PRESET_2_ML);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_24, 0);
    lv_obj_center(label2);

    _btn_preset3 = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_preset3, btn_width, btn_height);
    lv_obj_align(_btn_preset3, LV_ALIGN_TOP_LEFT, 20 + (btn_width + spacing) * 2, start_y);
    lv_obj_add_event_cb(_btn_preset3, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)PRESET_3_ML);
    lv_obj_t* label3 = lv_label_create(_btn_preset3);
    lv_label_set_text_fmt(label3, "%d ml", PRESET_3_ML);
    lv_obj_set_style_text_font(label3, &lv_font_montserrat_24, 0);
    lv_obj_center(label3);

    _btn_preset4 = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_preset4, btn_width, btn_height);
    lv_obj_align(_btn_preset4, LV_ALIGN_TOP_LEFT, 20 + (btn_width + spacing) * 3, start_y);
    lv_obj_add_event_cb(_btn_preset4, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)PRESET_4_ML);
    lv_obj_t* label4 = lv_label_create(_btn_preset4);
    lv_label_set_text_fmt(label4, "%d ml", PRESET_4_ML);
    lv_obj_set_style_text_font(label4, &lv_font_montserrat_24, 0);
    lv_obj_center(label4);

    // Custom amount button
    _btn_custom = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_custom, 350, 80);
    lv_obj_align(_btn_custom, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_style_bg_color(_btn_custom, lv_color_hex(0x3498DB), 0);
    lv_obj_add_event_cb(_btn_custom, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)-1);
    lv_obj_t* label_custom = lv_label_create(_btn_custom);
    lv_label_set_text(label_custom, "Custom Amount");
    lv_obj_set_style_text_font(label_custom, &lv_font_montserrat_24, 0);
    lv_obj_center(label_custom);

    // Settings button
    _btn_settings = lv_btn_create(_screen_main);
    lv_obj_set_size(_btn_settings, 200, 60);
    lv_obj_align(_btn_settings, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(_btn_settings, lv_color_hex(0x7F8C8D), 0);
    lv_obj_add_event_cb(_btn_settings, mainScreenEventHandler, LV_EVENT_CLICKED, (void*)-2);
    lv_obj_t* label_settings = lv_label_create(_btn_settings);
    lv_label_set_text(label_settings, "Settings");
    lv_obj_set_style_text_font(label_settings, &lv_font_montserrat_20, 0);
    lv_obj_center(label_settings);
}

void UIManager::mainScreenEventHandler(lv_event_t* e) {
    int amount = (int)lv_event_get_user_data(e);

    if (amount == -1) {
        // Custom amount - show keypad
        uiManager.showScreen(SCREEN_KEYPAD);
    } else if (amount == -2) {
        // Settings
        uiManager.showScreen(SCREEN_CONFIG);
    } else {
        // Preset amount - start dispensing
        hardwareControl.startDispensing((float)amount);
        uiManager.showScreen(SCREEN_DISPENSING);
    }
}

void UIManager::updateMainStatus() {
    // Update WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text_fmt(_label_status, "WiFi: %s | IP: %s",
                              WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    } else {
        lv_label_set_text(_label_status, "WiFi: Not Connected");
    }
}

// ============================================================
// KEYPAD SCREEN
// ============================================================

void UIManager::createKeypadScreen() {
    _screen_keypad = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen_keypad, lv_color_hex(0x2C3E50), 0);

    // Title
    lv_obj_t* title = lv_label_create(_screen_keypad);
    lv_label_set_text(title, "Enter Amount (ml)");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Text area for amount input
    _textarea_amount = lv_textarea_create(_screen_keypad);
    lv_obj_set_size(_textarea_amount, 400, 80);
    lv_obj_align(_textarea_amount, LV_ALIGN_TOP_MID, 0, 70);
    lv_textarea_set_one_line(_textarea_amount, true);
    lv_textarea_set_max_length(_textarea_amount, 6);
    lv_obj_set_style_text_font(_textarea_amount, &lv_font_montserrat_24, 0);

    // Numeric keypad
    static const char* btnm_map[] = {"1", "2", "3", "\n",
                                     "4", "5", "6", "\n",
                                     "7", "8", "9", "\n",
                                     "Clear", "0", "Del", ""};

    _btnm_keypad = lv_btnmatrix_create(_screen_keypad);
    lv_btnmatrix_set_map(_btnm_keypad, btnm_map);
    lv_obj_set_size(_btnm_keypad, 500, 200);
    lv_obj_align(_btnm_keypad, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_event_cb(_btnm_keypad, keypadEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    // OK button
    _btn_keypad_ok = lv_btn_create(_screen_keypad);
    lv_obj_set_size(_btn_keypad_ok, 200, 60);
    lv_obj_align(_btn_keypad_ok, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(_btn_keypad_ok, lv_color_hex(0x27AE60), 0);
    lv_obj_add_event_cb(_btn_keypad_ok, keypadEventHandler, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t* label_ok = lv_label_create(_btn_keypad_ok);
    lv_label_set_text(label_ok, "Start");
    lv_obj_set_style_text_font(label_ok, &lv_font_montserrat_24, 0);
    lv_obj_center(label_ok);

    // Cancel button
    _btn_keypad_cancel = lv_btn_create(_screen_keypad);
    lv_obj_set_size(_btn_keypad_cancel, 200, 60);
    lv_obj_align(_btn_keypad_cancel, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(_btn_keypad_cancel, lv_color_hex(0xE74C3C), 0);
    lv_obj_add_event_cb(_btn_keypad_cancel, keypadEventHandler, LV_EVENT_CLICKED, (void*)0);
    lv_obj_t* label_cancel = lv_label_create(_btn_keypad_cancel);
    lv_label_set_text(label_cancel, "Cancel");
    lv_obj_set_style_text_font(label_cancel, &lv_font_montserrat_24, 0);
    lv_obj_center(label_cancel);
}

void UIManager::keypadEventHandler(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (obj == uiManager._btnm_keypad && code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char* txt = lv_btnmatrix_get_btn_text(obj, id);

        if (strcmp(txt, "Clear") == 0) {
            lv_textarea_set_text(uiManager._textarea_amount, "");
        } else if (strcmp(txt, "Del") == 0) {
            lv_textarea_del_char(uiManager._textarea_amount);
        } else {
            lv_textarea_add_text(uiManager._textarea_amount, txt);
        }
    } else if (code == LV_EVENT_CLICKED) {
        int action = (int)lv_event_get_user_data(e);

        if (action == 0) {
            // Cancel
            uiManager.showScreen(SCREEN_MAIN);
        } else if (action == 1) {
            // OK - start dispensing
            const char* text = lv_textarea_get_text(uiManager._textarea_amount);
            float amount = atof(text);

            if (amount > 0 && amount <= 10000) {
                hardwareControl.startDispensing(amount);
                uiManager.showScreen(SCREEN_DISPENSING);
            }
        }
    }
}

// ============================================================
// DISPENSING SCREEN
// ============================================================

void UIManager::createDispensingScreen() {
    _screen_dispensing = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen_dispensing, lv_color_hex(0x2C3E50), 0);

    // Title
    _label_disp_title = lv_label_create(_screen_dispensing);
    lv_label_set_text(_label_disp_title, "Dispensing...");
    lv_obj_set_style_text_font(_label_disp_title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(_label_disp_title, lv_color_white(), 0);
    lv_obj_align(_label_disp_title, LV_ALIGN_TOP_MID, 0, 30);

    // Current amount label
    _label_disp_amount = lv_label_create(_screen_dispensing);
    lv_label_set_text(_label_disp_amount, "0 ml");
    lv_obj_set_style_text_font(_label_disp_amount, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(_label_disp_amount, lv_color_hex(0x3498DB), 0);
    lv_obj_align(_label_disp_amount, LV_ALIGN_CENTER, 0, -80);

    // Target amount label
    _label_disp_target = lv_label_create(_screen_dispensing);
    lv_label_set_text(_label_disp_target, "/ 0 ml");
    lv_obj_set_style_text_font(_label_disp_target, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(_label_disp_target, lv_color_hex(0x95A5A6), 0);
    lv_obj_align(_label_disp_target, LV_ALIGN_CENTER, 0, -30);

    // Progress bar
    _bar_progress = lv_bar_create(_screen_dispensing);
    lv_obj_set_size(_bar_progress, 600, 40);
    lv_obj_align(_bar_progress, LV_ALIGN_CENTER, 0, 40);
    lv_bar_set_value(_bar_progress, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(_bar_progress, lv_color_hex(0x34495E), 0);
    lv_obj_set_style_bg_color(_bar_progress, lv_color_hex(0x27AE60), LV_PART_INDICATOR);

    // Progress percentage label
    _label_progress = lv_label_create(_screen_dispensing);
    lv_label_set_text(_label_progress, "0%");
    lv_obj_set_style_text_font(_label_progress, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(_label_progress, lv_color_white(), 0);
    lv_obj_align(_label_progress, LV_ALIGN_CENTER, 0, 90);

    // Pause button (shown during dispensing)
    _btn_pause = lv_btn_create(_screen_dispensing);
    lv_obj_set_size(_btn_pause, 200, 70);
    lv_obj_align(_btn_pause, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(_btn_pause, lv_color_hex(0xF39C12), 0);
    lv_obj_add_event_cb(_btn_pause, dispensingEventHandler, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t* label_pause = lv_label_create(_btn_pause);
    lv_label_set_text(label_pause, "PAUSE");
    lv_obj_set_style_text_font(label_pause, &lv_font_montserrat_24, 0);
    lv_obj_center(label_pause);

    // Resume button (shown when paused)
    _btn_resume = lv_btn_create(_screen_dispensing);
    lv_obj_set_size(_btn_resume, 200, 70);
    lv_obj_align(_btn_resume, LV_ALIGN_BOTTOM_LEFT, 100, -30);
    lv_obj_set_style_bg_color(_btn_resume, lv_color_hex(0x27AE60), 0);
    lv_obj_add_event_cb(_btn_resume, dispensingEventHandler, LV_EVENT_CLICKED, (void*)2);
    lv_obj_t* label_resume = lv_label_create(_btn_resume);
    lv_label_set_text(label_resume, "RESUME");
    lv_obj_set_style_text_font(label_resume, &lv_font_montserrat_24, 0);
    lv_obj_center(label_resume);
    lv_obj_add_flag(_btn_resume, LV_OBJ_FLAG_HIDDEN);  // Hidden by default

    // Stop button (shown when paused)
    _btn_stop = lv_btn_create(_screen_dispensing);
    lv_obj_set_size(_btn_stop, 200, 70);
    lv_obj_align(_btn_stop, LV_ALIGN_BOTTOM_RIGHT, -100, -30);
    lv_obj_set_style_bg_color(_btn_stop, lv_color_hex(0xE74C3C), 0);
    lv_obj_add_event_cb(_btn_stop, dispensingEventHandler, LV_EVENT_CLICKED, (void*)3);
    lv_obj_t* label_stop = lv_label_create(_btn_stop);
    lv_label_set_text(label_stop, "STOP");
    lv_obj_set_style_text_font(label_stop, &lv_font_montserrat_24, 0);
    lv_obj_center(label_stop);
    lv_obj_add_flag(_btn_stop, LV_OBJ_FLAG_HIDDEN);  // Hidden by default
}

void UIManager::dispensingEventHandler(lv_event_t* e) {
    int action = (int)lv_event_get_user_data(e);

    if (action == 1) {
        // Pause
        hardwareControl.pauseDispensing();
    } else if (action == 2) {
        // Resume
        hardwareControl.resumeDispensing();
    } else if (action == 3) {
        // Stop
        hardwareControl.stopDispensing();
        delay(500);  // Give time for valve to close
        uiManager.showScreen(SCREEN_MAIN);
    }
}

void UIManager::updateDispensingScreen() {
    float dispensed = hardwareControl.getDispensedAmount();
    float target = hardwareControl.getTargetAmount();
    uint8_t progress = hardwareControl.getProgress();
    DispensingState state = hardwareControl.getState();

    // Update labels
    lv_label_set_text_fmt(_label_disp_amount, "%.1f ml", dispensed);
    lv_label_set_text_fmt(_label_disp_target, "/ %.1f ml", target);
    lv_label_set_text_fmt(_label_progress, "%d%%", progress);

    // Update progress bar
    lv_bar_set_value(_bar_progress, progress, LV_ANIM_ON);

    // Update title and button visibility based on state
    if (state == DISPENSING) {
        // Show only pause button
        lv_label_set_text(_label_disp_title, "Dispensing...");
        lv_obj_set_style_text_color(_label_disp_title, lv_color_white(), 0);
        lv_obj_clear_flag(_btn_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_btn_resume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_btn_stop, LV_OBJ_FLAG_HIDDEN);
    } else if (state == PAUSED) {
        // Show resume and stop buttons
        lv_label_set_text(_label_disp_title, "Paused");
        lv_obj_set_style_text_color(_label_disp_title, lv_color_hex(0xF39C12), 0);
        lv_obj_add_flag(_btn_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_btn_resume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_btn_stop, LV_OBJ_FLAG_HIDDEN);
    }

    // Check if completed or error
    if (state == COMPLETED) {
        lv_label_set_text(_label_progress, "Complete!");
        lv_obj_set_style_text_color(_label_progress, lv_color_hex(0x27AE60), 0);
        delay(2000);
        uiManager.showScreen(SCREEN_MAIN);
    } else if (state == ERROR_TIMEOUT || state == ERROR_NO_FLOW) {
        lv_label_set_text(_label_progress, "Error!");
        lv_obj_set_style_text_color(_label_progress, lv_color_hex(0xE74C3C), 0);
        delay(2000);
        uiManager.showScreen(SCREEN_MAIN);
    }
}

// ============================================================
// CONFIG SCREEN
// ============================================================

void UIManager::createConfigScreen() {
    _screen_config = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen_config, lv_color_hex(0x2C3E50), 0);

    // Title
    lv_obj_t* title = lv_label_create(_screen_config);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // WiFi SSID
    lv_obj_t* label_ssid = lv_label_create(_screen_config);
    lv_label_set_text(label_ssid, "WiFi SSID:");
    lv_obj_set_style_text_color(label_ssid, lv_color_white(), 0);
    lv_obj_align(label_ssid, LV_ALIGN_TOP_LEFT, 50, 80);

    _textarea_ssid = lv_textarea_create(_screen_config);
    lv_obj_set_size(_textarea_ssid, 500, 50);
    lv_obj_align(_textarea_ssid, LV_ALIGN_TOP_LEFT, 50, 110);
    lv_textarea_set_one_line(_textarea_ssid, true);

    // WiFi Password
    lv_obj_t* label_password = lv_label_create(_screen_config);
    lv_label_set_text(label_password, "WiFi Password:");
    lv_obj_set_style_text_color(label_password, lv_color_white(), 0);
    lv_obj_align(label_password, LV_ALIGN_TOP_LEFT, 50, 180);

    _textarea_password = lv_textarea_create(_screen_config);
    lv_obj_set_size(_textarea_password, 500, 50);
    lv_obj_align(_textarea_password, LV_ALIGN_TOP_LEFT, 50, 210);
    lv_textarea_set_one_line(_textarea_password, true);
    lv_textarea_set_password_mode(_textarea_password, true);

    // WiFi status label
    _label_wifi_status = lv_label_create(_screen_config);
    lv_label_set_text(_label_wifi_status, "Not connected");
    lv_obj_set_style_text_color(_label_wifi_status, lv_color_hex(0x95A5A6), 0);
    lv_obj_align(_label_wifi_status, LV_ALIGN_TOP_LEFT, 50, 280);

    // Connect button
    _btn_wifi_connect = lv_btn_create(_screen_config);
    lv_obj_set_size(_btn_wifi_connect, 200, 50);
    lv_obj_align(_btn_wifi_connect, LV_ALIGN_TOP_RIGHT, -50, 280);
    lv_obj_set_style_bg_color(_btn_wifi_connect, lv_color_hex(0x27AE60), 0);
    lv_obj_add_event_cb(_btn_wifi_connect, configEventHandler, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t* label_connect = lv_label_create(_btn_wifi_connect);
    lv_label_set_text(label_connect, "Connect");
    lv_obj_center(label_connect);

    // Calibration button
    _btn_calibrate = lv_btn_create(_screen_config);
    lv_obj_set_size(_btn_calibrate, 300, 60);
    lv_obj_align(_btn_calibrate, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_bg_color(_btn_calibrate, lv_color_hex(0x3498DB), 0);
    lv_obj_add_event_cb(_btn_calibrate, configEventHandler, LV_EVENT_CLICKED, (void*)2);
    lv_obj_t* label_calibrate = lv_label_create(_btn_calibrate);
    lv_label_set_text(label_calibrate, "Calibrate Flow Sensor");
    lv_obj_center(label_calibrate);

    // Back button
    _btn_config_back = lv_btn_create(_screen_config);
    lv_obj_set_size(_btn_config_back, 200, 60);
    lv_obj_align(_btn_config_back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(_btn_config_back, lv_color_hex(0x7F8C8D), 0);
    lv_obj_add_event_cb(_btn_config_back, configEventHandler, LV_EVENT_CLICKED, (void*)0);
    lv_obj_t* label_back = lv_label_create(_btn_config_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_center(label_back);

    // Load saved WiFi credentials
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true)) {
        String ssid = prefs.getString("wifi_ssid", "");
        String password = prefs.getString("wifi_pass", "");
        if (ssid.length() > 0) {
            lv_textarea_set_text(_textarea_ssid, ssid.c_str());
            lv_textarea_set_text(_textarea_password, password.c_str());
        }
        prefs.end();
    }
}

void UIManager::configEventHandler(lv_event_t* e) {
    int action = (int)lv_event_get_user_data(e);

    if (action == 0) {
        // Back
        uiManager.showScreen(SCREEN_MAIN);
    } else if (action == 1) {
        // WiFi Connect
        const char* ssid = lv_textarea_get_text(uiManager._textarea_ssid);
        const char* password = lv_textarea_get_text(uiManager._textarea_password);

        lv_label_set_text(uiManager._label_wifi_status, "Connecting...");

        WiFi.begin(ssid, password);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
            delay(100);
            lv_timer_handler();
        }

        if (WiFi.status() == WL_CONNECTED) {
            lv_label_set_text_fmt(uiManager._label_wifi_status, "Connected! IP: %s", WiFi.localIP().toString().c_str());
            lv_obj_set_style_text_color(uiManager._label_wifi_status, lv_color_hex(0x27AE60), 0);

            // Save credentials
            Preferences prefs;
            if (prefs.begin(PREFS_NAMESPACE, false)) {
                prefs.putString("wifi_ssid", ssid);
                prefs.putString("wifi_pass", password);
                prefs.end();
            }
        } else {
            lv_label_set_text(uiManager._label_wifi_status, "Connection failed!");
            lv_obj_set_style_text_color(uiManager._label_wifi_status, lv_color_hex(0xE74C3C), 0);
        }
    } else if (action == 2) {
        // Calibration
        uiManager.showScreen(SCREEN_CALIBRATION);
    }
}

// ============================================================
// CALIBRATION SCREEN
// ============================================================

void UIManager::createCalibrationScreen() {
    _screen_calibration = lv_obj_create(NULL);
    if (_screen_calibration == NULL) {
        Serial.println("ERROR: Failed to create calibration screen!");
        return;
    }
    lv_obj_set_style_bg_color(_screen_calibration, lv_color_hex(0x2C3E50), 0);

    // Title
    lv_obj_t* title = lv_label_create(_screen_calibration);
    lv_label_set_text(title, "Flow Sensor Calibration");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Instructions
    _label_calib_instructions = lv_label_create(_screen_calibration);
    lv_label_set_text(_label_calib_instructions,
        "1. Enter known volume\n"
        "2. Press Start\n"
        "3. Dispense that volume\n"
        "4. Press Save");
    lv_obj_set_style_text_color(_label_calib_instructions, lv_color_white(), 0);
    lv_obj_align(_label_calib_instructions, LV_ALIGN_TOP_LEFT, 50, 80);

    // Volume input
    lv_obj_t* label_vol = lv_label_create(_screen_calibration);
    lv_label_set_text(label_vol, "Known Volume (ml):");
    lv_obj_set_style_text_color(label_vol, lv_color_white(), 0);
    lv_obj_align(label_vol, LV_ALIGN_TOP_LEFT, 50, 200);

    _textarea_calib_volume = lv_textarea_create(_screen_calibration);
    lv_obj_set_size(_textarea_calib_volume, 200, 50);
    lv_obj_align(_textarea_calib_volume, LV_ALIGN_TOP_LEFT, 250, 195);
    lv_textarea_set_one_line(_textarea_calib_volume, true);
    lv_textarea_set_text(_textarea_calib_volume, "1000");

    // Pulse count label
    _label_calib_pulses = lv_label_create(_screen_calibration);
    lv_label_set_text(_label_calib_pulses, "Pulses: 0");
    lv_obj_set_style_text_font(_label_calib_pulses, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(_label_calib_pulses, lv_color_hex(0x3498DB), 0);
    lv_obj_align(_label_calib_pulses, LV_ALIGN_CENTER, 0, 0);

    // Start button
    _btn_calib_start = lv_btn_create(_screen_calibration);
    lv_obj_set_size(_btn_calib_start, 200, 60);
    lv_obj_align(_btn_calib_start, LV_ALIGN_BOTTOM_LEFT, 50, -30);
    lv_obj_set_style_bg_color(_btn_calib_start, lv_color_hex(0x27AE60), 0);
    lv_obj_add_event_cb(_btn_calib_start, calibrationEventHandler, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t* label_start = lv_label_create(_btn_calib_start);
    lv_label_set_text(label_start, "Start");
    lv_obj_center(label_start);

    // Save button
    _btn_calib_save = lv_btn_create(_screen_calibration);
    lv_obj_set_size(_btn_calib_save, 200, 60);
    lv_obj_align(_btn_calib_save, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(_btn_calib_save, lv_color_hex(0x3498DB), 0);
    lv_obj_add_event_cb(_btn_calib_save, calibrationEventHandler, LV_EVENT_CLICKED, (void*)2);
    lv_obj_t* label_save = lv_label_create(_btn_calib_save);
    lv_label_set_text(label_save, "Save");
    lv_obj_center(label_save);

    // Cancel button
    _btn_calib_cancel = lv_btn_create(_screen_calibration);
    lv_obj_set_size(_btn_calib_cancel, 200, 60);
    lv_obj_align(_btn_calib_cancel, LV_ALIGN_BOTTOM_RIGHT, -50, -30);
    lv_obj_set_style_bg_color(_btn_calib_cancel, lv_color_hex(0xE74C3C), 0);
    lv_obj_add_event_cb(_btn_calib_cancel, calibrationEventHandler, LV_EVENT_CLICKED, (void*)0);
    lv_obj_t* label_cancel = lv_label_create(_btn_calib_cancel);
    lv_label_set_text(label_cancel, "Cancel");
    lv_obj_center(label_cancel);
}

void UIManager::calibrationEventHandler(lv_event_t* e) {
    int action = (int)lv_event_get_user_data(e);
    static uint32_t startPulses = 0;

    if (action == 0) {
        // Cancel
        hardwareControl.closeValve();
        uiManager.showScreen(SCREEN_CONFIG);
    } else if (action == 1) {
        // Start - reset pulse counter and open valve
        hardwareControl.resetFlowCounter();
        hardwareControl.openValve();
        startPulses = 0;
        lv_label_set_text(uiManager._label_calib_instructions, "Dispensing... Monitor pulses");
    } else if (action == 2) {
        // Save - calculate and save calibration
        hardwareControl.closeValve();

        const char* volText = lv_textarea_get_text(uiManager._textarea_calib_volume);
        float knownVolume = atof(volText);
        float dispensed = hardwareControl.getDispensedAmount();

        // Calculate pulses from actual dispensed amount
        float oldFactor = hardwareControl.getCalibrationFactor();
        float actualPulses = (dispensed / 1000.0) * oldFactor;
        float newFactor = (actualPulses / knownVolume) * 1000.0;

        hardwareControl.setCalibrationFactor(newFactor);

        lv_label_set_text_fmt(uiManager._label_calib_instructions,
            "Saved!\nNew factor: %.2f pulses/L", newFactor);

        delay(2000);
        uiManager.showScreen(SCREEN_CONFIG);
    }

    // Update pulse count display
    if (uiManager._currentScreen == SCREEN_CALIBRATION) {
        lv_label_set_text_fmt(uiManager._label_calib_pulses,
            "Amount: %.1f ml", hardwareControl.getDispensedAmount());
    }
}
