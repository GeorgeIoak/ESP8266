/*
  iSensor ADIS16203 
*/

/*******************************************************************************
****************************** Include Files ***********************************
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <Wire.h>

#include "adis16203.h"

/*******************************************************************************
**************************** Internal types ************************************
********************************************************************************/
/* Write data mode */
typedef enum {
   SPI_WRITE_ONE_REG = 1,         /* Write 1 register */
   SPI_WRITE_TWO_REG,             /* Write 2 registers */
} enWriteData;

typedef enum {
   SPI_READ_ONE_REG = 1,            /* Read one register */
   SPI_READ_TWO_REG,                /* Read two registers */
} enRegsNum;

/*******************************************************************************
**************************** Internal definitions ******************************
********************************************************************************/
#define int8_t    char
#define uint8_t   unsigned char
#define int32_t   int
#define uint32_t  unsigned int

/* ADIS16203 write command */
#define ADIS16203_WRITE         0x80

/* ADIS16203 read command */
#define ADIS16203_READ          0x00

/* Pin defines */
#if (1) // WeMos D1, NodeMCU
#define LED     LED_BUILTIN

#define REREST        D0
#define CS_PIN        D1   
#define DATA_RDY      D2
#endif 

volatile int32_t i32SensorT;
volatile int32_t i32SensorX;
volatile int32_t i32SensorY;

volatile uint32_t ui32SensorT;
volatile uint32_t ui32SensorX;
volatile uint32_t ui32SensorY;


float volatile f32supply  = 0.0f;
float volatile f32temp    = 0.0f;
float volatile f32x       = 0.0f;
float volatile f32y       = 0.0f;

/**************************** Function Definitions ****************************/

/**
   @brief SPI initialization
   
   @return none
**/
void SPI_Init(void)
{
  SPI.begin();
//  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

  SPI.setHwCs(1);
  pinMode(CS_PIN, OUTPUT);                  /* Set Chip Select pin as output */
  digitalWrite(CS_PIN, HIGH);         /* Deselect accelerometer */
}

/**
   @brief Writes a data, a command or a register via SPI.

   @param ui8address - ACC register address
   @param ui8Data - value to be written in 1 register write

   @return none

**/
void SPI_Write(uint8_t ui8address, uint8_t ui8Data)
{
  uint8_t ui8writeAddress;
 
  ui8writeAddress = ( ADIS16203_WRITE | ui8address );
  
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);         /* Select accelerometer */
  SPI.transfer(ui8writeAddress);     /* Send register address */
  SPI.transfer(ui8Data);             /* Send value to be written */
  digitalWrite(CS_PIN, HIGH);         /* Deselect accelerometer */  
  SPI.endTransaction();

  return;
}

/**
   @brief Reads two registers via SPI.

   @param ui8address - register address

   @return reading result

**/
uint32_t SPI_Read(uint8_t ui8address)
{
  uint32_t ui32Result = 0;
  uint32_t ui32valueL = 0;
  uint32_t ui32valueH = 0;
  
  uint8_t ui8ReadAddress;

  ui8ReadAddress = ( ADIS16203_READ | ui8address  );

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);      /* Select accelerometer */
  SPI.transfer(ui8ReadAddress);  /* Send register address */
  SPI.transfer(0x00);             /* Send Dummy Data */
  digitalWrite(CS_PIN, HIGH);      /* Select accelerometer */
  SPI.endTransaction();
  
  delay(100);

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);         /* Select accelerometer */
  ui32valueH = SPI.transfer(ui8ReadAddress); /* Read the first register value */
  ui32valueL = SPI.transfer(0xAA);           /* Read the second register value */
  digitalWrite(CS_PIN, HIGH);         /* Select accelerometer */
  SPI.endTransaction();

  Serial.println(ui32valueH, HEX);
  Serial.println(ui32valueL, HEX);
  
  ui32Result = ((ui32valueH << 8) | ui32valueL);

  return ui32Result;
}

/**
   @brief Initialization the accelerometer sensor

   @return none

**/
void ADIS16203_Init(void)
{
   pinMode(REREST, OUTPUT);                  /* Set RESET pin as output */
   pinMode(DATA_RDY, INPUT);

  digitalWrite(REREST, HIGH);
  delay(500);
  digitalWrite(REREST, LOW);
  delay(500);
  digitalWrite(REREST, HIGH);
  delay(500);
}

/**
   @brief Turn on Sensor.

   @return none

**/
void ADIS16203_Start_Sensor(void)
{
  digitalWrite(REREST, LOW);
  delay (500);
  digitalWrite(REREST, HIGH);
  delay(500);

  return;
}

/**
   @brief Turn off Sensor

   @return none

**/
void ADIS16203_Stop_Sensor(void)
{
  digitalWrite(REREST, LOW);
  delay(500);

  return;
}

/**
   @brief Convert the two's complement data in X,Y registers to signed integers

   @param ui32SensorData - raw data from register

   @return int32_t - signed integer data

**/
int32_t ADIS16203_Y_Data_Conversion (uint32_t ui32SensorData)
{
  int32_t volatile i32Conversion = 0;

  ui32SensorData = (ui32SensorData >> 4);
  ui32SensorData = (ui32SensorData & 0x00003FFF);

  if((ui32SensorData & 0x00002000) == 0x00002000){
         i32Conversion = ((~ui32SensorData+1) | 0x00001FFF);
  }else{
         i32Conversion = ui32SensorData;
  }
      
  return i32Conversion;
}

/**
   @brief Reads the accelerometer data.

   @return none

**/
void ADIS16203_Data_Scan(void)
{
  ui32SensorT = SPI_Read(ADIS16203_TEMP_OUT);
  ui32SensorX = SPI_Read(ADIS16203_XINCL_OUT);  
  ui32SensorY = SPI_Read(ADIS16203_YINCL_OUT);
  
  Serial.print("T = 0x");
  Serial.println(ui32SensorT, HEX);
  Serial.print("X = 0x");
  Serial.println(ui32SensorX, HEX);
  Serial.print("Y = 0x");
  Serial.println(ui32SensorY, HEX);

#if (0)
  ui32SensorT = ui32SensorT >> 4;  
  ui32SensorX = ui32SensorX >> 4);
  ui32SensorX = (ui32SensorX & 0x00003FFF);  
  f32x = (float)ui32SensorX * ADIS16203_ROAT_SEN;  
  i32SensorY = ADIS16203_Y_Data_Conversion(ui32SensorY);
  f32y = (float)i32SensorY * ADIS16203_ROAT_SEN;
#endif  
}

void ADIS16203_Factory_Cal()
{
  SPI_Write(ADIS16203_GLOB_CMD, 0x02);
}

void setup()
{
  volatile uint32_t ui32test;
  float f32sup    = 0.0f;

   // initialize LED pins as an output.
   pinMode(LED, OUTPUT);
     
   /* Initialize UART */
   Serial.begin(115200);
   Serial.println("***** ADIS16203 Simple Test *****");
   
   /* Initialize accelerometer */
   ADIS16203_Init();
 
   /* Initialize SPI */
   SPI_Init();  
   delay(500);

//  ADIS16203_Factory_Cal();
//  delay(500);

#if (0)
  while(1){
    digitalWrite(LED, HIGH);
    digitalWrite(REREST, HIGH);
    digitalWrite(CS_PIN, HIGH);
    delay(500);
    
    digitalWrite(LED, LOW);
    digitalWrite(REREST, LOW);
    digitalWrite(CS_PIN, LOW);
    delay(500);
    }
#endif

#if (0)
  do{
    digitalWrite(LED, HIGH);

    ui32test = SPI_Read(ADIS16203_SUPPLY_OUT);                  /* Read temperature register */       
    ui32test = ui32test>>4;
    ui32test = ui32test&0x0FFF;

    f32sup = (float)(ui32test) * ADIS16203_SUP_OUT_SF;
    Serial.print("SUPPLY_OUT = ");                /* Print the ID register */
    Serial.print(f32sup, 2);
    Serial.print("[mV], ");
    Serial.print((f32sup/1000), 2);
    Serial.println("[V]");
    Serial.println("");

    delay(500); 
    digitalWrite(LED, LOW);
    delay(500);     
     
  }while( f32sup==0 );
#endif
  
}

void loop() 
{
//    if( digitalRead(DATA_RDY) == HIGH )
    {
      digitalWrite(LED, HIGH);
      
      ADIS16203_Data_Scan();
#if (0) 
      f32temp = ((float)(ui32SensorT - ADIS16203_TEMP_BIAS) * ADIS16203_TEMP_SEN) + 25.0;
      
      Serial.print("The temperature data is: " );      /* Print the Temperature data */
      Serial.print(f32temp, 2);        
      Serial.println("[C]");
      
      Serial.print("X-rotational is: " );             /* Print the  data */
      Serial.print((float)f32x, 3);        
      Serial.println("[']");
      
      Serial.print("Y-rotational is: " );             /* Print the  data */
      Serial.print((float)f32y, 3);        
      Serial.println("[']");
      Serial.println("");
#endif  
  
      delay(500); 
    
      digitalWrite(LED, LOW);

      delay(1000);     
  }
}

/* End Of File */
