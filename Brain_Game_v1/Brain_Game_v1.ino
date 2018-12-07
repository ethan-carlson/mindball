/*
 Several EEG visualizers and brain game sketch
 for use with MindWave Mobile and accomplying
 Processing code.
 
 by: Ethan Carlson
 date: December 7, 2018
 license: Public domain

Game 1:  An EEG visualizer.  Takes the game value
from the Processing code and turns a 16x16 Neopixel
LED panel green if the user is in a meditative state
or red if the user is not.

Game 2:  An EEG visualizer with progress meter.
Performs the same function as Game 1, except the
visualizer is just the bottom bar of the panel and 
the rest of the panel is used as a progress meter.
A cursor binks and advances one pixel if the user
maintains a meditative state for the gamePace.

Game 3:  Mindball.  This optionally includes the EEG visualizer
from game 1, but the progress meter becomes a steel
ball on a magnetic track that moves towards or away
from the player depending on whether they meet a threshold.

This code could be modified to include a Game 4, which
would be the two-player version of Mindball where the
ball moves based on which player is in a more meditative
state.
 */

#include <SoftwareSerial.h>  
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//16x16 neopixel display
#define PIN 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);

//Game One Variables, simply an EEG visualizer
int neopixelLEDs = 256;
int oldRedValue = 0;
int oldGreenValue = 0;
int redValue = 0;
int greenValue = 0;
int brightnessMult = 2;  //Raise or lower to change the brightness value amplification
int fadeSteps = 8;  //Keeps the changes in LED color from being choppy
int maxBrightness = 50;  //The panel gets quite bright even at this level.  Max is 256.

//Game Two variables, an EEG visualizer with a progress meter for meditation feedback.
int cursorPosition = 0;
int gameTwoAdvanceThresh = 102;  //Game value at which the game progresses
int gamePace = 100;

//Game Three variables
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6
char user_input;
int x;
int y;
int state;
int gameThreeCloserThresh = 102;  //Value at which the ball moves closer to the player
int gameThreeFurtherThresh = 101;  //Value at which the ball moves farther away
bool calibrated = false;

//Change these to run various game scripts
bool testing = false;
bool gameOne = true;
bool gameTwo = false;
bool gameThree = true;

int packet; // variable to receive data from the serial port
int packetValue;

//Used for testing script
int numLEDs = 5;
int ledArray[5];

//Used to compute moving averge of game value to prevent jumpiness
int packetArray[5] = {150, 150, 150, 150, 150};
int packetCounter = 0;
float packetSum = 750.0;
int gameValue = 150;

int bluetoothTx = 8;  // TX-O pin of bluetooth mate
int bluetoothRx = 9;  // RX-I pin of bluetooth mate

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup() {

  if(testing){
    for(int i=0;i<numLEDs;i++){
      pinMode((i+2), OUTPUT);
    }
  }

  if(gameThree){
    pinMode(stp, OUTPUT);
    pinMode(dir, OUTPUT);
    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(EN, OUTPUT);
    resetEDPins(); //Set step, direction, microstep and enable pins to default states
  }
  
  delay(1000);
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(1000);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600

  strip.begin();  //initialize neopixel as clear
  clearStrip();
  strip.show();

  Serial.begin(9600);  //for debug
  delay(1000);
  Serial.println("Begin");
}


void loop() {
  if(bluetooth.available()){ // if data is available to read
    Serial.println("Data available");
    packet = bluetooth.read(); // read it and store it in 'packet'
    packetValue = int(packet);  //convert to an integer

    //debug
    Serial.println(packet);
    Serial.println(gameValue);
    //Serial.println(redValue);
    //Serial.println(greenValue);

    if(testing){
      Serial.println("testing");
      testingScript();
    }
    if(gameOne){
      Serial.println("Playing Game 1");
      gameOneScript();
    }
    if(gameTwo){
      Serial.println("Playing Game 2");
      gameTwoScript();
    }
    if(gameThree){
      Serial.println("Playing Game 3");
      gameThreeCalibrate();  //See calibration script.  No feedback on track position means you need to set the ball position manually.
      gameThreeScript();
    }
  }
  delay(10);
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

// 1/8th microstep foward mode function
void SmallStep()
{
  Serial.println("Stepping at 1/8th microstep mode.");
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for(x= 1; x<575; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

//Reverse default microstep mode function
void ReverseSmallStep()
{
  Serial.println("Moving in reverse at 1/8th microstep mode.");
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for(x= 1; x<575; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

void clearStrip(){    //Set all LEDs off
  for(int i=0;i<neopixelLEDs;i++){
    strip.setPixelColor(i,0);
  }
  strip.show();
}

void computeBrightness(){
  
  packetArray[packetCounter] = packetValue;  //store the value in the relevant array positiong
  packetSum = 0;  //reset sum value
  for(int i=0;i<5;i++){
    packetSum += packetArray[i];  //sum all current values
  }
  gameValue = int(packetSum/5);  //compute moving average
  
  redValue = brightnessMult * (110 - gameValue);  //compute red brightness
  if(redValue < 0){
    redValue = 0;
  }
  if(redValue > maxBrightness){
    redValue = maxBrightness;
  }

  greenValue = brightnessMult * (gameValue - 90);  //compute green brightness
  if(greenValue < 0){
    greenValue = 0;
  }
  if(greenValue > maxBrightness){
    greenValue = maxBrightness;
  }
}

void fade(int fadeMode){  //fades either the whole panel or just the bottom bar of the LED array
 
  float redStep = (redValue - oldRedValue)/fadeSteps;
  float greenStep = (greenValue - oldGreenValue)/fadeSteps;
  int redFade;
  int greenFade;


  if(fadeMode < 2){
    Serial.println("Fading whole");
    for(int j=1;j<=fadeSteps;j++){
      redFade = int((redStep*j) + oldRedValue);
      greenFade = int((greenStep*j) + oldGreenValue);
      for(int i=0;i<neopixelLEDs;i++){
        strip.setPixelColor(i,redFade,greenFade,0);
      }
      strip.show();
      delay(25);
    }
  }

  else if(fadeMode >= 2){
    Serial.println("Fading bottom");
    for(int j=1;j<=fadeSteps;j++){
      redFade = int((redStep*j) + oldRedValue);
      greenFade = int((greenStep*j) + oldGreenValue);
      for(int i=0;i<16;i++){
        strip.setPixelColor((i+240),redFade,greenFade,0);
      }
      strip.show();
      delay(25);
    }
  }
}

void testingScript(){

  packetArray[packetCounter] = packetValue;  //store the value in the relevant array positiong

  packetSum = 0;  //reset sum value
  for(int i=0;i<5;i++){
    packetSum += packetArray[i];  //sum all current values
  }
  gameValue = int(packetSum/5);  //compute moving average
  
  switch (gameValue){  //turn on LEDs based on how relaxed the player is
    case 0 ... 90:
      digitalWrite(3,LOW);
      digitalWrite(4,LOW);
      digitalWrite(5,LOW);
      digitalWrite(6,LOW);
      digitalWrite(7,LOW);
      break;
    case 91 ... 95:
      digitalWrite(3,HIGH);
      digitalWrite(4,LOW);
      digitalWrite(5,LOW);
      digitalWrite(6,LOW);
      digitalWrite(7,LOW);
      break;
    case 96 ... 100:
      digitalWrite(3,HIGH);
      digitalWrite(4,HIGH);
      digitalWrite(5,LOW);
      digitalWrite(6,LOW);
      digitalWrite(7,LOW);
      break;
    case 101 ... 105:
      digitalWrite(3,HIGH);
      digitalWrite(4,HIGH);
      digitalWrite(5,HIGH);
      digitalWrite(6,LOW);
      digitalWrite(7,LOW);
      break;
    case 106 ... 110:
      digitalWrite(3,HIGH);
      digitalWrite(4,HIGH);
      digitalWrite(5,HIGH);
      digitalWrite(6,HIGH);
      digitalWrite(7,LOW);
      break;
    case 111 ... 150:
      digitalWrite(3,HIGH);
      digitalWrite(4,HIGH);
      digitalWrite(5,HIGH);
      digitalWrite(6,HIGH);
      digitalWrite(7,HIGH);
      break;
  }

  if(packetCounter<4){  //cycle through the array positions
    packetCounter++;  
  }
  else{
    packetCounter = 0;
  }
}

void gameOneScript(){

  computeBrightness();
  fade(1);  //update whole sheet

  oldRedValue = redValue;  //Reset the color values
  oldGreenValue = greenValue;

  if(packetCounter<4){  //cycle through the array positions
    packetCounter++;  
  }
  else{
    packetCounter = 0;
  }
}

void gameTwoScript(){

  computeBrightness();
  fade(2);  //update bottom strip

  strip.setPixelColor(cursorPosition,0,25,0);  //blink the cursor position
  strip.show();
  delay(gamePace);
  strip.setPixelColor(cursorPosition,0,0,0);
  strip.show();

  if (gameValue > gameTwoAdvanceThresh){  //Advance the cursor if applicable
    strip.setPixelColor(cursorPosition,0,25,0);
    strip.show();
    cursorPosition++;
  }

  if (cursorPosition >= 239){  //Game win condition
    smiley();
  }

  oldRedValue = redValue;  //Reset the color values
  oldGreenValue = greenValue;

  if(packetCounter<4){  //cycle through the array positions
    packetCounter++;  
  }
  else{
    packetCounter = 0;
  }
}

void smiley(){  //Game win condition for Game 2
clearStrip();
strip.setPixelColor(89, 25, 25, 0);//Left eye
strip.setPixelColor(102, 25, 25, 0);

strip.setPixelColor(86, 25, 25, 0);//right eye
strip.setPixelColor(105, 25, 25, 0);

strip.setPixelColor(138, 25, 25, 0);//mouth
strip.setPixelColor(150, 25, 25, 0);
strip.setPixelColor(151, 25, 25, 0);
strip.setPixelColor(152, 25, 25, 0);
strip.setPixelColor(153, 25, 25, 0);
strip.setPixelColor(133, 25, 25, 0);
strip.show();
delay(1000000);  //Stay here forever
}

void gameThreeCalibrate(){  //Need to manually set the ball position with this version of the code
  
  while (!calibrated){  //Loop until calibrated
    bool yesNo = false;
    Serial.println("Is the ball in the starting position? y/n");

    while (!yesNo){  //Wait for answer
    
    while(Serial.available()){
      user_input = Serial.read();
      Serial.println(user_input);
      if (user_input == 'y'){
        Serial.println("Game 3 starting");
        calibrated  = true;
        yesNo = true;
      }
      if (user_input == 'n'){
        bool hold = true;
        Serial.println("input 1 for five steps forward, 2 for fifteen steps forward, 3 for fifty steps forward, 4 for back");
        
        while (hold){
          while(Serial.available()){
            user_input = Serial.read();
            digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
            if (user_input =='1'){  //Small step away (5 steps)
              for (int i=0;i<5;i++){
                SmallStep();  
              }
              yesNo = true;
              hold = false;
            }
            if (user_input =='2'){  //Medium step away (15 steps)
              for (int i=0;i<15;i++){
                SmallStep(); 
              }
              yesNo = true;
              hold = false;
            }
            if (user_input =='3'){  //Large step away (50 steps)
              for (int i=0;i<50;i++){
                SmallStep();
              }
              yesNo = true;
              hold = false;
            }
            if (user_input =='4'){  //Small step closer (5 steps)
              for (int i=0;i<5;i++){
                ReverseSmallStep();
              }
              yesNo = true;
              hold = false;
            }
            else{
              Serial.println("Invalid option entered.");
            }
            resetEDPins();
          }
        }
      }
      else{
        Serial.println("Invalid option entered.");
      }
      delay(10);  
    }
    delay(10);
    }
  }
}

void gameThreeScript(){

  gameValue = packetValue;
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control

  if (gameValue > gameThreeCloserThresh){  //Move closer to the player
    if (cursorPosition < 100){
      ReverseSmallStep();
    }
    cursorPosition++;
    if (cursorPosition >= 100){
      smiley();
    }
  }

  if (gameValue < gameThreeFurtherThresh){  //Move further from the player
    if (cursorPosition > 0){
      SmallStep();
    }
    cursorPosition--;
  }


  if(packetCounter<4){  //cycle through the array positions
    packetCounter++;  
  }
  else{
    packetCounter = 0;
  }
}

