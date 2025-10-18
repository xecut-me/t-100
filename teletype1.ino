#include <protothreads.h>
#include <CircularBuffer.hpp>

pt ptTXThread;
pt ptRXThread;
CircularBuffer<char, 16> queue;
  
int rxThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static int i = 0;

  // Loop forever
  for(;;) {
  // read the state of the pushbutton value:
    PT_WAIT_UNTIL(pt, digitalRead(6) == LOW);
    PT_SLEEP(pt, 10);
    x = 0;
    for(i=0;i<5;i++){
      PT_SLEEP(pt, 20);
      if(digitalRead(6) == HIGH) { // 1
        x = x | 0x01;
      }
      x = x << 1;
    }
    PT_SLEEP(pt, 40);
    Serial.write(x);
  }

  PT_END(pt);
}

int txThread(struct pt* pt) {
  PT_BEGIN(pt);

  static char x;
  static int i = 0;

  // Loop forever
  for(;;) {
  // read the state of the pushbutton value:
    PT_WAIT_UNTIL(pt, !queue.isEmpty());
    x = queue.pop();
    digitalWrite(7, LOW);
    PT_SLEEP(pt, 20);
    for(i=0;i<5;i++){
      if(x & 0x01) {
        digitalWrite(7, LOW);
      }else{
        digitalWrite(7, HIGH);
      }
      PT_SLEEP(pt, 20);
      x = x >> 1;
    }
    digitalWrite(7, HIGH);
    PT_SLEEP(pt, 30);
  }

  PT_END(pt);
}




void setup() {
  // put your setup code here, to run once:
  PT_INIT(&ptRXThread);
  PT_INIT(&ptTXThread);  
  pinMode(7, OUTPUT);
  pinMode(6, INPUT);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    if(queue.isFull()) {
      Serial.write("Overflow!");
    }else{
      queue.push(Serial.read());
    }
  }
  PT_SCHEDULE(rxThread(&ptRXThread));
  PT_SCHEDULE(txThread(&ptTXThread));
}
