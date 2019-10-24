/**
    @file     hhtronik_onoffbtn.cpp
    @author   Yannic Staudt (HHTronik)
    @license  BSD (see licence.txt)
    
    Arduino driver for the HHTronik ÖnÖffBTN.

    Visit https://hhtronik.com for more information
*/
#include <Wire.h>
#include "hhtronik_onoffbtn.h"

/////////////////////////////////////////////////////////
// Constructors:

HHTronik_OnOffBTN::HHTronik_OnOffBTN()
{
}

/////////////////////////////////////////////////////////
// Private:

uint8_t 
HHTronik_OnOffBTN::_i2c_readByte(uint8_t reg) 
{
    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(reg);                        // select register
    Wire.endTransmission(false);            // end write, but don't send STOP condition
    Wire.requestFrom(i2c_addr, (uint8_t)1); // now start read of 1 byte
    return (uint8_t)Wire.read();            // read 1 byte
}

void 
HHTronik_OnOffBTN::_i2c_writeByte(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(reg);                        // select register
    Wire.write(value);                      // write value
    Wire.endTransmission();                 // done.
}

uint16_t 
HHTronik_OnOffBTN::_i2c_readShort(uint8_t reg) 
{
    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(reg);                        // select register
    Wire.endTransmission(false);            // end write, but don't send STOP condition
    Wire.requestFrom(i2c_addr, (uint8_t)2); // now start read of 1 byte
    
    uint16_t result = 0;
    result = ((uint16_t)Wire.read()) << 8;  // read 1 byte
    result |= (uint8_t)Wire.read();         // read 2nd byte

    return result;
}

void 
HHTronik_OnOffBTN::_i2c_writeShort(uint8_t reg, uint16_t value)
{
    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(reg);                        // select register
    Wire.write(value >> 8);                 // write MSB value
    Wire.write(value & 0xff);               // write LSB value
    Wire.endTransmission();                 // done.
    Wire.flush();
}


/////////////////////////////////////////////////////////
// Public:

bool 
HHTronik_OnOffBTN::begin(uint8_t addr)
{
    this->i2c_addr = addr;
    Wire.setClock(400000);      // we work in fast mode
    Wire.begin();
    return true;
}

bool 
HHTronik_OnOffBTN::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t addr)
{
    this->i2c_addr = addr;
    Wire.setClock(400000);      // we work in fast mode
    Wire.begin();
    return true;
}

void 
HHTronik_OnOffBTN::clearFramebuffer( void )
{
    Wire.beginTransmission(i2c_addr);   // send address
    Wire.write(0xd0);                   // select first byte of framebuffer

    for(uint8_t i = 0; i < ONOFFBTN_NUM_PIXELS * 3; i++)
    {
        Wire.write(0);
    }

    Wire.endTransmission();
}

void 
HHTronik_OnOffBTN::setPixel(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b)
{   
    // avoid writing over the boundaries 
    if(pixel > ONOFFBTN_NUM_PIXELS) return;

    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xd0 + pixel * 3);           // select first byte of selected pixel
    Wire.write(r); 
    Wire.write(g); 
    Wire.write(b); 
    Wire.endTransmission();           
}

void 
HHTronik_OnOffBTN::setPixels(const uint8_t *subpixel, const uint8_t length, const uint8_t offset = 0)
{
    if(offset > ONOFFBTN_NUM_PIXELS) return;
    
    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xd0 + offset);              // select first byte of selected subpixel

    for(uint8_t idx = offset; idx < length + offset; idx++)
    {
        if(idx > ONOFFBTN_NUM_PIXELS * 3) break;
        Wire.write(*subpixel++);
    }

    Wire.endTransmission();
}

OnOffBTN_StatusRegister 
HHTronik_OnOffBTN::getButtonStatus()
{    
    int rawValue = _i2c_readByte(0x00);     // BUTTON STATUS register at 0x00
    
    OnOffBTN_StatusRegister result =
    {
        .Down           = (rawValue >> 0) & 1,
        .ShortPress     = (rawValue >> 1) & 1,
        .LongPress      = (rawValue >> 2) & 1,
        .DoubleClick    = (rawValue >> 3) & 1,
        .PowerOn        = (rawValue >> 4) & 1,
        .RTC_Alarm      = (rawValue >> 5) & 1
    };

    return result;
}

void 
HHTronik_OnOffBTN::TriggerLatch(bool immediate)
{
    // normal reset
    uint8_t value = 1;

    // or immediate reset is asked for
    if(immediate)
        value = 1 << 1;

    _i2c_writeByte(0x01, value);
}

void 
HHTronik_OnOffBTN::TriggerReset(bool immediate)
{
    // normal reset
    uint8_t value = 1 << 2;

    // or immediate reset is asked for
    if(immediate)
        value = 1 << 3;

    _i2c_writeByte(0x01, value);
}

uint16_t 
HHTronik_OnOffBTN::getLongPressThreshold( void )
{
    return _i2c_readShort(0x02);
}

void 
HHTronik_OnOffBTN::setLongPressThreshold(uint16_t value)
{
    _i2c_writeShort(0x02, value);
}


OnOffBTN_HardResetBehaviorRegister 
HHTronik_OnOffBTN::getHardResetBehaviorConfiguration( void )
{
    uint8_t regValue = _i2c_readByte(0x04);

    OnOffBTN_HardResetBehaviorRegister result;
    result.DisableHardReset         = (bool)((regValue & 1)   >> 0); // mask 0b00000001
    result.HardResetHoldDuration    =       ((regValue & 30)  >> 1); // mask 0b00011110
    result.AutoRestartAfterReset    = (bool)((regValue & 32)  >> 5); // mask 0b00100000
    result.AutoRestartDelay         =       ((regValue & 192) >> 6); // mask 0b11000000

    return result;
}

void 
HHTronik_OnOffBTN::setHardResetBehaviorConfiguration( OnOffBTN_HardResetBehaviorRegister config )
{
    uint8_t regValue = 0;

    regValue |= (((uint8_t)config.DisableHardReset)      & 1  ) << 0; // mask 0b00000001
    regValue |= (((uint8_t)config.HardResetHoldDuration) & 15 ) << 1; // mask 0b00001111
    regValue |= (((uint8_t)config.AutoRestartAfterReset) & 1  ) << 5; // mask 0b00000001
    regValue |= (((uint8_t)config.AutoRestartDelay)      & 3  ) << 6; // mask 0b00000011
    
    _i2c_writeByte(0x04, regValue);
}

OnOffBTN_PowerBehaviorRegister 
HHTronik_OnOffBTN::getPowerOnResetConfiguration( void )
{
    uint8_t regValue = _i2c_readByte(0x05);

    OnOffBTN_PowerBehaviorRegister result;
    result.PoR_DefaultOn            = (bool)((regValue & 1)   >> 0); // mask 0b00000001
    result.PoR_RestoreFramebuffer   = (bool)((regValue & 2)   >> 1); // mask 0b00000010
    result.AutoLatchOnOnPress       = (bool)((regValue & 4)   >> 2); // mask 0b00000100
    result.AutoLatchOnOffPress      = (bool)((regValue & 8)   >> 3); // mask 0b00001000

    return result;
}

void 
HHTronik_OnOffBTN::setPowerBehaviorConfiguration( OnOffBTN_PowerBehaviorRegister config )
{
    uint8_t regValue = 0;
    regValue |= (((uint8_t)config.PoR_DefaultOn)            & 1 ) << 0; // mask 0b00000001
    regValue |= (((uint8_t)config.PoR_RestoreFramebuffer)   & 1 ) << 1; // mask 0b00000001    
    regValue |= (((uint8_t)config.AutoLatchOnOnPress)       & 1 ) << 2; // mask 0b00000001    
    regValue |= (((uint8_t)config.AutoLatchOnOffPress)      & 1 ) << 3; // mask 0b00000001    
    _i2c_writeByte(0x05, regValue);
}

uint16_t 
HHTronik_OnOffBTN::getOnDelay( void )
{
    return _i2c_readShort(0x06);
}

void 
HHTronik_OnOffBTN::setOnDelay(uint16_t value)
{
    _i2c_writeShort(0x06, value);
}

uint16_t 
HHTronik_OnOffBTN::getOffDelay( void )
{
    return _i2c_readShort(0x08);
}

void 
HHTronik_OnOffBTN::setOffDelay(uint16_t value)
{
    _i2c_writeShort(0x08, value);
}

void 
HHTronik_OnOffBTN::selectAnimation(OnOffBTN_PowerState state, OnOffBTN_Animation animation)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0a;     // animation_on_state_selection at 0x0a
    else
        reg = 0x0d;     // animation_off_state_selection at 0x0d

    _i2c_writeByte(reg, (uint8_t)animation);
}

OnOffBTN_Animation 
HHTronik_OnOffBTN::getSelectedAnimation(OnOffBTN_PowerState state)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0a;     // animation_on_state_selection at 0x0a
    else
        reg = 0x0d;     // animation_off_state_selection at 0x0d
    
    return (OnOffBTN_Animation)_i2c_readByte(reg);
}

void 
HHTronik_OnOffBTN::setAnimationSpeed(OnOffBTN_PowerState state, uint8_t tickSpeed)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0b;     // animation_on_state_tickspeed at 0x0b
    else
        reg = 0x0e;     // animation_off_state_tickspeed at 0x0e

    _i2c_writeByte(reg, tickSpeed);
}

uint8_t 
HHTronik_OnOffBTN::getAnimationSpeed(OnOffBTN_PowerState state)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0b;     // animation_on_state_tickspeed at 0x0b
    else
        reg = 0x0e;     // animation_off_state_tickspeed at 0x0e
    
    return _i2c_readByte(reg);
}

void 
HHTronik_OnOffBTN::setAnimationConfiguration(OnOffBTN_PowerState state, uint8_t value)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0c;     // animation_on_state_configuration at 0x0c
    else
        reg = 0x0f;     // animation_off_state_configuration at 0x0f

    _i2c_writeByte(reg, value);
}

uint8_t 
HHTronik_OnOffBTN::getAnimationConfiguration(OnOffBTN_PowerState state)
{
    uint8_t reg;
    if(state == PowerOn)
        reg = 0x0c;     // animation_on_state_configuration at 0x0c
    else
        reg = 0x0f;     // animation_off_state_configuration at 0x0f
    
    return _i2c_readByte(reg);
}

void 
HHTronik_OnOffBTN::saveAnimationFramebuffer(OnOffBTN_PowerState state)
{
    uint8_t value = 1 << 2;

    if(state == PowerOff)
        value = 1 << 3;

    _i2c_writeByte(0x10, value);
}

void 
HHTronik_OnOffBTN::clearStoredAnimationFramebuffer(OnOffBTN_PowerState state)
{
    uint8_t value = 1 << 4;

    if(state == PowerOff)
        value = 1 << 5;

    _i2c_writeByte(0x10, value);
}

void 
HHTronik_OnOffBTN::restoreStoredAnimationFramebuffer(OnOffBTN_PowerState state)
{
    uint8_t value = 1 << 6;

    if(state == PowerOff)
        value = 1 << 7;

    _i2c_writeByte(0x10, value);
}

void 
HHTronik_OnOffBTN::setFramebufferRestoreBehavior(bool restoreOnState, bool restoreOffState)
{
    uint8_t value = 0;

    if(restoreOnState)
        value |= 1 << 0;

    if(restoreOnState)
        value |= 1 << 1;

    _i2c_writeByte(0x10, value);
}

uint8_t 
HHTronik_OnOffBTN::getUserEEPROMByte(uint8_t byteIndex)
{
    if(byteIndex > 15) return 255;  // we have 16 bytes of storage for the user
    
    return _i2c_readByte(0x30 + byteIndex);
}

void 
HHTronik_OnOffBTN::setUserEEPROMByte(uint8_t byteIndex, uint8_t value)
{
    if(byteIndex > 15) return;  // we have 16 bytes of storage for the user
    
    _i2c_writeByte(0x30 + byteIndex, value);
}

OnOffBTN_RTCControlRegister 
HHTronik_OnOffBTN::getRTCConfiguration( void )
{
    int rawValue = _i2c_readByte(0xb0);     // BUTTON STATUS register at 0x00
    
    OnOffBTN_RTCControlRegister result =
    {
        .AlarmEnabled           = (rawValue >> 0) & 1,
        .AlarmAction            = (OnOffBTN_RTCAlarmAction)((rawValue >> 1) & 3), // 2 bits
        .AlarmAutoRearm         = (rawValue >> 3) & 1,
        .UseAmPmFormat          = (rawValue >> 4) & 1,
        .AlarmCancelationDelay  = (OnOffBTN_DelayValue)((rawValue >> 5) & 3) // 2 bits again     
    };

    return result;
}

void 
HHTronik_OnOffBTN::setRTCConfiguration(OnOffBTN_RTCControlRegister configuration)
{
    uint8_t value = 0;

    value |= (((uint8_t)configuration.AlarmEnabled)          & 1) << 0; // 1 bit
    value |= (((uint8_t)configuration.AlarmAction)           & 3) << 1; // 2 bits
    value |= (((uint8_t)configuration.AlarmAutoRearm)        & 1) << 3; // 1 bit
    value |= (((uint8_t)configuration.UseAmPmFormat)         & 1) << 4; // 1 bit
    value |= (((uint8_t)configuration.AlarmCancelationDelay) & 3) << 5; // 2 bits

    _i2c_writeByte(0xb0, value);
}

#define OOB_DATETIMELENGTH (7)

OnOffBTN_DateTime 
HHTronik_OnOffBTN::getDateTime( void )
{
    uint8_t i = 0;
    uint8_t bytesRcv[OOB_DATETIMELENGTH];

    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xb1);                       // select register 0xb1 (RTC seconds)
    Wire.endTransmission(false);            // end write, but don't send STOP condition
    Wire.requestFrom(i2c_addr, (uint8_t)OOB_DATETIMELENGTH);

    // read the 7 bytes
    while(Wire.available() > 0 && i <= OOB_DATETIMELENGTH)
        bytesRcv[i++] = Wire.read();

    OnOffBTN_DateTime result;
    result.Seconds = bcdToDec(bytesRcv[0]);
    result.Minutes = bcdToDec(bytesRcv[1]);
    result.Hours = bcdToDec(bytesRcv[2]);
    result.DayOfMonth = bcdToDec(bytesRcv[3]);
    result.Month = bcdToDec(bytesRcv[4]);
    result.Year = bcdToDec(bytesRcv[5]);
    result.DayOfWeek = bytesRcv[6];         // this one's not BCD coded!

    return result;
}

void 
HHTronik_OnOffBTN::setDateTime(OnOffBTN_DateTime datetime)
{
    uint8_t bytesSnd[OOB_DATETIMELENGTH];
    bytesSnd[0] = decToBcd(datetime.Seconds);
    bytesSnd[1] = decToBcd(datetime.Minutes);
    bytesSnd[2] = decToBcd(datetime.Hours);
    bytesSnd[3] = decToBcd(datetime.DayOfMonth);
    bytesSnd[4] = decToBcd(datetime.Month);
    bytesSnd[5] = decToBcd(datetime.Year);
    bytesSnd[6] = datetime.DayOfWeek & 7;   // not bcd coded, 3 bits

    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xb1);                       // select register 0xb1 (RTC seconds)

    for(uint8_t i = 0; i < OOB_DATETIMELENGTH; i++) 
        Wire.write(bytesSnd[i]);

    Wire.endTransmission();                 // done.
}

#define OOB_ALARMTIMELENGTH (3)
#define OOB_ALARMDAYDATELENGTH (1)

OnOffBTN_AlarmTime 
HHTronik_OnOffBTN::getAlarmTime( void )
{
    uint8_t i = 0;
    uint8_t bytesRcv[OOB_ALARMTIMELENGTH];

    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xb8);                       // select register 0xb8 (RTC ALMAR1)
    Wire.endTransmission(false);            // end write, but don't send STOP condition
    Wire.requestFrom(i2c_addr, (uint8_t)OOB_ALARMTIMELENGTH);

    // read the 4 bytes
    while(Wire.available() > 0 && i <= OOB_ALARMTIMELENGTH)
        bytesRcv[i++] = Wire.read();

    OnOffBTN_AlarmTime result;
    result.Seconds      = bcdToDec(bytesRcv[0] & 127);  // 0b01111111 / 1st bis is MaskSeconds 
    result.Minutes      = bcdToDec(bytesRcv[1] & 127);  // 0b01111111 / 1st bis is MaskMinutes 
    result.Hours        = bcdToDec(bytesRcv[2] & 127);  // 0b01111111 / 1st bis is Hours     
    result.MaskSeconds  = (bytesRcv[0] & 128) > 0;      // 0b10000000
    result.MaskMinutes  = (bytesRcv[1] & 128) > 0;      // 0b10000000
    result.MaskHours    = (bytesRcv[2] & 128) > 0;      // 0b10000000

    return result;
}

void
HHTronik_OnOffBTN::setAlarmTime(OnOffBTN_AlarmTime alarmTime)
{
     uint8_t bytesSnd[OOB_ALARMTIMELENGTH];
    bytesSnd[0] = (decToBcd(alarmTime.Seconds)  & 127) | ((((uint8_t)alarmTime.MaskSeconds) << 7) & 128);
    bytesSnd[1] = (decToBcd(alarmTime.Minutes)  & 127) | ((((uint8_t)alarmTime.MaskMinutes) << 7) & 128);
    bytesSnd[2] = (decToBcd(alarmTime.Hours)    & 127) | ((((uint8_t)alarmTime.MaskHours)   << 7) & 128);

    Wire.beginTransmission(i2c_addr);       // send address
    Wire.write(0xb8);                       // select register 0xb8 (RTC ALMAR1)

    for(uint8_t i = 0; i < OOB_ALARMTIMELENGTH; i++) 
        Wire.write(bytesSnd[i]);

    Wire.endTransmission();                 // done.
}

OnOffBTN_AlarmDayDate 
HHTronik_OnOffBTN::getAlarmDayDate( void )
{
    uint8_t almar4 = _i2c_readByte(0xbb);

    OnOffBTN_AlarmDayDate result;
    result.DayDateMasked  = (bool)((almar4 & 128)   >> 7);
    result.IsWeekDayAlarm = (bool)((almar4 & 64)    >> 6);

    if(result.IsWeekDayAlarm)
        result.Value = (almar4 & 7); // 0b00000111 / weekday on 3 bits
    else
        result.Value = bcdToDec(almar4 & 63);   // 0b00111111 / bcd coded day of month

    return result;
}

void 
HHTronik_OnOffBTN::setAlarmDayDate(OnOffBTN_AlarmDayDate  value)
{
    uint8_t almar4 = 0;
    
    almar4 |= ((uint8_t)value.DayDateMasked) << 7;
    almar4 |= ((uint8_t)value.IsWeekDayAlarm) << 6;

    if(value.IsWeekDayAlarm)
        almar4 |= (value.Value & 7);            // 0b00000111 / weekday on 3 bits
    else
        almar4 |= decToBcd(value.Value & 63);   // 0b00111111 / bcd coded day of month

    
    _i2c_writeByte(0xbb, almar4);
}