import processing.serial.*; 
 
Serial myPort;                       // Create object from Serial class

char cLED;
 
void setup() { 
  size(200, 200); 
  noStroke(); 
  frameRate(10); 
  
  println(Serial.list());
  // Open the port that the board is connected to and use the same speed (9600 bps)
  myPort = new Serial(this, Serial.list()[2], 9600);
  
  cLED = 'L';
  fill(0);
} 
 
void draw() { 
  background(255);
  
  if ( (mouseOverRect() == true) & (cLED == 'L') )  
  {  // If mouse is over square,
    println("LED ON");
    cLED = 'H';
    fill(204);                     // change color and  
    myPort.write('H');             // send an H to indicate mouse is over square 
  }
  else if( (mouseOverRect() == false) & (cLED == 'H' ) ) 
  {    // If mouse is not over square,
    cLED = 'L';
    println("LED OFF");
    fill(0);                       // change color and
    myPort.write('L');             // send an L otherwise
  } 
  rect(50, 50, 100, 100);          // Draw a square
} 


boolean mouseOverRect() {        // Test if mouse is over square 
  return ((mouseX >= 50) && (mouseX <= 150) && (mouseY >= 50) && (mouseY <= 150)); 
} 