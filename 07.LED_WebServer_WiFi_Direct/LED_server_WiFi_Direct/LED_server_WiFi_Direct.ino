#include <ESP8266WiFi.h>

#define LED_PIN LED_BUILTIN
WiFiServer server(80); //Initialize the server on Port 80

void setup() 
{
  // Step 1: Opening the "Connect to a Network" Window
  // Step 2: Writing the SimpleWebServer Sketch 
  WiFi.mode(WIFI_AP); //Our ESP8266-12E is an AccessPoint 
  WiFi.softAP("Hello_IoT", "12345678"); // Provide the (SSID, password); . 
  server.begin(); // Start the HTTP Server
  // Step 3: Checking Our Server 
  // Step 4: Looking Under the Hood

  // Step 5: Get HTTP Server Information From the ESP8266-12E
  //Looking under the hood
  Serial.begin(115200); //Start communication between the ESP8266-12E and the monitor window
  IPAddress HTTPS_ServerIP= WiFi.softAPIP(); // Obtain the IP of the Server 
  Serial.println("");
  Serial.println("Server IP is: "); // Print the IP to the monitor window 
  Serial.println(HTTPS_ServerIP);

  // Step 8: Turning the LED ON/OFF Through the Web Browser
  pinMode(LED_PIN, OUTPUT); //LED_BUILTIN is an OUTPUT pin;
  digitalWrite(LED_PIN, LOW); //Initial state is ON
}

void loop() 
{ 
  // Step 6: Web Browser Connects/Talks to Server
  WiFiClient client = server.available();
  if (!client) 
  { 
    return; 
  } 

  //Looking under the hood 
  Serial.println("Somebody has connected :)"); 

  // Step 7: Listen to What the Browser Is Sending to the HTTP Server
  //Read what the browser has sent into a String class and print the request to the monitor
  String request = client.readString(); 
//  String request = client.readStringUntil('\r'); 
  //Looking under the hood 
  Serial.println(request);

  //Step 8: Turning the LED ON/OFF Through the Web Browser
  // Handle the Request
  if (request.indexOf("/OFF") != -1)
  { 
    digitalWrite(LED_PIN, HIGH); 
    Serial.println("<< LED OFF >>");
  }
  else if(request.indexOf("/ON") != -1)
  { 
    digitalWrite(LED_PIN, LOW); 
    Serial.println("<< LED ON >>");
  }

  // Step 9: Let Us Get a Little Fancy
  // Prepare the HTML document to respond and add buttons:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Contenct-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<br><input type=\"button\" name=\"b1\" value=\"Turn LED ON\" onclick=\"location.href='/ON'\">";
  s += "<br><br><br>";
  s += "<input type=\"button\" name=\"b1\" value=\"Turn LED OFF\" onclick=\"location.href='/OFF'\">";
  s += "</html>\n";
  
  //Serve the HTML document to the browser.
  client.flush(); //clear previous info in the stream 
  client.print(s); // Send the response to the client 
  delay(1); 
  Serial.println("Client disonnected"); //Looking under the hood
}





