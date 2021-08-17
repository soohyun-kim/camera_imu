#include <Metro.h>
#include <Bounce2.h>

Metro myT = Metro(5);
Metro slew = Metro(20);

Bounce b = Bounce();

uint8_t outPWM;

bool rising, falling;

void setup() {
  // put your setup code here, to run once:
  pinMode(20, OUTPUT);
  pinMode(35, INPUT);
  Serial.begin(9600);
  analogWriteFrequency(20, 50);

  b.attach(35);
  b.interval(40);

  outPWM = 51;
  rising = false;
  falling = false;
}

void loop() {
  // put your main code here, to run repeatedly:
  b.update();
  
  if (b.rose() && !falling) {
    outPWM = 52;
    slew.reset();
    rising = true;
  }
  if (b.fell() && !rising) {
    outPWM = 52;
    slew.reset();
    falling = true;
  }

  if (rising) {
    if (slew.check()) {
      outPWM = 53;
      rising = false;
    }
  }
  if (falling) {
    if (slew.check()) {
      outPWM = 51;
      falling = false;
    }
  }

  if (myT.check()) {
    //Serial.println(analogRead(A9));
    Serial.println(outPWM);
  }
  analogWrite(20, outPWM);
}
