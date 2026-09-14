#include <Arduino.h>

// Pin Configuration
const int ELECTROMAGNET_PIN = 4; // GPIO 4 (Pin D3 on XIAO ESP32-S3)

// ESP32 PWM (LEDC) Configuration
const int PWM_CHANNEL = 0;       // LEDC Channel 0 (0-15 available)
const int PWM_FREQ = 5000;       // 5 kHz frequency
const int PWM_RESOLUTION = 8;    // 8-bit resolution (0 to 255)

void setup() {
  Serial.begin(115200);

  // 1. Configure the PWM channel properties
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

  // 2. Attach the GPIO pin to the configured PWM channel
  ledcAttachPin(ELECTROMAGNET_PIN, PWM_CHANNEL);

  // Start with magnet fully de-energized
  ledcWrite(PWM_CHANNEL, 0);

  Serial.println("Electromagnet PWM controller initialized.");
}

void turnOffMagnetSmoothly() {
  // Rapidly ramp down duty cycle from 255 (100%) down to 0 over ~100ms
  for (int duty = 255; duty >= 0; duty -= 15) {
    ledcWrite(PWM_CHANNEL, duty);
    delay(5);
  }
  ledcWrite(PWM_CHANNEL, 0); // Ensure output is 0V
}

void loop() {
  // 1. Turn ON the electromagnet at 100% duty cycle
  Serial.println("Electromagnet ON (Engaged)");
  ledcWrite(PWM_CHANNEL, 255);
  delay(3000); // Hold for 2 seconds

  // 2. Smoothly taper OFF to break residual magnetism
  Serial.println("Electromagnet OFF (Disengaging)");
  turnOffMagnetSmoothly();

  // 3. Rest period to prevent overheating
  delay(4000); // Rest for 3 seconds
}