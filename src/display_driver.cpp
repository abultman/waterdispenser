#include "display_driver.h"

esp_lcd_panel_handle_t panel_handle = NULL;

bool initRGBDisplay() {
    Serial.println("Initializing RGB LCD...");

    esp_lcd_rgb_panel_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));

    panel_config.clk_src = LCD_CLK_SRC_PLL160M;
    panel_config.timings.pclk_hz = 16 * 1000 * 1000;
    panel_config.timings.h_res = SCREEN_WIDTH;
    panel_config.timings.v_res = SCREEN_HEIGHT;
    panel_config.timings.hsync_pulse_width = 48;
    panel_config.timings.hsync_back_porch = 40;
    panel_config.timings.hsync_front_porch = 40;
    panel_config.timings.vsync_pulse_width = 31;
    panel_config.timings.vsync_back_porch = 13;
    panel_config.timings.vsync_front_porch = 1;
    panel_config.timings.flags.hsync_idle_low = 0;
    panel_config.timings.flags.vsync_idle_low = 0;
    panel_config.timings.flags.de_idle_high = 0;
    panel_config.timings.flags.pclk_active_neg = 1;
    panel_config.timings.flags.pclk_idle_high = 0;

    panel_config.data_width = 16;
    panel_config.sram_trans_align = 64;  // Match PSRAM alignment
    panel_config.psram_trans_align = 64;
    panel_config.hsync_gpio_num = LCD_HSYNC;
    panel_config.vsync_gpio_num = LCD_VSYNC;
    panel_config.de_gpio_num = LCD_DE;
    panel_config.pclk_gpio_num = LCD_PCLK;
    panel_config.disp_gpio_num = -1;

    panel_config.data_gpio_nums[0] = LCD_B0;
    panel_config.data_gpio_nums[1] = LCD_B1;
    panel_config.data_gpio_nums[2] = LCD_B2;
    panel_config.data_gpio_nums[3] = LCD_B3;
    panel_config.data_gpio_nums[4] = LCD_B4;
    panel_config.data_gpio_nums[5] = LCD_G0;
    panel_config.data_gpio_nums[6] = LCD_G1;
    panel_config.data_gpio_nums[7] = LCD_G2;
    panel_config.data_gpio_nums[8] = LCD_G3;
    panel_config.data_gpio_nums[9] = LCD_G4;
    panel_config.data_gpio_nums[10] = LCD_G5;
    panel_config.data_gpio_nums[11] = LCD_R0;
    panel_config.data_gpio_nums[12] = LCD_R1;
    panel_config.data_gpio_nums[13] = LCD_R2;
    panel_config.data_gpio_nums[14] = LCD_R3;
    panel_config.data_gpio_nums[15] = LCD_R4;

    panel_config.flags.disp_active_low = 0;
    panel_config.flags.fb_in_psram = 1;  // Enable PSRAM for framebuffer

    // Create RGB panel
    esp_err_t ret = esp_lcd_new_rgb_panel(&panel_config, &panel_handle);
    if (ret != ESP_OK) {
        Serial.printf("Failed to create RGB panel: %d\n", ret);
        return false;
    }

    // Reset and initialize panel
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // Setup backlight
    pinMode(LCD_BL, OUTPUT);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LCD_BL, 0);
    ledcWrite(0, 200);

    Serial.println("RGB LCD initialized successfully");
    return true;
}

void drawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data) {
    if (panel_handle) {
        esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + w, y + h, data);
    }
}

void* getFramebuffer() {
    // Direct framebuffer access not supported in this ESP-IDF version
    return NULL;
}
