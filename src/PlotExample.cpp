#include <Six302.h>
#include <math.h>

// Create CommManager with default step period and report period
CommManager cm(5000, 50000);

float t, output;

// Task handle for debugging
TaskHandle_t commTaskHandle = NULL;

void setup() {
   // Initialize serial first and wait for stability
   Serial.begin(115200);
   delay(1000);
   
   Serial.println("\n\nStarting ESP32 WiFi PlotExample");
   
   // Add plot before anything else
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   
   // Create a proper password string (not NULL) for open network
   char ssid[] = "Frohne-2.4GHz";
   char password[] = "";
   
   // Connect with stable parameters
   Serial.println("Connecting to WiFi...");
   WiFi.disconnect(true);
   delay(1000);
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   
   // Wait for connection with timeout
   int timeout = 0;
   while (WiFi.status() != WL_CONNECTED && timeout < 20) {
     delay(500);
     Serial.print(".");
     timeout++;
   }
   
   if (WiFi.status() == WL_CONNECTED) {
     Serial.println("\nWiFi connected!");
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());
     
     // Now tell CommManager to use the existing connection
     Serial.println("Setting up CommManager with existing connection...");
     // Call modified connect method that doesn't try to reconnect WiFi
     cm.connect(ssid, password);  
   } else {
     Serial.println("WiFi connection failed!");
   }
}

void loop() {
   // Generate sine wave
   output = sin(t = t + 0.02);
   
   // Add delay to reduce CPU usage
   delay(10);
   
   // Only call step() if connected
   if (WiFi.status() == WL_CONNECTED) {
     cm.step();
   } else {
     Serial.println("WiFi disconnected");
     delay(1000);
   }
}