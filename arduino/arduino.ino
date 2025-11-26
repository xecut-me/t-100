#include <CircularBuffer.hpp>
#include <U8g2lib.h>
#include <WiFi.h>
#include <Wire.h>

#include "convert.h"
#include "credentials.h"
#include "gui.hpp"
#include "types.hpp"

// Subj. Basically - make that thing fully wireless.
//
// Firmware for ESP32, that can be connected to teletype via current loop
// hardware.
//
// It should:
//
//    V - connect to wifi via hardcoded credentials
//    V - get IP over dhcp
//    X - listen on port 1337 for raw tcp connection
//    X - when there is no connection - it should enable loopback mode on
//        teletype (send everything that is typed on keyboard - back to
//        teletype).
//    X - when new connection arrives - old connection should be dropped.
//    X - any symbols typed on teletype should go to tcp socket, and any
//        symbols from tcp socket should go to the teletype.

// connected display: 0.91" oled, 128x32 SD1306, sda - 2, scl - 3

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0);

// Server will listen on this port
WiFiServer server(1337);

devstatus_t current_status;

#define TXPIN 0
#define RXPIN 1
#define SDAPIN 2
#define SCLPIN 3

// Delays and width
#define LINE_WIDTH 69              // FIXME - check if it is correct
#define CARRIAGE_RETURN_DELAY 2500 // in milliseconds
#define LINE_FEED_DELAY 2500       // in milliseconds
//

#include "convert.h"

CircularBuffer<uint8_t, 64> queue;

void handleTTYrx() {
  // process available data from Serial1
  if (!Serial1.available()) {
    return;
  };
  uint8_t x = Serial1.read();
  current_status.last_rx = millis();
  if (current_status.loopback) {
    queue.push(x);
  };
  switch (current_status.mode) {
  case MODE_RAW:
    Serial.write(x);
    break;
  case MODE_ASCII:
    if ((x == BAUD_FIGS) || (x == BAUD_LTRS)) {
      current_status.rx_is_in_ltrs = (x == BAUD_LTRS);
    } else {
      x = current_status.rx_is_in_ltrs ? (x) : (x | 0x20);
      Serial.write(baudot2ascii[x]);
    };
    break;
  case MODE_PUNCH:
    break; // in punch mode - ignore keyboard
  default:
    break;
  };
};

void handleTTYtx() {
  if (queue.isEmpty()) {
    return;
  };
  if (millis() < current_status.tx_wait_to) {
    return;
  }
  // send data via Serial1
  uint8_t x = queue.shift();
  Serial1.write(x);
  current_status.last_tx = millis();
  switch (x) {
  case BAUD_CR: {
    current_status.line_position = 0;
    current_status.tx_wait_to = millis() + CARRIAGE_RETURN_DELAY;
    break;
  };
  case BAUD_LF: {
    current_status.tx_wait_to = millis() + LINE_FEED_DELAY;
    break;
  };
  case BAUD_LTRS:
  case BAUD_NULL:
  case BAUD_FIGS:
    break;
  default: {
    current_status.line_position++;
  };
  };
  if (current_status.line_position == LINE_WIDTH) {
    current_status.tx_wait_to = millis() + CARRIAGE_RETURN_DELAY;
    current_status.line_position = 0;
  };
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(50, SERIAL_5N2, RXPIN /* RX */, TXPIN /* TX */,
                false /* invert */);
  Wire.setPins(SDAPIN, SCLPIN);
  Wire.setClock(1000000);
  Wire.begin();
  u8g2.begin();

  current_status.wifi_connected = true;
  current_status.current_ip = "N/A";
  current_status.loopback = true;
  current_status.mode = MODE_ASCII;
  current_status.tx_wait_to = current_status.last_rx = current_status.last_tx =
      millis();
  current_status.rx_is_in_ltrs = true;
  current_status.tx_is_in_ltrs = true;
  current_status.line_position = 0;

  // Connect to WPA2 Wi-Fi network
  WiFi.begin(ssid, password);

  /*  Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start the server
    server.begin();
    Serial.println("Server started, listening on port 80");*/
}

void loop() {
  // Update status
  current_status.current_ip = WiFi.localIP().toString();
  current_status.wifi_connected = (WiFi.status() == WL_CONNECTED);
  current_status.rx_count = queue.size();
  current_status.rx_total = queue.capacity;
  // End of update status
  // Redraw display
  u8g2.firstPage();
  do {
    draw_status(&current_status);
  } while (u8g2.nextPage());
  // End of redraw display

  // if data from teletype available, process it
  handleTTYrx();
  // if there is data in TX fifo, send it
  handleTTYtx();

  if (Serial.available()) {
    if (queue.size() >= (queue.capacity >> 1)) {
      switch (current_status.mode) {
      case MODE_RAW:
      case MODE_PUNCH: {
        Serial.write("Overflow!");
        break;
      };
      case MODE_ASCII: {
        Serial.write(XOFF);
        break;
      };
      default:
        break;
      };
      //  rx_allowed = false; // fixme
    } else {
      uint8_t ascii = Serial.read();
      // drop out of loopback mode on first received character
      current_status.loopback = false;
      switch (current_status.mode) {
      case MODE_RAW: {
        queue.push(ascii);
        break;
      };
      case MODE_ASCII: {
        uint8_t baudot;
        if (ascii > 127) {
          baudot = 0b00000000; // ignore symbols, with codes above 127
        } else {
          baudot = ascii2baudot[ascii];
          // perform conversion
          // upper bits (765) - mean:
          // 7 - special
          // 6 - can be in letters
          // 5 - can be in figures
          // when both 5 & 6 are zero - ignore symbol
        };
        if (baudot & SPECIAL) {
          // special treatment symbol
          baudot &= 0x1f;
          if (baudot >= MAX_SPECIAL) {
            baudot = 0;
          };
          bool has_switch = false;
          for (auto counter = 1; counter <= special_seq[baudot][0]; counter++) {
            uint8_t temp = special_seq[baudot][counter];
            if ((temp == BAUD_LTRS) || (temp == BAUD_FIGS)) {
              has_switch = true;
            };
            queue.push(temp);
          };
          if (has_switch) {
            if (current_status.tx_is_in_ltrs) {
              queue.push(BAUD_LTRS);
            } else {
              queue.push(BAUD_FIGS);
            };
          };
        } else {
          uint8_t flags =
              (baudot & (CANFIG | CANLTR)) >> 5; // 011_____ -> 0000 0011
          if (!current_status.tx_is_in_ltrs) {
            flags |= 0x04;
          };
          switch (flags) { // !is_in_ltrs | canfig | cantrl
          case 0b010:      // tx = LTRS, symbol can only be in FIGS - switch
          {
            queue.push(BAUD_FIGS);
            current_status.tx_is_in_ltrs = false;
            break;
          };
          case 0b101: // tx = FIGS, symbol can only be in LTRS - switch
          {
            queue.push(BAUD_LTRS);
            current_status.tx_is_in_ltrs = true;
            break;
          };
          }
          if (flags & ((CANFIG | CANLTR) >> 5)) { // not an ignore symbol
            queue.push(baudot);
          };
        };
        break;
      };
      case MODE_PUNCH: {
        break; // FIXME - implement font
      };
      default: {
        break;
      };
      };
    }
  }
  // Check if a client has connected
  /*  WiFiClient client = server.accept();
    if (client) {
      Serial.println("New client connected");
      // Wait until the client sends some data
      while (client.connected()) {
        if (client.available()) {
          String request = client.readStringUntil('\r');
          Serial.print("Received request: ");
          Serial.println(request);
          // Reply to the client
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");
          client.println();
          client.println("Hello from ESP32!");
          break;
        }
      }
      delay(1);
      client.stop();
      Serial.println("Client disconnected");
    }*/
}
