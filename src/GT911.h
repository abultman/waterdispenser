#ifndef GT911_H
#define GT911_H

#include <Arduino.h>
#include <Wire.h>

#define GT911_ADDR1 0x5D
#define GT911_ADDR2 0x14

#define GT911_POINT_INFO  0x814E
#define GT911_POINT_1     0x814F

struct TouchPoint {
    uint16_t x;
    uint16_t y;
    bool touched;
};

class GT911 {
public:
    GT911(uint8_t sda, uint8_t scl, uint8_t int_pin, uint8_t rst_pin, int width, int height);
    bool begin();
    TouchPoint readTouch();
    void setRotation(uint8_t rotation);

private:
    uint8_t _sda, _scl, _int_pin, _rst_pin;
    uint8_t _addr;
    uint8_t _rotation;
    uint16_t _width, _height;

    void writeReg(uint16_t reg, uint8_t value);
    void readReg(uint16_t reg, uint8_t* buf, uint8_t len);
    void reset();
};

#endif // GT911_H
