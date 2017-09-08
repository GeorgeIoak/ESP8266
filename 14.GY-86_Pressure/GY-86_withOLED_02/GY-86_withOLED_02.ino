/*
  HMC5883L Triple Axis Digital Compass + MPU6050 (GY-86 / GY-87). Output for HMC5883L_processing.pde
  Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-magnetometr-hmc5883l.html
  GIT: https://github.com/jarzebski/Arduino-HMC5883L
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <HMC5883L.h>
#include <MPU6050.h>
#include <MS561101BA.h>

//#define _USE_HMC5883L_
//#define _USE_MS561101_

HMC5883L compass;
MPU6050 mpu;

/* Note:
 * ====
 * 정확한 높이를 측정하기 위해서는 해면 기압 정보를 사용하여야 합니다.
 * http://www.kma.go.kr/weather/observation/aws_table_popup.jsp 로부터
 * 기압 정보를 얻어 sea_press에 넣었습니다.
 */
MS561101BA baro = MS561101BA();

int previousDegree;
Vector initAcc;

#define MOVAVG_SIZE             32
#define STANDARD_SEA_PRESSURE  1013.25

const float sea_press = 1014.0;
float movavg_buff[MOVAVG_SIZE];
int movavg_i=0;

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("***** GY-86 ******");

//  Wire.pins(4,5); //  Wire.pins(int sda, int scl)
//  Wire.begin(4,5);
  
  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.print(".");
    delay(500);
  }
  mpu.calibrateGyro();
  Serial.println("");
  initAcc = mpu.readScaledAccel();
  
#ifdef _USE_HMC5883L_
  // Enable bypass mode
  mpu.setI2CMasterModeEnabled(false);
  mpu.setI2CBypassEnabled(true);
  mpu.setSleepEnabled(false);

  // Initialize HMC5883L
  while (!compass.begin())
  {
    delay(500);
  }

  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);

  // Set calibration offset. See HMC5883L_calibration.ino
  compass.setOffset(0, 0); 
#endif

#ifdef _USE_MS561101_
  // 기압계를 초기화합니다
  baro.init(MS561101BA_ADDR_CSB_LOW);
  delay(100);
  
  // 평균 압력 정보를 얻기 위하여 미리 값을 초기화 합니다
  for(int i=0; i<MOVAVG_SIZE; i++) {
    float p = baro.getPressure(MS561101BA_OSR_4096);
    if(p == NULL)
      p = STANDARD_SEA_PRESSURE;
      
    movavg_buff[i] = p;    
  }  
#endif  
}

void loop()
{
  float temperature = NULL, pression = NULL;
  Vector acc,gyr;
  long x = micros();

#ifdef _USE_HMC5883L_
  // Enable bypass mode
  mpu.setI2CMasterModeEnabled(false);
  mpu.setI2CBypassEnabled(true);
  mpu.setSleepEnabled(false);
  
  Vector norm = compass.readNormalize();

  // Calculate heading
  float heading = atan2(norm.YAxis, norm.XAxis);

  // Set declination angle on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Poland declination angle is 4'26E (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
  heading += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg
  if (heading < 0)
  {
    heading += 2 * PI;
  }
 
  if (heading > 2 * PI)
  {
    heading -= 2 * PI;
  }

  // Convert to degrees
  float headingDegrees = heading * 180/M_PI; 

  // Fix HMC5883L issue with angles
  float fixedHeadingDegrees;
 
  if (headingDegrees >= 1 && headingDegrees < 240)
  {
    fixedHeadingDegrees = map(headingDegrees, 0, 239, 0, 179);
  } else
  if (headingDegrees >= 240)
  {
    fixedHeadingDegrees = map(headingDegrees, 240, 360, 180, 360);
  }

  // Smooth angles rotation for +/- 3deg
  int smoothHeadingDegrees = round(fixedHeadingDegrees);

  if (smoothHeadingDegrees < (previousDegree + 3) && smoothHeadingDegrees > (previousDegree - 3))
  {
    smoothHeadingDegrees = previousDegree;
  }
  
  previousDegree = smoothHeadingDegrees;

  // Output
  Serial.print(norm.XAxis);
  Serial.print(":");
  Serial.print(norm.YAxis);
  Serial.print(":");
  Serial.print(norm.ZAxis);
  Serial.print(":");
  Serial.print(headingDegrees);
  Serial.print(":");
  Serial.print(fixedHeadingDegrees);
  Serial.print(":");
  Serial.print(smoothHeadingDegrees);  
  Serial.println();
#endif
//  mpu.setSleepEnabled(true);
//  mpu.setI2CMasterModeEnabled(true);
//  mpu.setI2CBypassEnabled(false);
  
  acc = mpu.readScaledAccel();
  gyr = mpu.readNormalizeGyro();

#if (1)
  Serial.print(acc.XAxis*1000,3); Serial.print(",");
  Serial.print(acc.YAxis*1000,3); Serial.print(",");
  Serial.println(acc.ZAxis*1000,3);
#else
  Serial.print((acc.XAxis-initAcc.XAxis)*1000,3); Serial.print("mg\t");
  Serial.print((acc.YAxis-initAcc.YAxis)*1000,3); Serial.print("mg\t");
  Serial.print((acc.ZAxis-initAcc.ZAxis)*1000,3); Serial.print("mg\t");
  
  Serial.print(gyr.XAxis); Serial.print("\t");
  Serial.print(gyr.YAxis); Serial.print("\t");
  Serial.print(gyr.ZAxis); Serial.println("\t");
#endif

#ifdef _USE_MS561101_
  mpu.setI2CMasterModeEnabled(true);
  
  while(temperature == NULL) {
    temperature = baro.getTemperature(MS561101BA_OSR_4096);
  }

  while(pression == NULL) {
    pression = baro.getPressure(MS561101BA_OSR_4096);
  }

  // 온도와 기압 그리고 높이를 출력합니다
  Serial.print("temp: ");
  Serial.print(temperature);
  Serial.print(" ℃ pres: ");
  Serial.print(pression);
  Serial.println(" mbar");
#endif

  // One loop: ~5ms @ 115200 serial.
  // We need delay ~28ms for allow data rate 30Hz (~33ms)
  delay(30);
  delay(100);
}

