#include <Adafruit_FXAS21002C.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Metro.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h>

// enter measured zero offset drift error for calibration (rad/s)
float cal_x = -0.0139;
float cal_y = 0.0013;
float cal_z = 0.0032;

// declarations
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);

Metro readSensor = Metro(41.66666);  // 41.667ms = 1/24th of 1s, corresponds to 24fps
Metro incrementReminder = Metro(1000);
Metro positionReminder = Metro(100);

Bounce recButton = Bounce();

// extrinsic euler angles characterizing camera rotation
float x_cum, y_cum, z_cum;
float x_diff, y_diff, z_diff; 
int fileIncrement;
File gyroLog;
bool recording;

void setup(void) {
  Serial.begin(9600);

  // status indicator LED
  pinMode(13, OUTPUT);

  // record button, pulled down with 10k and pulsed high
  pinMode(35, INPUT);
  recButton.attach(35);
  recButton.interval(30);

  // initialize sensor, panic if not found
  if (!gyro.begin()) {
    while (1) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      Serial.println("Sensor not detected");
    }
  }

  // debug range (default 250dps)
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
      digitalWrite(13, HIGH);
      delay(300);
      digitalWrite(13, LOW);
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
  
  // initialize record flag
  recording = false;
}

void loop(void) {
  recButton.update();
  if (recButton.rose()) {
    recordStart();
  }
  if (recButton.fell()) {
    recordStop();
  }
  
  if (readSensor.check()) {
    updateRotation();
    if (recording) {
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
  digitalWrite(13, recording);
}

void updateRotation() {
  sensors_event_t event;
  gyro.getEvent(&event);
  // sensor returns deg per second angular velocities of each axis
  // integrate the gyro angular velocity values to find cumulative rotation
  // use riemann sums of step 41.666 ms; convert to rad and correct for drift
  x_diff = gyro.raw.x*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_x*0.04166666;
  y_diff = gyro.raw.y*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_y*0.04166666;
  z_diff = gyro.raw.z*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_z*0.04166666;
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
  readSensor.reset();
}

void recordStop() {
  recording = false;
  gyroLog.close();
  Serial.println("record stopped");
  fileIncrement++;
}
