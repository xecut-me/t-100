#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <Arduino.h>

typedef enum { MODE_RAW, MODE_ASCII, MODE_PUNCH } teletype_mode_t;

typedef struct {
  bool wifi_connected;
  String current_ip;
  bool loopback;
  teletype_mode_t mode;
  unsigned long last_rx;
  bool rx_lit;
  unsigned long last_tx;
  bool tx_lit;

  uint8_t tx_count;
  uint8_t tx_total;
  uint8_t rx_count;
  uint8_t rx_total;
} devstatus_t;

#endif // __TYPES_HPP__
