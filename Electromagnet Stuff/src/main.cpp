#include <Arduino.h>

// Pin Configuration
const int ELECTROMAGNET_PIN = 4; // GPIO 4 (Pin D3 on XIAO ESP32-S3)

// ESP32 PWM (LEDC) Configuration
const int PWM_CHANNEL = 0;       // LEDC Channel 0 (0-15 available)
const int PWM_FREQ = 5000;       // 5 kHz frequency
const int PWM_RESOLUTION = 8;    // 8-bit resolution (0 to 255)

// Power Tuning (Lower power = less core heat = far less residual magnetism)
const int HOLD_DUTY = 180;       // ~70% power (enough to hold, avoids core saturation)

void setup() {
  Serial.begin(115200);

  // 1. Configure the PWM channel properties
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

  // 2. Attach the GPIO pin to the configured PWM channel
  ledcAttachPin(ELECTROMAGNET_PIN, PWM_CHANNEL);

  // Start with magnet fully de-energized
  ledcWrite(PWM_CHANNEL, 0);

  Serial.println("Electromagnet PWM controller initialized with demagnetization sequence.");
}

void turnOffMagnetWithDegaussDecay() {
  // Step 1: Rapid decaying oscillation simulation (exponential duty drop)
  // Stepping down with non-linear intervals breaks residual flux alignment
  int dutySteps[] = {180, 120, 160, 90, 120, 50, 70, 20, 30, 0};
  int numSteps = sizeof(dutySteps) / sizeof(dutySteps[0]);

  for (int i = 0; i < numSteps; i++) {
    ledcWrite(PWM_CHANNEL, dutySteps[i]);
    delay(15); // Short bursts allow flux field relaxation
  }

  // Step 2: Ensure hard 0V output
  ledcWrite(PWM_CHANNEL, 0);
  pinMode(ELECTROMAGNET_PIN, OUTPUT);
  digitalWrite(ELECTROMAGNET_PIN, LOW);
}

void loop() {
  // 1. Strike initial attraction at full power for 100ms
  Serial.println("Electromagnet ON (Engaging)");
  ledcWrite(PWM_CHANNEL, 255);
  delay(100); 

  // 2. Drop to holding power (180) to reduce heat & domain lock-in
  ledcWrite(PWM_CHANNEL, HOLD_DUTY);
  delay(2900); // Complete 3-second hold cycle

  // 3. Run demagnetization release decay
  Serial.println("Electromagnet OFF (Decaying Residual Magnetism)");
  turnOffMagnetWithDegaussDecay();

  // 4. Rest period to let coil & core cool completely
  delay(4000); 
}