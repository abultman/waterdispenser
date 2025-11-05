#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#include <Arduino.h>

enum DispensingState {
    IDLE,
    DISPENSING,
    PAUSED,
    STOPPING,
    COMPLETED,
    ERROR_TIMEOUT,
    ERROR_NO_FLOW
};

class HardwareControl {
public:
    HardwareControl();
    void begin();

    // Valve control
    void openValve();
    void closeValve();
    bool isValveOpen();

    // Flow sensor
    void resetFlowCounter();
    float getDispensedAmount();  // Returns amount in ml
    float getFlowRate();  // Returns flow rate in ml/s

    // Dispensing control
    void startDispensing(float targetML);
    void pauseDispensing();
    void resumeDispensing();
    void stopDispensing();
    DispensingState getState();
    float getTargetAmount();
    float getRemainingAmount();
    uint8_t getProgress();  // Returns 0-100

    // Calibration
    void setCalibrationFactor(float pulsesPerLiter);
    float getCalibrationFactor();

    // Update loop - call this regularly
    void update();

    // Interrupt handler (must be public for ISR attachment)
    void IRAM_ATTR handleFlowPulse();

private:
    volatile uint32_t _pulseCount;
    float _pulsesPerLiter;
    float _targetML;
    float _dispensedML;
    DispensingState _state;
    bool _valveOpen;

    unsigned long _lastPulseTime;
    unsigned long _dispensingStartTime;
    unsigned long _lastFlowCheckTime;
    unsigned long _pauseStartTime;
    unsigned long _totalPausedTime;
    uint32_t _lastPulseCount;
};

// Global instance for ISR access
extern HardwareControl hardwareControl;

#endif // HARDWARE_CONTROL_H
