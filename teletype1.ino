#include <protothreads.h>
#include <CircularBuffer.hpp>

#define TXPIN   7
#define RXPIN   6
#define HALF_BIT    10
const char PROGMEM LEDS[4]={A0,A1,A2,A3};

#define TTY_RX_LED  0
#define TTY_TX_LED  1
#define BUFFER_FULL_LED 2


#define LTRS    0x1f
#define FIGS    0x1b

// if defined - sends data as-is
// if not defined - converts ascii -> baudot and baudot -> ascii
#define IS_RAW

#ifndef IS_RAW
#include "convert.h"
bool tx_is_in_ltrs = true;
#endif

pt ptTXThread;
pt ptRXThread;
CircularBuffer<char, 16> queue;
  
int rxThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static uint8_t i = 0;

  // Loop forever
  for(;;) {
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == HIGH);// wait for --+       to indicate start bit (this pair prevents freerunning zeroes reading on broken loop)
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == LOW); //            +----
    digitalWrite(LEDS[TTY_RX_LED], HIGH);
    PT_SLEEP(pt, HALF_BIT);                       // sleep till the middle of start bit (10 ms)
    x = 0;
    for(i=0b000001;i!=0b100000;i<<=1){
      PT_SLEEP(pt, HALF_BIT*2);                   // wait till next bit
      if(digitalRead(RXPIN) == HIGH) {            // 1  ---+  +--+--+--+--+--+---   
        x = x | i;                                //       |S | 0| 1| 2| 3| 4|STP    - bits are 20ms, stop bit - 30ms
      };                                          //       +--+--+--+--+--+--+   
    }
    PT_SLEEP(pt, HALF_BIT*4);                     // 1.5 stop bits ( 0.5 bit forward from last bit + 1.5 bits of stop)
#ifdef IS_RAW
    Serial.write(x);
#else
    Serial.write(x);                              // FIXME: here should be conversion
#endif
    digitalWrite(LEDS[TTY_RX_LED], LOW);
  }

  PT_END(pt);
}

int txThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static uint8_t i = 0;

  // Loop forever
  for(;;) {
    PT_WAIT_UNTIL(pt, !queue.isEmpty());        // wait till there is something to send
    digitalWrite(LEDS[TTY_TX_LED], HIGH);
    x = queue.pop();
    digitalWrite(TXPIN, LOW);                   // send start bit
    PT_SLEEP(pt, HALF_BIT*2);                   //
    for(i=0b000001;i!=0b100000;i<<=1){
      digitalWrite(TXPIN, (x & i)?HIGH:LOW);
      PT_SLEEP(pt, HALF_BIT*2);
    }
    digitalWrite(TXPIN, HIGH);
    digitalWrite(LEDS[TTY_TX_LED], LOW);
    PT_SLEEP(pt, HALF_BIT*3);                   // wait 1.5 bits
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
  for(unsigned int i=0;i<sizeof(LEDS)/sizeof(LEDS[0]);i++){
      pinMode(LEDS[i], OUTPUT);
      digitalWrite(LEDS[i], LOW);
  }
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    if(queue.isFull()) {
      Serial.write("Overflow!"); // FIXME: add xon/xoff flow control
    }else{
      uint8_t ascii = Serial.read();
#ifdef IS_RAW
      queue.push(ascii);
#else
      uint8_t baudot;
      if( ascii > 127) {
          baudot = 0b00000000;      // ignore symbols, with codes above 127
      }else {
        baudot = ascii2baudot[ascii]; // perform conversion
                                      // upper bits (765) - mean:
                                      // 7 - special
                                      // 6 - can be in letters
                                      // 5 - can be in figures
                                      // when both 5 & 6 are zero - ignore symbol
      };
      if( baudot & 0x80) {
          // special treatment symbol
          // FIXME - implement
      } else {
          uint8_t flags = (baudot & 0x60) >> 5;
          if ( tx_is_in_ltrs ) {
              flags |= 0x04;
          };
          switch(flags) {
              case 0b010:       // tx = FIGS, symbol can only be in LTRS - switch
                queue.push(LTRS);
                tx_is_in_ltrs = true;
                break;
              case 0b101:       // tx = LTRS, symbol can only be in FIGS - switch
                queue.push(FIGS);
                tx_is_in_ltrs = false;
                break;
          }
          if(flags & 0x3) { // not an ignore symbol
             queue.push(baudot);
          };
      }
#endif
    }
  }
  PT_SCHEDULE(rxThread(&ptRXThread));
  PT_SCHEDULE(txThread(&ptTXThread));
}
