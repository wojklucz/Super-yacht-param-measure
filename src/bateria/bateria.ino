#include <time.h>

#define NUM_SAMPLES 5
#define MAX_VOLTAGE 5.0
#define VOLTAGE_SCALE 5.56
#define MEASURE_INTERVAL 5000
#define A2_NEUTRUM 2.5
#define VOLT_PER_AMPER 0.028 
#define VOLT_MEAS_TOLERANCE 0.1
#define GATE_PIN 8
#define MILIS_IN_MINUTE 60000

int sumA0 = 0;
int sumA2 = 0;
String dane;
unsigned char sampleCount = 0;
float voltageA0 = 0.0;   
float amperageA2 = 0.0;         
float lastMeasureTime = 0;
float lastMeasureVoltageTime = 0;
float deltaEnergy = 0.0;
float batteryMax; // in Wh
float batteryStatus; // in Wh
bool gateClosed = true;
bool measureCapacity = false;

String volt;
String curr;
String cap;
String stat;
String datatowrite;


void setup() {
  // put your setup code here, to run once:
  // start serial for output at baud rate 9600
  Serial.begin(9600);

  batteryMax = 0.5 * 9;
  pinMode(GATE_PIN,OUTPUT);
  digitalWrite(GATE_PIN, LOW);
}

void loop() {
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
  if(millis() - lastMeasureTime >= MEASURE_INTERVAL)
  {
    lastMeasureTime = millis();

    sampleCount = 0;
    sumA0 = 0;
    sumA2 = 0;
    
    while (sampleCount < NUM_SAMPLES) {
      sumA0 += analogRead(A0);
      sumA2 += analogRead(A2);
      sampleCount++;
      delay(10);
    }
    
    voltageA0 = ((float)sumA0 / (float)NUM_SAMPLES * MAX_VOLTAGE) / 1024.0;
    amperageA2 = ((float)sumA2 / (float)NUM_SAMPLES * MAX_VOLTAGE) / 1024.0;
    if(voltageA0 < VOLT_MEAS_TOLERANCE)
      voltageA0 = 0.0;
    else
      voltageA0 = (voltageA0-VOLT_MEAS_TOLERANCE) * VOLTAGE_SCALE + 0.7;
    amperageA2 -= A2_NEUTRUM;
    if(abs(amperageA2) < (VOLT_MEAS_TOLERANCE/10))
      amperageA2 = 0.0;
    amperageA2 /= VOLT_PER_AMPER;


    deltaEnergy = voltageA0 * amperageA2 * MEASURE_INTERVAL / 3600.0;

    if(!measureCapacity)
    {
      batteryStatus -= deltaEnergy/1000.0; //Wh
      if (batteryStatus < 0)
        batteryStatus = 0;
      
      if(voltageA0 > 1 && gateClosed)
      {
        gateClosed = false;
        digitalWrite(GATE_PIN, HIGH);
      }
      else if(voltageA0 < 1 && !gateClosed)
      {
        gateClosed = true;
        digitalWrite(GATE_PIN, LOW);
      }
    }
    else
    {
      if(gateClosed)
      {
        if(voltageA0 < 7)
        {
          measureCapacity = false;
        }
        else
        {
          digitalWrite(GATE_PIN, HIGH);
          gateClosed = false;
        }
      }
      else
      {
        batteryMax += deltaEnergy/1000.0; //Wh
//        batteryStatus = batteryMax;
        if(millis() - lastMeasureVoltageTime > (MILIS_IN_MINUTE * 10) )
        {
          lastMeasureVoltageTime = millis();
          digitalWrite(GATE_PIN, LOW);
          gateClosed = true;
          delay(1000);
        }
      }
      
    }
    
    sendData();
  }
  
}

void sendData()
{
  volt = String(voltageA0, 3);
  while (volt.length() < 7) volt = "0" + volt;
  
  curr = String(amperageA2, 3);
  while (curr.length() < 7) curr = "0" + curr;

  cap = String(batteryMax, 3);
  while (cap.length() < 7) cap = "0" + cap;

  double statubat = batteryStatus / batteryMax *100;
  stat = String(statubat, 3);
  while (stat.length() < 7) stat = "0" + stat;

  datatowrite = volt + "|" + curr + "|" + cap + "|" + stat;
  Serial.write(datatowrite.c_str());
}
void readData(String dane)
{
  int i = 0;
  i = getValue(dane, '|', 0).toInt();
  if (i == 000)
  {
    return;
  }
  else if(i==721)
  {
    batteryMax = getValue(dane, '|', 1).toFloat();
    batteryStatus = batteryMax;
    
  }
  else if(i==821)
  {
    //tu start procedury pomiaru pojemności
    batteryMax = 0;
    batteryStatus = 0;
    measureCapacity = true;
    gateClosed = false;
    digitalWrite(GATE_PIN, HIGH);
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

