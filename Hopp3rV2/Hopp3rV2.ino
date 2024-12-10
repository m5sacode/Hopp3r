#define SOP '<'
#define EOP '>'
float lastp = 0, lasty=0,lastr=0;
bool started = false;
bool ended = false;
char inData[80];
byte index;
String mode = "rc";
char *pack = NULL;
char *strings[4];
#include <printf.h>
#include <RF24_config.h>
#include <filters.h>
#include <filters_defs.h>
#include <Wire.h>
#include <PID_v1.h>
#include <Servo.h>
#include <HCSR04.h>
#include <SPI.h>
int thrustlimit = 1000; //CHECK BEFORE FLIGHT THAT THIS IS SET TO 2000 OR MOTORS WON'T REACH FULL THRUST!!!!!
float rolli = 0, pitchi = 0, yawi = 0;
boolean calibrando = true;
int answer = 0;
IIR::ORDER  order  = IIR::ORDER::OD3;
double altitude = 0;
float radardis;
float throttle1;
float throttle2;
double throttle = 0;
double lastt = 0;
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
double rollKp = 1, rollKi = 0, rollKd = 0;
double pitchKp = 20, pitchKi = 0, pitchKd = 0;
double yawKp = 20, yawKi = 0, yawKd = 0;
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
float filterFrequency = 10.0;
double altiset = 0;
boolean motor1dis = false, motor2dis = false;
double altiKp = 0, altiKi = 0, altiKd = 0;
float twisti = 0.9;
Servo motor1;
Servo motor2;
Servo pitchservo;
Servo yawservo;
//PID rollpid(&frollangle, &rawroll, &rollset, rollKp, rollKi, rollKd, DIRECT);
PID pitchpid(&fpitchangle , &rawpitch, &pitchset, pitchKp, pitchKi, pitchKd, DIRECT);
PID yawpid(&fyawangle , &rawyaw, &yawset, yawKp, yawKi, yawKd, REVERSE);
PID altipid(&altitude, &throttle, &altiset, altiKp, altiKi, altiKd, DIRECT);
HCSR04 hcs(2, 3);
void setup() {
  
  Serial.begin(115200);
  motor1.attach(A0);
  motor2.attach(A1);
  pitchpid.SetMode(AUTOMATIC);
  yawpid.SetMode(AUTOMATIC);
  altipid.SetMode(AUTOMATIC);
  pitchservo.attach(6);
  yawservo.attach(5);

  throttle1 = 0;
  throttle2 = 0;
  throttle = 0;
  Serial.println(F("ESC calibration menu"));
  motor1dis = false;
  motor2dis = false;
  bool cal = true;
  while (calibrando == true) {
    if (lastt != throttle) {
      Serial.print("<");
      Serial.print(throttle);
      Serial.println(">");
      lastt = throttle;
    }
    //delay(1000/20);
    while (Serial.available() > 0)
    {
      char inChar = Serial.read();
      if (inChar == SOP)
      {
        index = 0;
        inData[index] = '\0';
        started = true;
        ended = false;
      }
      else if (inChar == EOP)
      {
        ended = true;
        break;
      }
      else
      {
        if (index < 79)
        {
          inData[index] = inChar;
          index++;
          inData[index] = '\0';
        }
      }
    }

    // We are here either because all pending serial
    // data has been read OR because an end of
    // packet marker arrived. Which is it?
    if (started && ended)
    {
      // The end of packet marker arrived. Process the packet
      throttle = float(atoi(inData));
      // Reset for the next packet
      started = false;
      ended = false;
      index = 0;
      inData[index] = '\0';
    }
    if (throttle >= 0 &&  ((throttle <= 85) || (throttle == 100)))
    {
      DELAY1 = (twisti*throttle * 10) + 1000;
      DELAY2 = (throttle * 10) + 1000;
      if (motor1dis == false && DELAY1<1800) {
        motor1.writeMicroseconds(DELAY1);
      }
      if (motor2dis == false && DELAY2 < 1800) {
        motor2.writeMicroseconds(DELAY2);
      }
      //Serial.print("\n");

    } else if (throttle >= 300 &&  (throttle <= 400))
    {
      pitchservo.write((throttle-350)+90);
    } else if (throttle >= 600 &&  (throttle <= 700))
    {
      yawservo.write((throttle-650)+90);
    }
    else if (throttle >= 830 &&  (throttle <= 880))
    {
      twisti = 0.8+((throttle-850)/25);
    }
    else if (throttle == 111) {
      calibrando = false;
      throttle = 0;
      DELAY1 = 0;
      DELAY2 = 0;
      //break;------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!---------------------------------------REMOVE BEFORE FLIGHT: OVERRIDES CAL END
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
      //break;---------------------------------------------------------------------------------------------------------REMOVE BEFORE FLIGHT: OVERRIDES CAL END
    }

  }
  if (cal) {
    Serial.println(F("calibrating servo gimbal..."));
    pitchservo.write(80);
    yawservo.write(80);
    delay(3000);
    pitchservo.write(100);
    yawservo.write(100);
    delay(3000);
    pitchservo.write(80);
    yawservo.write(80);
    delay(3000);
    pitchservo.write(90);
    yawservo.write(90);
    delay(3000);
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
    Serial.println("ESCs are called");
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
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
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
    delay(2000);
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
  Serial.println(F("launching in 10 seconds"));
  for (int t = 5; t > -1; t--) {
    Serial.println(t);
    if (Serial.available() > 1) {
      Serial.println(F("ABORT!!!"));
      delay(100000000);
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
  //rollset = 0;
  //pitchset = 0;
  //yawset = 0;
  Serial.println(zerror);
  Serial.println(yerror);
  Time = millis();
  float rolli = 0, yawi = 0, pitchi = 0;
  roll = 0;
  rollangle = 0;
  Xspeed = 0;
  Serial.println('%');
  lastp=0;
  lasty=0;
  lastr=0;
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
  frollangle = 2*rollangle;
  fyawangle = 2 * ( yawangle - (zerror) ) + 0.0 * yaccang;
  fpitchangle = 2 * ( pitchangle - (yerror) ) + 0.0 * zaccang;
  Serial.print('<');
  if (abs(fpitchangle-lastp) < 5 ){
    Serial.print(fpitchangle);//0
  }
  else{Serial.print(lastp);}
  lastp=fpitchangle;
  Serial.print(',');
  if (abs(fyawangle-lasty) < 5 ){
    Serial.print(fyawangle);//1
  }
  else{Serial.print(lasty);}
  lasty=fyawangle;
  Serial.print(',');
  if (abs(frollangle-lastr) < 5 ){
    Serial.print(frollangle);//2
  }
  else{Serial.print(lastr);}
  lastr=frollangle;
  Serial.print(',');
  Serial.print(int(float(pitch)));//3
  Serial.print(',');
  Serial.print(int(float(yaw)));//4
  Serial.print(',');
  Serial.print(int(float(altitude)));//5
  Serial.print('>');
  if (Serial.read() == '*')pitchangle = 0;
  /*if (abs(fpitchangle-lastp) < 5 ){
    Serial.println(fpitchangle);//0
  }
  else{Serial.println(lastp);}
  lastp=fpitchangle;*///-----------------------------------------------------------------------------------------------------------------------remove!!!!!
  //altitude = 2; //REMOVE BEFORE FLIGHT OR IT WILL CRASH ;)
  while (Serial.available() > 0)
  {
    char inChar = Serial.read();
    
    if (inChar == SOP)
    {
      index = 0;
      inData[index] = '\0';
      started = true;
      ended = false;
    }
    else if (inChar == EOP)
    {
      ended = true;
      break;
    }
    else
    {
      if (index < 79)
      {
        inData[index] = inChar;
        index++;
        inData[index] = '\0';
      }
    }
  }

  // We are here either because all pending serial
  // data has been read OR because an end of
  // packet marker arrived. Which is it?
  if (started && ended)
  {
    // The end of packet marker arrived. Process the packet
    pack = strtok(inData, ",");
    index = 0;
    while (pack != NULL) {
      strings[index] = pack;
      index++;
      pack = strtok(NULL, ",");
    }
    if(atoi(strings[0])!=NULL)throttle = atoi(strings[0]);
    if(atoi(strings[1])!=NULL)yawset = atoi(strings[1]);
    if(atoi(strings[2])!=NULL)pitchset = atoi(strings[2]);
    if(atoi(strings[3])!=NULL)rollset = atoi(strings[3]);//----------------------------------------------------------------------------------mode1
    /*if(atoi(strings[1])!=NULL)yaw = atoi(strings[1]);
    if(atoi(strings[2])!=NULL)pitch = atoi(strings[2]);
    if(atoi(strings[3])!=NULL)roll = atoi(strings[3]);*/
    // Reset for the next packet
    started = false;
    ended = false;
    index = 0;
    inData[index] = '\0';
  }
  /*rolli = rolli + ((frollangle - rollset) * elapsedTime);
  rawroll = (rollKp * (frollangle - rollset)) + (rollKi * rolli) + (rollKd * Xspeed);
  roll = rawroll;
  pitchi = pitchi + ((fpitchangle - pitchset) * elapsedTime);
  pitch = (pitchKp * (fpitchangle - pitchset)) + (pitchKi * pitchi) + (pitchKd * Yspeed);
  yawi = yawi + ((fyawangle - yawset) * elapsedTime);
  yaw = (yawKp * (fyawangle - yawset)) + (yawKi * yawi) + (yawKd * Zspeed);
  //throttle = 0; // REMOVE BEFORE FLIGHT OR THERE WILL BE NO FLIGHT ;)
  //roll = 0; // REMOVE BEFORE FLIGHT OR THERE WILL BE NO FLIGHT ;)*/ //----------------------------------------------------------------------mode1
  
  throttle1 = throttle - roll;
  throttle2 = throttle + roll;
  DELAY1 = (throttle1 * 10) + 1000;
  DELAY2 = (throttle2 * 10) + 1000;
  if (DELAY1 <= thrustlimit && DELAY1 >= 1000) {
    motor1.writeMicroseconds(DELAY1);
  }
  else if (DELAY1 > thrustlimit) {
    motor1.writeMicroseconds(thrustlimit);
  }
  else {
    motor1.writeMicroseconds(1000);
  }
  if (DELAY2 <= thrustlimit && DELAY2 >= 1000) {
    motor2.writeMicroseconds(DELAY2);
  }
  else if (DELAY2 > thrustlimit) {
    motor2.writeMicroseconds(thrustlimit);
  }
  else {
    motor2.writeMicroseconds(1000);
  }
  if (pitch <= 127.5 && pitch >= -127.5) {
    if (pitch > 0) {
      pitchservo.writeMicroseconds(1600 + abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
    else {
      pitchservo.writeMicroseconds(1600 - abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
  }
  else if (pitch > 127.5) {
    pitch = 127.5;
    if (pitch > 0) {
      pitchservo.writeMicroseconds(1600 + abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
    else {
      pitchservo.writeMicroseconds(1600 - abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
  }
  else if (pitch < -127.5) {
    pitch = -127.5;
    if (pitch > 0) {
      pitchservo.writeMicroseconds(1600 + abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
    else {
      pitchservo.writeMicroseconds(1600 - abs(map(pitch, -127.5, 127.5, -400, 400)));
    }
  }
  if (yaw <= 127.5 && yaw >= -127.5) {
    if (yaw > 0) {
      yawservo.writeMicroseconds(1600 + abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
    else {
      yawservo.writeMicroseconds(1600 - abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
  }
  else if (yaw > 127.5) {
    yaw = 127.5;
    if (yaw > 0) {
      yawservo.writeMicroseconds(1600 + abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
    else {
      yawservo.writeMicroseconds(1600 - abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
  }
  else if (yaw < -127.5) {
    yaw = 127.5;
    if (yaw > 0) {
      yawservo.writeMicroseconds(1600 + abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
    else {
      yawservo.writeMicroseconds(1600 - abs(map(yaw, -127.5, 127.5, -400, 400)));
    }
  }
}
