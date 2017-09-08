#define CAYENNE_DEBUG         // Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space

#include <SPI.h>
#include "CayenneDefines.h"
#include "BlynkSimpleEsp8266.h"
#include "CayenneWiFiClient.h"
#include "DHT.h"

// Cayenne authentication token. This should be obtained from the Cayenne Dashboard.
char token[] = "fvcaj83cfz";

// Your network name and password.
char ssid[] = "jyoun-soohee";
char password[] = "07040251995";

// PIn config
#define DHTPIN  D1
#define LIGHT   D2
#define SW1     D3

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE, 25);

char preKeyValue=0, keyValue=0;
unsigned long previousMillis = 0;      // to know when to post the temperature
unsigned int minutes_interval = 1;    // variable in minutes for posting the temperature

void setup()
{
  // initialize digital pin 2 as an output.
  pinMode(LIGHT, OUTPUT);
  // initialize digital pin 3 as an input.
  pinMode(SW1, INPUT);
  
  Serial.begin(115200);
  Cayenne.begin(token, ssid, password);
}

void loop()
{
  unsigned long currentMillis = millis();

  Cayenne.run();
  checkKey();
  
  if(minutes_interval>0 && currentMillis - previousMillis >= minutes_interval*60000) 
  {      
    if( checkTemperature() == 0 )
      previousMillis = currentMillis; 
  }  
}

// This function will be called every time a Dashboard widget writes a value to Virtual Pin 2.
CAYENNE_IN(V2)
{
  CAYENNE_LOG("Got a value: %s", getValue.asStr());
  int i = getValue.asInt();
  
  if (i == 0)
  {
    digitalWrite(LIGHT, LOW);
  }
  else
  {
    digitalWrite(LIGHT, HIGH);
  }  
}

void checkKey()
{
    keyValue = digitalRead(SW1);
    if( preKeyValue != keyValue )
    {
      Cayenne.virtualWrite(V3, keyValue); 
      preKeyValue = keyValue;
    }
}

int checkTemperature()
{
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read humidity
  float h = dht.readHumidity();

  if( (t==0) || (h==0) )
    return -1;
    
  Cayenne.virtualWrite(V4, t);
  Cayenne.virtualWrite(V5, h);
  Cayenne.virtualWrite(V6, t);
  Cayenne.virtualWrite(V7, h);

#if (0)
  Serial.print("Temperature: ");
  Serial.println(t));
  Serial.print("Humidity: ");
  Serial.println(h);
#endif  

  return 0;
}
