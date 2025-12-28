#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "clock.h"
#include "display.h"
#include "brightness.h"

void clockInit() {
    rtc.begin();
    if (RESET_CLOCK || rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    now = rtc.now();
    hrs = now.hour();
    mins = now.minute();
    secs = now.second();
}

void clockTick() {
    dotFlag = !dotFlag;

    if (dotFlag) {
        // Обновление секунд каждые 2 вызова (500 мс → 1 сек)
        secs++;
        if (secs > 59) {
            secs = 0;
            mins++;
            if (mins <= 59 && mode == 0) {
                drawSensors();
            }
        }
        if (mins > 59) {
            mins = 0;
            hrs++;
            if (hrs > 23) hrs = 0;
            // Обновляем время из RTC для точности
            now = rtc.now();
            hrs = now.hour();
            mins = now.minute();
            secs = now.second();
            if (mode == 0) {
                drawSensors();
                if (DISPLAY_TYPE == 1 && !bigDig) {
                    drawData();
                }
            }
        }

        // Отображение секунд (только для DISPLAY_TYPE=1, DISP_MODE=1, !bigDig)
        if (DISPLAY_TYPE == 1 && DISP_MODE == 1 && mode == 0 && !bigDig) {
            lcd.setCursor(15, 1);
            if (secs < 10) lcd.print('0');
            lcd.print(secs);
        }
    }

    // Мигание двоеточия
    if (mode == 0) {
        byte code = dotFlag ? ((DISPLAY_TYPE == 1 && bigDig) ? 0 : 58) : 32;
        if (mode0scr == 0) {
            if (bigDig && DISPLAY_TYPE == 1) {
                lcd.setCursor(7, 2);
                lcd.write(code);
                lcd.setCursor(7, 1);
                lcd.write(code);
            } else if (DISPLAY_TYPE == 1) {
                lcd.setCursor(17, 3);
                lcd.write(code);
            }
        }
    }

    // Мигание LED при критических значениях (только при dotFlag == false)
    if (!dotFlag &&
        ((dispCO2 >= blinkLEDCO2 && LEDType == 0) ||
         (dispHum <= blinkLEDHum && LEDType == 1) ||
         (dispTemp >= blinkLEDTemp && LEDType == 2))) {
        // Выключаем LED на "темную" часть мигания
        analogWrite(LED_R, LED_OFF);
        analogWrite(LED_G, LED_OFF);
        analogWrite(LED_B, LED_OFF);
    } else {
        setLED();
    }
}

void clockLoad() {
    loadClock();  // вызов из display.cpp — там создаются символы часов и дня недели
}