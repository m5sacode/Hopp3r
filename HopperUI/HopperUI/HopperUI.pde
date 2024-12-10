 

import processing.serial.*;
import controlP5.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

import g4p_controls.*;

import net.java.games.input.*;
import org.gamecontrolplus.*;
import org.gamecontrolplus.gui.*;

ControlDevice stick;
ControlIO control2;
float throttlea, px, py, pz;

int count =0, count2=0;
boolean started= false, ended=false;
byte index;
String end;
char input;
String total;
Serial COM6;  // Create object from Serial class
char val;     // Data received from the serial port
int mode;
String lastm, lastm2 = "", lastm3 = "", lastm4 = "";
String m;
String m2;
String m3;
String m4;
ControlP5 cp5;

float throttle = 0;
int t = 0;
Textlabel myTextlabelA;
Textlabel myTextlabelB;
Textlabel myTextlabelC;
Textlabel myTextlabelD;
Textlabel myTextlabelE;
float n = 0;
int lastTime = 0;
int lastT = 0, lastT2 = 0;
int elapsed = 0, elapsed2 = 0;
int fps;
int pps;
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
boolean down = false, e= false, q=false, shift=false, control=false;
float pitch=0, yaw=0, roll=0, pitcha=0, yawa=0, alt=0, r=0;
List<String> message = new ArrayList<>();
List<String> messageend = new ArrayList<>();
List<String> pack2 = new ArrayList<>();

//graphthings
List<String> pitches = new ArrayList<>();
List<String> yaws = new ArrayList<>();
List<String> rolls = new ArrayList<>();
List<String> alts = new ArrayList<>();
List<String> times = new ArrayList<>();
List<String> ppss = new ArrayList<>();

float pxscale = 10;
float pyscale = 10;
int pgraphx = 700;
int pgraphy = 300;

float yxscale = 10;
float yyscale = 10;
int ygraphx = 700;
int ygraphy = 500;

String[] pack;
void setup() {
  for (int f = 0; f < 50; f++) {
    pitches.add("");
    yaws.add("");
    rolls.add("");
    ppss.add("");
    times.add("");
  }
  size(2000, 900);
  control2 = ControlIO.getInstance(this);
  stick = control2.getMatchedDevice("joystick");
  if (stick == null) {
    println("no se ha encontrado el joystick");
    System.exit(-1);
  }
  nav = loadImage("nav dish.jpg");
  dot = loadImage("dot2.png");
  dot2 = loadImage("target.png");
  target = loadImage("target.png");
  cp5 = new ControlP5(this);
  cp5.addSlider("Throttle", 0, 100, throttle, 10, 10, 100, 700); //Name, Min, max, pos, x coord, y coord, width, height
  cp5.addSlider("", -90, 90, r, 950, 290, 180*2, 10); //Name, Min, max, pos, x coord, y coord, width, height
  cp5.addSlider("Yaw", -90, 90, changex, 950, 310, 180*2, 10); //Name, Min, max, pos, x coord, y coord, width, height
  cp5.addSlider("Pitch", -90, 90, changey, 900, 350, 10, 180*2); //Name, Min, max, pos, x coord, y coord, width, height
  cp5.addSlider("alt", 0, 5, alt, 840, 350, 30, 180*2); //Name, Min, max, pos, x coord, y coord, width, height
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
  COM6 = new Serial(this, "COM3", 115200);
  COM6.buffer(1);
  mode = 1;

  myTextlabelA = cp5.addTextlabel("labelA")
    .setText("Mode:")
    .setPosition(1265, 5)
    .setFont(createFont("Impact", 20))
    ;
  myTextlabelD = cp5.addTextlabel("labelD")
    .setText("FPS: "+str(fps))
    .setPosition(1265, 25)
    .setFont(createFont("Impact", 20))
    ;
  myTextlabelE = cp5.addTextlabel("labelE")
    .setText("PPS:   "+str(pps))
    .setPosition(1265, 45)
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
  stick.getButton("gatillo").plug(this, "gatillof", ControlIO.ON_PRESS);

  lastm = "";
  message.clear();
  messageend.clear();
  index = 0;
  lastTime = 0;
  lastT= 0;
}

void draw() {
  elapsed = millis()-lastT;
  lastT = millis();
  getInput();
  fps = fps + (1000/(elapsed));
  background(20, 20, 20);
  image(nav, 950, 350, 2*180, 2*180);
  image(target, 1121+(2*(px)), 521-(2*(py)));
  image(dot, 1122-(2*yaw), 521-(2*pitch));
  //image(dot2, 1130+yaw-yawa, 529-pitch+pitcha, 4, 4);//add this
  image(dot2, 1130+yawa, 529+pitcha, 4, 4); // delete this, just for testing .........................................................................................................
  cp5.get("").setValue(pz);
  cp5.get("Yaw").setValue(px);
  cp5.get("Pitch").setValue(py);
  cp5.get("alt").setValue(alt/100);
  if(mode==1)cp5.get("Throttle").setValue((throttlea));
  for (int w=0; w<(pitches.size()-1); w++) {
    line((float(times.get(w))*pxscale)+pgraphx-((millis()/100)*pxscale), (float(pitches.get(w))*pyscale)+pgraphy, (float(times.get(w+1))*pxscale)+pgraphx-((millis()/100)*pxscale), (float(pitches.get(w+1))*pyscale)+pgraphy);
    stroke(255, 255, 255);
    strokeWeight(7);
  }
  for (int w=0; w<(yaws.size()-1); w++) {
    line((float(times.get(w))*yxscale)+ygraphx-((millis()/100)*yxscale), (float(yaws.get(w))*yyscale)+ygraphy, (float(times.get(w+1))*yxscale)+ygraphx-((millis()/100)*yxscale), (float(yaws.get(w+1))*yyscale)+ygraphy);
    stroke(255);
    strokeWeight(7);
  }
  yaws.add(49, str(yaw));
  yaws.remove(0);
  pitches.add(49, str(pitch));
  pitches.remove(0);
  times.add(49, str(millis()/100));
  times.remove(0);
  //myTextlabelC.setText(str(round(cp5.get("Throttle").getValue())));
  myTextlabelB.setText(str(mode));
  count = count +1;
  if (count >= 20) {
    myTextlabelD.setText("FPS:        "+str(fps/20));
    fps=0;
    count= 0;
  }
  if (COM6.available() > 0) {
    val = COM6.readChar();
    //myTextlabelC.setText(val);
    if (val == '%') {
      mode = 3;
    }
  }
  m = (str(round(cp5.get("Throttle").getValue())));
  m2 = (str(round((py/2)+350)));
  m3 = (str(round((px/2)+650)));
  m4 = (str(round((pz)+850)));
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
      //print(m);
      //println(lastm);
    }
    myTextlabelC.setText(str(val)+total);
    py = float(m2);
    //m2 = (str(py));
    if (float(lastm2) != float(m2)) {
      COM6.write('<');
      COM6.write(m2);
      COM6.write('>');
      lastTime = millis();
      lastm2 = m2;
      //print(m2);
      //println(lastm2);
    }
    //px = float(m3);
    //m3 = (str(pz));
    if (float(lastm3) != float(m3)) {
      COM6.write('<');
      COM6.write(m3);
      COM6.write('>');
      lastTime = millis();
      lastm3 = m3;
      //print(m3);
      //println(lastm3);
    }
    COM6.write('<');
    COM6.write(m4);
    COM6.write('>');
    println(m4);
  } else if (mode == 3) {
    if ((started && ended) && message!=null && message.size() < 35 && message.size() > 10) {
      messageend = message;
      //println("hi");
      //println();
      //process
      end = "";
      if (messageend.size()>5) {
        for (int r = 0; r < messageend.size(); r++) {
          if (messageend.size()>r)end = end + messageend.get(r);
          else break;
        }
      }
      println(end);
      pack2 = Arrays.asList(end.split(","));
      if (pack2.size() == 6) {
        if (int(pack2.get(0)) != 0) pitch = int(pack2.get(0));
        if (int(pack2.get(1)) != 0)yaw = int(pack2.get(1));
        if (int(pack2.get(2)) != 0)roll = int(pack2.get(2));
        if (int(pack2.get(3))/3 != 0)pitcha = int((pack2.get(3)))/3;
        if (int(pack2.get(4))/3 != 0)yawa = int((pack2.get(4)))/3;
        if (int(pack2.get(5))/100 > 0.2 && int(pack2.get(5))/100<5)alt = int(pack2.get(5));
        println(alt);
      }
      //endprocess
      started=false;
      ended=false;
      elapsed2 = millis() - lastT2;
      lastT2 = millis();
      pps = pps+(1000/elapsed2);
      if (count2 >=20) {
        print(pps/20);
        myTextlabelE.setText("PPS:        "+str(pps/count2));
        pps=0;
        count2 =0;
      }

      index=0;
      messageend.clear();
      //for(int f = 0; f < 50; f++){
      //message.add("");
      //}
      //COM6.write("<"+throttle+","+changex+","+changey+","+roll+">");
    }
    //COM6.write("<"+throttle+","+changex+","+changey+","+roll+">");
    COM6.write("<"+throttle+","+px+","+py+","+pz+">");
    throttle = throttlea;
    cp5.get("Throttle").setValue(throttle);
  } else {
    myTextlabelC.setText(str(val));
  }
  count2= count2 +1;
  if (left == true)
  {
    if (changex>-89) changex = changex-1;
  }
  if (right == true)
  {
    if (changex<89) changex = changex+1;
  }
  if (up == true)
  {
    if (changey<89)changey = changey+1;
  }
  if (down == true)
  {
    if (changey>-89) changey = changey-1;
  }
  if (q == true)
  {
    if (r>-89)r = r-1.5;
  }
  if (e == true)
  {
    if (r<89) r = r+1.5;
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
    //mode = 2;
    delay(100);
    COM6.write("<");
    COM6.write("111");
    COM6.write(">");
    //mode = 2;
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
  //mode = 3;
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
void serialEvent(Serial COM6) {
  if (mode==3) {
    input = COM6.readChar();
    //print(input);
    if (input == '<') {
      index=0;
      //message.set(index, "");
      started = true;
      ended= false;
      message.clear();
      end="";
    } else if (input == '>')
    {
      ended = true;
      //break;
    } else if (started && message.size()<35) {
      message.add(str(input));
      index++;
    } else if (message.size()>35) {
      message.clear();
      started = false;
      ended=false;
      end = "";
      index = 0;
    }
  }
}
void getInput() {
  py = -map(stick.getSlider("pitchaxis").getValue(), -1, 1, -80, 80);
  px = map(stick.getSlider("yawaxis").getValue(), -1, 1, -80, 80);
  pz = map(stick.getSlider("rollaxis").getValue(), -1, 1, -50, 50);
  throttlea = map(stick.getSlider("throttleaxis").getValue(), -1, 1, 0, 100);
}

void gatillof() {
  COM6.write("<111>");
}
