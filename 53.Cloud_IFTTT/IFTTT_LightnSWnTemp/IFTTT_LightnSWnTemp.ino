#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include "DHT.h"

#define USERNAME "jyounnim"
#define DEVICE_ID "esp8266"
#define DEVICE_CREDENTIAL "RSOo@ggBHOu!"

#define SSID "jyoun-soohee"
#define SSID_PASSWORD "07040251995"

// Pin config
#define DHTPIN  D1
#define LIGHT   D2
#define SW1     D3

// dht config
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE, 25);

ThingerWifi thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

char preKeyValue=0, keyValue=0;
unsigned long previousMillis = 0;      // to know when to post the temperature
unsigned int minutes_interval = 1;    // variable in minutes for posting the temperature

void call_key_endpoint(char key){
  digitalWrite(BUILTIN_LED, LOW);
  pson tweet_content;
  tweet_content["value1"] = dht.readTemperature();
  tweet_content["value2"] = dht.readHumidity();
  if( key == 0 )
    tweet_content["value3"] = "Release";
  else
    tweet_content["value3"] = "Press";
  thing.call_endpoint("temperature_tweet", tweet_content);
  digitalWrite(BUILTIN_LED, HIGH);

#if (0)
  Serial.print("Temperature: ");
  Serial.println(dht.readTemperature());
  Serial.print("Humidity: ");
  Serial.println(dht.readHumidity());
  Serial.print("Key: ");
  Serial.println(key);  
#endif  
}

void call_temperature_endpoint(){
  digitalWrite(BUILTIN_LED, LOW);
  pson tweet_content;
  tweet_content["value1"] = dht.readTemperature();
  tweet_content["value2"] = dht.readHumidity();
  if( preKeyValue == 0 )
    tweet_content["value3"] = "Release";
  else
    tweet_content["value3"] = "Press";
  thing.call_endpoint("temperature_tweet", tweet_content);
  digitalWrite(BUILTIN_LED, HIGH);

#if (0)
  Serial.print("Temperature: ");
  Serial.println(dht.readTemperature());
  Serial.print("Humidity: ");
  Serial.println(dht.readHumidity());
  Serial.print("Key: ");
  Serial.write(preKeyValue);  
  Serial.println();
#endif  
}

void setup() {
  dht.begin();
  pinMode(BUILTIN_LED, OUTPUT);

  // initialize digital pin 2 as an output.
  pinMode(LIGHT, OUTPUT);
  // initialize digital pin 3 as an input.
  pinMode(SW1, INPUT);
  
//  Serial.begin(115200);

  // turn off led
  digitalWrite(BUILTIN_LED, HIGH);
  
  thing.add_wifi(SSID, SSID_PASSWORD);

  // resource for reading sensor temperature from API
  thing["dht11"] >> [](pson& out){
    out["humidity"] = dht.readHumidity();
    out["celsius"] = dht.readTemperature();
    out["fahrenheit"] = dht.readTemperature(true);
  };

  // resource that will allow changing the tweet interval
  thing["tweet_interval"] << [](pson& in){
    if(in.is_empty()) in = minutes_interval;
    else minutes_interval = (unsigned int) in;
  };

  // this resource will allow sending the tweet as requested
  thing["tweet_test"] = [](){
    call_temperature_endpoint();
  };
}

void loop() {
  thing.handle();

  unsigned long currentMillis = millis();

  checkKey();
  if(minutes_interval>0 && currentMillis - previousMillis >= minutes_interval*60000) 
  {
    previousMillis = currentMillis;   
    call_temperature_endpoint();
  }
}

void checkKey()
{
    keyValue = digitalRead(SW1);
    if( preKeyValue != keyValue )
    {
      call_key_endpoint(keyValue); 
      preKeyValue = keyValue;
    }
}
