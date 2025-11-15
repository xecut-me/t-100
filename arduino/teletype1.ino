#include <CircularBuffer.hpp>
#include <protothreads.h>

#define TXPIN 7
#define RXPIN 6
#define HALF_BIT 10

// Delays and width
#define LINE_WIDTH 69             // FIXME - check if it is correct
#define CARRIAGE_RETURN_DELAY 500 // in milliseconds
#define LINE_FEED_DELAY 500       // in milliseconds
//

const char PROGMEM LEDS[4] = {A0, A1, A2, A3};

#define TTY_RX_LED 0
#define TTY_TX_LED 1
#define BUFFER_FULL_LED 2

// if defined - sends data as-is
// if not defined - converts ascii -> baudot and baudot -> ascii
// #define IS_RAW

#ifndef IS_RAW
#include "convert.h"
bool tx_is_in_ltrs = true;
bool rx_is_in_ltrs = true;
bool is_in_loopback = true; // teletype starts in loopback mode till first
                            // received character from PC

bool rx_allowed = true; // buffer is not full
#endif

pt ptTXThread;
pt ptRXThread;
CircularBuffer<uint8_t, 64> queue;

int rxThread(struct pt *pt) {
  PT_BEGIN(pt);

  static uint8_t x;
  static uint8_t i = 0;

  // Loop forever
  for (;;) {
    // wait for --+
    //            +----
    // to indicate start bit (this pair prevents freerunning zeroes reading on
    // broken loop)
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == HIGH);
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == LOW);
    digitalWrite(LEDS[TTY_RX_LED], HIGH);
    PT_SLEEP(pt, HALF_BIT); // sleep till the middle of start bit (10 ms)
    x = 0;
    for (i = 0b000001; i != 0b100000; i <<= 1) {
      PT_SLEEP(pt, HALF_BIT * 2); // wait till next bit
      // 1  ---+  +--+--+--+--+--+---
      //       |S | 0| 1| 2| 3| 4|STP    - bits are 20ms, stop bit - 30 ms
      //       +--+--+--+--+--+--+
      if (digitalRead(RXPIN) == HIGH) {
        x = x | i;
      };
    }
    PT_SLEEP(pt, HALF_BIT * 4); // 1.5 stop bits ( 0.5 bit forward from last bit
                                // + 1.5 bits of stop)

    if (is_in_loopback) {
      // while in loopback mode - just write data to the type queue
      queue.push(x);
    };
#ifdef IS_RAW
    Serial.write(x);
#else
    if ((x == BAUD_FIGS) || (x == BAUD_LTRS)) {
      rx_is_in_ltrs = (x == BAUD_LTRS);
    } else {
      x = rx_is_in_ltrs ? (x) : (x | 0x20);
      Serial.write(pgm_read_byte(baudot2ascii + x));
    }
#endif
    digitalWrite(LEDS[TTY_RX_LED], LOW);
  }

  PT_END(pt);
}

int txThread(struct pt *pt) {
  PT_BEGIN(pt);

  static char x;
  static uint8_t i = 0;
  static uint8_t line_position = 0;

  // Loop forever
  for (;;) {
#ifndef IS_RAW
    if (queue.isEmpty() && !rx_allowed) {
      Serial.write(XON);
      rx_allowed = true;
    };
#endif
    PT_WAIT_UNTIL(pt, !queue.isEmpty()); // wait till there is something to send
    digitalWrite(LEDS[TTY_TX_LED], HIGH);
    x = queue.shift();
    digitalWrite(TXPIN, LOW);   // send start bit
    PT_SLEEP(pt, HALF_BIT * 2); //
    for (i = 0b000001; i != 0b100000; i <<= 1) {
      digitalWrite(TXPIN, (x & i) ? HIGH : LOW);
      PT_SLEEP(pt, HALF_BIT * 2);
    }
    digitalWrite(TXPIN, HIGH);
    digitalWrite(LEDS[TTY_TX_LED], LOW);
    PT_SLEEP(pt, HALF_BIT * 3); // wait 1.5 bits
    switch (x) {
    case BAUD_CR:
      line_position = 0;
      PT_SLEEP(pt, CARRIAGE_RETURN_DELAY);
      break;
    case BAUD_LF:
      PT_SLEEP(pt, LINE_FEED_DELAY);
      break;
    case BAUD_LTRS:
    case BAUD_NULL:
    case BAUD_FIGS:
      break;
    default:
      line_position++;
    };
    if (line_position == LINE_WIDTH) {
      PT_SLEEP(pt, CARRIAGE_RETURN_DELAY);
      line_position = 0;
    };
  }

  PT_END(pt);
}

void setup() {
  // put your setup code here, to run once:
  PT_INIT(&ptRXThread);
  PT_INIT(&ptTXThread);
  pinMode(TXPIN, OUTPUT);
  pinMode(RXPIN, INPUT);
  digitalWrite(RXPIN, HIGH);
  digitalWrite(TXPIN, HIGH);
  for (unsigned int i = 0; i < sizeof(LEDS) / sizeof(LEDS[0]); i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
  Serial.begin(9600);
#ifndef IS_RAW
  Serial.write(XON);
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
#ifndef IS_RAW
  digitalWrite(LEDS[BUFFER_FULL_LED], (rx_allowed) ? LOW : HIGH);
#endif

  if (Serial.available()) {
    if (queue.size() >= (queue.capacity >> 1)) {
#ifdef IS_RAW
      Serial.write("Overflow!");
#else
      Serial.write(XOFF);
      rx_allowed = false;
#endif // IS_RAW
    } else {
      uint8_t ascii = Serial.read();
      // drop out of loopback mode on first received character
      is_in_loopback = false;
#ifdef IS_RAW
      queue.push(ascii);
#else
      uint8_t baudot;
      if (ascii > 127) {
        baudot = 0b00000000; // ignore symbols, with codes above 127
      } else {
        baudot = pgm_read_byte(
            ascii2baudot + ascii); // perform conversion
                                   // upper bits (765) - mean:
                                   // 7 - special
                                   // 6 - can be in letters
                                   // 5 - can be in figures
                                   // when both 5 & 6 are zero - ignore symbol
      };
      if (baudot & SPECIAL) {
        // special treatment symbol
        baudot &= 0x1f;
        switch (baudot) {
        case 1: // vertical tab
          queue.push(BAUD_SPACE);
          queue.push(BAUD_SPACE);
          queue.push(BAUD_SPACE);
          queue.push(BAUD_SPACE);
          break;
        case 2: // form feed
          queue.push(BAUD_LF);
          queue.push(BAUD_LF);
          break;
        case 3: // escape
          queue.push(BAUD_FIGS);
          queue.push(BAUD_K_BR_L);
          queue.push(BAUD_LTRS);
          queue.push(BAUD_E_3);
          queue.push(BAUD_S_APO);
          queue.push(BAUD_C_COLO);
          queue.push(BAUD_FIGS);
          queue.push(BAUD_L_BR_R);
          if (tx_is_in_ltrs) {
            queue.push(BAUD_LTRS);
          };
          break;
        default:
          break;
        }
      } else {
        uint8_t flags =
            (baudot & (CANFIG | CANLTR)) >> 5; // 011_____ -> 0000 0011
        if (!tx_is_in_ltrs) {
          flags |= 0x04;
        };
        switch (flags) { // !is_in_ltrs | canfig | cantrl
        case 0b010:      // tx = LTRS, symbol can only be in FIGS - switch
          queue.push(BAUD_FIGS);
          tx_is_in_ltrs = false;
          break;
        case 0b101: // tx = FIGS, symbol can only be in LTRS - switch
          queue.push(BAUD_LTRS);
          tx_is_in_ltrs = true;
          break;
        }
        if (flags & ((CANFIG | CANLTR) >> 5)) { // not an ignore symbol
          queue.push(baudot);
        };
      }
#endif
    }
  }
  PT_SCHEDULE(rxThread(&ptRXThread));
  PT_SCHEDULE(txThread(&ptTXThread));
}
