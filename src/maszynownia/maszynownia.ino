#include <OnionLibrary.h>
#include <string.h>
#include "DHT.h"

#define DHTPIN 2
DHT dht(DHTPIN, DHT22);
String temp;
String hum;

Onion* onionSetup;
String datatowrite;
float timeSinceLastRead = -5000;

// the setup routine runs once when you press reset:
void setup() {
  onionSetup = new Onion;
  Serial.begin(9600);
}

void loop() 
{
  if (millis() - timeSinceLastRead >= 5000)
  {
    timeSinceLastRead = millis();
    switch (dht.read())
    {
      //struktura danych przesylanych (dlugosc 3 wielkosci tymczasowa) ddd.ddd|ddd.ddd|ddd.ddd. Jesli jest za malo wartosci, to dodac 000.000 na koncu.
      case DHT_OK:
        hum = String(dht.humidity, 3);
        while (hum.length() < 7) hum = "0" + hum;
        temp = String(dht.temperature, 3);
        while (temp.length() < 7) temp = "0" + temp;
        datatowrite = hum + "|" + temp + "|000.000";
        Serial.write(datatowrite.c_str());
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
