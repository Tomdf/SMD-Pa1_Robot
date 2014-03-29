/* Code for the SMD-PA1 robot. Written with Arduino IDE v1.0.5.-r2
 for a 5V Pro Mini.
 
 Pins in use = 2 interrupt on front left blue button
 3
 4 ultrasonic sensor
 5 head servo
 6 front green LED
 7 WS2812 RGB LED data
 8 left drive servo
 9 blue head led
 10 head buzzer
 11 front right blue button
 12 front middle yellow button
 13 right drive servo
 A0
 A1
 A2
 A3
 
 Tom Flock
 3-22-14
 
 Requires the Adafruit NeoPixel library. It's awesome, go get it.
 https://github.com/adafruit/Adafruit_NeoPixel
 */

#include <Servo.h>    // include servo library
#include "pitches.h"  // include pitches h file?

#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"

// Definitions for use with the NeoPixel library
#define PIN 7    // WS2812 pin
#define LED_COUNT 1    // the number of WS2812s connected

// Create an instance of the Adafruit_NeoPixel class called "leds".
// That'll be what we refer to from here on...
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

Servo headServo;  //create servo object to control a servo
Servo leftDrive;
Servo rightDrive;

const int pingPin = 4;   //connection to ultrasonic sensor
const int grnLED = 6;   //connetion to front green LED
const int headLED = 9;   //connetion to blue head LED
const int yellowBtn = 12; //connection to front yellow button

//var for holding operation mode, default 0
int modeSelect = 0;

boolean panDirection = true; //for telling if the head is in the left or right position

//variable used for interrupt debouncing
unsigned long lastMillis = 0;

long eyeColor = 16777215;  //value for the WS2812 eye LEDs
unsigned long distance = 0;  //ultrasonic sensor value
unsigned long randNumber;  //variable for holding random numbers

void setup()
{
  Serial.begin(9600);
  attachInterrupt(0, interruptOne, RISING); //create interrupt on pin 2
  headServo.attach(5);
  leftDrive.attach(13);
  rightDrive.attach(8);
  leftDrive.write(94.25); //stop position of right drive servo
  rightDrive.write(93); //stop position of left drive servo
  pinMode(headLED, OUTPUT);
  pinMode(grnLED, OUTPUT);
  leds.begin();  //initiate WS2812 LEDs
  clearLEDs();   //turns all LEDs off...
  setColor(eyeColor,1); //set eye color and brightness
  melodyHello();  //play a little ditty
  randomSeed(analogRead(0)); //read noise for a random seed
}

void loop(){ 
  Serial.print("Current Mode:");
  Serial.println(modeSelect);
  //choose a function based on the modeSelect value
  switch(modeSelect){
  case 0:
    idleMode();
    break;
  case 1:
    proxAlarm();
    break;  
  case 2:
    wanderer();
    break;  
  }

  if (digitalRead(yellowBtn)){
    setColor(YELLOW,1);
    Serial.println("Yellow Button Pressed.");
    if (modeSelect <= 2){
      ++modeSelect;
    }
    if (modeSelect >= 3){      
      modeSelect = 0;
      headServo.write(90);
    }
    delay(500);
  } 
}

void interruptOne(){
  unsigned long currentMillis = millis();
  if(currentMillis - lastMillis > 1500){    
    setColor(BLUE,1);
  }
  delay(250);
  lastMillis = currentMillis;
}

//while in idle mode the bot sits and waits for an interrupt
//to change the mode. The random head turn is added for flavor
void idleMode(){
  setColor(0x99CC00,1);
  randHeadTurn();
  //leftDrive.write(10);
  //rightDrive.write(170);
  digitalWrite(grnLED, HIGH);
  delay(1000);
  digitalWrite(grnLED, LOW);
  delay(750);
}

void proxAlarm(){
  setColor(RED,5); 
  randHeadTurn(); //randomly turn head and play a song
  distance = ping();
  Serial.println(distance);
  //if somthing is within the specified distance, play a song
  //and turn head
  if (distance <= 5000){
    setColor(RED,1);
    randNumber = random(20, 160);
    headServo.write(randNumber);
    randomMelody();
    randNumber = random(20, 160);
    headServo.write(randNumber);
    delay(200);
    setColor(RED,5);     
  }
  delay(250); 
}

void wanderer(){
  static int leftDistance;
  static int centerDistance;
  static int rightDistance;

  setColor(PURPLE,1);  //change eye color
  delay(500);          //delay for human interaction

  if(panDirection){
    if(headServo.read() != 20){
      headServo.write(20); //turn head to the left
      delay(800);          //delay for servo turning
    }    
    rightDistance = ping();
    headServo.write(90); //turn head to center
    delay(800);          //delay for servo turning
    centerDistance = ping();
    headServo.write(160); //turn head to the right 
    delay(800);          //delay for servo turning
    leftDistance = ping();
    panDirection = false;
  }
  else{
    leftDistance = ping();
    headServo.write(90);
    delay(800);
    centerDistance = ping();
    headServo.write(20); 
    delay(800);
    rightDistance = ping();
    panDirection = true;
  }

  Serial.print("Right Distance = ");
  Serial.println(rightDistance);
  Serial.print("Center Distance = ");
  Serial.println(centerDistance);
  Serial.print("Left Distance = ");
  Serial.println(leftDistance);

  if ((rightDistance < centerDistance) && (centerDistance > leftDistance)){
    Serial.print("Forward");
    leftDrive.write(160);
    rightDrive.write(20);
    delay(2000);    
    leftDrive.write(94.25); //stop position of right drive servo
    rightDrive.write(93); //stop position of left drive servo
  }
  if ((rightDistance > centerDistance) && (rightDistance > leftDistance)){
    Serial.print("Right");
    leftDrive.write(160);
    rightDrive.write(160);
    delay(750);    
    leftDrive.write(94.25); //stop position of right drive servo
    rightDrive.write(93); //stop position of left drive servo
  }
  if ((leftDistance > centerDistance) && (rightDistance < leftDistance)){
    Serial.print("Left");
    leftDrive.write(20);
    rightDrive.write(20);
    delay(750);    
    leftDrive.write(94.25); //stop position of right drive servo
    rightDrive.write(93); //stop position of left drive servo
  }
}

void randHeadTurn(){
  //randomly turn head and play a song
  randNumber = random(500);
  if (randNumber == 1){
    randomMelody();
    randNumber = random(20, 160);
    headServo.write(randNumber);    
  }
}

void setColor(unsigned long color, byte brightness){
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);

  for (int i=0; i<=LED_COUNT-1; i++){
    leds.setPixelColor(i, red/brightness, green/brightness, blue/brightness);
  }
  leds.show();  // Turn the LEDs on
}

// Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs(){
  for (int i=0; i<LED_COUNT; i++){
    leds.setPixelColor(i, 0);
    leds.show();
  }
}

unsigned long ping(){
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  return pulseIn(pingPin, HIGH);
  delay(100);
}

void randomMelody(){
  randNumber = random(1, 4);
  if (randNumber == 1){
    melodyHello();
  }
  else if (randNumber == 2){
    melodySup();
  }
  else if (randNumber == 3){
    melodyDanger();
  }
}

void melodyHello(){
  // notes in the melody:
  int melody[] = {
    NOTE_C4, NOTE_E3, NOTE_G3, 0, NOTE_F3, NOTE_A3, NOTE_A4, NOTE_B4              };
  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  int noteDurations[] = {
    8, 10, 6, 8,10,8,8,10              };
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++){
    digitalWrite(headLED, HIGH);
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(10, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(10);
    digitalWrite(headLED, LOW);
  }
}

void melodySup(){
  // notes in the melody:
  int melody[] = {
    NOTE_B4, NOTE_A4, NOTE_B4              };
  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  int noteDurations[] = {
    8,10,8              };
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 3; thisNote++){
    digitalWrite(headLED, HIGH);
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(10, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(10);
    digitalWrite(headLED, LOW);
  }
}

void melodyDanger(){
  // notes in the melody:
  int melody[] = {
    NOTE_A6, NOTE_A6, NOTE_C7              };
  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  int noteDurations[] = {
    10,10,6              };
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 3; thisNote++){
    digitalWrite(headLED, HIGH);
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(10, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(10);
    digitalWrite(headLED, LOW);
  }
}

















