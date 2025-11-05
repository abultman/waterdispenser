#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>

enum UIScreen {
    SCREEN_MAIN,
    SCREEN_KEYPAD,
    SCREEN_DISPENSING,
    SCREEN_CONFIG,
    SCREEN_CALIBRATION
};

class UIManager {
public:
    UIManager();
    void begin();
    void update();
    void showScreen(UIScreen screen);
    UIScreen getCurrentScreen();

private:
    lv_obj_t* _screen_main;
    lv_obj_t* _screen_keypad;
    lv_obj_t* _screen_dispensing;
    lv_obj_t* _screen_config;
    lv_obj_t* _screen_calibration;

    UIScreen _currentScreen;

    // Main screen elements
    lv_obj_t* _btn_preset1;
    lv_obj_t* _btn_preset2;
    lv_obj_t* _btn_preset3;
    lv_obj_t* _btn_preset4;
    lv_obj_t* _btn_custom;
    lv_obj_t* _btn_settings;
    lv_obj_t* _label_status;

    // Keypad screen elements
    lv_obj_t* _textarea_amount;
    lv_obj_t* _btnm_keypad;
    lv_obj_t* _btn_keypad_cancel;
    lv_obj_t* _btn_keypad_ok;

    // Dispensing screen elements
    lv_obj_t* _label_disp_title;
    lv_obj_t* _label_disp_amount;
    lv_obj_t* _label_disp_target;
    lv_obj_t* _bar_progress;
    lv_obj_t* _label_progress;
    lv_obj_t* _btn_pause;
    lv_obj_t* _btn_resume;
    lv_obj_t* _btn_stop;

    // Config screen elements
    lv_obj_t* _textarea_ssid;
    lv_obj_t* _textarea_password;
    lv_obj_t* _btn_wifi_connect;
    lv_obj_t* _btn_calibrate;
    lv_obj_t* _btn_config_back;
    lv_obj_t* _label_wifi_status;

    // Calibration screen elements
    lv_obj_t* _label_calib_instructions;
    lv_obj_t* _textarea_calib_volume;
    lv_obj_t* _label_calib_pulses;
    lv_obj_t* _btn_calib_start;
    lv_obj_t* _btn_calib_save;
    lv_obj_t* _btn_calib_cancel;

    // Screen creation methods
    void createMainScreen();
    void createKeypadScreen();
    void createDispensingScreen();
    void createConfigScreen();
    void createCalibrationScreen();

    // Event handlers
    static void mainScreenEventHandler(lv_event_t* e);
    static void keypadEventHandler(lv_event_t* e);
    static void dispensingEventHandler(lv_event_t* e);
    static void configEventHandler(lv_event_t* e);
    static void calibrationEventHandler(lv_event_t* e);

    // Helper methods
    void updateDispensingScreen();
    void updateMainStatus();
};

// Global instance
extern UIManager uiManager;

#endif // UI_MANAGER_H
