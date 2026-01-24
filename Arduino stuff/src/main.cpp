#include <Arduino.h>

int redPin = A2;
int greenPin = A1;
int sensorPin = A5;

// Adjust this number (0-1023) to change sensitivity
// Higher = triggered by bright light
int threshold = 700; 
 
// Makes it so it only triggers once per light event
bool isTriggered = false;

void setup() {
  Serial.begin(9600); // Opens the communication gate to the computer
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  //Uncomment this line if using a button instead of a sensor
  // the pullup acts as a pullup resistor so you don't need an external one
  // pinMode(sensorPin, INPUT_PULLUP);
}

void loop() {
  int lightLevel = analogRead(sensorPin);

  // Print the light level to the Serial Monitor
  Serial.print("Light Level: ");
  Serial.println(lightLevel);

  // Trigger if light exceeds threshold (e.g., a flashlight hits it)
  // Switch to < threshold if you want it to trigger when you cover it
  if (lightLevel > threshold && !isTriggered) {
    
    // 1. Pick the winner FIRST
    int choice = random(0, 10); 
    int winningPin;

    if (choice < 4) {
      winningPin = redPin;
    } else {
      winningPin = greenPin;
    }
    delay(3000); // Wait 3 seconds before revealing the winner
    
    // 2. Flash ONLY the winning color for 3 seconds
    // (10 cycles of 150ms ON + 150ms OFF = 3 seconds)
    for (int i = 0; i < 10; i++) {
      digitalWrite(winningPin, HIGH);
      delay(150);
      digitalWrite(winningPin, LOW);
      delay(150);
    }

    // 3. Make sure it ends in the OFF state
    digitalWrite(winningPin, LOW);

    isTriggered = true; 
  }

  // Reset and keep the random seed moving while waiting
  if (lightLevel < (threshold -50)) {
    isTriggered = false;
    randomSeed(micros()); 
  }
}