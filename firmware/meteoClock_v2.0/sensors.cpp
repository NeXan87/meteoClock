#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "sensors.h"
#include <Wire.h>
#include <Adafruit_BME280.h>
#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>
#endif

int delta = 0;

// === Статические переменные ===
static Adafruit_BME280 bme;
#if (CO2_SENSOR == 1)
static MHZ19_uart mhz19;
#endif

// === Вспомогательная функция таймера (дублируется из functions.txt) ===
static bool testTimer(unsigned long& last, unsigned long interval) {
    if (millis() - last >= interval) {
        last = millis();
        return true;
    }
    return false;
}

// === Инициализация датчиков ===
void sensorsInit() {
    bme.begin(&Wire);
    bme.setSampling(
        Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::FILTER_OFF
    );

#if (CO2_SENSOR == 1)
    mhz19.begin(MHZ_TX, MHZ_RX);
    mhz19.setAutoCalibration(false);
#endif

    // Инициализация массива давления текущим значением
    bme.takeForcedMeasurement();
    uint32_t initPress = bme.readPressure();
    for (byte i = 0; i < 6; i++) {
        pressure_array[i] = initPress;
    }

    // Первое чтение для инициализации dispAlt
    dispAlt = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

// === Основной опрос датчиков ===
void sensorsRead() {
    bme.takeForcedMeasurement();
    dispTemp = bme.readTemperature() + TEMP_OFFSET;
    dispHum = (byte)bme.readHumidity();
    dispPres = (int)((float)bme.readPressure() * 0.00750062f);  // Pa → mmHg
    dispAlt = (dispAlt + bme.readAltitude(SEALEVELPRESSURE_HPA)) * 0.5f;  // усреднение

#if (CO2_SENSOR == 1)
    dispCO2 = mhz19.getPPM();
#endif
}

// === Обновление буферов графиков ===
void sensorsUpdatePlots() {
    // Часовой график (4 или 5 минут)
    if (testTimer(hourPlotTimerD, HOUR_PLOT_INTERVAL)) {
        for (byte i = 0; i < 14; i++) {
            tempHour[i] = tempHour[i + 1];
            humHour[i] = humHour[i + 1];
            pressHour[i] = pressHour[i + 1];
            co2Hour[i] = co2Hour[i + 1];
            altHour[i] = altHour[i + 1];
        }
        tempHour[14] = (int)dispTemp;
        humHour[14] = dispHum;
        co2Hour[14] = dispCO2;
        altHour[14] = (int)dispAlt;
        if (PRESSURE) {
            pressHour[14] = dispRain;
        } else {
            pressHour[14] = dispPres;
        }
    }

    // Суточный график (усреднение по часовым)
    if (testTimer(dayPlotTimerD, DAY_PLOT_INTERVAL)) {
        long sumTemp = 0, sumHum = 0, sumPress = 0, sumCO2 = 0, sumAlt = 0;
        for (byte i = 0; i < 15; i++) {
            sumTemp += tempHour[i];
            sumHum += humHour[i];
            sumPress += pressHour[i];
            sumCO2 += co2Hour[i];
            sumAlt += altHour[i];
        }
        byte avgTemp = sumTemp / 15;
        byte avgHum = sumHum / 15;
        int avgPress = sumPress / 15;
        int avgCO2 = sumCO2 / 15;
        int avgAlt = sumAlt / 15;

        for (byte i = 0; i < 14; i++) {
            tempDay[i] = tempDay[i + 1];
            humDay[i] = humDay[i + 1];
            pressDay[i] = pressDay[i + 1];
            co2Day[i] = co2Day[i + 1];
            altDay[i] = altDay[i + 1];
        }
        tempDay[14] = avgTemp;
        humDay[14] = avgHum;
        pressDay[14] = avgPress;
        co2Day[14] = avgCO2;
        altDay[14] = avgAlt;
    }
}

// === Прогноз погоды (линейная аппроксимация) ===
void sensorsUpdatePrediction() {
    if (!testTimer(predictTimerD, PREDICT_INTERVAL)) return;

    // Усреднённое давление за 10 измерений
    long avgPress = 0;
    for (byte i = 0; i < 10; i++) {
        bme.takeForcedMeasurement();
        avgPress += bme.readPressure();
        delay(1);
    }
    avgPress /= 10;

    // Сдвиг массива
    for (byte i = 0; i < 5; i++) {
        pressure_array[i] = pressure_array[i + 1];
    }
    pressure_array[5] = avgPress;

    // Линейная регрессия: y = a*x + b
    uint32_t sumX = 0, sumY = 0, sumX2 = 0, sumXY = 0;
    for (int i = 0; i < 6; i++) {
        sumX += i;
        sumY += pressure_array[i];
        sumX2 += i * i;
        sumXY += i * pressure_array[i];
    }

    // Расчёт наклона a
    float a = (6.0f * sumXY - sumX * sumY) / (6.0f * sumX2 - sumX * sumX);
    delta = (int)(a * 6.0f);  // изменение за 6 шагов
    dispRain = map(delta, -250, 250, 100, -100);  // → проценты
}