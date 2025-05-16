#include <Six302.h>
#include <math.h>
#include <WiFi.h>

CommManager cm(5000, 50000);
float t, output;

void setup() {
   Serial.begin(115200);
   delay(1000);
   
   Serial.println("Starting ESP32 WiFi 6302view");
   Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
   
   // Add plot BEFORE connecting
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   
   // SIMPLIFIED APPROACH: Let CommManager handle WiFi connection
   // Instead of managing WiFi connection separately and then calling cm.connect()
   char ssid[] = "Frohne-2.4GHz";
   
   Serial.println("Letting CommManager handle WiFi connection...");
   
   // IMPORTANT: Don't pass NULL, use empty string for open networks
   cm.connect(ssid, "");  // Empty string for open networks
   
   Serial.println("CommManager setup complete");
}

void loop() {
   output = sin(t = t + 0.02);
   delay(10);
   cm.step();
}