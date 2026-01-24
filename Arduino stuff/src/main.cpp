/* * Project: iGem Dino Prototype (Bootcamp)
 * Board: Arduino MKR WiFi 1010
 */

#include <Arduino.h> // Base Arduino functions
#include <SPI.h>     // Provides the "highway" to the WiFi chip
#include <WiFiNINA.h> // The "driver" that tells the WiFi chip how to connect

// Wifi Network Credentials
// const char* ssid = "Jordan's S23 FE";
// const char* password = "jesuslovesyou";
WiFiServer server(80); // Create a server that listens on port 80

// Hardware Pin Definitions
int redPin = A2;    // Red LED connected to Analog Pin 2
int greenPin = A1;  // Green LED connected to Analog Pin 1
int sensorPin = A5; // Light sensor (LDR or Photodiode) connected to Analog Pin 5

// Configuration Settings
int threshold = 700;   // The light level that triggers flashing light
bool isTriggered = false; // "Lock" to prevent the game from looping
bool testPushed = false;  // Flag to indicate if the test button was pushed

void setup() {
  // Initialize Hardware and Serial Communication
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  Serial.begin(9600); 

  // Wait for Serial Monitor with a 5-second timeout so it doesn't hang forever
  unsigned long startWait = millis();
  while (!Serial && millis() - startWait < 5000); 

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    // FIXED: Added 'password' back in so it can connect to your S23 FE
    WiFi.begin(ssid, password);
    Serial.print(".");
    delay(2000); // 2 seconds is better for mobile hotspots
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin(); // Start the web server
}

void loop() {
  int lightLevel = analogRead(sensorPin);
  
  // --- WEB SERVER LOGIC ---
  WiFiClient client = server.available(); 
  if (client) {                             
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<h1>AllSafe Dino Remote</h1>");
            client.print("<p><a href=\"/T\" style='font-size:50px;'>[ CLICK TO TEST ]</a></p>");
            client.println();
            break;
          } else { currentLine = ""; }
        } else if (c != '\r') { currentLine += c; }

        if (currentLine.endsWith("GET /T")) {
           testPushed = true;    // This is our "Go" signal
        }
      }
    }
    client.stop(); 
  }

  // --- TRIGGER LOGIC ---
  // If the button was pushed, run the sequence ONCE
  if (testPushed) {
    
    int winningPin;

    // 1. DECIDE: Look at the light level right now
    if (lightLevel > threshold) {
      winningPin = redPin;   // It's Bright!
      Serial.println("Result: Red");
    } else {
      winningPin = greenPin; // It's Dark!
      Serial.println("Result: Green");
    }
    
    // 2. DELAY: Optional 1s pause for suspense
    delay(500); 

    // 3. ACTION: Flash the chosen LED 10 times
    for (int i = 0; i < 10; i++) {
      digitalWrite(winningPin, HIGH);
      delay(150);
      digitalWrite(winningPin, LOW);
      delay(150);
    }

    // 4. RESET: Set testPushed to false so it stops until the next click
    testPushed = false; 
    isTriggered = true; // Prevents re-triggering until light reset if needed
  }

  // Reset logic for the light sensor threshold
  if (analogRead(sensorPin) < (threshold - 50)) {
    isTriggered = false;
  }
}