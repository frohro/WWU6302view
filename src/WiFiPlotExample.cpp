#include <Six302.h>
#include <math.h>

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
   const char ssid[] = "Frohne-2.4GHz"; // Copy string literals to RAM
   const char pw[] = "";
   cm.connect(ssid, pw);
   
   // Or use a direct cast if you modified the library to accept const char*
   // cm.connect("Frohne-2.4GHz", "");
   delay(1000);
}

void loop() {
   output = sin(t = t + 0.02);
   cm.step();
}