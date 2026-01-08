#include <CircularBuffer.hpp>
#include <U8g2lib.h>
#include <WiFi.h>
#include <Wire.h>

#include "convert.h"
#include "credentials.h"
#include "gui.hpp"
#include "serial.hpp"
#include "types.hpp"

// connected display: 0.91" oled, 128x32 SD1306, sda - 2, scl - 3

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0);

// Server will listen on this port
WiFiServer server(1337);
WiFiClient currentClient;

devstatus_t current_status;

#define TXPIN 0
#define RXPIN 1
#define SDAPIN 2
#define SCLPIN 3

// Delays and width
// moved to serial.hpp

CircularBuffer<uint8_t, 128> queue;

uint32_t last_ota_time = 0;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    current_status.wifi_connected = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    WiFi.reconnect();
    current_status.wifi_connected = false;
    current_status.loopback = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    current_status.current_ip = WiFi.localIP().toString();
    server.begin();
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    current_status.current_ip = "N/A";
    current_status.loopback = true;
    server.end();
    break;
  default:
    break;
  }
};

void handleNetDisconnect() {
  // clear TX and RX buffers, reset state, send CR/LF
  current_status.mode = MODE_ASCII;
  queue.clear();
  Serial1.end();
  Serial1.begin(50, SERIAL_5N2, RXPIN /* RX */, TXPIN /* TX */,
                false /* invert */);

  queue.push(BAUD_CR);
  queue.push(BAUD_LF);
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(50, SERIAL_5N2, RXPIN /* RX */, TXPIN /* TX */,
                false /* invert */);

  Serial1.setRxBufferSize(16);
  Wire.setPins(SDAPIN, SCLPIN);
  Wire.setClock(1000000);
  Wire.begin();
  u8g2.begin();

  current_status.wifi_connected = false;
  current_status.current_ip = "N/A";
  current_status.loopback = true;
  current_status.mode = MODE_ASCII;
  current_status.tx_wait_to = current_status.last_rx = current_status.last_tx =
      millis();
  current_status.rx_is_in_ltrs = true;
  current_status.tx_is_in_ltrs = true;
  current_status.line_position = 0;

  // Connect to WPA2 Wi-Fi network
  WiFi.onEvent(WiFiEvent, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiEvent, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(WiFiEvent, ARDUINO_EVENT_WIFI_STA_LOST_IP);
  WiFi.onEvent(WiFiEvent, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  // server.begin();
}

void loop() {
  // Update status
  current_status.rx_count = queue.size();
  current_status.rx_total = queue.capacity;
  current_status.loopback = !(currentClient && currentClient.connected());
  // End of update status

  // Redraw display
  handlegui(&current_status);
  // End of redraw display

  // if data from teletype available, process it
  handleTTYrx();
  // if there is data in TX fifo, send it
  handleTTYtx();

  // handle wifi connection
  WiFiClient newClient = server.available();
  if (newClient) {
    if (currentClient && currentClient.connected()) {
      currentClient.stop(); // Disconnect previous client
      handleNetDisconnect();
    }
    currentClient = newClient; // Update with new client connection
  };
  // end handling wifi connection

  if (currentClient && currentClient.connected() &&
      ((queue.capacity - queue.size()) > 10)) {
    {
      current_status.tx_count = currentClient.available();
      uint8_t ascii = currentClient.read();
      // drop out of loopback mode on first received character
      current_status.loopback = false;
      handleNetworkRX(ascii);
    }
  }
}
