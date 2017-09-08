#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <Wire.h>

// SHT15
#include <SHT1x.h>

// Pressure
#include <Adafruit_BMP085.h>

// HMC5883 Magnetometer
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

// Thinger config
#define USERNAME "jyounnim@gmail.com"
#define DEVICE_ID "esp8266"
#define DEVICE_CREDENTIAL "humax1234"

// Wifi config
#define SSID "LeTV_Butler"
#define SSID_PASSWORD "MeebleLabs"

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  D1
#define clockPin D2
SHT1x sht1x(dataPin, clockPin);

// Pressure Sensor
Adafruit_BMP085 bmp;

// Analog input for UV Index
int ReadUVintensityPin = A0; //Output from the sensor

// Magnetometer
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// Thinger instance
ThingerWifi thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  // initialize I2C in NodeMCU in D4 and D5
  Wire.begin(D4, D5);
  
  bmp.begin();  // initialize bmp sensor
  mag.begin();  // initialize magnemotemter
  pinMode(ReadUVintensityPin, INPUT); // pin mode for uv index
  pinMode(BUILTIN_LED, OUTPUT); // pin output for board led

  // initialize here thinger wifi and resources
  thing.add_wifi(SSID, SSID_PASSWORD);

  // led resource
  thing["led"] << [](pson& in){ digitalWrite(BUILTIN_LED, in ? LOW : HIGH); };

  thing["temperature"] >> [](pson& out){
    out = sht1x.readTemperatureC();
  };

  thing["humidity"] >> [](pson& out){
    out = sht1x.readHumidity();
  };

  thing["temp_bmp"] >> [](pson& out){
    out = bmp.readTemperature();
  };
  
  thing["pressure"] >> [](pson& out){
    out = bmp.readPressure();
  };

  thing["altitude"] >> [](pson& out){
    out = bmp.readAltitude(101500);
  };

  thing["uv_index"] >> [](pson& out){
    int uvLevel = averageAnalogRead(ReadUVintensityPin, 8); 
    float outputVoltage = 3.3 * uvLevel/1024;
    out = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
  };

  thing["heading"] >> [](pson& out){
    sensors_event_t event; 
    mag.getEvent(&event);
    float heading = atan2(event.magnetic.y, event.magnetic.x);
    // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
    // Find yours here: http://www.magnetic-declination.com/
    // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
    float declinationAngle = 0.0168715; // declination angle in radians (for madrid)
    heading += declinationAngle;
    
    // Correct for when signs are reversed.
    if(heading < 0) heading += 2*PI;
      
    // Check for wrap due to addition of declination.
    if(heading > 2*PI) heading -= 2*PI;
     
    // Convert radians to degrees for readability.
    out = heading * 180/M_PI;
  };
}

// our loop will only call the thing handle
void loop() {
  thing.handle();
}

// average analog reading for more stable input
int averageAnalogRead(int pinToRead, byte numberOfReadings)
{
  unsigned int runningValue = 0; 
  for(int x = 0 ; x < numberOfReadings ; x++){
    runningValue += analogRead(pinToRead);
  }
  return runningValue / numberOfReadings;  
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
