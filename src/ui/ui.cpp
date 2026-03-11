#include "ui/ui.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "button/button.h"
#include "config.h"
#include "display/display.h"
#include "led/led-indicator.h"
#include "plot/plot.h"
#include "ui/enums.h"

namespace UI {

static Mode mode = Mode::Clock;
static uint8_t podMode = 1;
static MainDisplayMode mainDisplayMode = MainDisplayMode::Time;
static bool isBigDigitsEnabled = false;

// маски отображения и масштабирования
static int MAX_ONDATA = DEFAULT_MAX_ONDATA;
static int VIS_ONDATA = DEFAULT_VIS_ONDATA;

uint8_t currentMode() { return static_cast<uint8_t>(mode); }

int getMaxOnData() { return MAX_ONDATA; }
int getVisOnData() { return VIS_ONDATA; }

uint8_t getMode0Scr() { return static_cast<uint8_t>(mainDisplayMode); }
bool isBigDigits() { return isBigDigitsEnabled; }

void init() {
    // прочитать сохранённые настройки из EEPROM
    if (EEPROM.read(0) == 122) {
        MAX_ONDATA = EEPROM.read(2) | (EEPROM.read(3) << 8);
        VIS_ONDATA = EEPROM.read(4) | (EEPROM.read(5) << 8);
        mainDisplayMode = static_cast<MainDisplayMode>(EEPROM.read(6));
        isBigDigitsEnabled = EEPROM.read(7);
        // яркость и режим светодиода при желании можно считать здесь
    }
}

void tick() {
    Button::tick();
    bool isChanged = false;
    if (Button::isSingle()) {
        if (mode >= Mode::LED_Mode) {
            podMode++;
            switch (static_cast<Mode>(mode)) {
                case Mode::LED_Mode:
                    if (podMode > 4) podMode = 0;
                    LED::setMode(static_cast<UI::LEDBindMode>(podMode));
                    isChanged = true;
                    break;
                case Mode::LED_Bright:
                    if (podMode > 11) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case Mode::LED_Manual:
                    if (podMode > 11) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case Mode::Menu:
                    if (podMode > 15) podMode = 1;
                    isChanged = true;
                    break;
                default:
                    break;
            }
        } else {
            do {
                mode = static_cast<Mode>(static_cast<uint8_t>(mode) + 1);
                if (mode > Mode::Sensor10) mode = Mode::Clock;
#if (CO2_SENSOR == 0)
                if (mode == Mode::Sensor1) mode = Mode::Sensor3;
#endif
            } while (((VIS_ONDATA & (1 << (static_cast<uint8_t>(mode) - 1))) == 0) &&
                     (mode > Mode::Clock));
            isChanged = true;
        }
    }
    if (Button::isDouble()) {
        if (mode > Mode::Clock && mode <= Mode::Sensor10) {
            MAX_ONDATA ^= (1 << (static_cast<uint8_t>(mode) - 1));
        } else if (mode == Mode::Clock) {
            mainDisplayMode = static_cast<MainDisplayMode>(static_cast<uint8_t>(mainDisplayMode) + 1);
#if (CO2_SENSOR == 0)
            if (mainDisplayMode == MainDisplayMode::CO2) mainDisplayMode = static_cast<MainDisplayMode>(static_cast<uint8_t>(mainDisplayMode) + 1);
#endif
            if (mainDisplayMode > MainDisplayMode::Altitude) mainDisplayMode = MainDisplayMode::Time;
        } else if (mode >= Mode::LED_Mode) {
            podMode = 1;
        }
        isChanged = true;
    }
    if (Button::isTriple() && mode == Mode::Clock) {
        mode = Mode::Menu;
        podMode = 3;
        isChanged = true;
    }
    if (Button::isHolded()) {
        switch (static_cast<Mode>(mode)) {
            case Mode::Clock:
                isBigDigitsEnabled = !isBigDigitsEnabled;
                break;
            case Mode::LED_Mode:
                mode = Mode::Menu;
                podMode = 1;
                break;
            case Mode::LED_Bright:
                mode = Mode::Menu;
                podMode = 1;
                break;
            case Mode::LED_Manual:
                mode = Mode::Menu;
                podMode = 1;
                break;
            case Mode::Menu:
                if (podMode == 2 || podMode == 1) mode = Mode::Clock;
                if (podMode >= 3 && podMode <= 5) mode = static_cast<Mode>(255 - podMode + 2);
                if (podMode >= 6 && podMode <= 17) VIS_ONDATA ^= (1 << (podMode - 6));
                if (podMode == 1) {
                    // сохранить настройки массива и экрана
                    EEPROM.write(2, MAX_ONDATA & 255);
                    EEPROM.write(3, (MAX_ONDATA >> 8) & 255);
                    EEPROM.write(4, VIS_ONDATA & 255);
                    EEPROM.write(5, (VIS_ONDATA >> 8) & 255);
                    EEPROM.write(6, static_cast<uint8_t>(mainDisplayMode));
                    EEPROM.write(7, isBigDigitsEnabled);
                    // яркость и режим светодиода сохраняются в других ячейках
                    EEPROM.write(0, 122);
                }
                if (podMode < 6) podMode = 1;
                break;
            default:
                mode = Mode::Clock;
        }
        isChanged = true;
    }

    if (isChanged) {
        // когда режим изменился, перерисовываем экран через модуль Display
        if (mode == Mode::Clock) {
            Display::clear();
            Display::drawSensors();
        } else if (mode <= Mode::Sensor10) {
            Plot::tick();
            Display::redrawPlot(static_cast<uint8_t>(mode));
        } else {
            // меню и настройки пока обрабатываются Display/другими модулями
            Display::clear();
        }
    }
}

}  // namespace UI
