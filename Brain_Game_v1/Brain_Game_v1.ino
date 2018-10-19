#include <SoftwareSerial.h>  
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//16x16 neopixel display
#define PIN 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);
int neopixelLEDs = 256;
int oldRedValue = 0;
int oldGreenValue = 0;
int redValue = 0;
int greenValue = 0;
int brightnessMult = 2;
int fadeSteps = 8;
int maxBrightness = 50;

//Game Two variables
int cursorPosition = 0;
int gameTwoAdvanceThresh = 102;
int gamePace = 100;

bool testing = false;
bool gameOne = false;
bool gameTwo = true;

int packet; // variable to receive data from the serial port
int packetValue;

int ledpin = 13; // Arduino LED pin 13 (on-board LED)
int numLEDs = 5;
int ledArray[5];

int packetArray[5] = {150, 150, 150, 150, 150};
int packetCounter = 0;
float packetSum = 750.0;
int gameValue = 150;

int bluetoothTx = 8;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 9;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup() {

  if(testing){
    for(int i=0;i<numLEDs;i++){
      pinMode((i+2), OUTPUT);
    }
  }
  
  delay(1000);
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(1000);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600}

  strip.begin();  //initialize neopixel as clear
  clearStrip();
  strip.show();

  Serial.begin(9600);  //for debug
}


void loop() {
  if(bluetooth.available()){ // if data is available to read
    packet = bluetooth.read(); // read it and store it in 'packet'
    packetValue = int(packet);  //convert to an integer

    //debug
    Serial.println(packet);
    Serial.println(gameValue);
    Serial.println(redValue);
    Serial.println(greenValue);

    if(testing){
      testingScript();
      Serial.println("testing");
    }
    else if(gameOne){
      gameOneScript();
      Serial.println("Playing Game 1");
    }
    else if(gameTwo){
      gameTwoScript();
      Serial.println("Playing Game 2");
    }
  }
  delay(10);
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

void fade(int fadeMode){
 
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

  oldRedValue = redValue;
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

  if (gameValue > gameTwoAdvanceThresh){
    strip.setPixelColor(cursorPosition,0,25,0);
    strip.show();
    cursorPosition++;
  }

  if (cursorPosition >= 239){
    smiley();
  }

  oldRedValue = redValue;
  oldGreenValue = greenValue;

  if(packetCounter<4){  //cycle through the array positions
    packetCounter++;  
  }
  else{
    packetCounter = 0;
  }
}

void smiley(){
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
delay(1000000);
}

