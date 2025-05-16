#include <WiFi.h>

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("ESP32 WiFi STA Mode Test");
  Serial.println("========================");
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  
  // Disconnect from any previous connections
  WiFi.disconnect(true);
  delay(100);
  
  // WiFi connection parameters - using open network
  const char* ssid = "Frohne-2.4GHz";
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  // Start WiFi connection
  WiFi.begin(ssid, ""); // Empty string for open networks
  
  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  // Check if connected successfully
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("");
    Serial.println("Failed to connect!");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
  }
}

void loop() {
  // Print connection status every 5 seconds
  Serial.print("WiFi status: ");
  switch(WiFi.status()) {
    case WL_CONNECTED:
      Serial.println("Connected");
      break;
    case WL_NO_SHIELD:
      Serial.println("No WiFi shield");
      break;
    case WL_IDLE_STATUS:
      Serial.println("Idle");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("No SSID available");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("Scan completed");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Connection failed");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("Connection lost");
      break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    default:
      Serial.println("Unknown");
  }
  
  delay(5000);
}