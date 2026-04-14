/* * ======================================================================================
 * Project:      ChompSafe Food Scanner v1.0
 * File Path:    include/header.h
 * Description:  
 * Central configuration header containing all hardware pin assignments and sensor
 * calibration settings. This file is included by main.cpp to keep configuration
 * separate from main logic.
 * ======================================================================================
 */

#ifndef HEADER_H
#define HEADER_H

#include <Arduino.h>

// --- HARDWARE PIN ASSIGNMENTS ---
// LED indicator pins for visual feedback
#define RED_PIN A2      // Red LED for "Unsafe" indication
#define GREEN_PIN A1    // Green LED for "Safe" indication
#define SENSOR_PIN A5   // Analog input for light sensor

// --- SENSOR CALIBRATION ---
// Threshold value determines the transition point between "Safe" and "Unsafe" readings.
// The BLE firmware owns the configurable threshold; this legacy sketch keeps a stable default.
#define SENSOR_THRESHOLD_VALUE 700

// --- LED ANIMATION SETTINGS ---
#define BLINK_COUNT 20        // Number of times to blink the LED
#define BLINK_ON_TIME 100     // LED on duration in milliseconds
#define BLINK_OFF_TIME 100    // LED off duration in milliseconds

#endif // HEADER_H
