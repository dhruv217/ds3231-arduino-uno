// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "LedControl.h" //  need the library
#include <EEPROMAnything.h>

#define ALWAYS 0
#define SET_TIME_HH 1
#define SET_TIME_MM 2
#define SET_ON_TIME_HH 3
#define SET_ON_TIME_MM 4
#define SET_OFF_TIME_HH 5
#define SET_OFF_TIME_MM 6
#define SET_OVERFLOW 7

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

int RelayTimeAddr = 0;
struct RelayTimes_t {
    char Ontime[4];
    char Offtime[4];
} RelayTimes;

char time[4];
char previousTime[4];
char OnTime[4];
char OffTime[4];
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
    

    // Read Values From eeprom
    EEPROM.get(RelayTimeAddr, RelayTimes);
    strcpy(OnTime, RelayTimes.Ontime);
    strcpy(OffTime, RelayTimes.Offtime);
}

void loop() {
    if (digitalRead(buttonNext) == LOW && Mode != ALWAYS){
        Mode = Mode + 1;
        delay(100);
    }
    if (digitalRead(buttonNext) == LOW && digitalRead(buttonExit) == LOW && Mode == ALWAYS){
        Mode = SET_TIME_HH;
        delay(1000);
    }
    if (digitalRead(buttonExit) == LOW && Mode != ALWAYS){
        StoreArgs();
        delay(500);
        clearDisplay(1);
        Mode = ALWAYS;
        delay(50);
    }
    switch (Mode)
    {
    case ALWAYS:
        digitalWrite(OnTimeLED, LOW);
        digitalWrite(OffTimeLED, LOW);
        DisplayTime();
        ToggleOnOff();
        clearDisplay(1);
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
    case SET_OVERFLOW:
        clearDisplay(1);
        Mode = SET_TIME_HH;
        break;
    }
    delay(100);
}

void getTime(void)
{
    //get the time and put it into "time" chars
    DateTime now = rtc.now();

    sprintf(time, "%02hhu%02hhu", now.hour(), now.minute());
    strcpy( previousTime, time);
    delay(400);
}

void printTimeToLED(int display, char HHMM[])
{
    display = display * 4;
    //minutes
    lc.setChar(0, display, HHMM[3], false);
    lc.setChar(0, display+1, HHMM[2], false);
    //hours
    lc.setChar(0, display+2, HHMM[1], false);
    lc.setChar(0, display+3, HHMM[0], false);
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
    printTimeToLED(0, time);
}

void ToggleOnOff(void)
{
    if (compairTime(time, OnTime) && digitalRead(relayOut) == LOW){
        digitalWrite(relayOut, HIGH);
        digitalWrite(blueOut, HIGH);
    }
    if (compairTime(time, OffTime) && digitalRead(relayOut) == HIGH)
    {
        digitalWrite(relayOut, LOW);
        digitalWrite(blueOut, LOW);
    }
}

bool compairTime(char time1[], char time2[]) {
    if (time1[0] == time2[0] && 
        time1[1] == time2[1] && 
        time1[2] == time2[2] && 
        time1[3] == time2[3])
    {
        return true;
    }
    return false;
}

void DisplaySetTimeHH(void)
{
    digitalWrite(OnTimeLED, LOW);
    digitalWrite(OffTimeLED, LOW);
    if (digitalRead(buttonUp) == LOW) {    // Increment
        if (time[0] == '2' && time[1] == '3') {
            time[0] = '0';
            time[1] = '0';
        }
        else if (time[1] == '9') {
            time[0] = (int)time[0] + 1;
            time[1] = '0';
        }
        else {
            time[1] = (int)time[1] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW) {  // Decrement
        if (time[0] == '0' && time[1] == '0')
        {
            time[0] = '2';
            time[1] = '3';
        }
        else if (time[1] == '0')
        {
            time[0] = (int)time[0] - 1;
            time[1] = '9';
        }
        else
        {
            time[1] = (int)time[1] - 1;
        }
        delay(50);
    }
    printTimeToLED(0, time);
}

void DisplaySetTimeMM(void)
{
    digitalWrite(OnTimeLED, LOW);
    digitalWrite(OffTimeLED, LOW);
    if (digitalRead(buttonUp) == LOW)
    { // Increment
        if (time[2] == '5' && time[3] == '9')
        {
            time[2] = '0';
            time[3] = '0';
        }
        else if (time[3] == '9')
        {
            time[2] = (int)time[2] + 1;
            time[3] = '0';
        }
        else
        {
            time[3] = (int)time[3] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW)
    { // Decrement
        if (time[2] == '0' && time[3] == '0')
        {
            time[2] = '5';
            time[3] = '9';
        }
        else if (time[3] == '0')
        {
            time[2] = (int)time[2] - 1;
            time[3] = '9';
        }
        else
        {
            time[3] = (int)time[3] - 1;
        }
        delay(50);
    }
    printTimeToLED(0, time);
}

void DisplaySetONTimeHH(void)
{
    digitalWrite(OnTimeLED, HIGH);
    digitalWrite(OffTimeLED, LOW);
    if (digitalRead(buttonUp) == LOW)
    { // Increment
        if (OnTime[0] == '2' && OnTime[1] == '3')
        {
            OnTime[0] = '0';
            OnTime[1] = '0';
        }
        else if (OnTime[1] == '9')
        {
            OnTime[0] = (int)OnTime[0] + 1;
            OnTime[1] = '0';
        }
        else
        {
            OnTime[1] = (int)OnTime[1] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW)
    { // Decrement
        if (OnTime[0] == '0' && OnTime[1] == '0')
        {
            OnTime[0] = '2';
            OnTime[1] = '3';
        }
        else if (OnTime[1] == '0')
        {
            OnTime[0] = (int)OnTime[0] - 1;
            OnTime[1] = '9';
        }
        else
        {
            OnTime[1] = (int)OnTime[1] - 1;
        }
        delay(50);
    }
    printTimeToLED(1, OnTime);
}

void DisplaySetONTimeMM(void)
{
    digitalWrite(OnTimeLED, HIGH);
    digitalWrite(OffTimeLED, LOW);
    if (digitalRead(buttonUp) == LOW)
    { // Increment
        if (OnTime[2] == '5' && OnTime[3] == '9')
        {
            OnTime[2] = '0';
            OnTime[3] = '0';
        }
        else if (OnTime[3] == '9')
        {
            OnTime[2] = (int)OnTime[2] + 1;
            OnTime[3] = '0';
        }
        else
        {
            OnTime[3] = (int)OnTime[3] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW)
    { // Decrement
        if (OnTime[2] == '0' && OnTime[3] == '0')
        {
            OnTime[2] = '5';
            OnTime[3] = '9';
        }
        else if (OnTime[3] == '0')
        {
            OnTime[2] = (int)OnTime[2] - 1;
            OnTime[3] = '9';
        }
        else
        {
            OnTime[3] = (int)OnTime[3] - 1;
        }
        delay(50);
    }
    printTimeToLED(1, OnTime);
}

void DisplaySetOFFTimeHH(void)
{
    digitalWrite(OnTimeLED, LOW);
    digitalWrite(OffTimeLED, HIGH);
    if (digitalRead(buttonUp) == LOW)
    { // Increment
        if (OffTime[0] == '2' && OffTime[1] == '3')
        {
            OffTime[0] = '0';
            OffTime[1] = '0';
        }
        else if (OffTime[1] == '9')
        {
            OffTime[0] = (int)OffTime[0] + 1;
            OffTime[1] = '0';
        }
        else
        {
            OffTime[1] = (int)OffTime[1] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW)
    { // Decrement
        if (OffTime[0] == '0' && OffTime[1] == '0')
        {
            OffTime[0] = '2';
            OffTime[1] = '3';
        }
        else if (OffTime[1] == '0')
        {
            OffTime[0] = (int)OffTime[0] - 1;
            OffTime[1] = '9';
        }
        else
        {
            OffTime[1] = (int)OffTime[1] - 1;
        }
        delay(50);
    }
    printTimeToLED(1, OffTime);
}

void DisplaySetOFFTimeMM(void)
{
    digitalWrite(OnTimeLED, LOW);
    digitalWrite(OffTimeLED, HIGH);
    if (digitalRead(buttonUp) == LOW)
    { // Increment
        if (OffTime[2] == '5' && OffTime[3] == '9')
        {
            OffTime[2] = '0';
            OffTime[3] = '0';
        }
        else if (OffTime[3] == '9')
        {
            OffTime[2] = (int)OffTime[2] + 1;
            OffTime[3] = '0';
        }
        else
        {
            OffTime[3] = (int)OffTime[3] + 1;
        }
        delay(50);
    }
    if (digitalRead(buttonDown) == LOW)
    { // Decrement
        if (OffTime[2] == '0' && OffTime[3] == '0')
        {
            OffTime[2] = '5';
            OffTime[3] = '9';
        }
        else if (OffTime[3] == '0')
        {
            OffTime[2] = (int)OffTime[2] - 1;
            OffTime[3] = '9';
        }
        else
        {
            OffTime[3] = (int)OffTime[3] - 1;
        }
        delay(50);
    }
    printTimeToLED(1, OffTime);
}

void StoreArgs(void)
{
    Serial.println(previousTime);
    Serial.println(time);

    if (strcmp(previousTime, time) != 0)
    {
        int hoursupg = ((time[0] - '0') * 10) + (time[1] - '0');
        int minutesupg = ((time[2] - '0') * 10) + (time[3] - '0');
        DateTime datatime = rtc.now();
        rtc.adjust(DateTime(datatime.year(), datatime.month(), datatime.day(), hoursupg, minutesupg, 0));
        Serial.println("Time Changed");
    }
    RelayTimes_t relayTmp;
    strcpy(relayTmp.Ontime, OnTime);
    strcpy(relayTmp.Offtime, OffTime);
    EEPROM.put(RelayTimeAddr, relayTmp);
    delay(500);
}

void clearDisplay(int addr) {
    addr = addr * 4;
    lc.setChar(0, addr, " ", false);
    lc.setChar(0, addr + 1, " ", false);
    lc.setChar(0, addr + 2, " ", false);
    lc.setChar(0, addr + 3, " ", false);
}
