#include "HardwareControl.h"
#include "config.h"
#include <Preferences.h>

// Global instance
HardwareControl hardwareControl;

// ISR wrapper function
void IRAM_ATTR flowPulseISR() {
    hardwareControl.handleFlowPulse();
}

HardwareControl::HardwareControl() {
    _pulseCount = 0;
    _pulsesPerLiter = DEFAULT_PULSES_PER_LITER;
    _targetML = 0;
    _dispensedML = 0;
    _state = IDLE;
    _valveOpen = false;
    _lastPulseTime = 0;
    _dispensingStartTime = 0;
    _lastFlowCheckTime = 0;
    _pauseStartTime = 0;
    _totalPausedTime = 0;
    _lastPulseCount = 0;
}

void HardwareControl::begin() {
    // Setup valve pin
    pinMode(VALVE_PIN, OUTPUT);
    closeValve();

    // Setup flow sensor pin with interrupt
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowPulseISR, RISING);

    // Load calibration from preferences
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true)) {
        _pulsesPerLiter = prefs.getFloat("pulses_per_l", DEFAULT_PULSES_PER_LITER);
        prefs.end();
        Serial.printf("Loaded calibration: %.2f pulses/L\n", _pulsesPerLiter);
    }
}

void HardwareControl::openValve() {
    digitalWrite(VALVE_PIN, HIGH);
    _valveOpen = true;
    Serial.println("Valve OPEN");
}

void HardwareControl::closeValve() {
    digitalWrite(VALVE_PIN, LOW);
    _valveOpen = false;
    Serial.println("Valve CLOSED");
}

bool HardwareControl::isValveOpen() {
    return _valveOpen;
}

void HardwareControl::resetFlowCounter() {
    noInterrupts();
    _pulseCount = 0;
    _dispensedML = 0;
    interrupts();
}

float HardwareControl::getDispensedAmount() {
    noInterrupts();
    uint32_t pulses = _pulseCount;
    interrupts();

    // Convert pulses to milliliters
    return (pulses / _pulsesPerLiter) * 1000.0;
}

float HardwareControl::getFlowRate() {
    unsigned long now = millis();
    unsigned long timeDiff = now - _lastFlowCheckTime;

    if (timeDiff < 100) {
        return 0;  // Too soon to measure
    }

    noInterrupts();
    uint32_t currentPulses = _pulseCount;
    interrupts();

    uint32_t pulseDiff = currentPulses - _lastPulseCount;
    _lastPulseCount = currentPulses;
    _lastFlowCheckTime = now;

    // Calculate flow rate in ml/s
    float pulsesPerSecond = (pulseDiff * 1000.0) / timeDiff;
    return (pulsesPerSecond / _pulsesPerLiter) * 1000.0;
}

void HardwareControl::startDispensing(float targetML) {
    Serial.printf("Starting to dispense %.2f ml\n", targetML);

    _targetML = targetML;
    _dispensedML = 0;
    _state = DISPENSING;
    _dispensingStartTime = millis();
    _lastFlowCheckTime = millis();
    _lastPulseCount = 0;
    _totalPausedTime = 0;

    resetFlowCounter();
    openValve();
}

void HardwareControl::pauseDispensing() {
    if (_state != DISPENSING) {
        return;
    }

    closeValve();
    _state = PAUSED;
    _pauseStartTime = millis();
    Serial.printf("Dispensing paused at %.2f ml\n", getDispensedAmount());
}

void HardwareControl::resumeDispensing() {
    if (_state != PAUSED) {
        return;
    }

    // Track total paused time to adjust timeout calculations
    _totalPausedTime += (millis() - _pauseStartTime);

    _state = DISPENSING;
    _lastFlowCheckTime = millis();
    openValve();
    Serial.printf("Dispensing resumed from %.2f ml\n", getDispensedAmount());
}

void HardwareControl::stopDispensing() {
    closeValve();

    if (_state == DISPENSING || _state == PAUSED) {
        _state = STOPPING;
    }

    Serial.printf("Dispensing stopped. Dispensed: %.2f ml\n", getDispensedAmount());
}

DispensingState HardwareControl::getState() {
    return _state;
}

float HardwareControl::getTargetAmount() {
    return _targetML;
}

float HardwareControl::getRemainingAmount() {
    float remaining = _targetML - getDispensedAmount();
    return remaining > 0 ? remaining : 0;
}

uint8_t HardwareControl::getProgress() {
    if (_targetML <= 0) return 0;

    float progress = (getDispensedAmount() / _targetML) * 100.0;
    return progress > 100 ? 100 : (uint8_t)progress;
}

void HardwareControl::setCalibrationFactor(float pulsesPerLiter) {
    _pulsesPerLiter = pulsesPerLiter;

    // Save to preferences
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false)) {
        prefs.putFloat("pulses_per_l", pulsesPerLiter);
        prefs.end();
        Serial.printf("Saved calibration: %.2f pulses/L\n", pulsesPerLiter);
    }
}

float HardwareControl::getCalibrationFactor() {
    return _pulsesPerLiter;
}

void HardwareControl::update() {
    // Only update when actively dispensing (not when paused)
    if (_state != DISPENSING) {
        return;
    }

    unsigned long now = millis();
    _dispensedML = getDispensedAmount();

    // Check if target reached (with overshoot compensation)
    if (_dispensedML >= (_targetML - OVERSHOOT_COMPENSATION)) {
        stopDispensing();
        _state = COMPLETED;
        Serial.println("Target reached!");
        return;
    }

    // Check for flow timeout (accounting for paused time)
    if (now - _lastPulseTime > FLOW_TIMEOUT) {
        stopDispensing();
        _state = ERROR_TIMEOUT;
        Serial.println("Error: Flow timeout!");
        return;
    }

    // Check if flow has started (accounting for total paused time)
    unsigned long activeTime = (now - _dispensingStartTime) - _totalPausedTime;
    if (activeTime > 2000 && _pulseCount < 5) {
        stopDispensing();
        _state = ERROR_NO_FLOW;
        Serial.println("Error: No flow detected!");
        return;
    }
}

void IRAM_ATTR HardwareControl::handleFlowPulse() {
    _pulseCount++;
    _lastPulseTime = millis();
}
