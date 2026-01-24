/* * Project: iGem Dino Prototype (Bootcamp)
 * Board: Arduino MKR WiFi 1010
 */

#include <Arduino.h> // Base Arduino functions
#include <SPI.h>     // Provides the "highway" to the WiFi chip
#include <WiFiNINA.h> // The "driver" that tells the WiFi chip how to connect

// Wifi Network Credentials
const char* ssid = "Hardy Party";
const char* password = "timmyg456";
WiFiServer server(80); // Create a server that listens on port 80

// Hardware Pin Definitions
int redPin = A2;    // Red LED connected to Analog Pin 2
int greenPin = A1;  // Green LED connected to Analog Pin 1
int sensorPin = A5; // Light sensor (LDR or Photodiode) connected to Analog Pin 5

// Configuration Settings
int threshold = 700;   // The light level that triggers flashing light
bool isTriggered = false; // "Lock" to prevent the game from looping

void setup() {
  // Initialize Hardware and Serial Communication
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  Serial.begin(9600); 

  // Wifi Stuff
  while (!Serial); 

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin(); // Start the web server
  delay(10000); // Wait for 10 seconds before starting the main loop
}

void loop() {
  int lightLevel = analogRead(sensorPin);
  
  // --- NEW: WEB SERVER LOGIC ---
  WiFiClient client = server.available(); 
  if (client) {                             
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            // Send standard HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // What you see on your phone:
            client.print("<h1>AllSafe Dino Remote</h1>");
            client.print("<p><a href=\"/T\">[ CLICK TO TEST ]</a></p>");
            client.println();
            break;
          } else { currentLine = ""; }
        } else if (c != '\r') { currentLine += c; }

        // Check if the user clicked the link
        if (currentLine.endsWith("GET /T")) {
           isTriggered = false; // Force a reset so the sequence can run
           lightLevel = 999;    // "Fake" a light trigger to bypass the if-statement below
        }
      }
    }
    client.stop(); // Close the connection
  }

  // Print the light level to the Serial Monitor
  Serial.print("Light Level: ");
  Serial.println(lightLevel);

  // --- YOUR ORIGINAL TRIGGER LOGIC ---
  if (lightLevel > threshold && !isTriggered) {
    
    int choice = random(0, 10); 
    int winningPin;

    if (choice < 4) {
      winningPin = redPin;
    } else {
      winningPin = greenPin;
    }
    
    delay(3000); 

    for (int i = 0; i < 10; i++) {
      digitalWrite(winningPin, HIGH);
      delay(150);
      digitalWrite(winningPin, LOW);
      delay(150);
    }

    digitalWrite(winningPin, LOW);
    isTriggered = true; 
  }

  if (lightLevel < (threshold - 50)) {
    isTriggered = false;
    randomSeed(micros()); 
  }
}