#include "ui/ui.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"
#include "display/display.h"
#include "drivers/button.h"
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
    if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE) {
        MAX_ONDATA = EEPROM.read(EEPROM_MAX_ONDATA_ADDR) | (EEPROM.read(EEPROM_MAX_ONDATA_ADDR + 1) << 8);
        VIS_ONDATA = EEPROM.read(EEPROM_VIS_ONDATA_ADDR) | (EEPROM.read(EEPROM_VIS_ONDATA_ADDR + 1) << 8);
        mainDisplayMode = static_cast<MainDisplayMode>(EEPROM.read(EEPROM_MAIN_DISPLAY_ADDR));
        isBigDigitsEnabled = EEPROM.read(EEPROM_BIGDIG_ADDR);
        // яркость и режим светодиода
        LED::setBrightness(EEPROM.read(EEPROM_LED_BRIGHT_ADDR));
        LED::setMode(static_cast<UI::LEDBindMode>(EEPROM.read(EEPROM_LED_TYPE_ADDR)));
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
                    if (podMode >= LED_BIND_MODE_COUNT) podMode = 0;
                    LED::setMode(static_cast<UI::LEDBindMode>(podMode));
                    isChanged = true;
                    break;
                case Mode::LED_Bright:
                    if (podMode > LED_PRESET_MAX) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case Mode::LED_Manual:
                    if (podMode > LED_PRESET_MAX) podMode = 0;
                    LED::setBrightness(podMode);
                    isChanged = true;
                    break;
                case Mode::Menu:
                    if (podMode > MENU_PODMODE_MAX) podMode = 1;
                    isChanged = true;
                    break;
                default:
                    break;
            }
        } else {
            do {
                mode = static_cast<Mode>(static_cast<uint8_t>(mode) + 1);
                if (mode > Mode::Sensor8) mode = Mode::Clock;
#if (CO2_SENSOR == 0)
                if (mode == Mode::Sensor1) mode = Mode::Sensor3;
#endif
            } while (((VIS_ONDATA & (1 << (static_cast<uint8_t>(mode) - 1))) == 0) &&
                     (mode > Mode::Clock));
            isChanged = true;
        }
    }
    if (Button::isDouble()) {
        if (mode > Mode::Clock && mode <= Mode::Sensor8) {
            MAX_ONDATA ^= (1 << (static_cast<uint8_t>(mode) - 1));
        } else if (mode == Mode::Clock) {
            if (mainDisplayMode >= MainDisplayMode::Humidity) {
                mainDisplayMode = MainDisplayMode::Time;
            } else {
                mainDisplayMode = static_cast<MainDisplayMode>(static_cast<uint8_t>(mainDisplayMode) + 1);
            }
#if (CO2_SENSOR == 0)
            if (mainDisplayMode == MainDisplayMode::CO2) mainDisplayMode = static_cast<MainDisplayMode>(static_cast<uint8_t>(mainDisplayMode) + 1);
#endif
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
                // сохранить флаг больших цифр в EEPROM
                EEPROM.write(EEPROM_BIGDIG_ADDR, static_cast<uint8_t>(isBigDigitsEnabled));
                EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
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
                    EEPROM.write(EEPROM_MAX_ONDATA_ADDR, static_cast<uint8_t>(MAX_ONDATA & 255));
                    EEPROM.write(EEPROM_MAX_ONDATA_ADDR + 1, static_cast<uint8_t>((MAX_ONDATA >> 8) & 255));
                    EEPROM.write(EEPROM_VIS_ONDATA_ADDR, static_cast<uint8_t>(VIS_ONDATA & 255));
                    EEPROM.write(EEPROM_VIS_ONDATA_ADDR + 1, static_cast<uint8_t>((VIS_ONDATA >> 8) & 255));
                    EEPROM.write(EEPROM_MAIN_DISPLAY_ADDR, static_cast<uint8_t>(mainDisplayMode));
                    EEPROM.write(EEPROM_BIGDIG_ADDR, static_cast<uint8_t>(isBigDigitsEnabled));
                    // яркость и режим светодиода
                    EEPROM.write(EEPROM_LED_BRIGHT_ADDR, LED::getBrightness());
                    EEPROM.write(EEPROM_LED_TYPE_ADDR, static_cast<uint8_t>(LED::getMode()));
                    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
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
            Display::resetState();  // сбросить состояние для полной перерисовки
            Display::drawSensors();
        } else if (mode <= Mode::Sensor8) {
            Plot::tick();
            Display::redrawPlot(static_cast<uint8_t>(mode));
        } else {
            // меню и настройки
            Display::drawMenu(static_cast<uint8_t>(mode), podMode, VIS_ONDATA);
        }
    } else if (mode >= Mode::LED_Mode) {
        // в режиме меню перерисовываем при каждом tick для обновления значений
        Display::drawMenu(static_cast<uint8_t>(mode), podMode, VIS_ONDATA);
    }
}

}  // namespace UI
