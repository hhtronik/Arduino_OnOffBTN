ÖnÖffBTN Arduino driver 
=======================

This is the Arduino driver for HHTronik's ÖnÖffBTN smart switch / HMI.

How to install
--------------

For now you'll have to download and install the ZIP. We'll add this driver to the Arduino Library manager asap!
Follow this guide if you don't know how: https://www.arduino.cc/en/Guide/Libraries

What's the ÖnÖffBTN?
--------------------

The ÖnÖffBTN is a I2C addressable digital power switch with an integrated RGB led ring and a tactile button.
It enables you to add soft-button functionality to your projects by enabling attached systems (for example an
Arduino or as RaspberryPi) to control their power supply.

A typical flow might be:

1. User pushes the button
2. ÖnÖffBTN sends an interrupt pulse to system
3. System initiates shutdown
4. ...when everything important is safe for power off...
5. System tells the ÖnÖffBTN to latch
6. ÖnÖffBTN cuts the power supply after a configurable delay 

Here's the features list:

- Load switch: 20mOhm MOSFET for up to 8A, 12V max.
- Supply voltage: 3.3V - 13V
- Configurable via I2C (Fast Mode / 400kHz)
- Max. 4 Pins on host side (considering we already have a common ground)
    - 2 pins for I2C 
    - 1 `INT`(errupt) pin pulsed low by the ÖnÖffBTN on events
    - 1 `Latch` pin to trigger a latch (pulse low)
- No Host-MCU required, simply connect `INT` and `Latch` with a jumper
- HMI offloading
    - 9 addressable RGB LEDs
    - host-CPU independent LED driving and animations (`Breath`, `Spinner`, `Flash`, `Flicker`, `Spooky`, `Rainbow`, `LinearFade`)
    - two persistent framebuffer and animation configurations that can be automatically restored on power switch events
    - comfortable button handling (debounced, states accessible via I2C: `single click`, `double click`, `long press`, `button down`)
- in-built RTC (with the possiblity to connect a backup battery)
    - for time-keeping
    - alarms (single shot or auto-rearming)
    - can trigger on time, date and day basis, with maskable components (`every 1 minute at the second 42`, `every day at 8:00:00` or `every Monday at 10:15:32`)
    - with programmable alarms and actions (wake by alarm, shutdown by alarm, toggle by alarm or reset by alarm)
    - alarm actions can be cancelled by attached system
- 16 bytes of User-EEPROM
- Latching can be triggered (and cancelled) via either I2C or pulsing the latch pin
- Configurable hard-reset function (hard off, power reset with configurable off-time, can be disabled via configuration)
- Plenty of configuration options:
    - Default power-on-reset behavior (automatically switch on or stay off, restore framebuffer & animation)
    - Delays for latching events (time between latch event and actually toggling power)
    - and more...

Usage example
-------------

The driver relies on the native Arduino `Wire` library, so all its limitations apply.
On an `Arduino Genuino / UNO` the I2C pins are `A4/SDA` and `A5/SCL`. In this example we're going to use pin `2` as interrupt pin and pin `4` for the latch pin.

The example below shows how to do a basic usage of the ÖnÖffBTN:
- fill the framebuffer with some pixel data
- set the `long press threshold` to 1000ms
- set the `off delay` to 500ms
- enable switching the power supply on without interaction from the attached system (which is required if you cut the power supply to the Arduino using the ÖnÖffBTN)
- set the animation for the "on" state to "flash"
- set the animation for the "off" state to "breath"

When the Arduino receives an interrupt from the ÖnÖffBTN it retrieves the `button status register` to evaluate what caused the interrupt
- when `button down` is `true` we output "Button: down" via `Serial.println()`
- when `button doubleClick` is `true` we output "Button: double click" via `Serial.println()`
- when `button shortPress` is `true` we toggle the inbuilt LED and write "Button: short press" to serial
- when `button longPress` is `true` (which happens when the user presses the button for longer than `long press threshold`) we trigger the ÖnÖffBTN to latch the power


```c++
#include "hhtronik_onoffbtn.h"

HHTronik_OnOffBTN btn = HHTronik_OnOffBTN();
OnOffBTN_StatusRegister status;

// loop status variables
bool interruptReceived = false;
bool ledState = false;

void setup() 
{
  Serial.begin(9600);
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
```

Compatibility
-------------

The driver was successfully tested with:

- Arduino Uno
- Arduino Duemilanove
- Arduino Mega 2560
- Sparkfun Pro-Micro (ATmega32U4)