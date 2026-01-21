#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

MKRIoTCarrier carrier;
int ledPin = A1; // Use Pin 1 for a breadboard LED
int buttonPin = A3; // Use Pin 3 for a breadboard button

void setup() {
  // The carrier board is not in the plastic case right now so we set this to false
  // This helps the library adjust brightness and other settings, especially with the capacitive touch sensors, accordingly
  CARRIER_CASE = false;
  
  // Initializing the carrier board, so it knows that we are using it
  //carrier.begin();

  pinMode(ledPin, OUTPUT);
  // INPUT_PULLUP makes the pin "HIGH" by default. 
  //Basically, it adds the pullup resistor so that we don't need to add an external one, which is pretty lit
 // When you press the button to GND, it becomes "LOW".
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Check if button is pressed (connected to GND)
  if (digitalRead(buttonPin) == LOW) {
    digitalWrite(ledPin, HIGH); // LED ON
  } else {
    digitalWrite(ledPin, LOW);  // LED OFF
  }
}
