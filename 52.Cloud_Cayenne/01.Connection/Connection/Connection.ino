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

void setup()
{
  Serial.begin(9600);
  Cayenne.begin(token, ssid, password);
}

void loop()
{
  Cayenne.run();
}



