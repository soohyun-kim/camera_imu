#include <Metro.h>
#include <Bounce2.h>

bool flag;

Metro switcher = Metro(10);

void setup() {
  // put your setup code here, to run once:
pinMode(29, OUTPUT);
pinMode(28, OUTPUT);
pinMode(27, OUTPUT);
pinMode(26, OUTPUT);
Serial.begin(9600);
flag = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (switcher.check()) {
    flag = !flag;
  }

  digitalWrite(26, flag);
  digitalWrite(29, !flag);
  
  digitalWrite(28, flag);
  digitalWrite(27, !flag);
}
