/* * ======================================================================================
 * Project:      AllSafe Food Scanner v1.0
 * File Path:    include/header.h
 * Description:  
 * Central configuration header containing all hardware pin assignments and sensor
 * calibration settings. This file is included by main.cpp to keep configuration
 * separate from main logic.
 * ======================================================================================
 */

#ifndef HEADER_H
#define HEADER_H

// --- HARDWARE PIN ASSIGNMENTS ---
// LED indicator pins for visual feedback
const int RED_PIN = A2;      // Red LED for "Unsafe" indication
const int GREEN_PIN = A1;    // Green LED for "Safe" indication
const int SENSOR_PIN = A5;   // Analog input for light sensor

// --- SENSOR CALIBRATION ---
// Threshold value determines the transition point between "Safe" and "Unsafe" readings
// Values above this threshold trigger the red LED, values below trigger green LED
// This can be overridden at compile-time via platformio.ini build flags from .env
#ifndef SENSOR_THRESHOLD
const int SENSOR_THRESHOLD = 700;
#else
const int SENSOR_THRESHOLD = SENSOR_THRESHOLD;
#endif

// --- LED ANIMATION SETTINGS ---
const int BLINK_COUNT = 20;        // Number of times to blink the LED
const int BLINK_ON_TIME = 100;     // LED on duration in milliseconds
const int BLINK_OFF_TIME = 100;    // LED off duration in milliseconds

#endif // HEADER_H
