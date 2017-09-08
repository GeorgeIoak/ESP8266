#if(1)
/*
 Sharp Optical Dust Sensor GP2Y1010AU0F
*/
  
int measurePin = A0; // Connect dust sensor
int ledPower = 2;    // Led driver pins of dust sensor (GPIO_2:D4)
  
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
  
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
  
void setup(){
  Serial.begin(115200);
  pinMode(ledPower,OUTPUT);
}
  
void loop(){
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  
  voMeasured = analogRead(measurePin); // read the dust value
  
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (3.2 / 1024.0);
  //calcVoltage = voMeasured * (5.0 / 1024.0);
  
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  if( calcVoltage <= 3.0 )
    dustDensity = 0.17 * calcVoltage - 0.1;
  else
    dustDensity = 0.5;

  if( dustDensity < 0)
    dustDensity = 0;
  
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
  
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
  
  Serial.print(" - Dust Density: ");
  Serial.print(dustDensity); // unit: mg/m3
  Serial.println(" mg/m^3");
  
  delay(1000);
}

#else

#include <SharpDust.h>

#define DUST_LED_PIN		2
#define DUST_MEASURE_PIN	A0

void setup() {
  Serial.begin(115200);

  SharpDust.begin(DUST_LED_PIN, DUST_MEASURE_PIN);
}

void loop() {
  while (1) {
    Serial.print(F("Dust :"));
    Serial.print(SharpDust.measure());
    Serial.println(F(" mg/m^3"));
    delay(1000);
  }
}
#endif
