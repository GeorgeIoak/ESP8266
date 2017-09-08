#define LED     2
#define SW_PIN  5
 
void setup() { 
  Serial.begin(9600);         	       // Start serial communication at 9600bps 

  // initialize pins mode
  pinMode(SW_PIN, INPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);  
  Serial.println("***** Switch test *****");
} 
 
void loop() { 
  if (digitalRead(SW_PIN) == HIGH) 
  {  // If switch is ON, 
    Serial.println("Key is HIGH");
    digitalWrite(LED, HIGH);   
  } else {                          // If the switch is not ON,
    Serial.println("Key is LOW");
    digitalWrite(LED, LOW);    
  } 
  delay(100);                       // Wait 100 milliseconds 
} 

