#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "eeprom.h"
#include "sensors.h"
#include "brightness.h"
#include "clock.h"
#include "modes.h"
#include "led.h"

void setup() {
    Serial.begin(9600);

    // Инициализация периферии
    pinMode(BACKLIGHT, OUTPUT);
    ledInit();
    setLEDcolor(0);  // выключить LED

    // Загрузка настроек
    eepromLoadSettings();

    // Инициализация дисплея
    lcd.init();
    lcd.backlight();
    lcd.clear();

#if (DEBUG == 1 && DISPLAY_TYPE == 1)
    boolean status = true;
    setLEDcolor(3); // красный — тест CO2
#if (CO2_SENSOR == 1)
    lcd.setCursor(0, 0);
    lcd.print(F("MHZ-19... "));
    mhz19.begin(MHZ_TX, MHZ_RX);
    mhz19.setAutoCalibration(false);
    mhz19.getStatus();
    delay(500);
    if (mhz19.getStatus() == 0) {
        lcd.print(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        status = false;
    }
#endif

    setLEDcolor(15); // жёлтый — тест RTC
    lcd.setCursor(0, 1);
    lcd.print(F("RTC... "));
    if (rtc.begin()) {
        lcd.print(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        status = false;
    }

    setLEDcolor(12); // зелёный — тест BME280
    lcd.setCursor(0, 2);
    lcd.print(F("BME280... "));
    if (bme.begin(&Wire)) {
        lcd.print(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        status = false;
    }

    setLEDcolor(0);
    lcd.setCursor(0, 3);
    if (status) {
        lcd.print(F("All good"));
    } else {
        lcd.print(F("Check wires!"));
    }

    for (byte i = 0; i < 20; i++) {
        lcd.setCursor(14, 1);
        lcd.print("P:");
        lcd.print(analogRead(PHOTO));
        delay(250);
    }
#else
#if (CO2_SENSOR == 1)
    mhz19.begin(MHZ_TX, MHZ_RX);
    mhz19.setAutoCalibration(false);
#endif
    rtc.begin();
    bme.begin(&Wire);
#endif

    // Настройка BME280
    bme.setSampling(
        Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::FILTER_OFF
    );

    // Инициализация модулей
    clockInit();
    sensorsInit();
    brightnessInit();

    // Заполнение буферов начальными значениями
    sensorsRead();
    for (byte i = 0; i < 15; i++) {
        tempHour[i] = (int)dispTemp;
        tempDay[i] = (int)dispTemp;
        humHour[i] = dispHum;
        humDay[i] = dispHum;
        altHour[i] = (int)dispAlt;
        altDay[i] = (int)dispAlt;
        if (PRESSURE) {
            pressHour[i] = 0;
            pressDay[i] = 0;
        } else {
            pressHour[i] = dispPres;
            pressDay[i] = dispPres;
        }
#if (CO2_SENSOR == 1)
        co2Hour[i] = dispCO2;
        co2Day[i] = dispCO2;
#endif
    }

    // Первая отрисовка
    lcd.clear();
    clockLoad();
    drawSensors();
    if (DISPLAY_TYPE == 1) drawData();
}

void loop() {
    if (testTimer(brightTimerD, BRIGHT_INTERVAL)) {
        brightnessUpdate();
    }
    if (testTimer(sensorsTimerD, SENS_TIME)) {
        sensorsRead();
    }
    if (testTimer(clockTimerD, CLOCK_TICK_INTERVAL)) {
        clockTick();
    }
    sensorsUpdatePlots();
    sensorsUpdatePrediction();
    modesTick();

    if (mode == 0) {
        if (testTimer(drawSensorsTimerD, SENS_TIME)) {
            drawSensors();
            if (DISPLAY_TYPE == 1 && !bigDig) {
                drawData();
            }
        }
    } else if (mode >= 1 && mode <= 10) {
        if (testTimer(plotTimerD, HOUR_PLOT_INTERVAL)) {
            redrawPlot();
        }
    }
}