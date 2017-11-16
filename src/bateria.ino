#define NUM_SAMPLES 10
#define MAX_VOLTAGE 5.0
#define VOLTAGE_SCALE 5.56
#define MEASURE_INTERVAL 5000
#define A2_NEUTRUM 2.5
#define VOLT_PER_AMPER 0.028 
#define VOLT_MEAS_TOLERANCE 0.08

int sumA0 = 0;
int sumA2 = 0;
unsigned char sampleCount = 0;
float voltageA0 = 0.0;   
float amperageA2 = 0.0;         
float lastMeasureTime = 0;
float deltaEnergy = 0.0;
float batteryMax; // in Ah
float batteryStatus; // in Ah

void setup() {
  // put your setup code here, to run once:
  // start serial for output at baud rate 9600
  Serial.begin(9600);

  batteryMax = 0.5 * 9;
  batteryStatus = 0.5 * 9;
}

void loop() {

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
    voltageA0 = voltageA0 * VOLTAGE_SCALE;
    amperageA2 -= A2_NEUTRUM;
    if(abs(amperageA2) < (VOLT_MEAS_TOLERANCE/10))
      amperageA2 = 0.0;
    amperageA2 /= VOLT_PER_AMPER;


    deltaEnergy = voltageA0 * amperageA2 * MEASURE_INTERVAL / 3600.0;

    batteryStatus -= deltaEnergy/1000.0;

    Serial.print("Measured voltage: ");
    Serial.print(voltageA0);
    Serial.print(" V, current: ");
    Serial.print(amperageA2);
    Serial.print(" A, energy lost: ");
    Serial.print(deltaEnergy);
    Serial.print(" mWh, battery status: ");
    Serial.print(batteryStatus / batteryMax *100);
    Serial.println (" %.");
  }

  
}
