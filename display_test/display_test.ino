#include <Metro.h>

bool flag;

Metro switcher = Metro(10);
Metro incrementer = Metro(500);

int number;

void setup() {
  // put your setup code here, to run once:
  pinMode(29, OUTPUT);  // bjt digit 1
  pinMode(30, OUTPUT);  // bjt digit 2

  pinMode(36, OUTPUT);  // ser data
  pinMode(37, OUTPUT);  // srclk
  pinMode(38, OUTPUT);  // rclk
  pinMode(39, OUTPUT);  // srclr

  digitalWrite(37, LOW);
  digitalWrite(38, LOW);

  // clear data
  digitalWrite(39, LOW);
  delay(10);
  digitalWrite(39, HIGH);

  Serial.begin(9600);   
  flag = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (switcher.check()) {
    flag = !flag;
    if (flag) {
      printDigit((number-(number%10))/10);
    }
    if (!flag) {
      printDigit(number%10);
    }
    digitalWrite(29, flag);
    digitalWrite(30, !flag);
  }
  
  if (incrementer.check()) {
    number++;
    if (number==100) {
      number = 0;
    }
  }
}

void printDigit(int digit) {
  switch (digit) {
    case 0:
      dataIn(false, 2);
      dataIn(true, 6);
      break;
    case 1:
      dataIn(false, 5);
      dataIn(true, 2);
      dataIn(false, 1);
      break;
    case 2:
      dataIn(true, 2);
      dataIn(false, 1);
      dataIn(true, 2);
      dataIn(false, 1);
      dataIn(true, 2);
      break;
    case 3:
      dataIn(true, 2);
      dataIn(false, 2);
      dataIn(true, 4);
      break;
    case 4:
      dataIn(true, 3);
      dataIn(false, 2);
      dataIn(true, 2);
      dataIn(false, 1);
      break;
    case 5:
      dataIn(true, 3);
      dataIn(false, 1);
      dataIn(true, 2);
      dataIn(false, 1);
      dataIn(true, 1);
      break;
    case 6:
      dataIn(true, 6);
      dataIn(false, 1);
      dataIn(true, 1);
      break;
    case 7:
      dataIn(false, 5);
      dataIn(true, 3);
      break;
    case 8:
      dataIn(true, 8);
      break;
    case 9:
      dataIn(true, 3);
      dataIn(false, 1);
      dataIn(true, 4);
      break;
  }
  digitalWrite(38, HIGH);
  digitalWrite(38, LOW);
}

void dataIn(bool state, int no) {
  digitalWrite(36, state);
  for (int i=0; i<no; i++) {
    digitalWrite(37, HIGH);
    delayMicroseconds(10);
    digitalWrite(37, LOW);
    delayMicroseconds(10);
  }
}
