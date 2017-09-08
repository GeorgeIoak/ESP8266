// Test for ESP8266 WPS connection.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

const int led = LED_BUILTIN;

void handleRoot() 
{
  digitalWrite(led, 1);
  server.send(200, "text/plain", "Hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup() 
{
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  
  // WPS works in STA (Station mode) only.
  WiFi.mode(WIFI_STA);
  delay(1000);
  
  // Called to check if SSID and password has already been stored by previous WPS call.
  // The SSID and password are stored in flash memory and will survive a full power cycle.
  // Calling ("",""), i.e. with blank string parameters, appears to use these stored values.
  WiFi.begin("","");
  
  // Long delay required especially soon after power on.
  delay(4000);
  
  // Check if WiFi is already connected and if not, begin the WPS process. 
  if (WiFi.status() != WL_CONNECTED) 
  {
      Serial.println("\nAttempting connection ...");
      WiFi.beginWPSConfig();
      // Another long delay required.
      delay(3000);
      if (WiFi.status() == WL_CONNECTED) 
      {
        Serial.println("Connected!");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.macAddress());
      } else {
        Serial.println("Connection failed!");
      }
  } else {
    Serial.println("\nConnection already established.");
  }

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) 
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() 
{
  server.handleClient();
}

