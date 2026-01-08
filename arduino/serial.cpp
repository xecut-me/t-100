#include "serial.hpp"
#include "convert.h"
#include "types.hpp"
#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <WiFi.h>

extern devstatus_t current_status;
extern CircularBuffer<uint8_t, 128> queue;
extern WiFiClient currentClient;

void handleTTYrx() {
  // process available data from Serial1
  if (!Serial1.available()) {
    return;
  };
  uint8_t x = Serial1.read();
  current_status.last_rx = millis();
  if (current_status.loopback) {
    queue.push(x);
  } else {
    switch (current_status.mode) {
    case MODE_RAW:
      currentClient.write(x);
      break;
    case MODE_ASCII:
      if ((x == BAUD_FIGS) || (x == BAUD_LTRS)) {
        current_status.rx_is_in_ltrs = (x == BAUD_LTRS);
      } else {
        x = current_status.rx_is_in_ltrs ? (x) : (x | 0x20);
        currentClient.write(baudot2ascii[x]);
      };
      break;
    case MODE_PUNCH:
      break; // in punch mode - ignore keyboard
    default:
      break;
    };
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
  case BAUD_FIGS: {
    break;
  };
  default: {
    current_status.line_position++;
  };
  };
  if (current_status.line_position == LINE_WIDTH) {
    current_status.tx_wait_to = millis() + CARRIAGE_RETURN_DELAY;
    current_status.line_position = 0;
  };
};

void handleNetworkRX(uint8_t ascii) {
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
      if (baudot >= (sizeof(special_seq) / sizeof(special_seq[0]))) {
        baudot = 0;
      };
      bool has_switch = false;
      for (uint8_t counter = 1; counter <= special_seq[baudot][0]; counter++) {
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
        queue.push(baudot & 0x1f);
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
};
