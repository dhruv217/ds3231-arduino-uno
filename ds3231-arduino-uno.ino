// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "LedControl.h" //  need the library

LedControl lc = LedControl(10, 9, 11, 1); // LedControl(dataPin,clockPin,csPin,numDevices)

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int hours[2];
int minutes[2];

char time[4];

void setup()
{
    // the zero refers to the MAX7219 number, it is zero for 1 chip
    lc.shutdown(0, false); // turn off power saving, enables display
    lc.setIntensity(0, 8); // sets brightness (0~15 possible values)
    lc.clearDisplay(0);    // clear screen

    Serial.begin(115200);

    delay(3000); // wait for console opening

    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
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
    
}

void loop()
{
    DateTime now = rtc.now();
    rtc.get
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
 
    Serial.println();
    getTime();

    printTimeToLED();
    
    delay(3000);
}

void getTime(void)
{
    //get the time and put it into "time" chars
    DateTime now = rtc.now();

    sprintf(time, "%02hhu%02hhu", now.hour(), now.minute());
    Serial.println(time);
    delay(1000);
}

void printTimeToLED(void)
{
    //minutes
    lc.setDigit(0, 0, time[3], false);
    lc.setChar(0, 1, time[2], true);
    //hours
    lc.setChar(0, 2, time[1], false);
    lc.setChar(0, 3, time[0], false);
}
