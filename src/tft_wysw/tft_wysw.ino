#include <Adafruit_ILI9340.h>
#include <OnionLibrary.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

/***************************************************
  This is an example sketch for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480
 
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
 
#include "SPI.h"

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _sdcs 7
#define _dc 9
#define _rst 8

const uint8_t maxUrz = 10;
Onion* onionSetup;
String data[maxUrz][4];
String nazwy[maxUrz];
uint8_t ilePol[maxUrz];
uint8_t ileUrz = 0;

uint8_t envInd = 4;
uint8_t wodInd = 4;
uint8_t batInd = 4;
int envNr = 0;
int wodNr = 0;
int batNr = 0;

String datatowrite;
String dane = "";
bool guzik = false;
float timeSinceLastRead = -5000;
float timeSinceLastButton = -100;
int screen = -1;
int targetScreen = 0;
bool startReadBatteryCap = false;
bool bugFulEnv = false;
bool bugFulWod = false;
bool bugFulBat = false;
// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

uint8_t plotDataTemp[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t plotDataHum[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t plotDataV[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t plotDataI[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t plotDataPow[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t plotDataWod[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void setup() {
  pinMode(5, INPUT);
  pinMode(4, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(2);
  for (int i = 0; i < maxUrz; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      data[i][j]="";
      ilePol[i]=0;
      nazwy[i]="";
    }
  }
}

void loop(void) {
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
  scanButtons();
  if(targetScreen == 0 && screen != 0)
  {
    tft.fillScreen(ILI9340_BLACK);
    screenMain();
    screen = 0;
  }
  if(targetScreen == 1 && screen != 1)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(1,0);
    screen = 1;
  }
  if(targetScreen == 2 && screen != 2)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(1,1);
    screen = 2;
  }
  if(targetScreen == 3 && screen != 3)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(1,2);
    screen = 3;
  }
  if(targetScreen == 4 && screen != 4)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(0,1);
    screen = 4;
  }
  if(targetScreen == 5 && screen != 5)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(0,2);
    screen = 5;
  }
  if(targetScreen == 6 && screen != 6)
  {
    tft.fillScreen(ILI9340_BLACK);
    wykresScreen(0,3);
    screen = 6;
  }
  if(targetScreen == 7 && screen != 7)
  {
    tft.fillScreen(ILI9340_BLACK);
    commandScreen();
    screen = 7;
  }
}
void scanButtons()
{
  if (digitalRead(5) == LOW && !guzik)
  {
    delay(5);
    if (digitalRead(5) == LOW)
    {
      targetScreen = (screen == 0) ? 7 : 0;
      guzik=true;
    }
  }
  if (digitalRead(4) == LOW && !guzik)
  {
    delay(5);
    if (digitalRead(4) == LOW)
    {
      guzik = true;
      targetScreen = (screen == 0) ? 1 : ( (screen == 1) ? 1 : ( (screen != 7) ? 1 : 7 ) );
      if (screen == 7)
      {
        startReadBatteryCap = true;
        tft.fillScreen(ILI9340_BLACK);
        commandScreen();
      }
    }
  }
  if (digitalRead(2) == LOW && !guzik)
  {
    delay(5);
    if (digitalRead(2) == LOW)
    {
      guzik=true;
      targetScreen = (screen == 0) ? 2 : ( (screen == 2) ? 2 : ( (screen != 7) ? 2 : 7 ) );
    }
  }
  if (digitalRead(3) == LOW && !guzik)
  {
    delay(5);
    if (digitalRead(3) == LOW)
    {
      targetScreen = (screen == 0) ? 3 : ( (screen == 3) ? 3 : ( (screen != 7) ? 3 : 7 ) );
      guzik=true;
    }
  }
  if ( digitalRead(5) == HIGH && digitalRead(4) == HIGH && digitalRead(2) == HIGH && digitalRead(3) == HIGH )
  {
    guzik = false;
  }
}
void drawMainBlock(int id)
{
  int           w = tft.width(),
                h = tft.height();
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(2);
  switch(id)
  {
    case 0:
      tft.fillRect(0, 0, w, h*3/10, ILI9340_BLACK);
      tft.setCursor(0, 0);
      tft.println("Srodowisko");
      tft.setCursor(0, 20);
      tft.println("Wilg.: " + data[envInd][0]);
      tft.setCursor(0, 40);
      tft.println("Temp.: " + data[envInd][1]);
//      tft.setCursor(0, 60);
//      tft.println("IR: " + data[envInd][2]);
      break;
     case 1:
      tft.fillRect(0, h*3/10, w, h*3/10, ILI9340_BLACK);
      tft.setCursor(0, h*3/10);
      tft.println("Zbiornik wody");
      tft.setCursor(0, h*3/10+20);
      tft.println("Stan: " + data[wodInd][0]);
      break;
    case 2:
      tft.fillRect(0, h*6/10, w, h*4/10, ILI9340_BLACK);
      tft.setCursor(0, h*6/10);
      tft.println("Bateria");
      tft.setCursor(0, h*6/10+20);
      tft.println("Napiecie: " + data[batInd][0]);
      tft.setCursor(0, h*6/10+40);
      tft.println("Prad: " + data[batInd][1]);
      tft.setCursor(0, h*6/10+60);
      tft.println("Pojemnosc: " + data[batInd][2]);
      tft.setCursor(0, h*6/10+80);
      tft.println("Stan: " + data[batInd][3]);
      break;
  }
}
void screenMain()
{
  int           w = tft.width(),
                h = tft.height();
  tft.drawLine(0, h*3/10, w, h*3/10, ILI9340_WHITE);
  tft.drawLine(0, h*6/10, w, h*6/10, ILI9340_WHITE);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(2);
  tft.println("Srodowisko");
  tft.setCursor(0, 20);
  tft.println("Wilg.: " + data[envInd][0]);
  tft.setCursor(0, 40);
  tft.println("Temp.: " + data[envInd][1]);
//  tft.setCursor(0, 60);
//  tft.println("IR: " + data[envInd][2]);
  
  tft.setCursor(0, h*3/10);
  tft.println("Zbiornik wody");
  tft.setCursor(0, h*3/10+20);
  tft.println("Stan: " + data[wodInd][0]);
  
  tft.setCursor(0, h*6/10);
  tft.println("Bateria");
  tft.setCursor(0, h*6/10+20);
  tft.println("Napiecie: " + data[batInd][0]);
  tft.setCursor(0, h*6/10+40);
  tft.println("Prad: " + data[batInd][1]);
  tft.setCursor(0, h*6/10+60);
  tft.println("Pojemnosc: " + data[batInd][2]);
  tft.setCursor(0, h*6/10+80);
  tft.println("Stan: " + data[batInd][3]);
}
void commandScreen()
{
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("Rozpocznij pomiar");
  tft.setCursor(20, 40);
  tft.println("pojemnosci bat.");
  tft.drawRect(10,10,220,50,ILI9340_WHITE);
  if(startReadBatteryCap)
  {
    tft.setCursor(20, 120);
    tft.println("Rozpoczynam pomiar");
  }
}
void redrawWykres(int hr, int id)
{
  switch(hr)
  {
    case 1:
      switch(id)
      {
        case 0:
          drawWykres(100, 0, plotDataHum, ILI9340_RED, 0, (bugFulEnv ? envNr/24 : 1));
          drawWykres(100, -20, plotDataTemp, ILI9340_GREEN, 1, (bugFulEnv ? envNr/24 : 1));
          break;
        case 1:
          drawWykres(100, 0, plotDataWod, ILI9340_RED, 0, (bugFulWod ? wodNr/24 : 1));
          break;
        case 2:
          drawWykres(30, 0, plotDataV, ILI9340_RED, 0, (bugFulBat ? batNr/24 : 1));
          drawWykres(50, -50, plotDataI, ILI9340_GREEN, 1, (bugFulBat ? batNr/24 : 1));
          drawWykres(150, -150, plotDataPow, ILI9340_YELLOW, 2, (bugFulBat ? batNr/24 : 1));
          break;
      }
      break;
    case 0:

      break;
  }
}
void wykresScreen(int hr, int id)
{
  tft.drawLine(20, 80, 20, 300, ILI9340_WHITE);
  tft.drawLine(20, 300, tft.width(), 300, ILI9340_WHITE);
  redrawWykres(hr,id);
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
  tft.setCursor(220, 305);
  tft.println((hr==1) ? "1h" : "24h");
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println((hr==1) ? "Wykres 1h" : "Wykres 24h");
  switch(id)
  {
    case 0:
      tft.setCursor(0, 20);
      tft.setTextColor(ILI9340_RED);
      tft.println("Wilgotnosc");
      tft.setCursor(0, 40);
      tft.setTextColor(ILI9340_GREEN);
      tft.println("Temperatura");
      break;
    case 1:
      tft.setCursor(0, 20);
      tft.setTextColor(ILI9340_RED);
      tft.println("Stan zbiornika");
      break;
    case 2:
      tft.setCursor(0, 20);
      tft.setTextColor(ILI9340_RED);
      tft.println("Napiecie");
      tft.setCursor(0, 40);
      tft.setTextColor(ILI9340_GREEN);
      tft.println("Prad");
      tft.setCursor(0, 60);
      tft.setTextColor(ILI9340_YELLOW);
      tft.println("Moc");
      break;
  }
  
}
uint8_t toPix(int maxim, int minim, float dana)
{
  float x = (dana-minim)/(maxim-minim);
  int dx = 200 * x;
  uint8_t d = dx;
  tft.setCursor(100, 20);
//  tft.fillRect(100,20,100,20,ILI9340_BLACK);
//  tft.setTextColor(ILI9340_YELLOW);  tft.setTextSize(2);
//  tft.println(String((dana-minim)));
  return d;
}
void drawWykres(int maxim, int minim, uint8_t* dane, uint16_t color, int id, int start)
{
  int i = 0;
  int d1, d2;
  float x1, x2;
  int pkt;
  tft.setCursor(0, 100+10*id);
  tft.setTextColor(color);  tft.setTextSize(1);
  tft.println(String(maxim));
  tft.setCursor(0, 300-10*id);
  tft.println(String(minim));
  for(i=start; i != (start+29)%30; i = (i+1)%30)
  {
    d1 = *(dane+i-1);
    d1 = -d1 + 300;
    d2 = *(dane+i);
    d2 = -d2 + 300;
    x1 = 20 + 200 * ((i + 30-start)%30)/30;
    x2 = 20 + 200 * ((i+1 + 30-start)%30)/30;
    tft.drawLine((int)x1, (int)d1, (int)x2, (int)d2, color);
  }
}
void readData(String dane)
{
  int i = 0;
  i = getValue(dane, '|', 0).toInt();
  if (i == 999)
  {
    if (ileUrz >= maxUrz)
      return;
    nazwy[ileUrz] = getValue(dane, '|', 1);
    if(nazwy[ileUrz][0] == 'M')
      envInd = ileUrz;
    if(nazwy[ileUrz][0] == 'Z')
      wodInd = ileUrz;
    if(nazwy[ileUrz][0] == 'B')
      batInd = ileUrz;
    ilePol[ileUrz] = getValue(dane, '|', 2).toInt();
    ileUrz++;
    return;
  }
  if (ileUrz >= maxUrz)
      return;
  for (int j = 0; j < ilePol[i]; j++)
  {
    data[i][j] = getValue(dane, '|', j+1);
    if (i == envInd)
    {
      if ((envNr)%24 == 0)
      {
        if (j == 0)
          plotDataHum[(envNr)/24] = toPix(100, 0, data[i][j].substring(0,6).toFloat());
        else if (j == 1)
          plotDataTemp[(envNr)/24] = toPix(100, -20, data[i][j].substring(0,6).toFloat());
      }
    }
    else if (i == wodInd)
    {
      if ((wodNr)%24 == 0)
      {
        plotDataWod[(wodNr)/24] = toPix(100, 0, data[i][j].substring(0,6).toFloat());
      }
    }
    else if (i == batInd)
    {
      if ((batNr)%24 == 0)
      {
        if (j == 0)
          plotDataV[(batNr)/24] = toPix(30, 0, data[i][j].substring(0,6).toFloat());
        else if (j == 1)
          plotDataI[(batNr)/24] = toPix(50, -50, data[i][j].substring(0,6).toFloat());
        else if (j == 2)
          plotDataPow[(batNr)/24] = toPix(150, -150, data[i][j].substring(0,6).toFloat());
      }
    }
  }
  if (i == envInd)
  {
    envNr = (envNr+1)%720;
    bugFulEnv = (envNr==0) ? true : bugFulEnv;
  }
  else if (i == wodInd)
  {
    wodNr = (wodNr+1)%720;
    bugFulWod = (wodNr==0) ? true : bugFulWod;
  }
  else if (i == batInd)
  {
    batNr = (batNr+1)%720;
    bugFulBat = (batNr==0) ? true : bugFulBat;
  }
  if(targetScreen == 0)
    drawMainBlock( (i==batInd ? 2 :(i==wodInd ? 1 : 0) ) );
  else if(shouldRefreshWykres(i))
  {
    tft.fillRect(21,100,199,199,ILI9340_BLACK);
    redrawWykres( (targetScreen > 3) ? 0 : 1, (targetScreen-1)%3);
  }
  String command = "000|000.000|000.000|000.000!";
  if(startReadBatteryCap && (i == batInd))
  {
    command = "821|000.000|000.000|000.000!";
    startReadBatteryCap = false;
    if(screen == 7)
    {
      tft.fillScreen(ILI9340_BLACK);
      commandScreen();
    }
  }
  else
  {
    command = "000|000.000|000.000|000.000!";
  }
  Serial.write(command.c_str());
}
bool shouldRefreshWykres(int i)
{
  int x = (i==batInd ? 2 :(i==wodInd ? 1 : 0) );
  bool first = (x == targetScreen-1);
  int y = (i==batInd ? batNr :(i==wodInd ? wodNr : envNr) );
  bool second = ((y)%24 == 0);
  return (first && second);
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

