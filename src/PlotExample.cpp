#include <Six302.h>
#include <math.h>

// Create CommManager with adequate step period and report period
CommManager cm(5000, 50000);

float t, output;

void setup() {
   // Initialize serial first and wait for stability
   Serial.begin(115200);
   delay(1000);
   
   Serial.println("\n\nStarting ESP32 WiFi PlotExample");
   
   // Add plot before connecting to WiFi
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   
   // Connect to WiFi through CommManager directly
   // Use a proper WiFi configuration
   Serial.println("Connecting to WiFi through CommManager...");
   char ssid[] = "Frohne-2.4GHz";
   char password[] = ""; // Empty string for open network
   
   // Let CommManager handle the WiFi connection
   cm.connect(ssid, password);
}

void loop() {
   // Generate sine wave
   output = sin(t = t + 0.02);
   
   // Add delay to reduce CPU usage
   delay(10);
   
   // Let CommManager handle everything
   cm.step();
}