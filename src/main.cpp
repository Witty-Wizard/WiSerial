#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#define RX_PIN 5         // ESP32 RX (connect to device TX)
#define TX_PIN 18        // ESP32 TX (connect to device RX)
#define BAUD_RATE 115200 // Set your desired baud rate

// WiFi credentials
const char *ssid = "House_42_EXT";
const char *password = "eleconhp2";

// TCP Configuration
AsyncServer server(81);
AsyncClient *client = nullptr;
IPAddress remoteIP;
bool remoteIPSet = false;

void onClientDisconnected(void *arg, AsyncClient *c) {
  Serial.println("Client disconnected");

  // Reset client variables
  client = nullptr;
  remoteIPSet = false;
}

// Function to handle data received from client
void onDataReceived(void *arg, AsyncClient *c, void *data, size_t len) {
  Serial2.write((uint8_t *)data, len);
}

// Function to handle new client connections
void onNewClient(void *arg, AsyncClient *newClient) {
  // Reject new connections if a client is already connected
  if (client != nullptr) {
    Serial.println("Another client tried to connect, but only one is allowed.");
    newClient->close(); // Close the new connection
    delete newClient;   // Clean up memory
    return;
  }

  Serial.println("New client connected");

  // Store client reference
  client = newClient;

  // Set the remote IP if not already set
  if (!remoteIPSet) {
    remoteIP = newClient->remoteIP();
    remoteIPSet = true;
    Serial.print("Remote IP set to: ");
    Serial.println(remoteIP);
  }

  // Attach callbacks
  client->onData(onDataReceived, nullptr);
  client->onDisconnect(onClientDisconnected, nullptr);
}

void setup() {
  Serial.begin(115200); // Debugging (USB Serial)
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); // UART2 on ESP32

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("esp32")) { // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Start Async TCP Server
  server.onClient(onNewClient, nullptr);
  server.begin();
}

void loop() {
  // Read from UART and send via TCP (byte-by-byte)
  if (client && client->connected()) {
    while (Serial2.available()) {
      char byteData = Serial2.read();
      client->write(&byteData, 1);
    }
  }
}