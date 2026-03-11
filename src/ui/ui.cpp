#include "ui/ui.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "button/button.h"
#include "config.h"
#include "display/display.h"
#include "led/led-indicator.h"
#include "plot/plot.h"

namespace UI {

static uint8_t mode = 0;
static uint8_t podMode = 1;
static uint8_t mode0scr = 0;
static bool isBigDigitsEnabled = false;

// маски отображения и масштабирования
static int MAX_ONDATA = DEFAULT_MAX_ONDATA;
static int VIS_ONDATA = DEFAULT_VIS_ONDATA;

uint8_t currentMode() { return mode; }

int getMaxOnData() { return MAX_ONDATA; }
int getVisOnData() { return VIS_ONDATA; }

uint8_t getMode0Scr() { return mode0scr; }
bool isBigDigits() { return isBigDigitsEnabled; }

void init() {
    // прочитать сохранённые настройки из EEPROM
    if (EEPROM.read(0) == 122) {
        MAX_ONDATA = EEPROM.read(2) | (EEPROM.read(3) << 8);
        VIS_ONDATA = EEPROM.read(4) | (EEPROM.read(5) << 8);
        mode0scr = EEPROM.read(6);
        isBigDigitsEnabled = EEPROM.read(7);
        // яркость и режим светодиода при желании можно считать здесь
    }
}

void tick() {
    Button::tick();
    bool isChanged = false;
    if (Button::isSingle()) {
        if (mode >= 240) {
            podMode++;
            switch (mode) {
                case 252:
                    if (podMode > 4) podMode = 0;
                    LED::setMode(podMode);
                    isChanged = true;
                    break;
                case 253:
                    if (podMode > 11) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case 254:
                    if (podMode > 11) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case 255:
                    if (podMode > 15) podMode = 1;
                    isChanged = true;
                    break;
            }
        } else {
            do {
                mode++;
                if (mode > 10) mode = 0;
#if (CO2_SENSOR == 0)
                if (mode == 1) mode = 3;
#endif
            } while (((VIS_ONDATA & (1 << (mode - 1))) == 0) && (mode > 0));
            isChanged = true;
        }
    }
    if (Button::isDouble()) {
        if (mode > 0 && mode < 11) {
            MAX_ONDATA ^= (1 << (mode - 1));
        } else if (mode == 0) {
            mode0scr++;
#if (CO2_SENSOR == 0)
            if (mode0scr == 1) mode0scr++;
#endif
            if (mode0scr > 5) mode0scr = 0;
        } else if (mode > 240) {
            podMode = 1;
        }
        isChanged = true;
    }
    if (Button::isTriple() && mode == 0) {
        mode = 255;
        podMode = 3;
        isChanged = true;
    }
    if (Button::isHolded()) {
        switch (mode) {
            case 0:
                isBigDigitsEnabled = !isBigDigitsEnabled;
                break;
            case 252:
                mode = 255;
                podMode = 1;
                break;
            case 253:
                mode = 255;
                podMode = 1;
                break;
            case 254:
                mode = 255;
                podMode = 1;
                break;
            case 255:
                if (podMode == 2 || podMode == 1) mode = 0;
                if (podMode >= 3 && podMode <= 5) mode = 255 - podMode + 2;
                if (podMode >= 6 && podMode <= 17) VIS_ONDATA ^= (1 << (podMode - 6));
                if (podMode == 1) {
                    // сохранить настройки массива и экрана
                    EEPROM.write(2, MAX_ONDATA & 255);
                    EEPROM.write(3, (MAX_ONDATA >> 8) & 255);
                    EEPROM.write(4, VIS_ONDATA & 255);
                    EEPROM.write(5, (VIS_ONDATA >> 8) & 255);
                    EEPROM.write(6, mode0scr);
                    EEPROM.write(7, isBigDigitsEnabled);
                    // яркость и режим светодиода сохраняются в других ячейках
                    EEPROM.write(0, 122);
                }
                if (podMode < 6) podMode = 1;
                break;
            default:
                mode = 0;
        }
        isChanged = true;
    }

    if (isChanged) {
        // когда режим изменился, перерисовываем экран через модуль Display
        if (mode == 0) {
            Display::clear();
            Display::drawSensors();
        } else if (mode <= 10) {
            Plot::tick();
            Display::redrawPlot(mode);
        } else {
            // меню и настройки пока обрабатываются Display/другими модулями
            Display::clear();
        }
    }
}

}  // namespace UI
