/*
  Blink
  Turns on/off LEDs 
*/

#define LED     LED_BUILTIN

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize serial port
  Serial.begin(9600); 
  
  // initialize LED pins as an output.
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, LOW);   
}

// the loop function runs over and over again forever
void loop() {
  char val;
  
  // Check serial input
  while( Serial.available() )
  {
    val = Serial.read();
  }
  
  if( val == 'H' )
  {
    // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED, HIGH);   
  } 
  else if ( val == 'L') 
  {
    // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED, LOW);   
  }
  delay(100);
}
