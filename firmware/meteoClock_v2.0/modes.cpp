#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "modes.h"
#include "eeprom.h"
#include "sensors.h"
#include "clock.h"
#include "plot.h"
#include "display.h"
#include "brightness.h"

void modesInit() {
    // Инициализация начального состояния — уже задано в globals.cpp
}

void modesTick() {
    button.tick();
    boolean changeFlag = false;

    if (button.isSingle()) {
        if (mode >= 240) {
            podMode++;
            switch (mode) {
                case 252: // Режим индикатора
                    if (podMode > 4) podMode = 0;
                    LEDType = podMode;
                    changeFlag = true;
                    break;
                case 253: // Яркость экрана
                    if (podMode > 11) podMode = 0;
                    LCD_BRIGHT = podMode;
                    brightnessUpdate();
                    changeFlag = true;
                    break;
                case 254: // Яркость индикатора
                    if (podMode > 11) podMode = 0;
                    LED_BRIGHT = podMode;
                    changeFlag = true;
                    break;
                case 255: // Главное меню
                    if (podMode > 15) podMode = 1;
                    changeFlag = true;
                    break;
            }
        } else {
            do {
                mode++;
                if (mode > 10) mode = 0;
#if (CO2_SENSOR == 0)
                if (mode == 1 || mode == 2) mode = 3;
#endif
            } while (mode > 0 && ((VIS_ONDATA & (1 << (mode - 1))) == 0));
            changeFlag = true;
        }
    }

    if (button.isDouble()) {
        if (mode >= 1 && mode <= 10) {
            MAX_ONDATA ^= (1 << (mode - 1));
        } else if (mode == 0) {
            mode0scr++;
#if (CO2_SENSOR == 0)
            if (mode0scr == 1) mode0scr++;
#endif
            if (mode0scr > 5) mode0scr = 0;
        } else if (mode >= 240) {
            podMode = 1;
        }
        changeFlag = true;
    }

    if (button.isTriple() && mode == 0) {
        mode = 255;
        podMode = 3;
        changeFlag = true;
    }

    if (button.isHolded()) {
        switch (mode) {
            case 0:
                bigDig = !bigDig;
                break;
            case 252:
            case 253:
            case 254:
                mode = 255;
                podMode = 1;
                break;
            case 255:
                if (podMode == 1 || podMode == 2) {
                    mode = 0;
                } else if (podMode >= 3 && podMode <= 5) {
                    mode = 255 - podMode + 2; // 252, 253, 254
                } else if (podMode >= 6 && podMode <= 17) {
                    VIS_ONDATA ^= (1 << (podMode - 6));
                }
                if (podMode == 1) {
                    eepromSaveSettings();
                }
                if (podMode < 6) podMode = 1;
                if (mode == 252) podMode = LEDType;
                if (mode == 253) podMode = LCD_BRIGHT;
                if (mode == 254) podMode = LED_BRIGHT;
                break;
            default:
                mode = 0;
        }
        changeFlag = true;
    }

    if (changeFlag) {
        if (mode >= 240) {
            lcd.clear();
            displayLoadClock();
            lcd.setCursor(0, 0);
        }

        if (mode == 255) {
#if (WEEK_LANG == 1)
            lcd.print("HACTPO\2K\4:");
#else
            lcd.print("Setup:");
#endif
            lcd.setCursor(0, 1);
            if (podMode == 1) {
#if (WEEK_LANG == 1)
                lcd.print("COXPAH\4T\1");
#else
                lcd.print("Save");
#endif
            } else if (podMode == 2) {
#if (WEEK_LANG == 1)
                lcd.print("B\6XO\3");
#else
                lcd.print("Exit");
#endif
            } else if (podMode == 5) {
#if (WEEK_LANG == 1)
                lcd.print("PE\10.\4H\3\4KATOPA");
#else
                lcd.print("indicator mode");
#endif
            } else if (podMode == 3) {
#if (WEEK_LANG == 1)
                lcd.print("\5PK.\4H\3\4KATOPA");
#else
                lcd.print("indicator brt.");
#endif
            } else if (podMode == 4) {
#if (WEEK_LANG == 1)
                lcd.print("\5PK.\7KPAHA");
#else
                lcd.print("Bright LCD");
#endif
            } else if (podMode >= 6 && podMode <= 17) {
                // Графики — вывод через plotDrawMenu()
                plotDrawMenu(podMode);
            }
        }

        if (mode == 252) {
            lcd.clear();
            lcd.setCursor(0, 0);
#if (WEEK_LANG == 1)
            lcd.print("PE\10.\4H\3\4KATOPA:");
#else
            lcd.print("indicator mode:");
#endif
            lcd.setCursor(0, 1);
            if (podMode == 0) lcd.print("CO2   ");
            else if (podMode == 1) lcd.print("B\6A\10H.");
            else if (podMode == 2) lcd.print("t\337     ");
            else if (podMode == 3) lcd.print("OCA\3K\5");
            else if (podMode == 4) lcd.print("\3AB\6EH\5E");
        }

        if (mode == 253) {
            lcd.clear();
#if (WEEK_LANG == 1)
            lcd.print("\5PK.\7KPAHA:");
#else
            lcd.print("Bright LCD:");
#endif
            if (LCD_BRIGHT == 11) {
#if (WEEK_LANG == 1)
                lcd.print("ABTO ");
#else
                lcd.print("Auto ");
#endif
            } else {
                char buf[5];
                itoa(LCD_BRIGHT * 10, buf, 10);
                lcd.print(buf);
                lcd.print("%");
            }
        }

        if (mode == 254) {
            lcd.clear();
#if (WEEK_LANG == 1)
            lcd.print("\5PK.\4H\3\4K.:");
#else
            lcd.print("indic.brt.:");
#endif
            if (LED_BRIGHT == 11) {
#if (WEEK_LANG == 1)
                lcd.print("ABTO ");
#else
                lcd.print("Auto ");
#endif
            } else {
                char buf[5];
                itoa(LED_BRIGHT * 10, buf, 10);
                lcd.print(buf);
                lcd.print("%");
            }
        }

        if (mode == 0) {
            lcd.clear();
            clockLoad();
            drawSensors();
            if (DISPLAY_TYPE == 1) displayDrawData();
        } else if (mode >= 1 && mode <= 10) {
            plotLoad();
            redrawPlot();
        }
    }
}

void redrawPlot() {
    lcd.clear();
    plotDraw(mode);
}