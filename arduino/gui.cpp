#include "types.hpp"
#include <U8g2lib.h>
#include <WiFi.h>

const char *TW_S = "Wa";
const char *CONN = "CONN";
const char *LOOP = "LOOP";
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

const unsigned char image_arrow_down_bits[] = {0x04, 0x04, 0x04, 0x04,
                                               0x15, 0x0e, 0x04};
const unsigned char image_arrow_up_bits[] = {0x04, 0x0e, 0x15, 0x04,
                                             0x04, 0x04, 0x04};
const unsigned char image_WiFi_bits[] = {0x22, 0x49, 0x55, 0x49,
                                         0x2a, 0x08, 0x08, 0x3e};

void draw_status(devstatus_t *current_status) {
  unsigned long now = millis();
  bool rx_lit, tx_lit, tx_wait;
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  rx_lit = (now - current_status->last_rx) < 300;
  tx_lit = (now - current_status->last_tx) < 300;
  tx_wait = (now < current_status->tx_wait_to);
  u8g2.setFont(u8g2_font_amstrad_cpc_extended_8r);
  if (rx_lit) {
    u8g2.drawXBM(11, 0, 5, 7, image_arrow_down_bits);
  }
  u8g2.drawStr(0, 7, (current_status->rx_is_in_ltrs ? "A" : "1"));
  if (tx_lit && !tx_wait) {
    u8g2.drawXBM(7, 0, 5, 7, image_arrow_up_bits);
  }
  if (tx_wait) {
    u8g2.drawStr(27, FONT_HEIGHT * (0 + 1), TW_S);
  }
  u8g2.drawStr(18, 7, (current_status->tx_is_in_ltrs ? "A" : "1"));
  u8g2.drawStr(FONT_WIDTH * 6, FONT_HEIGHT * (0 + 1),
               current_status->loopback ? LOOP : CONN);
  if (current_status->wifi_connected) {
    u8g2.drawXBM(120, 0, 7, 8, image_WiFi_bits);
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
  u8g2.setCursor(FONT_WIDTH * 0, FONT_HEIGHT * (1 + 1));
  u8g2.print(current_status->line_position);
  u8g2.setCursor(FONT_WIDTH * 0, FONT_HEIGHT * (2 + 1));
  u8g2.print(current_status->current_ip);
  char buffer[16];
  snprintf(buffer, 16, "%03d/%03d %03d/%03d", current_status->rx_count,
           current_status->rx_total, current_status->tx_count,
           current_status->tx_total);
  u8g2.drawStr(FONT_WIDTH * 0, FONT_HEIGHT * (3 + 1), buffer);
};

void handlegui(devstatus_t *current_status) {
  static bool lastpage = true;
  if (lastpage) {
    u8g2.firstPage();
  };
  draw_status(current_status);
  lastpage = !u8g2.nextPage();
};
