#include "PlotManager.h"

#include <Arduino.h>

#include "Sensors.h"
#include "config.h"

namespace Plot {

static int tempHourArr[15], tempDayArr[15];
static int humHourArr[15], humDayArr[15];
static int pressHourArr[15], pressDayArr[15];
static int altHourArr[15], altDayArr[15];
static int co2HourArr[15], co2DayArr[15];

static unsigned long hourPlotTimer;
static unsigned long dayPlotTimer;
static unsigned long predictTimer;
static unsigned long hourPlotTimerD;
static unsigned long dayPlotTimerD;
static unsigned long predictTimerD;

static uint32_t pressure_array[6];
static uint32_t sumX, sumY, sumX2, sumXY;
static float a;
static int delta;

void init() {
#if (DISPLAY_TYPE == 1)
    hourPlotTimer = ((long)4 * 60 * 1000);
    dayPlotTimer = ((long)1.6 * 60 * 60 * 1000);
#else
    hourPlotTimer = ((long)5 * 60 * 1000);
    dayPlotTimer = ((long)2 * 60 * 60 * 1000);
#endif
    predictTimer = ((long)10 * 60 * 1000);
    hourPlotTimerD = 0;
    dayPlotTimerD = 0;
    predictTimerD = 0;
    // массивы инициализируются нулями
    memset(tempHourArr, 0, sizeof(tempHourArr));
    memset(tempDayArr, 0, sizeof(tempDayArr));
    memset(humHourArr, 0, sizeof(humHourArr));
    memset(humDayArr, 0, sizeof(humDayArr));
    memset(pressHourArr, 0, sizeof(pressHourArr));
    memset(pressDayArr, 0, sizeof(pressDayArr));
    memset(altHourArr, 0, sizeof(altHourArr));
    memset(altDayArr, 0, sizeof(altDayArr));
    memset(co2HourArr, 0, sizeof(co2HourArr));
    memset(co2DayArr, 0, sizeof(co2DayArr));
}

bool testTimer(unsigned long& dataTimer, unsigned long setTimer) {
    if (millis() - dataTimer >= setTimer) {
        dataTimer = millis();
        return true;
    }
    return false;
}

void tick() {
    if (testTimer(hourPlotTimerD, hourPlotTimer)) {
        for (byte i = 0; i < 14; i++) {
            tempHourArr[i] = tempHourArr[i + 1];
            humHourArr[i] = humHourArr[i + 1];
            pressHourArr[i] = pressHourArr[i + 1];
            altHourArr[i] = altHourArr[i + 1];
            co2HourArr[i] = co2HourArr[i + 1];
        }
        tempHourArr[14] = Sensors::getTemp();
        humHourArr[14] = Sensors::getHumidity();
        pressHourArr[14] = Sensors::getPres();
        altHourArr[14] = Sensors::getAlt();
        co2HourArr[14] = Sensors::getCO2();
    }
    if (testTimer(dayPlotTimerD, dayPlotTimer)) {
        long averTemp = 0, averHum = 0, averPress = 0, averAlt = 0, averCO2 = 0;
        for (byte i = 0; i < 15; i++) {
            averTemp += tempHourArr[i];
            averHum += humHourArr[i];
            averPress += pressHourArr[i];
            averAlt += altHourArr[i];
            averCO2 += co2HourArr[i];
        }
        averTemp /= 15;
        averHum /= 15;
        averPress /= 15;
        averAlt /= 15;
        averCO2 /= 15;
        for (byte i = 0; i < 14; i++) {
            tempDayArr[i] = tempDayArr[i + 1];
            humDayArr[i] = humDayArr[i + 1];
            pressDayArr[i] = pressDayArr[i + 1];
            altDayArr[i] = altDayArr[i + 1];
            co2DayArr[i] = co2DayArr[i + 1];
        }
        tempDayArr[14] = averTemp;
        humDayArr[14] = averHum;
        pressDayArr[14] = averPress;
        altDayArr[14] = averAlt;
        co2DayArr[14] = averCO2;
    }
    if (testTimer(predictTimerD, predictTimer)) {
        long averPress = 0;
        for (byte i = 0; i < 10; i++) {
            // в данном месте подразумевается, что датчик BME280 будет доступен
            averPress += Sensors::getPres();
            delay(1);
        }
        averPress /= 10;
        for (byte i = 0; i < 5; i++) {
            pressure_array[i] = pressure_array[i + 1];
        }
        pressure_array[5] = averPress;
        sumX = sumY = sumX2 = sumXY = 0;
        for (int i = 0; i < 6; i++) {
            sumX += i;
            sumY += (long)pressure_array[i];
            sumX2 += i * i;
            sumXY += (long)i * pressure_array[i];
        }
        a = (long)6 * sumXY - (long)sumX * sumY;
        a /= (6 * sumX2 - sumX * sumX);
        delta = a * 6;
        int rainPercent = map(delta, -250, 250, 100, -100);
        Sensors::setRain(rainPercent);
    }
}

int* tempHour() { return tempHourArr; }
int* tempDay() { return tempDayArr; }
int* humHour() { return humHourArr; }
int* humDay() { return humDayArr; }
int* pressHour() { return pressHourArr; }
int* pressDay() { return pressDayArr; }
int* altHour() { return altHourArr; }
int* altDay() { return altDayArr; }
int* co2Hour() { return co2HourArr; }
int* co2Day() { return co2DayArr; }

}  // namespace Plot
