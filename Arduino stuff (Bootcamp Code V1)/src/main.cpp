/* * ======================================================================================
 * Project:      ChompSafe Food Scanner v1.0
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
#include <SPI.h>
#include <WiFiNINA.h>
#include "header.h"
#include "website.h" 

// --- NETWORK CONFIGURATION ---
<<<<<<<< HEAD:Arduino stuff (Bootcamp Code V1)/src/main.cpp
const char* ssid = "Jordan's S23 FE"; 
const char* password = "jesuslovesyou";

WiFiServer server(80); 

// --- HARDWARE PIN ASSIGNMENTS ---
const int redPin = A2;    
const int greenPin = A1;  
const int sensorPin = A4; 

// --- SENSOR CALIBRATION ---
// Threshold determines the transition point between "Safe" and "Unsafe" readings.
const int threshold = 75;      
========
// WiFi credentials are injected at compile-time from .env via platformio.ini
#ifndef WIFI_SSID
#define WIFI_SSID "YourNetworkName"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YourPassword"
#endif

const char* ssid = WIFI_SSID; 
const char* password = WIFI_PASSWORD;

WiFiServer server(80); 

// Hardware configuration and sensor settings are now in header.h      
>>>>>>>> fe3a983574720c6ca367a7903b777b44b283d586:src/main.cpp

void setup() {
  // Initialize hardware pins (configuration from header.h)
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  
  // Initialize serial communication for debugging
  Serial.begin(9600); 

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
               int lightLevel = analogRead(SENSOR_PIN);
               String result = "";

               // 1. PERFORM LOGIC
               if (lightLevel > SENSOR_THRESHOLD) {
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
                 for (int i = 0; i < BLINK_COUNT; i++) { 
                   digitalWrite(RED_PIN, HIGH); delay(BLINK_ON_TIME);
                   digitalWrite(RED_PIN, LOW);  delay(BLINK_OFF_TIME);
                 }
               } else {
                 for (int i = 0; i < BLINK_COUNT; i++) { 
                   digitalWrite(GREEN_PIN, HIGH); delay(BLINK_ON_TIME);
                   digitalWrite(GREEN_PIN, LOW);  delay(BLINK_OFF_TIME);
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