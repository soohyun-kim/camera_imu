#include <Adafruit_FXAS21002C.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Metro.h>

// zero offset drift error used for calibration
float cal_x = -0.0045*0.04166666;
float cal_y = 0.0028*0.04166666;
float cal_z = 0.0035*0.04166666;

// initialize
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);
float x_cum, y_cum, z_cum, x_diff, y_diff, z_diff;
Metro readSensor = Metro(41.66666);

void setup(void) {
  Serial.begin(9600);

  /* Wait for the Serial Monitor */
  while (!Serial) {
    delay(1);
  }

  Serial.println("Gyroscope Test");
  Serial.println("");

  /* Initialise the sensor */
  if (!gyro.begin()) {
    /* There was a problem detecting the FXAS21002C ... check your connections
     */
    Serial.println("Ooops, no FXAS21002C detected ... Check your wiring!");
    while (1)
      ;
  }

  /* Display some basic information on this sensor */
  Serial.println(gyro.getRange());
  
  x_cum = 0.0;
  y_cum = 0.0;
  z_cum = 0.0;
  x_diff = 0.0;
  y_diff = 0.0;
  z_diff = 0.0;
}

void loop(void) {
  if (readSensor.check()) {
    sensors_event_t event;
    gyro.getEvent(&event);
    x_diff = gyro.raw.x*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_x;
    y_diff = gyro.raw.y*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_y;
    z_diff = gyro.raw.z*GYRO_SENSITIVITY_250DPS*0.0174533*0.04166666 - cal_z;
    x_cum += x_diff;
    y_cum += y_diff;
    z_cum += z_diff;
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
}
