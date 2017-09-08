/**The MIT License (MIT)

Copyright (c) 2016 by Seokjin Seo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://usemodj.com
*/

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "SSD1306Wire.h"
#include "KMAWeatherJsonClient.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"
#include "NTPTimeClient.h"

/***************************
 * Begin Settings
 **************************/
// WIFI
const char* WIFI_SSID = "jyoun-soohee"; 
const char* WIFI_PWD = "07040251995";

// KMA Weather Settings
// *  www.data.go.kr
// *    OPEN API: (신)동네예보정보조회서비스
const String KMA_SERVICE_KEY = "CwXREWv%2F%2F8NT3KtZiu3VxVnERZjaGz5ywC6Sw9aLFOQhjNMMEOMi3YZAt9IhsDIJBg3WY9zFr%2FZ%2BG8mtgSmlNQ%3D%3D";
const String KMA_NX = "62"; //예보지점 X좌표(분당구:수내동)
const String KMA_NY = "123"; //예보지점 Y좌표(분당구:수내동)

// Setup
const int UPDATE_INTERVAL_SECS = 30 * 60; // Update every 30 minutes

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3D; // Display I2C address 0x3D or 0x3C
const int SDA_PIN = D3; //D3
const int SDC_PIN = D4; //D4
const int OLED_RESET = D5;

// TimeClient settings
const float UTC_OFFSET = 9; //South Korea (Seoul)

// Initialize the oled display 
SSD1306Wire   display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi ui     ( &display );

/***************************
 * End Settings
 **************************/
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawProgress(OLEDDisplay *display, int percentage, String label);
void setReadyForWeatherUpdate();
void updateData(OLEDDisplay *display);
void drawForecastToday(OLEDDisplay *display, int x, int y, int fcstIndex);
void drawForecastTomorrow(OLEDDisplay *display, int x, int y, int fcstIndex);

NTPTimeClient timeClient(UTC_OFFSET);

KMAWeatherJsonClient kmaWeather;

// this array keeps function pointers to all frames
// frames are the single views that slide from BOTTOM to TOP
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5, drawFrame6 };
int numberOfFrames = 6;

// flag changed in the ticker function every 10 minutes
bool readyForWeatherUpdate = false;
String lastUpdate = "--";
Ticker ticker; 

void setup() {
  Serial.begin(115200);
  Serial.println();

  /* reset Display */
  pinMode(OLED_RESET,OUTPUT);
  digitalWrite(OLED_RESET, HIGH);   
  delay(500);                  // wait 500ms
  digitalWrite(OLED_RESET, LOW);   
  delay(500);                  // wait 500ms
  digitalWrite(OLED_RESET, HIGH);  
  
  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);
  delay(500);
  
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();
    
    counter++;
  }

  ui.setTargetFPS(30);
  ui.setTimePerFrame(5000); //5sec
  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  //ui.setIndicatorPosition(LEFT);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  //ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrameAnimation(SLIDE_UP);

  // Add frames
  ui.setFrames(frames, numberOfFrames);

  // Inital UI takes care of initalising the display too.
  ui.init();

  // Flip Screen Vertically
  display.flipScreenVertically();
  
  Serial.println("");

  updateData(&display);

  ticker.attach(UPDATE_INTERVAL_SECS, setReadyForWeatherUpdate);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(3);
  }
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Updating time...");
  timeClient.updateTime();

  String baseDate = timeClient.getFormattedDate(""); //"20161012";
  String baseTime = "0200";
  int numOfRows = 200;

  drawProgress(display, 50, "Updating forecasts...");
  kmaWeather.updateForecast(KMA_SERVICE_KEY, KMA_NX, KMA_NY, baseDate, baseTime, numOfRows);
  lastUpdate = timeClient.getFormattedTime();
  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(1000);
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawRect(10, 28, 108, 12);
  display->fillRect(12, 30, 104 * percentage / 100 , 9);
  display->display();
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int textWidth;
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  String date = timeClient.getMonth() + "/" + timeClient.getDay();
  textWidth = display->getStringWidth(date);

  display->drawString(3 + x, 3 + y, date);
  display->setFont(ArialMT_Plain_10);
  display->drawString(8 + x + textWidth, 5 + y, "Weather");
  display->drawString(8 + x + textWidth, 15 + y, "Forcasting");
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  String time = timeClient.getHours() + ":" + timeClient.getMinutes()+ ":";
  textWidth = display->getStringWidth(time);
  display->drawString(20 + x, 25 + y, time);
  display->setFont(ArialMT_Plain_16);
  String second = timeClient.getSeconds();
  display->drawString(20 + textWidth + x, 33 + y, second);  
  //display->setTextAlignment(TEXT_ALIGN_LEFT);
 
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y+2, "T");
  int tWidth = display->getStringWidth("T");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+tWidth, y+10, "oday");
  
  display->setFont(ArialMT_Plain_10);
  display->drawString(x, y + 22, String((int)kmaWeather.getTempMaxMinToday()->tmn) 
  + "/" + String((int)kmaWeather.getTempMaxMinToday()->tmx) + + "°C");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+1, y + 40, "Humidity:"); 

  drawForecastToday(display, x + 36, y, 0);
  drawForecastToday(display, x + 64, y, 1);
  drawForecastToday(display, x + 92, y, 2);
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y+2, "T");

  int tWidth = display->getStringWidth("T");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+tWidth, y+10, "oday");
  
  display->setFont(ArialMT_Plain_10);
  display->drawString(x, y + 22, String((int)kmaWeather.getTempMaxMinToday()->tmn) 
  + "/" + String((int)kmaWeather.getTempMaxMinToday()->tmx) + + "°C");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+1, y + 40, "Humidity:");

  drawForecastToday(display, x + 36, y, 3);
  drawForecastToday(display, x + 64, y, 4);
  drawForecastToday(display, x + 92, y, 5);
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y+2, "T");
  int tWidth = display->getStringWidth("T");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+tWidth, y+2, "omo");
  display->drawString(x+tWidth, y+10, "rrow");
    
  display->setFont(ArialMT_Plain_10);
  display->drawString(x, y + 22, String((int)kmaWeather.getTempMaxMinTomorrow()->tmn) 
  + "/" + String((int)kmaWeather.getTempMaxMinTomorrow()->tmx) + + "°C");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+1, y + 40, "Humidity:"); 

  drawForecastTomorrow(display, x + 36, y, 0);
  drawForecastTomorrow(display, x + 64, y, 1);
  drawForecastTomorrow(display, x + 92, y, 2);
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y+2, "T");
  int tWidth = display->getStringWidth("T");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+tWidth, y+2, "omo");
  display->drawString(x+tWidth, y+10, "rrow");

  display->setFont(ArialMT_Plain_10);
  display->drawString(x, y + 22, String((int)kmaWeather.getTempMaxMinTomorrow()->tmn) 
  + "/" + String((int)kmaWeather.getTempMaxMinTomorrow()->tmx) + + "°C");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+1, y + 40, "Humidity:");

  drawForecastTomorrow(display, x + 36, y, 3);
  drawForecastTomorrow(display, x + 64, y, 4);
  drawForecastTomorrow(display, x + 92, y, 5);
}

void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x, y+2, "T");
  int tWidth = display->getStringWidth("T");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+tWidth, y+2, "omo");
  display->drawString(x+tWidth, y+10, "rrow");

  display->setFont(ArialMT_Plain_10);
  display->drawString(x, y + 22, String((int)kmaWeather.getTempMaxMinTomorrow()->tmn) 
  + "/" + String((int)kmaWeather.getTempMaxMinTomorrow()->tmx) + + "°C");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x+1, y + 40, "Humidity:");

  drawForecastTomorrow(display, x + 36, y, 6);
  drawForecastTomorrow(display, x + 64, y, 7);
}

void setReadyForWeatherUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}

void drawForecastToday(OLEDDisplay *display, int x, int y, int fcstIndex) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 21, y, kmaWeather.getForecastToday(fcstIndex)->fcstTime.substring(0,2) + "h");
  
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 10, kmaWeather.getWeatherIcon(kmaWeather.getForecastToday(fcstIndex)->sky, 
                                        kmaWeather.getForecastToday(fcstIndex)->pty, kmaWeather.getForecastToday(fcstIndex)->fcstTime.substring(0,2).toInt()));

  display->setFont(ArialMT_Plain_10);
  //display->drawString(x + 20, y + 20, String(kmaWeather.getForecastToday(fcstIndex)->pop) + "%"); // 강수확률(%)
  display->drawString(x + 20, y + 30, String((int)kmaWeather.getForecastToday(fcstIndex)->t3h) + "°C"); //3시간기온
  display->drawString(x + 20, y + 40, String(kmaWeather.getForecastToday(fcstIndex)->reh) + "%"); // 습도(%)
  
}

void drawForecastTomorrow(OLEDDisplay *display, int x, int y, int fcstIndex) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 21, y, kmaWeather.getForecastTomorrow(fcstIndex)->fcstTime.substring(0,2) + "h");
  
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 10, kmaWeather.getWeatherIcon(kmaWeather.getForecastToday(fcstIndex)->sky, 
                                        kmaWeather.getForecastTomorrow(fcstIndex)->pty, kmaWeather.getForecastTomorrow(fcstIndex)->fcstTime.substring(0,2).toInt()));

  display->setFont(ArialMT_Plain_10);
  //display->drawString(x + 20, y + 20, String(kmaWeather.getForecastTomorrow(fcstIndex)->pop) + "%"); // 강수확률(%)
  display->drawString(x + 20, y + 30, String((int)kmaWeather.getForecastTomorrow(fcstIndex)->t3h) + "°C"); //3시간기온
  display->drawString(x + 20, y + 40, String(kmaWeather.getForecastTomorrow(fcstIndex)->reh) + "%"); // 습도(%)
  
}





