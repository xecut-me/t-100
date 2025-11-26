#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <Arduino.h>

typedef enum { MODE_RAW, MODE_ASCII, MODE_PUNCH } teletype_mode_t;

typedef struct {
  bool wifi_connected;
  String current_ip;
  bool loopback; // teletype starts in loopback mode till first
                 // received character from PC
                 // it returns to loopback as soon as connection is lost
  teletype_mode_t mode;
  unsigned long last_rx;
  bool rx_lit;
  unsigned long last_tx;
  bool tx_lit;

  uint8_t tx_count;
  uint8_t tx_total;
  uint8_t rx_count;
  uint8_t rx_total;

  bool tx_is_in_ltrs = true;
  bool rx_is_in_ltrs = true;

  uint8_t line_position; // caret position, to enable delays
  unsigned long tx_wait_to;
} devstatus_t;

#endif // __TYPES_HPP__
