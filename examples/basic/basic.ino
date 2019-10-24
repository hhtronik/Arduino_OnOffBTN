/**
    @file     basic.ino
    @author   Yannic Staudt (HHTronik)
    @license  BSD (see licence.txt)
    
    Basic usage example for the ÖnÖffBTN with an Arduino

    How-to (for an Arduino UNO):

    - program your Arduino with this sketch
    - connect the power supply of your Arduino to the power-output of the ÖnÖffBTN (easier using the USB-addon board ;)
    - connect the I2C bus (I2C_SDA to A4, I2C_SCL to A5)
    - connect the INT pin to Pin2
    - connect the Latch pin to Pin4

    Visit https://hhtronik.com for more information
*/


#include "hhtronik_onoffbtn.h"

HHTronik_OnOffBTN btn = HHTronik_OnOffBTN();
OnOffBTN_StatusRegister status;

// loop status variables
bool interruptReceived = false;
bool ledState = false;

void setup() 
{
  Serial.begin(115200);
  btn.begin();
  
  // make sure the framebuffer is empty
  btn.clearFramebuffer();

  // fill the led ring with Red/Green/Blue pixels
  uint8_t i = 0;
  while(i < 9)
  {
    btn.setPixel(i++, 255, 0, 0);
    btn.setPixel(i++, 0, 255, 0);
    btn.setPixel(i++, 0, 0, 255);
  }

  // set the long press to be 1000ms
  btn.setLongPressThreshold(1000);

  // set the delay between a latch event and the mosfet
  // actually turning off to 500ms
  btn.setOffDelay(500);

  // get a clean configuration
  btn.setPowerBehaviorConfiguration({
    .PoR_DefaultOn = true,      // this makes sure that the button switches on our Arduino when it is power-reset
    .PoR_RestoreFramebuffer = false,
    .AutoLatchOnOnPress = true, // enables switching ON our arduino when the power supply is cut
    .AutoLatchOnOffPress = false
  });

  // set the "ON" state animation to be "Flash"
  btn.selectAnimation(PowerOn, Animation_Flash);
  btn.setAnimationSpeed(PowerOn, 100);

  //  set the "ON" state animation to be "rainbow"
  btn.selectAnimation(PowerOff, Animation_Breath);
  btn.setAnimationSpeed(PowerOff, 3);

  /**
   * setup the GPIOs
   */

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ledState);
  
  // Pin 2 for INT
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), handleBtnInterrupt, RISING);  // wait for the rising edge...

  // Pin 4 for Latch
  digitalWrite(4, HIGH);
  pinMode(4, OUTPUT);  
}

void loop() 
{
  if(interruptReceived)
  {
    interruptReceived = false;
    status = btn.getButtonStatus();

    if(status.Down)
      Serial.println("Button: down");
      
    if(status.DoubleClick)
      Serial.println("Button: double click");

    if(status.ShortPress) 
    {
      Serial.println("Button: short press");
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }

    if(status.LongPress)
    {
      // ... do some shutdown action ;)
      delay(100); 

      // pulse the Latch pin to toggle the button
      digitalWrite(4, LOW);
      delay(1); 
      digitalWrite(4, HIGH);

      // now we still have ~500ms (OffDelay) left to do important stuff 
      // before the power gets cut...
      while(1) 
      {
         Serial.println("Waiting for shutdown...");
         delay(250);
      } 
    }
  }

  delay(50);  
}

void handleBtnInterrupt() 
{
  interruptReceived = true;
}