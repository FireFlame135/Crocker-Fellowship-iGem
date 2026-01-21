#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

MKRIoTCarrier carrier;

void setup() {
  // The carrier board is in the plastic case right now so we set this to true
  // This helps the library adjust brightness and other settings, especially with the capacitive touch sensors, accordingly
  CARRIER_CASE = true;
  
  // Initializing the carrier board, so it knows that we are using it
  carrier.begin();
}

void loop() {
  // Red light, flash for half a second
  carrier.leds.fill(carrier.leds.Color(255, 0, 0), 0, 5);
  carrier.leds.show();
  delay(500);

  // Green light, flash for half a second
  carrier.leds.fill(carrier.leds.Color(0, 255, 0), 0, 5);
  carrier.leds.show();
  delay(500);

  // Blue light, flash for half a second
  carrier.leds.fill(carrier.leds.Color(0, 0, 255), 0, 5);
  carrier.leds.show();
  delay(500);

  // White light (all colors mixed), flash for half a second
  carrier.leds.fill(carrier.leds.Color(255, 255, 255), 0, 5);
  carrier.leds.show();
  delay(500);
  
  // OFF
  carrier.leds.clear();
  carrier.leds.show();
  delay(500);
}