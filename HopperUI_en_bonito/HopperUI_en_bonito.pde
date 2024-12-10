import g4p_controls.*;

import net.java.games.input.*;
import org.gamecontrolplus.*;
import org.gamecontrolplus.gui.*;

import processing.serial.*;
import controlP5.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

boolean started = false, ended = false;
byte index;
String end;

String total;
Serial COM6;  // Create object from Serial class
char val;     // Data received from the serial port
int mode;
String lastm;
String m;
ControlP5 cp5;

float throttle = 0;
int t = 0;
Textlabel myTextlabelA;
Textlabel myTextlabelB;
Textlabel myTextlabelC;
float n = 0;
int lastTime = 0;
int info = 1;
PImage nav;
PImage dot;
PImage dot2;
PImage target;
int changex = 0;
int changey = 0;
boolean left = false;
boolean right = false;
boolean up = false;
boolean down = false, e = false, q = false, shift = false, control = false;
float pitch = 0, yaw = 0, roll = 0, pitcha = 0, yawa = 0, alt = 0, r = 0;
List<String> message = new ArrayList<>();
List<String> pack2 = new ArrayList<>();
String[] pack;
void setup() {
  //for(int f = 0; f < 50; f++){
  //message.add("");
  //}
  size(2000, 900);
  nav = loadImage("nav dish.jpg");
  dot = loadImage("dot2.png");
  dot2 = loadImage("target.png");
  target = loadImage("target.png");
  cp5 = new ControlP5(this);
  Slider s = cp5.addSlider("Throttle", 0, 100, throttle, 10, 10, 100, 700); //Name, Min, max, pos, x coord, y coord, width, height
  Slider p = cp5.addSlider("", -90, 90, r, 950, 290, 180 * 2, 10); //Name, Min, max, pos, x coord, y coord, width, height
  Slider q = cp5.addSlider("Yaw", -90, 90, changex, 950, 310, 180 * 2, 10); //Name, Min, max, pos, x coord, y coord, width, height
  Slider r = cp5.addSlider("Pitch", -90, 90, changey, 900, 350, 10, 180 * 2); //Name, Min, max, pos, x coord, y coord, width, height
  Slider t = cp5.addSlider("alt", 0, 5, alt, 840, 350, 30, 180 * 2); //Name, Min, max, pos, x coord, y coord, width, height
  cp5.addButton("end_calibration")
    .setValue(0)
    .setPosition(150, 10)
    .setSize(200, 25)
    ;
  cp5.addButton("Full_throttle")
    .setValue(0)
    .setPosition(150, 50)
    .setSize(200, 40)
    ;
  cp5.addButton("Cut_throttle")
    .setValue(0)
    .setPosition(150, 95)
    .setSize(200, 40)
    ;
  cp5.addButton("Send_Launch_code")
    .setValue(0)
    .setPosition(390, 10)
    .setSize(200, 25)
    ;
  COM6 = new Serial(this, "COM3", 38400);
  mode = 1;

  myTextlabelA = cp5.addTextlabel("labelA")
    .setText("Mode:")
    .setPosition(1265, 5)
    .setFont(createFont("Impact", 20))
    ;
  myTextlabelB = cp5.addTextlabel("labelB")
    .setText(str(mode))
    .setPosition(1325, 5)
    .setFont(createFont("Impact", 20))
    ;
  val = 'N';
  myTextlabelC = cp5.addTextlabel("labelC")
    .setText(str(val))
    .setSize(200, 100)
    .setPosition(1000, 5)
    .setFont(createFont("Impact", 10))
    ;
  delay(1000);
  lastm = "";
  lastTime = 0;
}

void draw() {

  background(20, 20, 20);
  image(nav, 950, 350, 2 * 180, 2 * 180);
  image(target, 1121 + (2 * (changex)), 521 - (2 * (changey)));
  image(dot, 1122 + (2 * yaw), 521 - (2 * pitch));
  image(dot2, 1130 + yaw + yawa, 529 - pitch - pitcha, 4, 4);
  cp5.get("").setValue(r);
  cp5.get("Yaw").setValue(yaw);
  cp5.get("Pitch").setValue(pitch);
  cp5.get("alt").setValue(alt);
  //myTextlabelC.setText(str(round(cp5.get("Throttle").getValue())));
  myTextlabelB.setText(str(mode));
  if (COM6.available() > 0) {
    val = COM6.readChar();
    //myTextlabelC.setText(val);
    if (val == '%') {
      mode = 3;
    }
  }
  m = (str(round(cp5.get("Throttle").getValue())));
  throttle = float(m);
  if (mode == 1) {
    throttle = float(m);
    m = (str(round(cp5.get("Throttle").getValue())));
    if (float(lastm) != float(m)) {
      COM6.write('<');
      COM6.write(m);
      COM6.write('>');
      lastTime = millis();
      lastm = m;
      print(m);
      println(lastm);
    }
    myTextlabelC.setText(str(val) + total);
  } else if (mode == 3) {
    while (COM6.available() > 0) {
      char input = COM6.readChar();
      if (input == '<') {
        index = 0;
        message.set(index, "");
        started = true;
        ended = false;
      } else if (input == '>')
      {
        ended = true;
        break;
      } else {
        message.set(index, str(input));
        index++;
        message.set(index, "");
      }
    }
    if (started && ended) {
      //process
      for (int w = 0; w < message.size(); w++) {
        end = end + message.get(w);
      }
      pack2 = Arrays.asList(end.split(","));
      pitch = float(pack2.get(0));
      yaw = float(pack2.get(1));
      roll = float(pack2.get(2));
      pitcha = float(pack2.get(3));
      yawa = float(pack2.get(4));
      alt = float(pack2.get(5));
      //endprocess
      started = false;
      ended = false;
      index = 0;
      message.clear();
      //for(int f = 0; f < 50; f++){
      //message.add("");
      //}
      COM6.write("<" + throttle + "," + changex + "," + changey + "," + roll + ">");
    }
  } else {
    myTextlabelC.setText(str(val));
  }
  if (left == true)
  {
    if (changex > -89) changex = changex - 1;
  }
  if (right == true)
  {
    if (changex < 89) changex = changex + 1;
  }
  if (up == true)
  {
    if (changey < 89)changey = changey + 1;
  }
  if (down == true)
  {
    if (changey > -89) changey = changey - 1;
  }
  if (q == true)
  {
    if (r > -89)r = r - 1.5;
  }
  if (e == true)
  {
    if (r < 89) r = r + 1.5;
  }
  if (shift == true)
  {
    if (throttle <= 98.50)throttle = throttle + 1.5;
    cp5.get("Throttle").setValue(throttle);
  }
  if (control == true)
  {
    if (throttle >= 1.50) throttle = throttle - 1.5;
    cp5.get("Throttle").setValue(throttle);
  }
}

void end_calibration() {
  if (mode == 1) {
    mode = 2;
    delay(100);
    COM6.write("<");
    COM6.write("111");
    COM6.write(">");
    mode = 2;
  }
}
void Full_throttle() {
  //COM6.write("100");
  throttle = 100;
  cp5.get("Throttle").setValue(100);
}
void Cut_throttle() {
  //COM6.write("0");
  throttle = 0;
  cp5.get("Throttle").setValue(0);
}
void Send_Launch_code() {
  COM6.write("<");
  COM6.write("42");
  COM6.write(">");
  delay(5000);
  mode = 3;
}
void keyPressed()
{


  if (key == 'a')
  {
    left = true;
  }
  if (key == 'd')
  {
    right = true;
  }
  if (key == 'w')
  {
    up = true;
  }
  if (key == 's')
  {
    down = true;
  }
  if (key == 'e')
  {
    e = true;
  }
  if (key == 'q')
  {
    q = true;
  }
  if (keyCode == SHIFT)
  {
    shift = true;
  }
  if (keyCode == CONTROL)
  {
    control = true;
  }
  if (key == 'z')
  {
    throttle = 100;
    cp5.get("Throttle").setValue(throttle);
  }
  if (key == 'x')
  {
    throttle = 0;
    cp5.get("Throttle").setValue(throttle);
  }
}
void keyReleased()
{


  if (key == 'a')
  {
    left = false;
  }
  if (key == 'd')
  {
    right = false;
  }
  if (key == 'w')
  {
    up = false;
  }
  if (key == 's')
  {
    down = false;
  }
  if (key == 'e')
  {
    e = false;
  }
  if (key == 'q')
  {
    q = false;
  }
  if (keyCode == SHIFT)
  {
    shift = false;
  }
  if (keyCode == CONTROL)
  {
    control = false;
  }
}
