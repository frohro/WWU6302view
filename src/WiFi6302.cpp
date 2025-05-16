#include <Six302.h>
#include <math.h>
#include <WiFi.h>

CommManager cm(5000, 50000);
float t, output;

// Add this function before setup()
void printWifiStatus(int status) {
  switch(status) {
    case WL_IDLE_STATUS: Serial.println("WiFi status: IDLE"); break;
    case WL_NO_SSID_AVAIL: Serial.println("WiFi status: No SSID available"); break;
    case WL_SCAN_COMPLETED: Serial.println("WiFi status: Scan completed"); break;
    case WL_CONNECTED: Serial.println("WiFi status: Connected"); break;
    case WL_CONNECT_FAILED: Serial.println("WiFi status: Connection failed"); break;
    case WL_CONNECTION_LOST: Serial.println("WiFi status: Connection lost"); break;
    case WL_DISCONNECTED: Serial.println("WiFi status: Disconnected"); break;
    default: Serial.printf("WiFi status: Unknown (%d)\n", status);
  }
}

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
   char ssid[] = "Frohne-2.4GHz"; // Copy string literals to RAM
   char pw[] = ""; // Double-check this password is correct
   
   Serial.println("Connecting to WiFi...");
   
   // Get cleaner state by disconnecting and setting mode explicitly
   WiFi.disconnect(true);
   WiFi.mode(WIFI_OFF);
   delay(1000);
   WiFi.mode(WIFI_STA);
   
   // Scan for networks first to verify AP is available
   Serial.println("Scanning for WiFi networks...");
   int networksFound = WiFi.scanNetworks();
   bool foundNetwork = false;
   
   Serial.printf("Found %d networks\n", networksFound);
   for (int i = 0; i < networksFound; i++) {
      Serial.printf("%d: %s (Signal: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      if (WiFi.SSID(i) == ssid) {
         foundNetwork = true;
         Serial.printf("Target network '%s' found with signal strength: %d dBm\n", ssid, WiFi.RSSI(i));
      }
   }
   
   if (!foundNetwork) {
      Serial.printf("WARNING: Target network '%s' not found in scan results!\n", ssid);
   }
   
   // Add a timeout and retry mechanism
   int attempts = 0;
   const int maxAttempts = 5;
   
   while (attempts < maxAttempts) {
      Serial.printf("\nAttempt %d to connect to %s\n", attempts + 1, ssid);
      
      // Try direct WiFi connection
      WiFi.begin(ssid, NULL);
      
      // Wait for connection or timeout
      unsigned long startTime = millis();
      bool connected = false;
      
      while (millis() - startTime < 15000) { // 15 second timeout
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
         Serial.print("Signal strength: ");
         Serial.println(WiFi.RSSI());
         
         // Now connect the CommManager
         cm.connect(ssid, pw);
         break;
      } else {
         Serial.println();
         Serial.printf("Connection attempt %d failed\n", attempts + 1);
         printWifiStatus(WiFi.status());
         
         // Disconnect before retry
         WiFi.disconnect(true);
         delay(2000); // Longer delay between attempts
         attempts++;
      }
   }
   
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi after multiple attempts");
      Serial.println("Please check the following:");
      Serial.println("1. SSID spelling and case (NatsNet)");
      Serial.println("2. Password correctness (curiosity)");
      Serial.println("3. Router is powered on and in range");
      Serial.println("4. Router is not blocking this device");
   }
}
void loop() {
   output = sin(t = t + 0.02);
   
   // Add delay to reduce potential for stack overflows
   delay(10);
   
   cm.step();
}