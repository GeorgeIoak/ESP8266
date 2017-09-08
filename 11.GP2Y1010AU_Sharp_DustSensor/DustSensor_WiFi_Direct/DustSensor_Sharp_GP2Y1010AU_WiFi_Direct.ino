/*
 * Reference :
 * https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-ap-web-server
 * 
 * USAGE : http://192.168.4.1
 */
 
#include <ESP8266WiFi.h>
 
const char WiFiAPPSK[] = "12345678";
 
const int ledPin = 2;      // onboard LED of ESP-12F (GPIO_2)
const int measurePin = A0; //Connect dust sensor to Arduino A0 pin
const int ledPower = 12;   //Connect led driver pin of dust sensor to ESP8266 GPIO_12

int samplingTime = 280;   //0.28ms delay
int deltaTime = 40;
int sleepTime = 9680;
  
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
 
WiFiServer server(80);
 
void setup() 
{
  Serial.begin(115200);
  
  pinMode(ledPower,OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  setupWiFi();
  server.begin();
}
 
void loop() 
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
//    Serial.print(".");
    return;    
  }
  digitalWrite(ledPin, HIGH);
  Serial.println();

  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  
  voMeasured = analogRead(measurePin); // read the dust value
  
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);
  
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 0.17 * calcVoltage - 0.1;
  
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
  
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
  
  Serial.print(" - Dust Density: ");
  if( dustDensity < 0)
    dustDensity = 0;
  Serial.print(dustDensity); // unit: mg/m3
  Serial.println(" mg/m^3");
 
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  client.flush();
 
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<FONT size=\"18\">\r\n<html>\r\n";

  s += "<br>";
  s += "<< DustSensor (Sharp GP2Y1010AU) >>";
  s += "<br>";
  
  s += "<FONT size=\"12\">\r\n<html>\r\n";  
  
  s += "<br>";
  s += "  Raw Signal Value (0-1023):  ";
  s += String(voMeasured);
  s += "<br>";

  s += "  Voltage:  ";
  s += String(calcVoltage);
  s += "V";
  s += "<br>";
  
  s += "  Dust Density:  ";
  s += String(dustDensity);
  s += "mg/m^3";
  s += "<br>"; 

  s += "</html>\n";
 
  client.print(s);
  delay(100);
  digitalWrite(ledPin, LOW);
  
  Serial.println("Client disonnected");
}

 void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}
