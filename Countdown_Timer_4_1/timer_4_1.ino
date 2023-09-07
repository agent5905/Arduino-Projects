/* Stopwatch for the LED-5604
   by: Daniel Garcia
   date: Jun 6, 2020
   license: MIT

   This code shows how you could use the Arduino SPI 
   library to interface with the LED-5604

   There are functions for clearing the display.

   The values for each segment are defined in the arrays.
   In order to display a decimal you must call the decimal function.
    
   The SPI.transfer() function is used to send a byte of the
   SPI wires. Notice that each SPI transfer(s) is prefaced by
   writing the SS pin LOW and closed by writing it HIGH.

   Each of the custom functions handle the ssPin writes as well
   as the SPI.transfer()'s.

   4.1 - Changed default hour interval to 1 min. Added a new debounce delay for
         Up/Down that is fasster. This allows the time to be set quicker without
         allowing accidental button press for the other buttons.
   4.0 - Changed the display to minutes when there is only one hour left
         in the countdown.
         Holding up/down buttons for 1.5 seconds increses the amount at
         which the setting moves.
         Fixed - Timer code now stops counting down when the timer reaches
         zero.
   3.1 - Added the output for the buzzer and light when the time reaches zero.
   3.0 - Production version, Cleaned up code.
   
   Circuit:
   Arduino -------------- Serial 7-Segment
     5V   --------------------  VCC
     GND  --------------------  GND
      8   --------------------  SS
     11   --------------------  SDI
     13   --------------------  SCK

   Buttons:
    select --------------------- 2
    up     --------------------- 3
    down   --------------------- 4
    start/stop ----------------- 5

   Mode:
    timer ---------------------- 0
    duration menu -------------- 1
    adjustment menu //deprecated 2
    minute setting ------------- 3
    hour setting --------------- 4
    interval adjustment //depricated 5
    adjust minute -------------- 6
    adjust hour ---------------- 7
   
*/
#include <SPI.h> // Include the Arduino SPI library

// Define the SS pin
//  This is the only pin we can move around to any available
//  digital pin.
const int ssPin = 8;
//Buttons
const int selectBtnPin = 2;
const int upBtnPin = 3;
const int downBtnPin = 4;
const int startStpBtnPin = 5;
const int outPin = 6;
//pin 6 is for the led.
//Character Codes
const char alaphabetLC [] =      {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
const uint8_t alaphabetCode [] = {136,131,167,161,134,142,144,137,249,225,127,199,200,171,192,140,152,175,146,127,193,127,127,127,145,164};
const char alaphabetUC [] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
const char specialChars [] = {'.','=','-','_','"',' '};
const uint8_t specialCharsCode [] = {127,183,191,247,221,255};
const char numberChars [] = {'0','1','2','3','4','5','6','7','8','9'};
const uint8_t num [] = {192,249,164,176,153,146,131,248,128,152};
const int alaphabetCharCount = 25;
const int specialCharCount = 5;

unsigned long defaultTimer = 1200; //60 = 1 min
unsigned long defaultSetInterval = 60; 
unsigned long defaultHourTimer = 720; //1 = 1 min
unsigned long defaultSetHourInterval = 1; 
unsigned long timer = 1200;

unsigned int seconds = 0;
int minutes = 0;
int outToLCD = 0;

bool isBlinkOn = false;  //This checks if the blink is running.
const long defaultInterval = 1000; //1000 <- Change this to adjust the clock speed 997
const long defaultHourInterval = 60 * defaultInterval; //1000 <- Change this to adjust the clock speed
long currentInterval = defaultInterval; //We use two timing methods so this sets the current one beeeing used.
bool isRunning = false;
bool blinkMode = false;
bool decimalOn = true;
unsigned int blinkCount = 0;

unsigned long previousMillis = 0;
unsigned long blinkMillis = 0;  //Blink millis count. This needs its own because we set it to zero.
unsigned long holdTimeMilis = 0;
unsigned long holdDownTimeMilis = 0;
unsigned long holdUpTimeMilis = 0;

byte mode = 0;
int selectBtnState = 0;
int upBtnState = 0;
int downBtnState = 0;
int startStpBtnState = 0;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelaySelectStart = 500;    // This is the delay between button checks    
long debounceDelayUpDown = 200;
bool set = 1;

void setup()
{
  Serial.begin(9600);
  //buttons
  pinMode(selectBtnPin, INPUT);
  pinMode(upBtnPin, INPUT);
  pinMode(downBtnPin, INPUT);
  pinMode(startStpBtnPin, INPUT);
  pinMode(outPin, OUTPUT);
  // -------- SPI initialization
  pinMode(ssPin, OUTPUT);  // Set the SS pin as an output
  digitalWrite(ssPin, HIGH);  // Set the SS pin HIGH
  SPI.begin();  // Begin SPI hardware
  clearDisplaySPI();  
  s7sSendStringSPI("on");
  delay(500);
  clearDisplaySPI();
  minutes = timer/60;
  seconds = ((timer)-(minutes*60));
  outToLCD = (minutes * 100) + seconds;
  setValueTime(outToLCD);
  delay(500);
}

void loop()
{
  //Get the buttons state.
  selectBtnState = digitalRead(selectBtnPin);
  upBtnState = digitalRead(upBtnPin);
  downBtnState = digitalRead(downBtnPin);
  startStpBtnState = digitalRead(startStpBtnPin);
  int timeCheck = 0;
  //mode 1 = duration menu
  //mode 2 = adjustment menu //deprecated
  //mode 3 = minute setting
  //mode 4 = hour setting
  //mode 5 = interval adjustment //depricated
  //mode 6 = minute
  //mode 7 = hour

  if ( (millis() - lastDebounceTime) > debounceDelaySelectStart) {
    if (startStpBtnState == HIGH) 
    {
      if(mode == 0)
      {
        if(isRunning)
        {
          isRunning = false;
          digitalWrite(outPin, LOW);
        }else{
          isRunning = true;
          previousMillis =  millis();
        }
      
        if(holdTimeMilis == 0)
        {
          holdTimeMilis  = millis();
        }else if((millis() - holdTimeMilis) >= 2000)
        {
          isRunning = false;
          timer = defaultTimer;
          blinkCount = 0;
          mode = 255; 
          set = 0;
        }
      }
      lastDebounceTime = millis(); //set the current time
    }else{
      holdTimeMilis  = 0;
    }
    
    if (selectBtnState == HIGH) {
       switch(mode)
       {
        case 0:
          isRunning = false;
          mode = 3; 
          set = 0;
          break;
        case 1:
          mode = 3; 
          set = 0;
          break;
        case 2:
          mode = 5; 
          set = 0;
          break;
        case 3:
          mode = 6; 
          set = 0;
          break;
        case 4:
          mode = 7; 
          set = 0;
          break;
        case 5:
          break;
        case 6:
          timer = defaultTimer;
          currentInterval = defaultInterval;
          blinkCount = 0;
          mode = 255; 
          blinkMode = false;
          break;
        case 7:
          timer = defaultHourTimer;
          currentInterval = defaultHourInterval;
          blinkCount = 0;
          mode = 255;
          blinkMode = true;
          break;
      }
      lastDebounceTime = millis();
    }
    
  }

if ( (millis() - lastDebounceTime) > debounceDelayUpDown) {
  if (upBtnState == HIGH) {
      switch(mode)
       {
          case 1:
            mode = 2; 
            break;
          case 2:
            mode = 1; 
            break;
          case 3:
            mode = 4;
            break;
          case 4:
            mode = 3;
            break;
          case 6:
            defaultTimer += defaultSetInterval;
            if(defaultTimer > 5400) {
              defaultTimer = 5400;
            }
            break;
           case 7:
            defaultHourTimer += defaultSetHourInterval;
            if(defaultHourTimer > 2880) {
              defaultHourTimer = 2880;
            }
            break;
       }
       set = 0;
       lastDebounceTime = millis();
       if(holdUpTimeMilis == 0){
          holdUpTimeMilis  = millis();
       }else if((millis() - holdUpTimeMilis) >= 1200){
          defaultSetInterval = 300; 
          defaultSetHourInterval = 30;
       }
    }else if(downBtnState != HIGH){
      holdUpTimeMilis = 0;
      defaultSetInterval = 60; 
      defaultSetHourInterval = 1;
    }

    if (downBtnState == HIGH) {
      switch(mode)
       {
          case 1:
            mode = 2; 
            break;
          case 2:
            mode = 1; 
            break;
          case 3:
            mode = 4;
            break;
          case 4:
            mode = 3;
            break;
          case 6:
            timeCheck = defaultTimer - defaultSetInterval;
            if(timeCheck <= 0) {
              defaultTimer = defaultSetInterval;
            }
            defaultTimer -= defaultSetInterval;
            break;
           case 7:
            timeCheck = defaultHourTimer - defaultSetHourInterval;
            if(timeCheck <= 10) {
              defaultHourTimer = 10 + defaultSetHourInterval;
            }
            defaultHourTimer -= defaultSetHourInterval;
            break;
       }
       set = 0;
       lastDebounceTime = millis();
      if(holdDownTimeMilis == 0){
          holdDownTimeMilis  = millis();
       }else if((millis() - holdDownTimeMilis) >= 1200){
          defaultSetInterval = 300; 
          defaultSetHourInterval = 30;
       }
    }else if(upBtnState != HIGH){
      holdDownTimeMilis = 0;
      defaultSetInterval = 60; 
      defaultSetHourInterval = 1;
    }
}


  if(mode == 0)
  {
    if(isRunning){
      spiTimer();
    }
  }else if(set == 0)
  {
    switch(mode)
    {
      case 1:
        clearDisplaySPI();  
        s7sSendStringSPI("DUR");
        break;
      case 2:
        clearDisplaySPI();  
        s7sSendStringSPI("ADJ");
        break;
      case 3:
        clearDisplaySPI();  
        s7sSendStringSPI(" .60");
        break;
      case 4:
        clearDisplaySPI();  
        s7sSendStringSPI("HOUA");
        break;
      case 5:
        clearDisplaySPI();  
        s7sSendStringSPI("0000");
        break;
      case 6:
        calculateTime(defaultTimer);
        break;
      case 7:
        clearDisplaySPI();  
        calculateTime(defaultHourTimer);
        break;
    }
    set = 1;
  }else if(mode == 255)
  {
    blinkTime(4);
  }

  
}

void blinkTime(int blinkMax){
  minutes = timer/60;
  seconds = ((timer)-(minutes*60));  
  outToLCD = (minutes * 100) + seconds;
  unsigned long currentMillis = millis();
    
  if (currentMillis - blinkMillis >= 300) {
      if(isBlinkOn == false){
        setValueTime(outToLCD);
        isBlinkOn = true;
        blinkCount += 1;
      }else{
        clearDisplaySPI();
        isBlinkOn = false;
      }
      blinkMillis = currentMillis;
   }
    
  if(blinkCount == blinkMax){
    mode = 0;
    blinkMillis = 0;
  }

}

void spiTimer()
{
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= currentInterval) {
    previousMillis = currentMillis;
    timer--;
    
    if(currentInterval == defaultHourInterval && timer == 60){
      timer = 3600;
      currentInterval = defaultInterval;
      blinkMode = false;
    }

    if(blinkMode){
      if(!decimalOn){
        decimalOn = true;
        calculateTime(timer);
      }else{
        decimalOn = false;
        calculateTimeNoDecimal(timer);
      }
    }else{
      calculateTime(timer);
    }
    
    if(timer <= 0){
      digitalWrite(outPin, HIGH);
      isRunning = false;
    }
  }else if(blinkMode){
    if (currentMillis - blinkMillis >= 500) {
      blinkMillis = currentMillis;
      if(!decimalOn){
        decimalOn = true;
        calculateTime(timer);
      }else{
        decimalOn = false;
        calculateTimeNoDecimal(timer);
      }
    }
  }
}

void calculateTime(unsigned long timer)
{
    minutes = timer/60;
    seconds = ((timer)-(minutes*60));
    clearDisplaySPI();  
    outToLCD = (minutes * 100) + seconds;
    setValueTime(outToLCD);
}

void calculateTimeNoDecimal(unsigned long timer)
{
    minutes = timer/60;
    seconds = ((timer)-(minutes*60));
    clearDisplaySPI();  
    outToLCD = (minutes * 100) + seconds;
    setValueTimeNoDecimal(outToLCD);
}
void sendSPICode(int value)
{
  digitalWrite(ssPin, LOW);
  SPI.transfer(value);
  digitalWrite(ssPin, HIGH);
}


// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringSPI(String toSend)
{
  for (int i=4; i>=0; i--)
  {
    sendSPICode(getCharCode(toSend[i]));
  }
}


//This functions converts a character to the correcct command value
int getCharCode(char character)
{
  int code = 255;
  for (int k = 0; k <= alaphabetCharCount; k++)
  {
    
    if(character == alaphabetLC[k] || character == alaphabetUC[k]){
      code = alaphabetCode[k];
      return code;
    }
  }
  for (int j = 0; j <= specialCharCount; j++)
  {
    if(character == specialChars[j]){
      code = specialCharsCode[j];
      return code;
    }
  }
  for (int l = 0; l <= 9; l++)
  {
    if(character == numberChars[l]){
      code = num[l];
      return code;
    }
  }
  return code;
}


//  This will clear the display and reset the cursor
void clearDisplaySPI()
{
  digitalWrite(ssPin, LOW);
  SPI.transfer(255);  // Clear display command
  digitalWrite(ssPin, HIGH);
  digitalWrite(ssPin, LOW);
  SPI.transfer(255);  // Clear display command
  digitalWrite(ssPin, HIGH);
  digitalWrite(ssPin, LOW);
  SPI.transfer(255);  // Clear display command
  digitalWrite(ssPin, HIGH);
  digitalWrite(ssPin, LOW);
  SPI.transfer(255);  // Clear display command
  digitalWrite(ssPin, HIGH); 
}

void setAsDecimal(int value)
{
  digitalWrite(ssPin, LOW);
  SPI.transfer(num[value] - 128); //The decimal value for the segment is just the difference of 128.
  digitalWrite(ssPin, HIGH);
}

void setValueUnit(int value)
{
  digitalWrite(ssPin, LOW);
  SPI.transfer(num[value]);
  digitalWrite(ssPin, HIGH);
}

void setValue(int value)
{
  digitalWrite(ssPin, LOW);
  while(value > 0){
    int digit = value % 10;
    setValueUnit(digit);
    value /= 10;
  }
  digitalWrite(ssPin, HIGH);
}

void setValueTime(int value)
{
  int pos = 0;
  int tempVal = value;
  
  while(value > 0){
    int digit = value % 10;
    if(pos == 2){
      setAsDecimal(digit);
    }else{
      setValueUnit(digit);
    }
    value /= 10;
    pos++;
  }
  
  if(tempVal < 1000 && tempVal >= 100){
     setValueUnit(0);
  }else if(tempVal < 100 && tempVal >= 10){
     setAsDecimal(0);
     setValueUnit(0);
  }else if(tempVal < 10 && tempVal > 0){
     setValueUnit(0);
     setAsDecimal(0);
     setValueUnit(0);
  }else if (tempVal <= 0){
     setValueUnit(0);
     setValueUnit(0);
     setAsDecimal(0);
     setValueUnit(0);
  }
}

void setValueTimeNoDecimal(int value)
{
  int tempVal = value;
  
  while(value > 0){
    int digit = value % 10;
    setValueUnit(digit);
    value /= 10;
  }
  
  if(tempVal < 1000 && tempVal >= 100){
     setValueUnit(0);
  }else if(tempVal < 100 && tempVal >= 10){
     setValueUnit(0);
     setValueUnit(0);
  }else if(tempVal < 10 && tempVal > 0){
     setValueUnit(0);
     setValueUnit(0);
     setValueUnit(0);
  }else if (tempVal <= 0){
     setValueUnit(0);
     setValueUnit(0);
     setValueUnit(0);
     setValueUnit(0);
  }
}
