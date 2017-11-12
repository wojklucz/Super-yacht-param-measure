#include <OnionLibrary.h>
#include <string.h>
#include <LiquidCrystal.h>
#include "DHT.h"
/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
*/
int lcd_key     = 0;
int adc_key_in  = 0;
#define DHTPIN 2     // what pin we're connected to

DHT dht(DHTPIN, DHT22);
String temp;
String hum;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define MAX_URZ 20
int maxUrz = MAX_URZ;
Onion* onionSetup;
String data[MAX_URZ][3];
int ileUrz = 1;
int index1 = 0;
int index2 = 0;
String datatowrite;
String dane = "";
bool guzik = false;
float timeSinceLastRead = -5000;
float timeSinceLastButton = -100;

String nazwy[MAX_URZ];
int ilePol[MAX_URZ];
// the setup routine runs once when you press reset:
void setup() {
  lcd.begin(16, 2);               // start the library
  
  onionSetup = new Onion;
  Serial.begin(9600);
  //lcd.print(nazwy[index1]);
  for (int i = 0; i < maxUrz; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      data[i][j]="";
      ilePol[i]=0;
      nazwy[i]="";
    }
  }
  nazwy[0] = "Sterownia";
  ilePol[0] = 2;
  printLCD();
}

void loop() {
  if (analogRead(0) > 1000)
    guzik = false;
  if (millis() - timeSinceLastButton >= 0 && !guzik)
  {
    timeSinceLastButton = millis();
    lcd_key = read_LCD_buttons();   // read the buttons
    switch (lcd_key)
    {
      case btnRIGHT: {
          index2 = (index2 + 1) % ilePol[index1];
          printLCD();
          break;
        }
      case btnLEFT: {
          index2 = (index2 + ilePol[index1]-1) % ilePol[index1];
          printLCD();
          break;
        }
      case btnUP: {
          index1 = (index1 + 1) % ileUrz;
          index2 = 0;
          printLCD();
          break;
        }
      case btnDOWN: {
          index1 = (index1 + ileUrz-1) % ileUrz;
          index2 = 0;
          printLCD();
          break;
        }
    }
  }
  //--------------------------------WAŻNE---------------------------------
  if (Serial.available() > 0) //odczytaj dane z portu szeregowego.
  { // W zasadzie możecie skopiować do swoich programów, tylko usuńcie printLCD().
    char c = char(Serial.read());
    if (c == '!')
    {
      readData(dane);
      printLCD();
      dane = "";
    }
    else
    {
      dane += c;
    }
  }
  //----------------------------------------------------------------------
  if (millis() - timeSinceLastRead >= 5000)
  {
    timeSinceLastRead = millis();
    switch (dht.read())
    {
      case DHT_OK:
        hum = String(dht.humidity, 3);
        //while (hum.length() < 7) hum = "0" + hum;
        temp = String(dht.temperature, 3);
        //while (temp.length() < 7) temp = "0" + temp;
        data[0][0] = hum + "%";
        data[0][1] = temp + "degC";
        if (index1 == 0)
          printLCD();
        break;
      case DHT_ERR_CHECK:
        break;
      case DHT_ERR_TIMEOUT:
        break;
      default:
        break;
    }
  }
}
//--------------------------------WAŻNE---------------------------------
void readData(String dane) //pierwszy znak to kod rozkazu -  wybierz sobie jakiś niezajęty numer i napisz kod obsługi rozkazu
{ // napisz w doc/kody rozkazów kod i opis funkcji
  int i = 0;
  i = getValue(dane, '|', 0).toInt();
  if (i == 999)
  {
    if (ileUrz >= maxUrz)
      return;
    nazwy[ileUrz] = getValue(dane, '|', 1);
    ilePol[ileUrz] = getValue(dane, '|', 2).toInt();
    ileUrz++;
    return;
  }
  for (int j = 0; j < ilePol[i]; j++)
  {
    data[i][j] = getValue(dane, '|', j+1);
  }
}
//----------------------------------------------------------------------

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

int read_LCD_buttons() {
  adc_key_in = analogRead(0);
  guzik = true;
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  guzik = false;
  return btnNONE;
}

void printLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(nazwy[index1]);
  lcd.setCursor(0, 1);
  lcd.print(data[index1][index2]);
}


