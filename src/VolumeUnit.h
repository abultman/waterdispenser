#ifndef VOLUME_UNIT_H
#define VOLUME_UNIT_H

#include <Arduino.h>

// Enum for unit types
enum VolumeUnitType {
    UNIT_MILLILITERS = 0,
    UNIT_LITERS = 1
};

// Abstract base class for volume units
class VolumeUnit {
public:
    virtual ~VolumeUnit() {}

    // Convert milliliters to display value
    virtual float toDisplay(int milliliters) const = 0;

    // Convert display value to milliliters
    virtual int toMilliliters(float displayValue) const = 0;

    // Format value for display (returns formatted string)
    virtual String format(int milliliters) const = 0;

    // Get unit suffix for labels
    virtual const char* getSuffix() const = 0;

    // Get unit type
    virtual VolumeUnitType getType() const = 0;
};

// Milliliters implementation
class MillilitersUnit : public VolumeUnit {
public:
    float toDisplay(int milliliters) const override {
        return (float)milliliters;
    }

    int toMilliliters(float displayValue) const override {
        return (int)displayValue;
    }

    String format(int milliliters) const override {
        return String(milliliters);
    }

    const char* getSuffix() const override {
        return "ml";
    }

    VolumeUnitType getType() const override {
        return UNIT_MILLILITERS;
    }
};

// Liters implementation
class LitersUnit : public VolumeUnit {
public:
    float toDisplay(int milliliters) const override {
        return milliliters / 1000.0f;
    }

    int toMilliliters(float displayValue) const override {
        return (int)(displayValue * 1000.0f);
    }

    String format(int milliliters) const override {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.3f", milliliters / 1000.0f);
        return String(buf);
    }

    const char* getSuffix() const override {
        return "L";
    }

    VolumeUnitType getType() const override {
        return UNIT_LITERS;
    }
};

// Factory function to get unit instance
inline const VolumeUnit* getVolumeUnit(VolumeUnitType type) {
    static MillilitersUnit mlUnit;
    static LitersUnit lUnit;

    switch (type) {
        case UNIT_MILLILITERS:
            return &mlUnit;
        case UNIT_LITERS:
            return &lUnit;
        default:
            return &mlUnit;
    }
}

#endif // VOLUME_UNIT_H
