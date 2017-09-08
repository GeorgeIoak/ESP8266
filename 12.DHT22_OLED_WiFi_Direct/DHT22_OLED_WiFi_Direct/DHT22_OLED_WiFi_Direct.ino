/*
 * Reference :
 * https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-ap-web-server
 * 
 * USAGE : http://192.168.4.1
 */
 
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <SSD1306.h>
#include "images.h"

#define DHTTYPE DHT22   // DHT 22  (AM2302)

const char WiFiAPPSK[] = "12345678";

// Define GPIO
const int ledPin = 2;      // onboard LED of ESP-12F (GPIO_2)
const int measurePin = A0; // Connect dust sensor to Arduino A0 pin
const int ledPower = 12;   // Connect led driver pin of dust sensor to ESP8266 GPIO_12
const int dhtPin = 13;     // what pin we're connected to
const int almLED = 14;     // Alrm LED


// Cycle
const int duration = 10000; // 10000ms, 10 second

// Setup values for GP2Y1010AU
int samplingTime = 280;   //0.28ms delay
int deltaTime = 40;
int sleepTime = 9680;

// Dust sensor values  
float voMeasured = 0;
float calcVoltage = 0;
//float dustDensity = 0;
int dustDensity = 0;

// Temperature and Humidity values
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

// Timer
unsigned long sampleTime;

// Initialize the OLED display using Wire library
SSD1306 display(0x3C, 4, 5); //GPIO 5 = D1(SCL), GPIO 4 = D2 (SDA)

// Initialize DHT sensor for ESP8266
DHT dht(dhtPin, DHTTYPE, 25); 

// Initialize Direct WiFi
WiFiServer server(80);
WiFiClient client;

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

void setup() 
{
  Serial.begin(115200);

  // Init GPIOs
  pinMode(ledPower,OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(almLED, OUTPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(almLED, LOW);

  // Init OLED
  display.init();  
  display.flipScreenVertically();

  display.clear();
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,2,"L1: ESP8266 Dust");
  display.drawString(0,13,"L2:Dust Sensor");
  display.drawString(0,23,"L3: by Jyoun");
  display.drawString(0,33,"L4: jyounnim@gmail.com");
  display.drawString(0,43,"L5:");
  display.drawString(0,53,"L6:");
  display.display();

  // Init DHT22
  dht.begin();

  // Init WiFi Direct
  setupWiFi();
  server.begin();

  delay(1000);
  getTemperatureAndHumidity();
  getDust();
  sampleTime = millis();
}

void getTemperatureAndHumidity()
{
  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp= dht.readTemperature(); 

  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("Temperaturea: ");
  Serial.print(temp);
  Serial.println(" Celsius");      
}

void getDust()
{
  // Dust Sensor
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
//  dustDensity = 0.17 * calcVoltage - 0.1;
  dustDensity = 172 * calcVoltage - 99;

  // Print dust values to serial monitor 
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.println(voMeasured);  
  Serial.print(" - Voltage: ");
  Serial.println(calcVoltage);
  Serial.print(" - Dust Density: ");
  if( dustDensity < 0)
    dustDensity = 0;
  Serial.print(dustDensity); // unit: ug/m3
  Serial.println(" ug/m^3");  
}

void webClient()
{
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();
    
    // Match the request
    client.flush();
   
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";
    s += "<FONT size=\"10\">\r\n<html>\r\n";
  
    s += "<< Temperature (DHT-22) >>";
    s += "<br>";

    s += "<br>";
    s += "  Temperature: ";
    s += String(temp);
    s += "Celsius";
    s += "<br>";
    
    s += "  Humidity(0~100%): ";
    s += String(hum);
    s += "%";
    s += "<br>";
 
    s += "<br>";
    s += "<< DustSensor (Sharp GP2Y1010AU) >>";
    s += "<br>";
    
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
    s += "ug/m^3";
    s += "<br>"; 
  
    s += "</html>\n";    

    client.print(s);
    delay(100);

    Serial.println("Client disonnected");
}

void loop() 
{
#if (0)  
  bitMapTest();
  return;
#endif
   
  // Check if a client has connected
  client = server.available();

  if(client)
  {
     webClient();
  }

  if( (millis()-sampleTime) < duration )
  {
    if( millis() < sampleTime )
      sampleTime = millis();  
    return;
  }
  
  sampleTime = millis();

  digitalWrite(ledPin, LOW);
  Serial.println();

  getTemperatureAndHumidity();
  getDust();

  // Display OLED
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64,0,"<< ESP8266 DHT22 >>");
  
  display.drawLine(0, 13, 128, 13);
  //display.setFont(ArialMT_Plain_16);
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32,17, String(temp) + "C");
  display.drawString(96,17, String(hum) + "%");
  display.drawString(32,27, String(dustDensity) + "ug/m^3");
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(64,40,"http://192.168.4.1");
  display.display();
 
  digitalWrite(ledPin, HIGH);
  if( (hum>=80) || (temp>=29) )
    digitalWrite(almLED, HIGH);
  else
    digitalWrite(almLED, LOW);
}

// Test code
void bitMapTest()
{
display.clear();

  display.clear();
  display.drawFastImage(0, 0, 128*8, 25, jyoun);
  display.display();
  delay(2000);  

  display.drawString(0,2,"n0");
  display.drawFastImage(0,20, 2*8, 25, n0);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n1");
  display.drawFastImage(0,20, 2*8, 25, n1);
  display.display();
  delay(2000);  

  display.clear();
  display.drawString(0,2,"n2");
  display.drawFastImage(0,20, 2*8, 25, n2);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n3");
  display.drawFastImage(0,20, 2*8, 25, n3);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n4");
  display.drawFastImage(0,20, 2*8, 25, n4);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n5");
  display.drawFastImage(0,20, 2*8, 25, n5);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n6");
  display.drawFastImage(0,20, 2*8, 25, n6);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n7");
  display.drawFastImage(0,20, 2*8, 25, n7);
  display.display();
  delay(2000);

  display.clear();
  display.drawString(0,2,"n8");
  display.drawFastImage(0,20, 2*8, 25, n8);
  display.display();
  delay(2000);              

  display.clear();
  display.drawString(0,2,"n9");
  display.drawFastImage(0,20, 2*8, 25, n9);
  display.display();
  delay(2000);              

  display.clear();
  display.drawString(0,2,"pm25");
  display.drawFastImage(0,20, 4*8, 10, pm25);
  display.display();
  delay(2000);   

  display.clear();
  display.drawString(0,2,"doci");
  display.drawFastImage(0,20, 1*8, 6, doci);
  display.display();
  delay(2000);   

  display.clear();
  display.drawString(0,2,"perc");
  display.drawFastImage(0,20, 1*8, 5, perc);
  display.display();
  delay(2000); 

  display.clear();
  display.drawString(0,2,"goo");
  display.drawFastImage(0,20, 4*8, 6, goo);
  display.display();
  delay(2000); 
 
  display.clear();
  display.drawString(0,2,"nor");
  display.drawFastImage(0,20, 4*8, 6, nor);
  display.display();
  delay(2000); 

  display.clear();
  display.drawString(0,2,"bad");
  display.drawFastImage(0,20, 4*8, 6, bad);
  display.display();
  delay(2000); 

  display.clear();
  display.drawString(0,2,"fuc");
  display.drawFastImage(0,20, 4*8, 6, fuc);
  display.display();
  delay(2000); 

  display.clear();
  display.drawString(0,2,"logo");
  display.drawFastImage(0,20, 8*8, 15, logo);
  display.display();
  delay(2000); 

  display.clear();
  display.drawString(0,2,"loading");
  display.drawFastImage(0,20, 9*8, 10, loading);
  display.display();
  delay(2000);  
}
