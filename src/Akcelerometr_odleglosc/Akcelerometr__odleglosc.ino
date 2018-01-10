#include <Wire.h>
#include <ADXL345.h>

int EchoPin = 2;           // connect Pin 2(Arduino digital io) to Echo at US-020
int TrigPin = 3;           // connect Pin 3(Arduino digital io) to Trig at US-020
long Time_Echo_us = 0;
long D  = 0;             // odczyt z czujnika ultradz
float x,y,z;
float d;                          // WYSOKOŚĆ SŁUPKA CIECZY
int lz = 260;                           // wysokość pojemnika
int lx = 150;                           //długość pojemnika mm
int ly = 150;                           //szerokość pojemnika mm
int lxa = 45;                          //umiejscowienie korka po x od lewej mm
int lxb = lx-lxa;                          //umiejscowienie korka po x od prawej mm
int lya = 100;                          //umiejsoweienie korka po y od frontu mm
int lyb = ly-lya;                          //umiejsowienie korka po y od tyłu mm
float V;                          //objętość pojemnika 
float a;                          //a' wymagane w obliczeniach
float b;                          //b' odwracanie macierzy
float c;                          //c' odwracanie macierzy
float m;
float n;
float l;
String dane;
String datatowrite;
String Ad, Bd, Cd;
String obj;
float timeSND = -5000;

int accX, accY, accZ;
double ax, ay, az, fulla;
double ox = 0, oz = 0;

ADXL345 adxl;

void setup() 
{
  Serial.begin(9600);
  adxl.powerOn();
  pinMode(EchoPin, INPUT);                    //Set EchoPin as input, to receive measure result from US-020
  pinMode(TrigPin, OUTPUT);                   //Set TrigPin as output, used to send high pusle to trig measurement (>10us)
}


void loop()
{
  if (Serial.available() > 0)
  {
    char c = char(Serial.read());
    if (c == '!')
    {
      readData(dane);
      dane = "";
    }
    else
    {
      dane += c;
    }
  }
  if(millis() - timeSND >= 5000)
  {
    timeSND = millis();
    adxl.readAccel(&accX, &accY, &accZ);
    ax = float(accX)/255;
    ay = float(accY)/255;
    az = float(accZ)/255;
    fulla = sqrt((ax*ay) + (ay*ay) + (az*az));
    if(fulla > 0.9 && fulla < 1.1)
    {
      x = ax/fulla;
      y = ay/fulla;
      z = az/fulla;
    }
    else
    {
      x = 0;
      y = 0;
      z = 1;
    }
    digitalWrite(TrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(TrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(TrigPin, LOW);
    
    Time_Echo_us = pulseIn(EchoPin, HIGH);               //calculate the pulse width at EchoPin, 
    if((Time_Echo_us < 60000) && (Time_Echo_us > 1))     //a valid pulse width should be between (1, 60000).
    {
      D = (Time_Echo_us * 34 / 100 / 2);      //calculate the distance by pulse width, D = (Time_Echo_us * 0.34mm/us) / 2 (mm)
      d= lz-D;
      a= (x*x-y*z)/(x*x*x-3*y*z*x+y*y*y+z*z*z);
      b= (y*y-x*z)/(x*x*x-3*y*z*x+y*y*y+z*z*z);
      c= (z*z-y*x)/(x*x*x-3*y*z*x+y*y*y+z*z*z);
      //objętość zbiorniks
      m = a/c;
      n= b/c;
      l= d/c;
      V= (m/2)*((lxb*lxb)-(lxa*lxa))*ly+(n/2)*((lyb*lyb)-(lya*lya))*lx+l*lx*ly;
      V = float(round(V/lx/ly/lz * 5))/5*100;
      if (V>100)
        V=100;
      if (V<0)
        V=0;
    }
    obj = String(V, 3);
    while (obj.length() < 7) obj = "0" + obj;
    Ad = String(x, 3);
    while (Ad.length() < 7) Ad = "0" + Ad;
    Bd = String(y, 3);
    while (Bd.length() < 7) Bd = "0" + Bd;
    Cd = String(z, 3);
    while (Cd.length() < 7) Cd = "0" + Cd;
//    datatowrite = obj+"|"+Ad+"|"+Bd+"|"+Cd;
    datatowrite = obj + "|xxx.xxx|xxx.xxx|xxx.xxx";
    Serial.write(datatowrite.c_str());
  } 
}

void readData(String dane)
{
  int i = 0;
  i = getValue(dane, '|', 0).toInt();
  if (i == 000)
  {
    return;
  }
  if (i == 721)
  {
    i = getValue(dane, '|', 1).toInt();
    lx = i;
    i = getValue(dane, '|', 2).toInt();
    ly = i;
    i = getValue(dane, '|', 3).toInt();
    lz = i;
    return;
  }
  if (i == 722)
  {
    i = getValue(dane, '|', 1).toInt();
    lxa = i;
    lxb = lx - lxa;
    i = getValue(dane, '|', 2).toInt();
    lya = i;
    lyb = ly -lya;
    return;
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//int getValue(int axis)
//{
//  
//  int Val = 0;
//  int h,l; 
//  if (axis == 1)
//  {
//    l = readRegister(OUT_X_L);
//    h = readRegister(OUT_X_H);
//  }
//  else if (axis == 2)
//  {
//    l = readRegister(OUT_Y_L);
//    h = readRegister(OUT_Y_H);
//  }
//  else if (axis == 3)
//  {
//    l = readRegister(OUT_Z_L);
//    h = readRegister(OUT_Z_H);
//  }
//  Val = ((h << 8) | l);
//  
//  return Val;
//}
//
//byte readRegister(byte thisRegister)
//{
//  byte result = 0;
//  byte dataToSend = thisRegister | READ;
//  digitalWrite(ss,LOW);
//  SPI.transfer(dataToSend);
//  result = SPI.transfer(0x00);
//  digitalWrite(ss, HIGH);
//  return result;
//}
//
//void writeRegister(byte thisRegister, byte thisValue)
//{
//byte dataToSend = thisRegister & WRITE;
//digitalWrite(ss,LOW);
//SPI.transfer(dataToSend);
//SPI.transfer(thisValue);
//digitalWrite(ss,HIGH);
//}
