#include "types.hpp"
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <MUIU8g2.h>
#include <WiFi.h>

const char *RX_S = "RX";
const char *TX_S = "TX";
const char *CONN = "CONN";
const char *LOOP = "LOOP";
const char *WIFI = "WiFi";
const char *STATE_RAW = " RAW ";
const char *STATE_ASCII = "ASCII";
const char *STATE_PUNCH = "PUNCH";

extern U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8;

void draw_status(devstatus_t* current_status, devstatus_t* old_status) {
  unsigned long now = millis();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  current_status->rx_lit = (now - current_status->last_rx) < 250;
  current_status->tx_lit = (now - current_status->last_tx) < 250;
  if (current_status->rx_lit != old_status->rx_lit) {
    if(current_status->rx_lit) {
      u8x8.inverse();
    } else {
      u8x8.noInverse();
    }
    u8x8.drawString(0, 0, RX_S);
    old_status->rx_lit = current_status->rx_lit;
  };
  if (current_status->tx_lit != old_status->tx_lit) {
    if(current_status->tx_lit) {
      u8x8.inverse();
    } else {
      u8x8.noInverse();
    }
    u8x8.drawString(3, 0, TX_S);
    old_status->tx_lit = current_status->tx_lit;
  };  
  u8x8.noInverse();
  if (current_status->loopback != old_status->loopback) {
    if (current_status-> loopback ) {
      u8x8.drawString(6, 0, LOOP);
    } else {
      u8x8.drawString(6, 0, CONN);
    };
    old_status->loopback = current_status->loopback;
  };
  if (current_status->wifi_connected != old_status->wifi_connected) {
    if (current_status->wifi_connected) {
      u8x8.drawString(11, 0, WIFI);
    } else {
      u8x8.drawString(11, 0, "    ");
    }
    old_status->wifi_connected = current_status->wifi_connected;
  };
  if(current_status->mode != old_status->mode) {
    switch (current_status->mode) {
    case MODE_RAW:
      u8x8.drawString(10, 1, STATE_RAW);
      break;
    case MODE_ASCII:
      u8x8.drawString(10, 1, STATE_ASCII);
      break;
    case MODE_PUNCH:
      u8x8.drawString(10, 1, STATE_PUNCH);
      break;
    default:
      u8x8.drawString(10, 1, "Error");
      break;
    };
    old_status->mode = current_status->mode;
  };
  if(current_status->current_ip != old_status->current_ip) {
    u8x8.setCursor(0, 2);
    u8x8.print(current_status->current_ip);
    old_status->current_ip = current_status->current_ip;
  };
  char buffer[16];
  snprintf(buffer, 16, "%03d/%03d %03d/%03d", 0, 16, 0, 16);
  u8x8.drawString(0, 3, buffer);
};
