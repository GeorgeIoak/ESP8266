#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <SSD1306.h>

const char* ssid = "jyoun-soohee";
//const char* ssid = "jyoun_ASUS";
const char* password = "07040251995";

//int ledPin = 13; //GPIO 13
int ledPin = LED_BUILTIN;  //GPIO 16
WiFiServer server(80); 

// Initialize the oled display 
SSD1306 display(0x3c, 5, 4); //GPIO 4 = D2 (SDA), GPIO 5 = D1(SCL)

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(ledPin, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(ledPin, LOW); //LED off

  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //  display.flipScreenVertically();
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
//  display.setTextAlignment(TEXT_ALIGN_CENTER);
//  display.setContrast(255);
  delay(500);

  //Connect to wifi my network;
  Serial.println();
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); //와이파이에 연결

  display.clear();
  display.drawString(10, 10, "Connecting to WiFi");
  display.display();
    
  //wifi에 연결하는 동안 계속 쩜을 찍어 댄다.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");

  display.clear();
  display.drawString(10, 10, "WiFi Connected");
  display.display();
    
  //Start the server
  server.begin(); 
  Serial.println("Server started");

  //Print the IP address 
  Serial.println("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
  display.clear();
  display.drawString(10, 10, "Use this URL to connect:");
  display.drawString(10, 20, "http://" + String(WiFi.localIP()) );
  display.drawString(20, 30, "/LED=ON");
  display.drawString(30, 30, "/LED=OFF");
  display.display();
}

// the loop function runs over and over again forever
void loop() {
  //Check if a client has connected
  WiFiClient client = server.available();
  
  if(!client) {
    delay(1);
    return;
  }

  //Wait untill the client sends  some data
  Serial.println("new client");
  
  while(!client.available()){
    delay(1);
  }

  //Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush(); 

  //Match the request

  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if(request.indexOf("/LED=OFF") != -1) {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }

  //Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>NodeMCU Control</title>");
  client.println("<meta name='viewport' content='width=device-width, user-scalable=no'>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div style='width: 300px; margin: auto; text-align: center;'>");
  client.println("<h3>NodeMCU Web Server</h3>");
  client.println("<p>");
  client.print("Led pin is now : ");
  if(value == HIGH) {
    client.println("On");
  } else {
    client.println("Off");
  }
  client.println("</p>");
  //client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</div>");
  client.println("</body>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disconnected");
  Serial.print("");
  
}



