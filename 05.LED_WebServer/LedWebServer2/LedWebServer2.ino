/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <SSD1306.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "jyoun-soohee";
const char* password = "07040251995";

ESP8266WebServer server(80);

String webPage = "";

int gpio0_pin = LED_BUILTIN;  //GPIO 16
int gpio2_pin = 2;

// Initialize the oled display 
SSD1306 display(0x3c, 5, 4); //GPIO 4 = D2 (SDA), GPIO 5 = D1(SCL)

void setup(void){
  // preparing GPIOs
  pinMode(gpio0_pin, OUTPUT);
  digitalWrite(gpio0_pin, LOW);
  pinMode(gpio2_pin, OUTPUT);
  digitalWrite(gpio2_pin, LOW);

  Serial.begin(115200);
  delay(100);

  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //  display.flipScreenVertically();
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  delay(500);

  //Connect to wifi my network;
  Serial.println();
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);  
  display.clear();
  display.drawString(10, 10, "Connecting to WiFi");
  display.drawString(10, 20, "SSID:" + String(ssid));
  display.display();
  
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Print the IP address 
  display.clear();
  display.drawString(10, 10, "Use this URL to connect:");
  display.drawString(10, 20, "http://" + String(WiFi.localIP()) );
  display.display();
  
  webPage += "<h1>ESP8266 Web Server</h1><p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";
  webPage += "<p>Socket #2 <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/socket1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio0_pin, HIGH);
    delay(1000);
  });
  server.on("/socket1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio0_pin, LOW);
    delay(1000); 
  });
  server.on("/socket2On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio2_pin, HIGH);
    delay(1000);
  });
  server.on("/socket2Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio2_pin, LOW);
    delay(1000); 
  });
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  server.handleClient();
} 


