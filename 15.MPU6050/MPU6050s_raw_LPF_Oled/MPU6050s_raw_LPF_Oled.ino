
// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class
// 10/7/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added multiple output formats
//                 - added seamless Fastwire support
//      2011-10-07 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050s.h"

#define _FOR_PLOT_ 1

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t init_ax=0, init_ay=0, init_az=0;
int16_t gx, gy, gz;
int16_t init_gx=0, init_gy=0, init_gz=0;

// MPU6050_RANGE_2G:
#define rangePerDigit .061f //mg
// MPU6050_RANGE_4G:
//#define rangePerDigit .122f;  //mg
// MPU6050_RANGE_8G:
//#define rangePerDigit .244f;  //mg
//  MPU6050_RANGE_16G:
//#define rangePerDigit .4882f; //mg

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

#define INIT_COUNT 100

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

#define LED_PIN LED_BUILTIN
bool blinkState = false;

#define OLED_DC      2
#define OLED_RESET   15
#define OLED_CS      0

Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
//Adafruit_SSD1306 display();

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(115200);
    SPI.begin();
    display.begin(SSD1306_SWITCHCAPVCC);

    delay(100);   
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("* www.ino-on.co.kr *");
    display.println("** MPU6050 **");
    display.println("Initializing...");
    display.println("Waiting 3 seconds.....");
    display.display();
  
    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  
    for(char i=0 ; i<=INIT_COUNT ; i++)
    {
      delay(10);
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      
      init_ax = (init_ax*i/INIT_COUNT) + (ax*(INIT_COUNT-i)/INIT_COUNT);
      init_ay = (init_ay*i/INIT_COUNT) + (ay*(INIT_COUNT-i)/INIT_COUNT);
      init_az = (init_az*i/INIT_COUNT) + (az*(INIT_COUNT-i)/INIT_COUNT);

      init_gx = (init_gx*i/INIT_COUNT) + (gx*(INIT_COUNT-i)/INIT_COUNT);
      init_gy = (init_gy*i/INIT_COUNT) + (gy*(INIT_COUNT-i)/INIT_COUNT);
      init_gz = (init_gz*i/INIT_COUNT) + (gz*(INIT_COUNT-i)/INIT_COUNT);
    }

    accelgyro.setDLPFMode(MPU6050_DLPF_BW_42); 
    // use the code below to change accel/gyro offset values
//    accelgyro.setXAccelOffset(accelgyro.getXAccelOffset());
//    accelgyro.setYAccelOffset(accelgyro.getYAccelOffset());
//    accelgyro.setZAccelOffset(accelgyro.getZAccelOffset());
    
    /*
    Serial.println("Updating internal sensor offsets...");
    // -76	-2359	1688	0	0	0
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    accelgyro.setXGyroOffset(220);
    accelgyro.setYGyroOffset(76);
    accelgyro.setZGyroOffset(-85);
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    */

    // configure Arduino LED for
    pinMode(LED_PIN, OUTPUT);
}


char loop_count;

void loop() {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
#if _FOR_PLOT_
  #if (1)
        Serial.print((ax-init_ax)*rangePerDigit); Serial.print(",");
        Serial.print((ay-init_ay)*rangePerDigit); Serial.print(",");
        Serial.println((az-init_az)*rangePerDigit);
    if( loop_count%0x1F == 0 )
    {
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("* www.ino-on.co.kr *");
        display.print("X: ");
        display.print((float)(ax-init_ax)*rangePerDigit, 3);
        display.println("mg");
        display.print("Y: ");
        display.print((float)(ay-init_ay)*rangePerDigit, 3);
        display.println("mg");
        display.print("Z: ");
        display.print((float)(az-init_az)*rangePerDigit, 3);    
        display.println("mg");
        display.display();
    }        
  #else
        Serial.print((gx-init_gx)); Serial.print("\t");
        Serial.print((gy-init_gy)); Serial.print("\t");
        Serial.println((gz-init_gz));
  #endif        
#else
        Serial.print("a/g:\t");
        Serial.print((ax-init_ax)*rangePerDigit); Serial.print("mg\t");
        Serial.print((ay-init_ay)*rangePerDigit); Serial.print("mg\t");
        Serial.print((az-init_az)*rangePerDigit); Serial.print("mg\t");
        Serial.print((gx-init_gx)); Serial.print("\t");
        Serial.print((gy-init_gy)); Serial.print("\t");
        Serial.println((gz-init_gz));
#endif        
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
    loop_count++;
}
