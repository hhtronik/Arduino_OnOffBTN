/**
    @file     rtcAlarm.ino
    @author   Yannic Staudt (HHTronik)
    @license  BSD (see licence.txt)
    
    RTC usage example for the ÖnÖffBTN with an Arduino.

    Configures the RTC to toggle the power to the attached Arduino every minute when 
    the second-counter of the RTC is equal to 30. 

    When the user clicks the button ("Short press") we toggle the on-board LED of the
    Arduino. When the LED is on, we consider that there's some important activity 
    running (hey, our led's on! Ain't switching off then! ;) and we cancel alarm 
    events if they occur. 

    You will see the displayed animation change from "flashing" to "breath" and back 
    on power state change.

    How-to (for an Arduino UNO):

    - program your Arduino with this sketch
    - connect the power supply of your Arduino to the power-output of the ÖnÖffBTN
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
  
  // fill the led ring with Red pixels
  uint8_t i = 0;
  while(i < 9) btn.setPixel(i++, 255, 0, 0);

  OnOffBTN_RTCControlRegister currentConfig = btn.getRTCConfiguration();

  // we didn't run this example probably... so let's configure the RTC + alarm
  if(currentConfig.AlarmEnabled == false)
  {

    // set an arbitrary time
    OnOffBTN_DateTime now;
    now.Hours = 19;
    now.Minutes = 0;
    now.Seconds = 0;
    now.DayOfMonth = 23;
    now.Month = 10;
    now.Year = 19;
    now.DayOfWeek = 3;    // the 23rd of October 2019 was a Wednesday, Monday being 1


    btn.setDateTime(now);

    // we set the alarm to trigger every time
    // the second counter is == 30
    // (note the masking of the Hour and Minute components)
    btn.setAlarmTime({
      .Seconds = 30,
      .Minutes = 0,
      .Hours = 0,    
      .MaskSeconds = false,
      .MaskMinutes = true,    // <- masked!
      .MaskHours = true       // <- masked!
    });

    // we want the alarm to happend on every day 
    // so we must mask this component too
    //
    // you could also set a date of month or day of Week
    // as value an set the IsWeekDayAlarm property accordingly
    btn.setAlarmDayDate({
      .Value = 0,
      .IsWeekDayAlarm = false,
      .DayDateMasked = true   // <- masked
    });

    // finally, we want to 
    // 1/ enable the alarm
    // 2/ toggle the power everytime the alarm is triggered
    // 3/ auto-rearm the alarm
    // 4/ leave the attached system 100ms to cancel the alarm
    // 5/ give this arduino 1000ms to react to the alarm before the power gets cut!
    btn.setRTCConfiguration({
      .AlarmEnabled = true,
      .AlarmAction = RTCAlarm_Toggle,
      .AlarmAutoRearm = true,
      .UseAmPmFormat = false,
      .AlarmCancelationDelay = delay1000ms
    });

  }

  // get a clean configuration
  // we don't want to handle the interrupt pins,
  // so we make sure we don't "software latch" at all
  btn.setPowerBehaviorConfiguration({
    .PoR_DefaultOn = true,
    .PoR_RestoreFramebuffer = false,
    .AutoLatchOnOnPress = false,      
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

    if(status.ShortPress) 
    {
      Serial.println("Button: short press");
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }

    // if we have an RTC alarm, we want to make the actual shutdown
    // depend on the value of ledState
    if(status.RTC_Alarm)
    {
      // if the LED is on, we want to keep running!
      if(ledState)
      {
        Serial.println("RTC Alarm: cancel");
        // pulse the Latch pin to abort the power toggle
        digitalWrite(4, LOW);
        delay(10); 
        digitalWrite(4, HIGH);
      }
      else
      { 
        /* THIS would be the place for some shutdown action */

        while(1) 
        {
          Serial.println("RTC Alarm: Waiting for shutdown...");
          delay(250);
        } 
      }
    }    
  }

  delay(10);  
}

void handleBtnInterrupt() 
{
  interruptReceived = true;
}