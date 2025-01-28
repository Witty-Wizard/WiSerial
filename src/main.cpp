#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#define RX_PIN 5         // ESP32 RX (connect to device TX)
#define TX_PIN 18        // ESP32 TX (connect to device RX)
#define BAUD_RATE 115200 // Set your desired baud rate

// WiFi credentials
const char *ssid = "WiFi-SSID";
const char *password = "WiFi-Password";

// TCP Configuration
AsyncServer server(80);
AsyncClient *client = nullptr;
IPAddress remoteIP;
bool remoteIPSet = false;

void setup()
{
  Serial.begin(115200);                                 // Debugging (USB Serial)
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); // UART2 on ESP32

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("esp32"))
  { // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Start Async TCP Server
  server.onClient([](void *arg, AsyncClient *newClient)
                  {
                        Serial.println("New client connected");
                        if (!remoteIPSet)
                        {
                            remoteIP = newClient->remoteIP();
                            remoteIPSet = true;
                            Serial.print("Remote IP set to: ");
                            Serial.println(remoteIP);
                        }
                        client = newClient;
                        client->onData([](void *arg, AsyncClient *c, void *data, size_t len)
                                       {
                                           Serial2.write((uint8_t *)data, len);
                                       }, NULL); }, NULL);
  server.begin();
}

void loop()
{
  // Read from UART and send via TCP (byte-by-byte)
  if (client && client->connected())
  {
    while (Serial2.available())
    {
      char byteData = Serial2.read();
      client->write(&byteData, 1);
    }
  }
}