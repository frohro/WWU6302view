#include <Six302.h>
#include <math.h>
#include <WiFi.h>

CommManager cm(5000, 50000);
float t, output;

void setup() {
   // Initialize serial with sufficient time to stabilize
   Serial.begin(115200);
   delay(1000);
   
   // Print debug information
   Serial.println("Starting ESP32-C3 WiFi 6302view");
   Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
   
   // Example plot
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   
   // Connect with string copies (workaround if you can't modify the library)
   char ssid[] = "NatsNet"; // Copy string literals to RAM
   char pw[] = "curiosity"; // Double-check this password is correct
   
   Serial.print("Connecting to WiFi");
   
   // Set WiFi mode explicitly to station mode
   WiFi.mode(WIFI_STA);
   
   // Disconnect from any previous connection
   WiFi.disconnect(true);
   delay(1000);
   
   // Add a timeout and retry mechanism
   int attempts = 0;
   const int maxAttempts = 10;
   
   while (attempts < maxAttempts) {
      Serial.printf("\nAttempt %d to connect to %s\n", attempts + 1, ssid);
      
      // Try direct WiFi connection first
      WiFi.begin(ssid, pw);
      
      // Wait for connection or timeout
      unsigned long startTime = millis();
      bool connected = false;
      
      while (millis() - startTime < 10000) { // 10 second timeout
         if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            break;
         }
         Serial.print(".");
         delay(500);
      }
      
      if (connected) {
         Serial.println();
         Serial.println("WiFi connected successfully!");
         Serial.print("IP address: ");
         Serial.println(WiFi.localIP());
         
         // Now connect the CommManager
         cm.connect(ssid, pw);
         break;
      } else {
         Serial.println();
         Serial.printf("Connection attempt %d failed\n", attempts + 1);
         Serial.printf("WiFi status: %d\n", WiFi.status());
         attempts++;
         delay(1000);
      }
   }
   
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi after multiple attempts");
      Serial.println("Please check the SSID and password");
      // You could add fallback behavior here
   }
}

void loop() {
   output = sin(t = t + 0.02);
   
   // Add delay to reduce potential for stack overflows
   delay(10);
   
   cm.step();
}