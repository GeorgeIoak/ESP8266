/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define RED_PIN   14 // D5
#define GREEN_PIN 12  // D6
#define BLUE_PIN  13  // D7

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "your_auth";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "your_ssid";
char pass[] = "your_pass";

BLYNK_WRITE(V0)
{   
  redValue = param[0].asInt();
  greenValue = param[1].asInt();
  blueValue = param[2].asInt();

  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);

  Blynk.virtualWrite(V1, redValue);
  Blynk.virtualWrite(V2, greenValue);
  Blynk.virtualWrite(V3, blueValue);
}

BLYNK_WRITE(V1)
{
  redValue = param.asInt();

  analogWrite(RED_PIN, redValue);
  Blynk.virtualWrite(V0, redValue, greenValue, blueValue);
}

BLYNK_WRITE(V2)
{
  greenValue = param.asInt();

  analogWrite(GREEN_PIN, greenValue);
  Blynk.virtualWrite(V0, redValue, greenValue, blueValue);
}

BLYNK_WRITE(V3)
{
  blueValue = param.asInt();

  analogWrite(BLUE_PIN, blueValue);
  Blynk.virtualWrite(V0, redValue, greenValue, blueValue);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);
}

void loop()
{
  Blynk.run();
}
