/*
  Blink
  Turns on LEDs on for one second, then off for one second, repeatedly.
*/

#define LED     LED_BUILTIN

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize serial port
  Serial.begin(9600); 
  
  // initialize LED pins as an output.
  pinMode(LED, OUTPUT);

  Serial.println("------- LED Blink -------"); 
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("LEDs ON"); 
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                  // wait for 500ms

  Serial.println("LEDs OFF"); 
  digitalWrite(LED, LOW);
  delay(500);                  // wait for 500ms
}
