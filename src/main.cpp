

/**
 * @file main.cpp
 * @brief This file contains the implementation of a serial communication bridge
 * using ESP32.
 *
 * This program sets up a serial communication bridge between the USB CDC Serial
 * and UART2 on the ESP32. It reads data from one serial port and writes it to
 * the other, allowing for communication between the ESP32 and another device
 * via serial.
 *
 * @date 2023-10-10
 * @author WittyWizard
 * @license GPL-3.0-only
 */

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#define RX_PIN 2         // ESP32 RX (connect to device TX)
#define TX_PIN 1         // ESP32 TX (connect to device RX)
#define BAUD_RATE 115200 // Set your desired baud rate

void setup() {
  Serial.begin();                                       // USB CDC Serial
  Serial1.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); // UART2 on ESP32
}

void loop() {
  while (Serial.available()) {
    Serial1.write(Serial.read());
  }

  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
