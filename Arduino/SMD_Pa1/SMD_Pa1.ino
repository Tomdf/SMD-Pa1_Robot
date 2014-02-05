/* Code for the SMD-PA1 robot
 Tom Flock
 2-4-14
 
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

Servo myservo;  // create servo object to control a servo 

const int pingPin = 4;    // connection to ultrasonic sensor
int headLED = 9;
int pos = 0;    // variable to store the servo position

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_E3, NOTE_G3, 0, NOTE_F3, NOTE_A3, NOTE_A4, NOTE_B4};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  8, 10, 6, 8,10,8,8,10 };

void setup()
{
  Serial.begin(9600);
  myservo.attach(5);
  pinMode(headLED, OUTPUT);
  leds.begin();  // Call this to start up the LED strip.
  clearLEDs();   // This function, defined below, turns all LEDs off...
  setColor(PURPLE,1);

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

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
  }
}

void loop(){
  digitalWrite(headLED, HIGH);
  for(pos = 20; pos < 160; pos += 5)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);    // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  delay(500);
  digitalWrite(headLED, LOW);

  Serial.print("PostionRight = ");
  ping();
  Serial.println();

  delay(3000);

  digitalWrite(headLED, HIGH);
  for(pos = 160; pos>=25; pos-=5)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  }
  delay(500);
  digitalWrite(headLED, LOW);

  Serial.print("PostionLeft = ");
  ping();
  Serial.println();

  delay(3000);  
}

void setColor(unsigned long color, byte brightness)
{
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);

  for (int i=0; i<=LED_COUNT-1; i++)
  {
    leds.setPixelColor(i, red/brightness, green/brightness, blue/brightness);
  }
  leds.show();  // Turn the LEDs on
}

// Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs()
{
  for (int i=0; i<LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
    leds.show();
  }
}

void ping()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

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
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

  delay(100);
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}




