/* * ======================================================================================
 * Project:      AllSafe Food Scanner v1.0
 * File Path:    src/main.cpp
 * Description:  
 * This file acts as the central controller for the device. It performs three main roles:
 * 1. Hardware Management: Initializes GPIO pins for LEDs and reads analog sensor data.
 * 2. Network Server: Connects to WiFi and establishes a web server on port 80.
 * 3. Request Handling: implementing a "No-Refresh" AJAX workflow. It serves the 
 * frontend interface in fragmented packets to preserve memory and handles 
 * background scan requests (/T) by returning raw text data.
 * ======================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include "website.h" 

// --- NETWORK CONFIGURATION ---
const char* ssid = "Jordan's S23 FE"; 
const char* password = "jesuslovesyou";

WiFiServer server(80); 

// --- HARDWARE PIN ASSIGNMENTS ---
const int redPin = D1;    
const int greenPin = D2;  
const int sensorPin = A0; // labeled D0 on the seeed but we want analog input

// --- SENSOR CALIBRATION ---
// Threshold determines the transition point between "Safe" and "Unsafe" readings.
const int threshold = 500;      

void setup() {
  // Initialize hardware pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  // Initialize serial communication for debugging
  Serial.begin(115200); // ESP32 standard baud rate

  // Wait for serial monitor to open (with 5-second timeout for standalone operation)
  unsigned long startWait = millis();
  while (!Serial && millis() - startWait < 5000); 

  // Check for WiFi hardware
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi Module Failed!");
    while (true);
  }

  // Attempt to connect to the WiFi network
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print(".");
    delay(2000); 
  }

  // Output connection details
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.begin(); 
}

void loop() {
  WiFiClient client = server.available(); 
  Serial.print("Raw Sensor Value: ");
  Serial.println(analogRead(sensorPin));
  delay(2000);

  if (client) {                     
    String currentLine = "";                
    String requestPath = ""; 

    // Process the incoming client request
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             
        
        // Accumulate characters to detect the request type (GET /T vs GET /)
        // Limiting length prevents memory exhaustion on long headers
        if (currentLine.length() < 100) currentLine += c;

        if (c == '\n') {                    
          // If the line is blank, the HTTP headers have ended.
          // We now respond based on the detected request path.
          if (currentLine.length() <= 2) { 
            
            // --- CASE 1: AJAX SCAN REQUEST ---
            // The frontend is requesting a sensor reading without reloading the page.
            if (requestPath.indexOf("GET /T") >= 0) {
               Serial.println("Scan Requested...");
               int lightLevel = analogRead(sensorPin);
               String result = "";

               // 1. PERFORM LOGIC
               if (lightLevel > threshold) {
                 result = "RED";
                 Serial.println("Result: UNSAFE");
               } else {
                 result = "GREEN";
                 Serial.println("Result: SAFE");
               }

               // 2. SEND RESPONSE IMMEDIATELY 
               // We send the data and close the connection BEFORE the blinking starts
               // so the browser can update the UI instantly.
               client.println("HTTP/1.1 200 OK");
               client.println("Content-Type: text/plain; charset=utf-8");
               client.println("Connection: close");
               client.println("Access-Control-Allow-Origin: *"); 
               client.println();
               client.print(result);
               client.flush(); // Ensure data is sent
               client.stop();  // Disconnect client to trigger frontend success

               // 3. VISUAL FEEDBACK (BLOCKING)
               // Now that the phone has its answer, we can block the CPU for the light show.
               if (result == "RED") {
                 for (int i = 0; i < 20; i++) { 
                   digitalWrite(redPin, HIGH); delay(100);
                   digitalWrite(redPin, LOW);  delay(100);
                 }
               } else {
                 for (int i = 0; i < 20; i++) { 
                   digitalWrite(greenPin, HIGH); delay(100);
                   digitalWrite(greenPin, LOW);  delay(100);
                 }
               }
            }
            
            // --- CASE 2: INITIAL PAGE LOAD ---
            // Serve the HTML interface. The content is split into smaller strings
            // to ensure the network buffer does not overflow during transmission.
            else {
               client.println("HTTP/1.1 200 OK");
               client.println("Content-Type: text/html; charset=utf-8"); 
               client.println("Connection: close");  
               client.println();
               
               client.print(HTML_HEAD);
               client.print(HTML_CSS_CORE); // Layout styles
               client.print(HTML_CSS_BTN);  // Button animation styles
               client.print(HTML_BODY_TOP);
               client.print(HTML_LOGO);
               client.print(HTML_MENU);
               client.print(HTML_CONTROLS);
               client.print(HTML_SCRIPTS);
            }
            
            break;
          } else { 
            // Capture the HTTP method and path
            if (currentLine.startsWith("GET ")) requestPath = currentLine;
            currentLine = ""; 
          }
        } else if (c != '\r') { 
          // Character is part of the current line, keep reading
        }
      }
    }
    client.stop(); 
  }
}