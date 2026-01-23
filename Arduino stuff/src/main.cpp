#include <Arduino.h>

int redPin = A2;
int greenPin = A1;
int buttonPin = A5;

bool buttonWasReleased = true; 

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  // Check for button press
  if (buttonState == LOW && buttonWasReleased) {
    
    // 1. Pick the winner FIRST
    int choice = random(0, 10); 
    int winningPin;

    if (choice < 4) {
      winningPin = redPin;
    } else {
      winningPin = greenPin;
    }

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

    buttonWasReleased = false; 
  }

  // Reset and keep the random seed moving while waiting
  if (buttonState == HIGH) {
    buttonWasReleased = true;
    randomSeed(micros()); 
  }
}