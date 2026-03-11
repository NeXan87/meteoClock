#include "plot/plot.h"

#include <Arduino.h>

#include "config.h"
#include "plot/enums.h"
#include "sensors/sensors.h"

namespace Plot {

static int tempHourArr[PLOT_SAMPLES], tempDayArr[PLOT_SAMPLES];
static int humHourArr[PLOT_SAMPLES], humDayArr[PLOT_SAMPLES];
static int pressHourArr[PLOT_SAMPLES], pressDayArr[PLOT_SAMPLES];
static int altHourArr[PLOT_SAMPLES], altDayArr[PLOT_SAMPLES];
static int co2HourArr[PLOT_SAMPLES], co2DayArr[PLOT_SAMPLES];

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
    // таймеры для LCD2004
    hourPlotTimer = PLOT_HOUR_INTERVAL_MS;
    dayPlotTimer = PLOT_DAY_INTERVAL_MS;
    predictTimer = PREDICT_INTERVAL_MS;
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
        for (byte i = 0; i < PLOT_SAMPLES - 1; i++) {
            tempHourArr[i] = tempHourArr[i + 1];
            humHourArr[i] = humHourArr[i + 1];
            pressHourArr[i] = pressHourArr[i + 1];
            altHourArr[i] = altHourArr[i + 1];
            co2HourArr[i] = co2HourArr[i + 1];
        }
        tempHourArr[PLOT_SAMPLES - 1] = Sensors::getTemp();
        humHourArr[PLOT_SAMPLES - 1] = Sensors::getHumidity();
        pressHourArr[PLOT_SAMPLES - 1] = Sensors::getPres();
        altHourArr[PLOT_SAMPLES - 1] = Sensors::getAlt();
        co2HourArr[PLOT_SAMPLES - 1] = Sensors::getCO2();
    }
    if (testTimer(dayPlotTimerD, dayPlotTimer)) {
        long averTemp = 0, averHum = 0, averPress = 0, averAlt = 0, averCO2 = 0;
        for (byte i = 0; i < PLOT_SAMPLES; i++) {
            averTemp += tempHourArr[i];
            averHum += humHourArr[i];
            averPress += pressHourArr[i];
            averAlt += altHourArr[i];
            averCO2 += co2HourArr[i];
        }
        averTemp /= PLOT_SAMPLES;
        averHum /= PLOT_SAMPLES;
        averPress /= PLOT_SAMPLES;
        averAlt /= PLOT_SAMPLES;
        averCO2 /= PLOT_SAMPLES;
        for (byte i = 0; i < PLOT_SAMPLES - 1; i++) {
            tempDayArr[i] = tempDayArr[i + 1];
            humDayArr[i] = humDayArr[i + 1];
            pressDayArr[i] = pressDayArr[i + 1];
            altDayArr[i] = altDayArr[i + 1];
            co2DayArr[i] = co2DayArr[i + 1];
        }
        tempDayArr[PLOT_SAMPLES - 1] = averTemp;
        humDayArr[PLOT_SAMPLES - 1] = averHum;
        pressDayArr[PLOT_SAMPLES - 1] = averPress;
        altDayArr[PLOT_SAMPLES - 1] = averAlt;
        co2DayArr[PLOT_SAMPLES - 1] = averCO2;
    }
    if (testTimer(predictTimerD, predictTimer)) {
        long averPress = 0;
        for (byte i = 0; i < PREDICT_SAMPLE_COUNT; i++) {
            // в данном месте подразумевается, что датчик BME280 будет доступен
            averPress += Sensors::getPres();
            delay(PREDICT_READ_DELAY_MS);
        }
        averPress /= 10;
        averPress /= PREDICT_SAMPLE_COUNT;
        for (byte i = 0; i < PRESSURE_SAMPLE_COUNT - 1; i++) {
            pressure_array[i] = pressure_array[i + 1];
        }
        pressure_array[PRESSURE_SAMPLE_COUNT - 1] = averPress;
        sumX = sumY = sumX2 = sumXY = 0;
        for (int i = 0; i < 6; i++) {
            sumX += i;
            sumY += (long)pressure_array[i];
            sumX2 += i * i;
            sumXY += (long)i * pressure_array[i];
        }
        a = (long)PRESSURE_SAMPLE_COUNT * sumXY - (long)sumX * sumY;
        a /= (PRESSURE_SAMPLE_COUNT * sumX2 - sumX * sumX);
        delta = a * PRESSURE_SAMPLE_COUNT;
        int rainPercent = map(delta, PRESSURE_DELTA_MIN, PRESSURE_DELTA_MAX, RAIN_MAP_OUT_MIN, RAIN_MAP_OUT_MAX);
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
