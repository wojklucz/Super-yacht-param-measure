#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

unsigned int EchoPin = 2;           // connect Pin 2(Arduino digital io) to Echo at US-020
unsigned int TrigPin = 3;           // connect Pin 3(Arduino digital io) to Trig at US-020
unsigned long Time_Echo_us = 0;
unsigned long D  = 0;             // odczyt z czujnika ultradz
float d;                          // WYSOKOŚĆ SŁUPKA CIECZY
int lz = 1;                           // wysokość pojemnika
int lx = 1;                           //długość pojemnika mm
int ly = 1;                           //szerokość pojemnika mm
int lxa = 1;                          //umiejscowienie korka po x od lewej mm
int lxb = 0;                          //umiejscowienie korka po x od prawej mm
int lya = 1;                          //umiejsoweienie korka po y od frontu mm
int lyb = 0;                          //umiejsowienie korka po y od tyłu mm
float V;                          //objętość pojemnika 
float a;                          //a' wymagane w obliczeniach
float b;                          //b' odwracanie macierzy
float c;                          //c' odwracanie macierzy
float m;
float n;
float l;
String dane;
String datatowrite;
String obj;
float timeSND = -5000;

LIS3DH myIMU; //Default constructor is I2C, addr 0x19.

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  myIMU.begin();

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
  if(millis() - timeSND >= 1000)
  {
    timeSND = millis();
    float x = myIMU.readFloatAccelX();
    float y = myIMU.readFloatAccelY();
    float z = myIMU.readFloatAccelZ();
    digitalWrite(TrigPin, HIGH);              //begin to send a high pulse, then US-020 begin to measure the distance
    delayMicroseconds(50);                    //set this high pulse width as 50us (>10us)
    digitalWrite(TrigPin, LOW);               //end this high pulse
    
    Time_Echo_us = pulseIn(EchoPin, HIGH);               //calculate the pulse width at EchoPin, 
    if((Time_Echo_us < 60000) && (Time_Echo_us > 1))     //a valid pulse width should be between (1, 60000).
    {
      D = (Time_Echo_us*34/100)/2;      //calculate the distance by pulse width, D = (Time_Echo_us * 0.34mm/us) / 2 (mm)
      d= lz-D;
      a= ((z*z)*(y*y)-(2*z*x*x*y)+(x*x*x*x))/((x*x-z*y)*(z*z*z-3*z*x*y+x*x*x+y*y*y));
      b= ((z*z)*(x*y)-(z*x*x*x)-(z*y*y*y)+(x*x*y*y))/((x*x-z*y)*(z*z*z-3*z*x*y+x*x*x+y*y*y));
      c= ((z*z)-(x*y))/((z*z*z)-(3*z*x*y)+(x*x*x)+(y*y*y));
      //objętość zbiorniks
      m = a/c;
      n= b/c;
      l= d/c;
      V= (a/2)*((lxb*lxb)-(lxa*lxa))*ly+(n/2)*((lyb*lyb)-(lya*lya))*lx+c*lx*ly;
    }
    obj = String(V/(lx*ly*lz), 3);
    while (obj.length() < 7) obj = "0" + obj;
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
