#include <Arduino.h>

const int redPin = D1;
const int greenPin = D2;
const int bluePin = D6;
const int sensorPin = D0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // For Common Cathode, LOW is OFF
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void loop() {
  // RED
  digitalWrite(redPin, HIGH); // ON
  delay(1000);
  digitalWrite(redPin, LOW);  // OFF

  // GREEN
  digitalWrite(greenPin, HIGH);
  delay(1000);
  digitalWrite(greenPin, LOW);

  // BLUE
  digitalWrite(bluePin, HIGH);
  delay(1000);
  digitalWrite(bluePin, LOW);
}