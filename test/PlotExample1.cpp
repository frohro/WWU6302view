#include <Six302.h>
#include <math.h>
#include <WiFi.h>

// Create CommManager with adequate stack space
CommManager cm(5000, 50000);
float t, output;

void setup() {
   Serial.begin(115200);
   delay(1000);
   
   Serial.println("Starting ESP32 WiFi 6302view");
   
   // First add your plot
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   
   // Use a char array for SSID to avoid string conversion issues
   char ssid[] = "Frohne-2.4GHz";
   
   // Connect using CommManager's built-in connection method
   // For open networks, use an empty string, not NULL
   Serial.println("Connecting to WiFi using CommManager...");
   cm.connect(ssid, "");
   
   Serial.println("Setup complete");
}

void loop() {
   // Generate sine wave
   output = sin(t = t + 0.02);
   
   // Add delay to reduce CPU usage
   delay(10);
   
   // Allow CommManager to do its work
   cm.step();
}