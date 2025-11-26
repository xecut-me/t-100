#include "types.hpp"
#include <U8g2lib.h>
#include <WiFi.h>

const char *RX_S = "RX";
const char *TX_S = "TX";
const char *CONN = "CONN";
const char *LOOP = "LOOP";
const char *WIFI = "WiFi";
const char *STATE_RAW = " RAW ";
const char *STATE_ASCII = "ASCII";
const char *STATE_PUNCH = "PUNCH";

extern U8G2_SSD1306_128X32_UNIVISION_2_HW_I2C u8g2;

#define FONT_HEIGHT 8
#define FONT_WIDTH 8

// display layout:
// resolution 16x4 symbols
// +----------------+
// |RX TX CONN WIFI |
// |          STATE |
// |192.168.001.001 |
// |000/128 000/128 |
// +----------------+
// meanings:
// RX        - inverts on received symbol
// TX        - inverts on transmitted symbol
// CONN|LOOP - connection to port established or loop
//  * LOOP   - loopback mode enabled
//  * CONN   - there is tcp connection
// WIFI      - connection to wifi established
// STATE     = [RAW|ASCII|PUNCH]
//  * RAW    - data sent as is (lower 5 bits)
//  * ASCII  - data sent converted from baudot to ascii
//  * PUNCH  - punches received data on the punch, in 3x5 font, garbage on paper
// 192.168.001.001 - IP of device
// 000/128   - RX fifo state
// 000/128   - TX fifo state

void draw_status(devstatus_t *current_status) {
  unsigned long now = millis();
  bool rx_lit, tx_lit;
  u8g2.setFont(u8g2_font_amstrad_cpc_extended_8r);
  u8g2.clearBuffer();
  rx_lit = (now - current_status->last_rx) < 300;
  tx_lit = (now - current_status->last_tx) < 300;
  if (rx_lit) {
    u8g2.drawStr(FONT_WIDTH * 0, FONT_HEIGHT * (0 + 1), RX_S);
  }
  if (tx_lit) {
    u8g2.drawStr(FONT_WIDTH * 3, FONT_HEIGHT * (0 + 1), TX_S);
  }
  if (current_status->loopback) {
    u8g2.drawStr(FONT_WIDTH * 6, FONT_HEIGHT * (0 + 1), LOOP);
  } else {
    u8g2.drawStr(FONT_WIDTH * 6, FONT_HEIGHT * (0 + 1), CONN);
  };
  if (current_status->wifi_connected) {
    u8g2.drawStr(FONT_WIDTH * 11, FONT_HEIGHT * (0 + 1), WIFI);
  };
  switch (current_status->mode) {
  case MODE_RAW:
    u8g2.drawStr(FONT_WIDTH * 10, FONT_HEIGHT * (1 + 1), STATE_RAW);
    break;
  case MODE_ASCII:
    u8g2.drawStr(FONT_WIDTH * 10, FONT_HEIGHT * (1 + 1), STATE_ASCII);
    break;
  case MODE_PUNCH:
    u8g2.drawStr(FONT_WIDTH * 10, FONT_HEIGHT * (1 + 1), STATE_PUNCH);
    break;
  default:
    u8g2.drawStr(FONT_WIDTH * 10, FONT_HEIGHT * (1 + 1), "Error");
    break;
  };
  u8g2.setCursor(FONT_WIDTH * 0, FONT_HEIGHT * (2 + 1));
  u8g2.print(current_status->current_ip);
  char buffer[16];
  snprintf(buffer, 16, "%03d/%03d %03d/%03d", 0, 16, 0, 16);
  u8g2.drawStr(FONT_WIDTH * 0, FONT_HEIGHT * (3 + 1), buffer);
};
