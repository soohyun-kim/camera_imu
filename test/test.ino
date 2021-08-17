#include <Adafruit_FXAS21002C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SensorLab.h>
#include <Wire.h>
#include <Metro.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h>

#define NUMBER_SAMPLES 1000

// declarations
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);
Adafruit_SensorLab lab;
Adafruit_Sensor *gyroCal;
sensors_event_t event;

// temporary variables used for calibration
float min_x, max_x, mid_x;
float min_y, max_y, mid_y;
float min_z, max_z, mid_z;

// enter sensor refresh rate in fps (higher results in better rotation accuracy)
float frameRate = 120;
// enter camera capture frame rate
float logRate = 24;

Metro readSensor = Metro((1/frameRate)*1000);
Metro captureTimer = Metro((1/logRate)*1000);
Metro incrementReminder = Metro(1000);
Metro positionReminder = Metro(100);

Bounce recButton = Bounce();

// angles characterizing camera rotation
float x_cum, y_cum, z_cum;
float x_diff, y_diff, z_diff; 
float cal_x, cal_y, cal_z;
int fileIncrement;
File gyroLog;
bool recording;

void setup(void) {
  Serial.begin(9600);

  // status indicator LEDs
  //pinMode(13, OUTPUT);
  pinMode(30, OUTPUT);  // red
  pinMode(31, OUTPUT);  // green
  pinMode(32, OUTPUT);  // blue

  // record button, pulled down with 10k and pulsed high
  pinMode(35, INPUT);
  recButton.attach(35);
  recButton.interval(40);

  // initialize sensor, panic if not found
  if (!gyro.begin()) {
    while (1) {
      //digitalWrite(13, HIGH);
      digitalWrite(30, HIGH);
      digitalWrite(32, HIGH);
      delay(100);
      //digitalWrite(13, LOW);
      digitalWrite(30, LOW);
      digitalWrite(32, LOW);
      delay(100);
      Serial.println("Sensor not detected");
    }
  }

  // debug range (default 250dps)
  delay(750);
  Serial.print("Sensor DPS: ");
  Serial.println(gyro.getRange());

  // initialize rotation values
  x_cum = 0.0;
  y_cum = 0.0;
  z_cum = 0.0;
  x_diff = 0.0;
  y_diff = 0.0;
  z_diff = 0.0;

  // initialize SD card, panic if not found
  if (!SD.begin(BUILTIN_SDCARD)) {
    while (1) {
      //digitalWrite(13, HIGH);
      digitalWrite(31, HIGH);
      digitalWrite(30, HIGH);
      delay(300);
      //digitalWrite(13, LOW);
      digitalWrite(31, LOW);
      digitalWrite(30, LOW);
      delay(300);
      Serial.println("SD card not detected");
    }
  }

  // determine file increment position in SD card
  fileIncrement = 0;
  String suffix = ".csv";
  while (1) {
    String testFile = fileIncrement + suffix;
    if (!SD.exists(testFile.c_str())) {
      break;
    }
    fileIncrement++;
  }
  Serial.print("fileIncrement: ");
  Serial.println(fileIncrement);

  digitalWrite(31, HIGH);
  calibrateSensor();
  
  // initialize record flag
  recording = false;

  Serial.println();
  Serial.print("Sensor update rate: ");
  Serial.println(frameRate);
  Serial.print("Camera framerate: ");
  Serial.println(logRate);
  Serial.println();
  Serial.println("Setup complete.");
  delay(2000);
  digitalWrite(31, LOW);

  readSensor.reset();
  captureTimer.reset();
  incrementReminder.reset();
  positionReminder.reset();
}

void loop(void) {
  recButton.update();
  if (recButton.rose()) {
    if (!recording) {
      recordStart();
    }
    else if (recording) {
      recordStop();
    }
  }
  
  if (readSensor.check()) {
    updateRotation();
  }
  if (recording) {
    if (captureTimer.check()) {
      writeToSD();
    }
  }
  
  if (incrementReminder.check()) {
    Serial.print("file increment: ");
    Serial.println(fileIncrement);
  }
  if (positionReminder.check()) {
    Serial.print("rotation: ");
    Serial.print("X: ");
    Serial.print(x_cum);
    Serial.print("  ");
    Serial.print("Y: ");
    Serial.print(y_cum);
    Serial.print("  ");
    Serial.print("Z: ");
    Serial.print(z_cum);
    Serial.print("  ");
    Serial.println("rad");
  }

  // tally light (lights up if recording)
  digitalWrite(32, !recording);
  digitalWrite(30, recording);
}

void updateRotation() {
  sensors_event_t eventTemp;
  gyro.getEvent(&eventTemp);
  // sensor returns deg per second angular velocities of each axis
  // integrate the gyro angular velocity values to find cumulative rotation
  // use riemann sums of step 41.666 ms; convert to rad and correct for drift
  x_diff = gyro.raw.x*GYRO_SENSITIVITY_250DPS*0.0174533*(1/frameRate) - cal_x*(1/frameRate);
  y_diff = gyro.raw.y*GYRO_SENSITIVITY_250DPS*0.0174533*(1/frameRate) - cal_y*(1/frameRate);
  z_diff = gyro.raw.z*GYRO_SENSITIVITY_250DPS*0.0174533*(1/frameRate) - cal_z*(1/frameRate);
  x_cum += x_diff;
  y_cum += y_diff;
  z_cum += z_diff;
}

void writeToSD() {
  String comma = ",";
  gyroLog.print(x_cum, 5);
  gyroLog.print(comma);
  gyroLog.print(y_cum, 5);
  gyroLog.print(comma);
  gyroLog.print(z_cum, 5);
  gyroLog.println();
}

void recordStart() {
  recording = true;
  String suffix = ".csv";
  String fileName = fileIncrement + suffix;
  gyroLog = SD.open(fileName.c_str(), FILE_WRITE);
  Serial.println("record started");
  captureTimer.reset();
}

void recordStop() {
  recording = false;
  gyroLog.close();
  Serial.println("record stopped");
  fileIncrement++;
}

void calibrateSensor() {
  delay(1000);
  Serial.println(F("Sensor Lab - Gyroscope Calibration!"));
  lab.begin();
  
  gyroCal = lab.getGyroscope();
  
  gyroCal->printSensorDetails();
  delay(100);

  gyroCal->getEvent(&event);
  min_x = max_x = event.gyro.x;
  min_y = max_y = event.gyro.y;
  min_z = max_z = event.gyro.z;
  delay(10);

  Serial.println(F("Place gyro on flat, stable surface!"));

  Serial.print(F("Fetching samples in 3..."));
  delay(1000);
  Serial.print("2...");
  delay(1000);
  Serial.print("1...");
  delay(1000);
  Serial.println("NOW!");

  Metro flasher = Metro(500);
  bool calLed = false;
  
  float x, y, z;
  for (uint16_t sample = 0; sample < NUMBER_SAMPLES; sample++) {
    gyroCal->getEvent(&event);
    x = event.gyro.x;
    y = event.gyro.y;
    z = event.gyro.z;
    Serial.print(F("Gyro: ("));
    Serial.print(x); Serial.print(", ");
    Serial.print(y); Serial.print(", ");
    Serial.print(z); Serial.print(")");

    min_x = min(min_x, x);
    min_y = min(min_y, y);
    min_z = min(min_z, z);
  
    max_x = max(max_x, x);
    max_y = max(max_y, y);
    max_z = max(max_z, z);
  
    mid_x = (max_x + min_x) / 2;
    mid_y = (max_y + min_y) / 2;
    mid_z = (max_z + min_z) / 2;

    Serial.print(F(" Zero rate offset: ("));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.print(mid_z, 4); Serial.print(")");  
  
    Serial.print(F(" rad/s noise: ("));
    Serial.print(max_x - min_x, 3); Serial.print(", ");
    Serial.print(max_y - min_y, 3); Serial.print(", ");
    Serial.print(max_z - min_z, 3); Serial.println(")");
    
    if (flasher.check()) {
      calLed = !calLed;
    }
    digitalWrite(31, calLed);
    
    delay(10);
  }
  Serial.println(F("\n\nFinal zero rate offset in radians/s: "));
  Serial.print(mid_x, 4); Serial.print(", ");
  Serial.print(mid_y, 4); Serial.print(", ");
  Serial.println(mid_z, 4);

  cal_x = mid_x;
  cal_y = mid_y;
  cal_z = mid_z;
}
