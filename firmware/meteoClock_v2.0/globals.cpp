#include "globals.h"

// Определения глобальных переменных (без extern!)
int8_t hrs = 0, mins = 0, secs = 0;
RTC_DS3231 rtc;
DateTime now;

float dispTemp = 0;
byte dispHum = 0;
int dispPres = 0;
int dispCO2 = -1;
int dispRain = 0;
float dispAlt = 0;

int tempHour[15] = {0}, tempDay[15] = {0};
int humHour[15] = {0}, humDay[15] = {0};
int pressHour[15] = {0}, pressDay[15] = {0};
int co2Hour[15] = {0}, co2Day[15] = {0};
int altHour[15] = {0}, altDay[15] = {0};
uint32_t pressure_array[6] = {0};

int MAX_ONDATA = CHART_CO2_HOUR | CHART_CO2_DAY | CHART_HUM_HOUR | CHART_HUM_DAY |
                 CHART_TEMP_HOUR | CHART_TEMP_DAY | CHART_RAIN_HOUR | CHART_RAIN_DAY |
                 CHART_PRESS_HOUR | CHART_PRESS_DAY | CHART_ALT_HOUR | CHART_ALT_DAY;

int VIS_ONDATA = MAX_ONDATA;

byte mode0scr = 0;
boolean bigDig = false;
byte LED_BRIGHT = 10;
byte LCD_BRIGHT = 10;
byte LEDType = 0;

unsigned long sensorsTimerD = 0;
unsigned long drawSensorsTimerD = 0;
unsigned long clockTimerD = 0;
unsigned long hourPlotTimerD = 0;
unsigned long dayPlotTimerD = 0;
unsigned long predictTimerD = 0;
unsigned long brightTimerD = 0;
unsigned long plotTimerD = 0;

byte mode = 0;
byte podMode = 1;
bool dotFlag = false;