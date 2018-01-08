// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "LedControl.h" //  need the library

#define ALWAYS 0
#define SET_TIME_HH 1
#define SET_TIME_MM 2
#define SET_ON_TIME_HH 3
#define SET_ON_TIME_MM 4
#define SET_OFF_TIME_HH 5
#define SET_OFF_TIME_MM 6
#define STORE_ARGS 7

int Mode = ALWAYS;

/* Pin Comfig */
int relayOut = 13;
int blueOut = 7;
int buttonNext = 4;
int buttonUp = 3;
int buttonDown = 2;
int buttonExit = 5;
int OnTimeLED = 6;
int OffTimeLED = 12;
const byte secondPulsePin = 14;

RTC_DS3231 rtc;
LedControl lc = LedControl(10, 9, 11, 1); // LedControl(dataPin,clockPin,csPin,numDevices)


char time[4];

volatile byte flag = false;

void rtc_interrupt()
{
    Serial.println("RTC Intrupt");
    flag = true;
} // end of rtc_interrupt

void setup()
{
    // Define All Pins
    pinMode(buttonNext, INPUT_PULLUP);
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    pinMode(buttonExit, INPUT_PULLUP);
    pinMode(OnTimeLED, OUTPUT);
    pinMode(OffTimeLED, OUTPUT);
    pinMode(relayOut, OUTPUT);
    pinMode(blueOut, OUTPUT);

    // the zero refers to the MAX7219 number, it is zero for 1 chip
    lc.shutdown(0, false); // turn off power saving, enables display
    lc.setIntensity(0, 8); // sets brightness (0~15 possible values)
    lc.clearDisplay(0);    // clear screen

    Serial.begin(115200);

    // enable the 1 Hz output
    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);

    // set up to handle interrupt from 1 Hz pin
    pinMode(secondPulsePin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(secondPulsePin), rtc_interrupt, CHANGE);

    delay(3000); // wait for console opening

    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, lets set the time!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    digitalWrite(OnTimeLED, HIGH);
}

void loop() {
    if (buttonNext == LOW && Mode != ALWAYS){
        Mode += Mode;
    }
    /* if (longpresscondition && Mode == ALWAYS)
    {
        Mode = SET_TIME_HH
    } */
    switch (Mode)
    {
    case ALWAYS:
        DisplayTime();
        ToggleOnOff();
        break;
    case SET_TIME_HH:
        DisplaySetTimeHH();
        break;
    case SET_TIME_MM:
        DisplaySetTimeMM();
        break;
    case SET_ON_TIME_HH:
        DisplaySetONTimeHH();
        break;
    case SET_ON_TIME_MM:
        DisplaySetONTimeMM();
        break;
    case SET_OFF_TIME_HH:
        DisplaySetOFFTimeHH();
        break;
    case SET_OFF_TIME_MM:
        DisplaySetOFFTimeMM();
        break;
    case STORE_ARGS:
        StoreArgs();
        delay(500);
        Mode = ALWAYS;
        break;
    }
    delay(100);
}

void getTime(void)
{
    //get the time and put it into "time" chars
    DateTime now = rtc.now();

    sprintf(time, "%02hhu%02hhu", now.hour(), now.minute());
    delay(400);
}

void printTimeToLED(void)
{
    //minutes
    lc.setChar(0, 0, time[3], false);
    lc.setChar(0, 1, time[2], false);
    //hours
    lc.setChar(0, 2, time[1], false);
    lc.setChar(0, 3, time[0], false);
}

void DisplayTime(void){
    static unsigned long previousTime = 0;
    DateTime now = rtc.now();
    unsigned long timeNow = now.unixtime();

    if (timeNow != previousTime)
    {
        lc.setChar(0, 2, time[1], true);  // flash the led
        delay(500);                       // wait a little bit
        lc.setChar(0, 2, time[1], false); // turn off led
        previousTime = timeNow;           // remember previous time
    }
    getTime();
    printTimeToLED();
}

void ToggleOnOff(void)
{
}

void DisplaySetTimeHH(void)
{
}

void DisplaySetTimeMM(void)
{
}

void DisplaySetONTimeHH(void)
{
}

void DisplaySetONTimeMM(void)
{
}

void DisplaySetOFFTimeHH(void)
{
}

void DisplaySetOFFTimeMM(void)
{
}

void StoreArgs(void)
{
}
