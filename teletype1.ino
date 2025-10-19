#include <protothreads.h>
#include <CircularBuffer.hpp>

#define TXPIN   7
#define RXPIN   6
#define HALF_BIT    10
const char PROGMEM LEDS[4]={A0,A1,A2,A3};

pt ptTXThread;
pt ptRXThread;
CircularBuffer<char, 16> queue;
  
int rxThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static int i = 0;

  // Loop forever
  for(;;) {
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == HIGH);// wait for --+       to indicate start bit (this pair prevents freerunning zeroes reading on broken loop)
    PT_WAIT_UNTIL(pt, digitalRead(RXPIN) == LOW); //            +----
    digitalWrite(LEDS[0], HIGH);
    PT_SLEEP(pt, HALF_BIT);                       // sleep till the middle of start bit (10 ms)
    x = 0;
    for(i=0;i<5;i++){
      PT_SLEEP(pt, HALF_BIT*2);                   // wait till next bit
      if(digitalRead(RXPIN) == HIGH) {            // 1  ---+  +--+--+--+--+--+---   
        x = x | 0x01;                             //       |S | 0| 1| 2| 3| 4|STP    - bits are 20ms, stop bit - 30ms
      };                                          //       +--+--+--+--+--+--+   
      x = x << 1;                                 // shift the result
    }
    PT_SLEEP(pt, HALF_BIT*4);                     // 1.5 stop bits ( 0.5 bit forward from last bit + 1.5 bits of stop)
    Serial.write(x);                              // FIXME: here should be conversion
    digitalWrite(LEDS[0], LOW);
  }

  PT_END(pt);
}

int txThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static int i = 0;

  // Loop forever
  for(;;) {
    PT_WAIT_UNTIL(pt, !queue.isEmpty());        // wait till there is something to send
    digitalWrite(LEDS[1], HIGH);
    x = queue.pop();
    digitalWrite(TXPIN, LOW);                   // send start bit
    PT_SLEEP(pt, HALF_BIT*2);                   //
    for(i=0;i<5;i++){
      if(x & 0x01) {
        digitalWrite(TXPIN, LOW);               // loop control is inverted, low breaks the loop
      }else{
        digitalWrite(TXPIN, HIGH);
      }
      PT_SLEEP(pt, HALF_BIT*2);
      x = x >> 1;
    }
    digitalWrite(TXPIN, HIGH);
    digitalWrite(LEDS[1], LOW);
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
  for(auto i=0;i<sizeof(LEDS)/sizeof(LEDS[0]);i++){
      pinMode(LEDS[i], OUTPUT);
      digitalWrite(LEDS[i], LOW);
  }
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    if(queue.isFull()) {
      Serial.write("Overflow!");
    }else{
      queue.push(Serial.read()); // FIXME: here should be conversion
    }
  }
  PT_SCHEDULE(rxThread(&ptRXThread));
  PT_SCHEDULE(txThread(&ptTXThread));
}
