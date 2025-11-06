#include "GT911.h"
#include "config.h"

GT911::GT911(uint8_t sda, uint8_t scl, uint8_t int_pin, uint8_t rst_pin, int width, int height) {
    _sda = sda;
    _scl = scl;
    _int_pin = int_pin;
    _rst_pin = rst_pin;
    _rotation = 0;
    _width = width;
    _height = height;
}

bool GT911::begin() {
    // Only configure INT pin if it's valid
    if (_int_pin >= 0) {
        pinMode(_int_pin, INPUT);
    }

    // Only configure RST pin if it's valid
    if (_rst_pin >= 0) {
        pinMode(_rst_pin, OUTPUT);
    }

    // Reset sequence
    reset();

    Wire.begin(_sda, _scl);
    Wire.setClock(400000);

    // Try both possible I2C addresses
    Wire.beginTransmission(GT911_ADDR1);
    if (Wire.endTransmission() == 0) {
        _addr = GT911_ADDR1;
        Serial.println("GT911 found at address 0x5D");
        return true;
    }

    Wire.beginTransmission(GT911_ADDR2);
    if (Wire.endTransmission() == 0) {
        _addr = GT911_ADDR2;
        Serial.println("GT911 found at address 0x14");
        return true;
    }

    Serial.println("GT911 not found!");
    return false;
}

void GT911::reset() {
    if (_rst_pin < 0) {
        // No reset pin, just wait
        delay(200);
        return;
    }

    digitalWrite(_rst_pin, LOW);
    delay(10);
    digitalWrite(_rst_pin, HIGH);
    delay(10);

    if (_int_pin >= 0) {
        digitalWrite(_int_pin, LOW);
        delay(50);
        pinMode(_int_pin, INPUT);
    }

    delay(100);
}

void GT911::writeReg(uint16_t reg, uint8_t value) {
    Wire.beginTransmission(_addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(value);
    Wire.endTransmission();
}

void GT911::readReg(uint16_t reg, uint8_t* buf, uint8_t len) {
    Wire.beginTransmission(_addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.endTransmission();

    Wire.requestFrom(_addr, len);
    for (uint8_t i = 0; i < len && Wire.available(); i++) {
        buf[i] = Wire.read();
    }
}

TouchPoint GT911::readTouch() {
    TouchPoint point = {0, 0, false};

    uint8_t point_info;
    readReg(GT911_POINT_INFO, &point_info, 1);

    // Check if touch data is ready
    if (!(point_info & 0x80)) {
        return point;
    }

    // Get number of touch points
    uint8_t touch_num = point_info & 0x0F;

    if (touch_num > 0) {
        uint8_t data[8];  // GT911 sends 8 bytes per touch point
        readReg(GT911_POINT_1, data, 8);

        // GT911 format: track_id(1) + x_low(1) + x_high(1) + y_low(1) + y_high(1) + size_low(1) + size_high(1) + reserved(1)
        uint16_t x = data[1] | (data[2] << 8);
        uint16_t y = data[3] | (data[4] << 8);

        point.x = (float)(((float)x / (float) _width) * 800.0);
        point.y = (float)(((float)y / (float) _height) * 480.0);

        point.touched = true;
    }

    // Clear touch flag
    writeReg(GT911_POINT_INFO, 0);

    return point;
}

void GT911::setRotation(uint8_t rotation) {
    _rotation = rotation % 4;
}
