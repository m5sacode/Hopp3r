#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include <filters.h>
#include <filters_defs.h>
#include <Wire.h>
#include <PID_v1.h>
#include <Servo.h>
#include <HCSR04.h>
#include <SPI.h>
byte direccion[5] = {'c', 'a', 'n', 'a', 'l'};
float datos[6];
float pitchPIDError = 12.5, yawPIDError = -12.5;
boolean calibrando = true;
int answer = 0;
IIR::ORDER  order  = IIR::ORDER::OD3;
double altitude = 0;
float radardis;
float throttle1;
float throttle2;
double throttle = 0;
int DELAY1;
int DELAY2;
float zaccang, yaccang;
float xerror;
float yerror;
float zerror;
double roll = 0;
double pitch = 0;
double yaw = 0;
double rawroll = 0;
double rawpitch = 0;
double rawyaw = 0;
double rollangle = 0;
double pitchangle = 0;
double yawangle = 0;
double frollangle = 0;
double fpitchangle = 0;
double fyawangle = 0;
double rollset = 0;
double pitchset = 0;
double yawset = 0;
double rollKp = 0, rollKi = 0, rollKd = 0;
double pitchKp = 10, pitchKi = 0, pitchKd = 0;
double yawKp = 10, yawKi = 0, yawKd = 0;
double vertspeed;
float rawX;
float rawY;
float rawZ;
float Xspeed;
float Yspeed;
float Zspeed;
float Xspeederror = 0;
float Yspeederror = 0;
float Zspeederror = 0;
float Xerror;
float Yerror;
float Zerror;
float timePrev = 0;
float elapsedTime = 0;
float Time = 0;
float filterFrequency = 4.0;
double altiset = 0;
boolean motor1dis = false, motor2dis = false;
double altiKp = 0, altiKi = 0, altiKd = 0;
Filter x(filterFrequency, elapsedTime, order);
Filter y(filterFrequency, elapsedTime, order);
Filter z(filterFrequency, elapsedTime, order);
Servo motor1;
Servo motor2;
Servo pitchservo;
Servo yawservo;
PID rollpid(&frollangle, &rawroll, &rollset, rollKp, rollKi, rollKd, DIRECT);
PID pitchpid(&fpitchangle , &rawpitch, &pitchset, pitchKp, pitchKi, pitchKd, DIRECT);
PID yawpid(&fyawangle , &rawyaw, &yawset, yawKp, yawKi, yawKd, REVERSE);
PID altipid(&altitude, &throttle, &altiset, altiKp, altiKi, altiKd, DIRECT);
HCSR04 hcs(2, 3);
RF24 radio(10, 9);
void setup() {
  Serial.begin(9600);
  Serial.println(F("serial start"));
  motor1.attach(A0);
  motor2.attach(A1);
  Serial.println(F("motors attached"));
  rollpid.SetMode(AUTOMATIC);
  pitchpid.SetMode(AUTOMATIC);
  yawpid.SetMode(AUTOMATIC);
  altipid.SetMode(AUTOMATIC);
  pitchservo.attach(5);
  yawservo.attach(6);
  radio.begin();
  radio.openWritingPipe(direccion);

  throttle1 = 0;
  throttle2 = 0;
  throttle = 0;
  Serial.println(F("welcome to PureSky UI"));
  delay(500);
  Serial.println(F("Powered by A-Flights"));
  delay(2000);
  Serial.println(F("ESC calibration menu"));
  motor1dis = false;
  motor2dis = false;
  bool cal = true;
  while (calibrando == true) {
    if (Serial.available() > 1)
    {
      throttle = Serial.parseFloat();
      if (throttle > -1 &&  throttle <= 100)
      {
        DELAY1 = (throttle * 10) + 1000;
        DELAY2 = (throttle * 10) + 1000;
        if (motor1dis == false) {
          motor1.writeMicroseconds(DELAY1);
        }
        if (motor2dis == false) {
          motor2.writeMicroseconds(DELAY2);
        }
        Serial.print("\n");
        Serial.println(F("Throttle set to:")); 
        Serial.print("  "); 
        Serial.print(throttle); 
        Serial.println("%");
      }
      else if (throttle == 111) {
        calibrando = false;
        throttle = 0;
        DELAY1 = 0;
        DELAY2 = 0;
        Serial.println("");
        break;
      }
      else if (throttle == 123) {
        motor1.writeMicroseconds(800);
        motor2.writeMicroseconds(800);
      }
      else if (throttle == 1111) {
        motor1dis = true;
        Serial.println();
        Serial.println(F("motor 1 disarmed"));
      }
      else if (throttle == 2222) {
        motor2dis = true;
        Serial.println();
        Serial.println(F("motor 2 disarmed"));
      }
      else if (throttle == 222) {
        calibrando = false;
        cal = false;
        throttle = 0;
        DELAY1 = 0;
        DELAY2 = 0;
        break;
      }
    }
  }
  if (cal) {
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
    Wire.begin();
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);
    Serial.println(F("IMU started"));
    Serial.println(F("Calibrating IMU..."));
    delay(1000);
    for (int times = 0; times < 2000; times++) {
      Wire.beginTransmission(0x68);
      Wire.write(0x43);
      Wire.endTransmission(false);
      Wire.requestFrom(0x68, 6, true);
      Xspeederror += Wire.read() << 8 | Wire.read();
      Yspeederror += Wire.read() << 8 | Wire.read();
      Zspeederror += Wire.read() << 8 | Wire.read();
      delay(5);
    }

    Xspeederror = Xspeederror / 2000;
    Yspeederror = Yspeederror / 2000;
    Zspeederror = Zspeederror / 2000;

    Serial.println(Xspeederror);
    Serial.println(Yspeederror);
    Serial.println(Zspeederror);
    Serial.println();
    Serial.println(F("Calibrating radar..."));
    delay(2000);
    for (int times = 0; times < 20; times++) {
      Serial.println(hcs.dist());
    }
    Serial.println(F("calibrating servo gimbal..."));
    pitchservo.writeMicroseconds(1600);
    yawservo.writeMicroseconds(1600);
    delay(3000);
    pitchservo.writeMicroseconds(2000);
    yawservo.writeMicroseconds(2000);
    delay(3000);
    pitchservo.writeMicroseconds(1200);
    yawservo.writeMicroseconds(1200);
    delay(3000);
    pitchservo.writeMicroseconds(1600);
    yawservo.writeMicroseconds(1600);
    delay(3000);
    Serial.println();
    Serial.println(F("Calibration completed"));
    delay(5000);
  }
  else {
    Serial.println(F("WARNING!!! calibration skipped"));
  }
  Serial.println(F("Send launch code"));
  while (answer != 42) {
    if (Serial.available() > 0) {
      answer = Serial.parseInt();
    }
    else {
      motor1.writeMicroseconds(1000);
      motor2.writeMicroseconds(1000);
    }
  }
  Serial.println(F("launching in 30 seconds"));
  for (int t = 30; t > -1; t--) {
    Serial.println(t);
    datos[0] = t;
    datos[1] = t;
    datos[2] = t;
    datos[3] = t;
    datos[4] = t;
    datos[5] = t;
    datos[6] = t;
    if (Serial.available() > 1){
      Serial.println(F("ABORT!!!"));
      delay(100000000);
    }
    bool ok = radio.write(datos, sizeof(datos));
    if (ok) {
      Serial.println(F("paquete recibido"));
    }
    else {
      Serial.println(F("paquete no recibido"));
    }
    delay(1000);
  }


  long XrawAcc = 0, YrawAcc = 0, ZrawAcc = 0;
  for (int times = 0; times < 100; times++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);
    XrawAcc += Wire.read() << 8 | Wire.read();
    YrawAcc += Wire.read() << 8 | Wire.read();
    ZrawAcc += Wire.read() << 8 | Wire.read();
    delay(5);
  }
  XrawAcc = XrawAcc / 100;
  YrawAcc = YrawAcc / 100;
  ZrawAcc = ZrawAcc / 100;
  int xAng = map(XrawAcc, 265, 402, -90, 90);
  int yAng = map(YrawAcc, 265, 402, -90, 90);
  int zAng = map(ZrawAcc, 265, 402, -90, 90);

  yerror = (RAD_TO_DEG * (atan2(-xAng, -zAng) + PI)) - 87.5;
  zerror = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
  while (yerror > 180) {
    yerror = 360 - yerror;
  }
  while (zerror > 180) {
    zerror = -(360 - zerror);
  }
  rollset = 0;
  pitchset = 0;
  yawset = 0;
  Serial.println(zerror);
  Serial.println(yerror);
  Time = millis();
}

void loop() {
  HCSR04 hcs(2, 3);
  timePrev = Time;
  Time = millis();
  elapsedTime = (Time - timePrev) / 1000;
  altitude = cos(abs(fyawangle / 180) * 2 * PI) * ((cos(abs(fpitchangle / 180)) * 2 * PI  ) * (hcs.dist()));
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true);
  rawX = Wire.read() << 8 | Wire.read();
  rawY = Wire.read() << 8 | Wire.read();
  rawZ = Wire.read() << 8 | Wire.read();
  Xspeed = (rawX - Xspeederror) / 131;
  Yspeed = (rawY - Yspeederror) / 131;
  Zspeed = (rawZ - Zspeederror) / 131;
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);
  float XrawAcc = Wire.read() << 8 | Wire.read();
  float YrawAcc = Wire.read() << 8 | Wire.read();
  float ZrawAcc = Wire.read() << 8 | Wire.read();
  int xAng = map(XrawAcc, 265, 402, -90, 90);
  int yAng = map(YrawAcc, 265, 402, -90, 90);
  int zAng = map(ZrawAcc, 265, 402, -90, 90);
  yaccang = (RAD_TO_DEG * (atan2(-xAng, -zAng) + PI)) - 87.5;
  zaccang = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
  rollangle = rollangle + (Xspeed * elapsedTime);
  pitchangle = pitchangle + (Yspeed * elapsedTime);
  yawangle = yawangle + (Zspeed * elapsedTime);
  yawangle -= pitchangle * sin(Xspeed * elapsedTime * 3.14159 / 180);
  pitchangle += yawangle * sin(Xspeed * elapsedTime * 3.14159 / 180);
  frollangle = rollangle;
  fyawangle = 0.98 * ( yawangle - (zerror + yawPIDError) ) + 0.02 * yaccang;
  fpitchangle = 0.98 * ( pitchangle - (yerror + pitchPIDError) ) + 0.02 * zaccang;
  Serial.print(";");
  Serial.print(fpitchangle + (pitchPIDError));
  Serial.print(";");
  Serial.print(fyawangle + (yawPIDError));
  Serial.print(";");
  Serial.print(";");
  Serial.print(pitch);
  Serial.print(";");
  Serial.print(yaw);
  Serial.print(";");
  Serial.print(altitude);
  Serial.print(";");
  Serial.println(elapsedTime);
  if (Serial.available() > 2) {
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
    pitchservo.writeMicroseconds(1600);
    yawservo.writeMicroseconds(1600);
    Serial.println(F("ABORT!!!"));
    delay(1000000);
  }
  rollpid.Compute();
  pitchpid.Compute();
  yawpid.Compute();
  roll = rawroll - (127.5);
  pitch = rawpitch - (127.5);
  yaw = rawyaw - (127.5);
  throttle = 0; // REMOVE BEFORE FLIGHT OR THERE WILL BE NO FLIGHT ;)
  throttle1 = throttle - roll;
  throttle2 = throttle + roll;
  DELAY1 = (throttle1 * 10) + 1000;
  DELAY2 = (throttle2 * 10) + 1000;
  roll = 0; // REMOVE BEFORE FLIGHT OR THERE WILL BE NO FLIGHT ;)
  if (DELAY1 <= 2000 && DELAY1 >= 1000) {
    //motor1.writeMicroseconds(DELAY1);
  }
  else if (DELAY1 > 2000) {
    //motor1.writeMicroseconds(2000);
  }
  else {
    motor1.writeMicroseconds(1000);
  }
  if (DELAY2 <= 2000 && DELAY2 >= 1000) {
    //motor2.writeMicroseconds(DELAY2);
  }
  else if (DELAY2 > 2000) {
    //motor2.writeMicroseconds(2000);
  }
  else {
    motor2.writeMicroseconds(1000);
  }
  if (pitch <= 400 && pitch >= -400) {
    if (pitch > 0) {
      pitchservo.writeMicroseconds(1600 + abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
    else {
      pitchservo.writeMicroseconds(1600 - abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
  }
  if (yaw <= 400 && yaw >= -400) {
    if (yaw > 0) {
      yawservo.writeMicroseconds(1600 + abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
    else {
      yawservo.writeMicroseconds(1600 - abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
  }
}
