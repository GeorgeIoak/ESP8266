#define CAYENNE_DEBUG         // Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space

#include "CayenneDefines.h"
#include "BlynkSimpleEsp8266.h"
#include "CayenneWiFiClient.h"

// Cayenne authentication token. This should be obtained from the Cayenne Dashboard.
char token[] = "fvcaj83cfz";
// Your network name and password.
char ssid[] = "jyoun-soohee";
char password[] = "07040251995";

#define LIGHT D4

void setup()
{
  // initialize digital pin 2 as an output.
  pinMode(LIGHT, OUTPUT);
  Serial.begin(9600);
  Cayenne.begin(token, ssid, password);
}

void loop()
{
  Cayenne.run();
}

// This function will be called every time a Dashboard widget writes a value to Virtual Pin 2.
CAYENNE_IN(V2)
{
  CAYENNE_LOG("Got a value: %s", getValue.asStr());
  int i = getValue.asInt();
  
  if (i == 0)
  {
    digitalWrite(2, LOW);
  }
  else
  {
    digitalWrite(2, HIGH);
  }  
}



