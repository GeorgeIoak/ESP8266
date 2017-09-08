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

#define DHTTYPE DHT22   // DHT 22  (AM2302)

const char WiFiAPPSK[] = "12345678";

//const int ledPin = 2;    //(GPIO_2, D4) onboard LED of ESP-12F  
const int ledPin = 14;     //(GPIO_14, D5) LED
const int measurePin = A0; // Connect dust sensor to Arduino A0 pin
const int ledPower = 12;   //(GPIO_12_D6) Connect led driver pin of dust sensor to ESP8266 GPIO_12 
const int dhtPin = 13;     //(GPIO_13, D7) what pin we're connected to 

int samplingTime = 280;   //0.28ms delay
int deltaTime = 40;
int sleepTime = 9680;
  
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

// Initialize the OLED display using Wire library
SSD1306 display(0x3C, 4, 5); //GPIO 5 = D1(SCL), GPIO 4 = D2 (SDA)
WiFiServer server(80);
DHT dht(dhtPin, DHTTYPE, 25); //// Initialize DHT sensor for normal 16mhz Arduino
 
void setup() 
{
  Serial.begin(115200);
  
  pinMode(ledPower,OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.init();  // initialize with the I2C
  display.flipScreenVertically();
  
  // init done
  display.clear();
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,2,"L1: ESP8266 OLED");
  display.drawString(0,13,"L2:Dust Sensor");
  display.drawString(0,23,"L3:");
  display.drawString(0,33,"L4:");
  display.drawString(0,43,"L5:");
  display.drawString(0,53,"L6:");
  display.display();

  dht.begin();
  
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

  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp= dht.readTemperature();

  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");   

  // Dust Sensor
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin); // read the dust value
  
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  //calcVoltage = voMeasured * (5.0 / 1024.0);
  calcVoltage = voMeasured * (4.2 / 1024.0);
  
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

  display.clear();
  display.drawString(0,2,"L1: ESP8266 OLED");
  display.drawString(0,13,"Temp:" + String(temp) + "C");
  display.drawString(0,23,"Hum :" + String(hum) + "%");
  display.drawString(0,33,"Dust:" + String(dustDensity*1000) + "ug/m^3");
  display.drawString(0,43,"L5:");
  display.drawString(0,53,"L6:");
  display.display();

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  client.flush();
 
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<FONT size=\"12\">\r\n<html>\r\n";

  s += "<br>";
  s += "<< Temperature (DHT-22) >>";
  s += "<br>";

  s += "<br>";
  s += "  Humidity(0~100%): ";
  s += String(hum);
  s += "%";
  s += "<br>";

  s += "<br>";
  s += "  Temperature: ";
  s += String(temp);
  s += "Celsius";
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
