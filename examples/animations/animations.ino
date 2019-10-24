/**
    @file     animations.ino
    @author   Yannic Staudt (HHTronik)
    @license  BSD (see licence.txt)
    
    Cycle through the different LED ring animations on button click.    

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
OnOffBTN_PowerState currentPowerState;

// loop status variables
bool interruptReceived = false;
uint8_t currentAnimation = 0;

void setup() 
{
  Serial.begin(115200);
  btn.begin();
  
  // fill the led ring with rainbow colors
  for(int i = 0; i < ONOFFBTN_NUM_PIXELS; i++)
  {
    if(i < 3)
    {
      btn.setPixel(i, i * 85, 255 - i * 85, 0);
    }
    else if(i < 6)
    {
      btn.setPixel(i, 255 - i * 85, 0, i * 85);
    }
    else
    {
      btn.setPixel(i, 0, i * 85, 255 - i * 85);
    }
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

  // disable RTC alarms
  btn.setRTCConfiguration({
    .AlarmEnabled = false,    // <--
    .AlarmAction = RTCAlarm_Toggle,
    .AlarmAutoRearm = true,
    .UseAmPmFormat = false,
    .AlarmCancelationDelay = delay1000ms
  });

  // set the "ON" state animation to be "Flash"
  btn.selectAnimation(PowerOn, Animation_None);
  

  /**
   * setup the GPIOs
   */
  
  // Pin 2 for INT
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), handleBtnInterrupt, RISING);  // wait for the rising edge...

  // Pin 4 for Latch
  digitalWrite(4, HIGH);
  pinMode(4, OUTPUT);  
}

void loop() 
{
  // block here until the user presses the button
  while(!interruptReceived)
  {
    delay(10);
  }

  interruptReceived = false;
  status = btn.getButtonStatus();

  // we're waiting for short presses
  if(!status.ShortPress)
    return;

  /**
   * Flow control:
   * select the next animation and get the current power state so we 
   * can set the correct animation configuration
   */
  currentAnimation++;
  currentPowerState = (status.PowerOn) ? PowerOn : PowerOff;


  switch ((OnOffBTN_Animation)currentAnimation)
  {
  case Animation_Breath:
    // the "breath" animation shows a full circle of the color of the
    // 1st pixel in the framebuffer and fades it in a pattern that resembles
    // what many i-Devices show in standby mode
    btn.selectAnimation(currentPowerState, Animation_Breath);

    // the animation contains  a relatively high number of frames, so we 
    // need to have less delay between frames for it to look fluid
    btn.setAnimationSpeed(currentPowerState, 1);  

    Serial.println("Animation: Breath");
    break;

  case Animation_Flash:
    // the "flash" animation flashes the full framebuffer at a 50% duty cycle
    // using the set animation speed as half-period
    btn.selectAnimation(currentPowerState, Animation_Flash);
    btn.setAnimationSpeed(currentPowerState, 100);  // this is ~100ms per step

    Serial.println("Animation: Flash");
    break;  

  case Animation_Flicker:
    // the "flicker" animation flashes the full framebuffer at random
    btn.selectAnimation(currentPowerState, Animation_Flicker);
    btn.setAnimationSpeed(currentPowerState, 50); 

    Serial.println("Animation: Flicker");
    break;  
  
  case Animation_Spooky:
    // the "spooky" animation is like "flicker" but at a pixel level
    // so instead of having the whole ring on or off in a given framebuffer
    // we apply the random on/off for each pixel on every frame.
    btn.selectAnimation(currentPowerState, Animation_Spooky);
    btn.setAnimationSpeed(currentPowerState, 10);  // this is ~10ms per step

    Serial.println("Animation: Spooky");
    break;
  
  case Animation_Spinner:
    // the "spinner" animation rotates the full framebuffer for one pixel every
    // frame, so shorter frame time == faster rotation.
    btn.selectAnimation(currentPowerState, Animation_Spinner);
    btn.setAnimationSpeed(currentPowerState, 40); 

    Serial.println("Animation: Spinner");
    break;
  
  case Animation_LinearFade:
    // the "linear fade" animation fades between the individual colors of every
    // pixel in the framebuffer and loops at the first "black/off" pixel (or the last pixel).
    //
    // This means that if you have:
    // LED1: rgb(255, 0, 0)   [red]
    // LED2: rgb(0, 0, 255)   [blue]
    // LED3: rgb(1, 1, 1)     [nearly black]
    // LED4: rgb(0, 0, 0)     [full off]
    //
    // The fade would be:
    // [red] => [blue] => [nearly black] => [red]
    btn.selectAnimation(currentPowerState, Animation_LinearFade);
    btn.setAnimationSpeed(currentPowerState, 10); 

    Serial.println("Animation: Linear fade");
    break;
  
  case Animation_Rainbow:
    // The rainbow animation cycles the whole LED ring through the RGB spectrum
    // incrementing one component at a time while decrementing the next one.
    // This animation requires you to set an "animation configuration" to 
    // specify the maximum compontent brightness. Below we use 128, which means
    // that we start at (128, 0, 0) which is red at 50% of the maximum brightness.
    btn.selectAnimation(currentPowerState, Animation_Rainbow);
    btn.setAnimationConfiguration(currentPowerState, 128);    // cycle at 50% of the full brightness scale
    btn.setAnimationSpeed(currentPowerState, 10); 

    Serial.println("Animation: Rainbow");
    break;

  case Animation_None:
  default:    
    // no animation => displays the framebuffer as it is!
    btn.selectAnimation(currentPowerState, Animation_None);

    Serial.println("Animation: None");
    /**
     * Flow control:
     * loop through the animation counter 
     */
    currentAnimation = 0; 
    break;
  }  
}

// interrupt service routine for the ÖnÖffBTN's INT pin (on Arduino pin 2)
void handleBtnInterrupt() 
{
  interruptReceived = true;
}