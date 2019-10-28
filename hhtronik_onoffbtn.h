/**
    @file     hhtronik_onoffbtn.h
    @author   Yannic Staudt (HHTronik)

    @license     
    Software License Agreement (BSD License)
    Copyright (c) 2019, HHTronik / Staudt Technologies GmbH
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    ---------------------
    Arduino driver for the HHTronik ÖnÖffBTN.
    Visit https://hhtronik.com for more information
*/

#ifndef _HHTRONIK_ONOFFBTN_H_
#define _HHTRONIK_ONOFFBTN_H_

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <Wire.h>

#define ONOFFBTN_DEFAULT_I2C_ADDRESS        (0x59) 
#define ONOFFBTN_NUM_PIXELS                 (9)

typedef enum {
  delay100ms = 0,
  delay1000ms = 1,
  delay5000ms = 2,
  delay10000ms = 3
} OnOffBTN_DelayValue;

typedef enum {
  Animation_None        = 0,
  Animation_Breath      = 1,
  Animation_Spinner     = 2,
  Animation_Flash       = 3,
  Animation_Flicker     = 4,
  Animation_Spooky      = 5,
  Animation_Rainbow     = 6,
  Animation_LinearFade  = 7
} OnOffBTN_Animation;

typedef enum {
  PowerOn,
  PowerOff,
  _LastState = PowerOff
} OnOffBTN_PowerState;

typedef enum {
  RTCAlarm_PowerOn = 0,
  RTCAlarm_PowerOff = 1, 
  RTCAlarm_Reset = 2,
  RTCAlarm_Toggle = 3
} OnOffBTN_RTCAlarmAction;

typedef struct
{
  bool Down                : 1;
  bool ShortPress          : 1;
  bool LongPress           : 1;  
  bool DoubleClick         : 1;
  bool PowerOn             : 1;
  bool RTC_Alarm           : 1;

} OnOffBTN_StatusRegister;

typedef struct
{
  bool DisableHardReset                 : 1;
  uint8_t HardResetHoldDuration         : 4;
  bool AutoRestartAfterReset            : 1; 
  OnOffBTN_DelayValue AutoRestartDelay  : 2;  
} OnOffBTN_HardResetBehaviorRegister;

typedef struct
{
  bool PoR_DefaultOn                    : 1;
  bool PoR_RestoreFramebuffer           : 1;
  bool AutoLatchOnOnPress               : 1;
  bool AutoLatchOnOffPress              : 1;
} OnOffBTN_PowerBehaviorRegister;

typedef struct
{
  bool AlarmEnabled                     : 1;
  OnOffBTN_RTCAlarmAction AlarmAction   : 2;
  bool AlarmAutoRearm                   : 1;
  bool UseAmPmFormat                    : 1;
  OnOffBTN_DelayValue AlarmCancelationDelay  : 2; 
} OnOffBTN_RTCControlRegister;

typedef struct 
{
  uint8_t Seconds;
  uint8_t Minutes;
  uint8_t Hours;
  uint8_t DayOfMonth;
  uint8_t Month;
  uint8_t Year;
  uint8_t DayOfWeek;
} OnOffBTN_DateTime;

typedef struct 
{
  uint8_t Seconds;
  uint8_t Minutes;
  uint8_t Hours;
  bool MaskSeconds  : 1;
  bool MaskMinutes  : 1;
  bool MaskHours    : 1;
} OnOffBTN_AlarmTime;

typedef struct 
{
  uint8_t Value       : 6; 
  bool IsWeekDayAlarm : 1;
  bool DayDateMasked  : 1;
} OnOffBTN_AlarmDayDate;


class HHTronik_OnOffBTN {
 public:
  HHTronik_OnOffBTN();

  /**
   * Try to connect to the ÖnÖffBTN at the given address
   */
  boolean begin(uint8_t addr = ONOFFBTN_DEFAULT_I2C_ADDRESS);
  
  /**
   * Try to connect to the ÖnÖffBTN at the given address.
   * This overload allows you to choose different pins for the Wire library / I2C driver
   * make sure your hardware supports I2C on the specified pins
   * @param sdaPin pin to use for I2C SDA line
   * @param sclPin pin to use for I2C SCL line
   */
  boolean begin(uint8_t sdaPin, uint8_t sclPin, uint8_t addr = ONOFFBTN_DEFAULT_I2C_ADDRESS);

  /**
   * Set all pixels to black
   */
  void clearFramebuffer( void );

  /**
    Set a single pixel to a given color
  
    @param pixel zero-based pixel index
    @param R Red color component
    @param G Green color component
    @param B Blue color componwnr
  */
  void setPixel(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);

  /**
    Send a full or parial frame to the ÖnÖffBTN. Each array entry is a
    subpixel value. Using the length and offset parameters you can
    update the LED ring partially
  
    @param frame a pointer to an array of subpixels
    @param the number of subpixels to send
    @param (default = 0) the subpixel offset (does not offset the pointer, just the target register address)    
  */  
  void setPixels(const uint8_t *subpixel, const uint8_t length, const uint8_t offset = 0);

  /**
   * Get the button status
   * @param pollMode (default true) use register 0x50 instead of 0x00 to not reset flags 
   * when polling frequently
   * @returns struct OnOffBTN_StatusRegister 
   */
  OnOffBTN_StatusRegister getButtonStatus();


  /**
   * Persist the current configuration to EEPROM.
   * 
   * @note: this call takes up to 500ms to complete. If your hardware doesn't
   * support I2C Clock Stretching, please insert some waiting time after calling
   * this to ensure the next I2C transfert won't fail if it is too close.
   */
  void SaveConfiguration( void );
  
  /**
   * Toggle the button.
   * 
   * @note when re-triggering a latch during the delay time cancels the whole procedure. 
   * You can use this behavior to cancel power off
   * 
   * @param immediate set to true to have an immediate power state toggle
   */
  void TriggerLatch(bool immediate = false);

  /**
   * Trigger a reset procedure. Depending on the configuration this will 
   * cut power supply for a number of seconds then reenable it. 
   * 
   * @param immediate set to true skip any waiting time before cutting power
   */
  void TriggerReset(bool immediate = false);
  
  /**
   * Read the configured "long press" threshold (in ms)
   */
  uint16_t getLongPressThreshold( void );

  /**
   * Set the "long button press" threshold (in ms)
   */
  void setLongPressThreshold(uint16_t value);

  /**
   * Get the current hard reset behavior configuration
   */
  OnOffBTN_HardResetBehaviorRegister getHardResetBehaviorConfiguration( void );

  /**
   * Update the reset behavior configuration
   */
  void setHardResetBehaviorConfiguration( OnOffBTN_HardResetBehaviorRegister config );

  /**
   * Get the current power behavior configuration
   */
  OnOffBTN_PowerBehaviorRegister getPowerOnResetConfiguration( void );

  /**
   * Update the power behavior configuration
   */
  void setPowerBehaviorConfiguration( OnOffBTN_PowerBehaviorRegister config );  

  /**
   * Read the configured "on" delay (in ms).
   * This delay occurs between a latch event (either software or hardware) and 
   * the power switch actually closing.  
   */
  uint16_t getOnDelay( void );

  /**
   * Set the "on" delay (in ms)
   * This delay occurs between a latch event (either software or hardware) and 
   * the power switch actually closing.  
   */
  void setOnDelay(uint16_t value);

  /**
   * Read the configured "off" delay (in ms).
   * This delay occurs between a latch event (either software or hardware) and 
   * the power switch actually opening.  
   */
  uint16_t getOffDelay( void );

 /**
   * Set the "off" delay (in ms).
   * This delay occurs between a latch event (either software or hardware) and 
   * the power switch actually opening.  
   */
  void setOffDelay(uint16_t value);

  /**
   * Select the animation used for the specified power state
   * @param state On or Off state
   * @param animation on of OnOffBTN_Animation 
   */
  void selectAnimation(OnOffBTN_PowerState state, OnOffBTN_Animation animation);

  /**
   * Get the configuration animation given a power state
   * @param state On or Off state 
   */
  OnOffBTN_Animation getSelectedAnimation(OnOffBTN_PowerState state);

  /**
   * Set the animation speed for a given power state
   * @param state On or Off state
   * @param tickSpeed ~how many milliseconds a "tick" of the animation takes
   */
  void setAnimationSpeed(OnOffBTN_PowerState state, uint8_t tickSpeed);  

  /**
   * Gets the animation speed for a given power state. One tick
   * roughly equates the number of milliseconds on "tick" of the animation takes
   */
  uint8_t getAnimationSpeed(OnOffBTN_PowerState state);

  /**
   * Set the animation configuration for a given power state
   * @param state On or Off state
   * @param value The configuration value. Value depends on the selected animation, see README
   */
  void setAnimationConfiguration(OnOffBTN_PowerState state, uint8_t value);

  /**
   * Reads the animation configuration value for a given power state
   * @param state On or Off state
   */
  uint8_t getAnimationConfiguration(OnOffBTN_PowerState state);

  /**
   * Persist the current framebuffer content to EEPROM for the specified
   * @param state On or off state
   */
  void saveAnimationFramebuffer(OnOffBTN_PowerState state);
  
  /**
   * Clear the specified stored framebuffer 
   * @param state On or off state
   */
  void clearStoredAnimationFramebuffer(OnOffBTN_PowerState state);
  
  /**
   * Restore a saved framebuffer
   * @param state On or off state
   */
  void restoreStoredAnimationFramebuffer(OnOffBTN_PowerState state);

  /**
   * Set the framebuffer restore behavior
   * @param restoreOnState set to true to have the ÖnÖffBTN restore the saved "on" state
   * framebuffer when the button latches to "on"
   * @param restoreOffState set to true to have the ÖnÖffBTN restore the saved "off" state
   * framebuffer when the button latches to "off"
   */
  void setFramebufferRestoreBehavior(bool restoreOnState, bool restoreOffState);

  /**
   * Read a persisted byte from the EEPROM
   * 
   * @param byteIndex 
   * @returns the saved byte at byteIndex
   */
  uint8_t getUserEEPROMByte(uint8_t byteIndex);

  /**
   * Write a persisted byte to the EEPROM
   * 
   * @param byteIndex the memory address
   * @param value value to write to EEPROM
   */
  void setUserEEPROMByte(uint8_t byteIndex, uint8_t value);

  /**
   * Read the RTC configuration
   */
  OnOffBTN_RTCControlRegister getRTCConfiguration( void );

  /**
   * Write the RTC configuration
   * 
   * @note: this call takes up to 500ms to complete. If your hardware doesn't
   * support I2C Clock Stretching, please insert some waiting time after calling
   * this to ensure the next I2C transfert won't fail if it is too close.
   */
  void setRTCConfiguration(OnOffBTN_RTCControlRegister configuration);

  /**
   * Read the RTC date/time
   */
  OnOffBTN_DateTime getDateTime( void );

  /**
   * Set the RTC date/time
   */
  void setDateTime(OnOffBTN_DateTime datetime);

  /**
   * Get the configured alarm time
   */
  OnOffBTN_AlarmTime getAlarmTime( void );

  /**
   * Set the alarm time 
   */
  void setAlarmTime(OnOffBTN_AlarmTime);

  /**
   * Get the RTC alarm week-day/date configuration
   */
  OnOffBTN_AlarmDayDate getAlarmDayDate( void );

  /**
   * Set the RTC alarm week-day/date configuration
   */ 
  void setAlarmDayDate(OnOffBTN_AlarmDayDate  value);

 private:
  uint8_t i2c_addr;

  uint8_t _i2c_readByte(uint8_t reg);
  void _i2c_writeByte(uint8_t reg, uint8_t value);

  
  uint16_t _i2c_readShort(uint8_t reg);
  void _i2c_writeShort(uint8_t reg, uint16_t value);

  /**
   * convert a decimal number to a bcd encoded value
   */
  uint8_t decToBcd(uint8_t val) { return((val / 10 * 16) + (val % 10)); }

  /**
   * convert a BCD encoded value to a decimal
   */
  uint8_t bcdToDec(uint8_t val) { return((val / 16 * 10) + (val % 16)); }
};

#endif
