#include "display/display.h"

#include <Arduino.h>
#include <Wire.h>

#include "clock/clock.h"  // для времени
#include "config.h"
#include "display/custom-chars.h"
#include "led/led-indicator.h"  // для яркости LED в меню
#include "plot/plot.h"          // для доступа к массивам графиков
#include "sensors/sensors.h"    // для получения данных
#include "ui/enums.h"
#include "ui/ui.h"
// character codes
#ifndef CHAR_DOT_BIG
#define CHAR_DOT_BIG 223
#endif
#ifndef CHAR_SPACE
#define CHAR_SPACE 32
#endif
#ifndef CHAR_COLON_ALT
#define CHAR_COLON_ALT 58
#endif

using UI::MainDisplayMode;

namespace Display {

static LiquidCrystal_I2C lcd(DISPLAY_ADDR, LCD_COLS, LCD_ROWS);  // только LCD2004

// символы, ранее были глобальными в main .ino
static uint8_t rowS[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b00000};
static uint8_t row7[8] = {0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row6[8] = {0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row5[8] = {0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row4[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row3[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};
static uint8_t row2[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
static uint8_t row1[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};

// другие наборы символов (буквы, цифры)
// ... (для краткости не всё включено здесь) можно добавить по необходимости

// текущее состояние дисплея для сравнения
struct DisplayState {
    float temp = -999.0f;
    uint8_t humidity = 255;
    int pres = -1;
    int co2 = -1;
    float alt = -9999.0f;
    int rain = -1;
    int hours = -1;
    int minutes = -1;
    bool dotOn = false;
    uint8_t mode0scr = 255;
    bool isBigDigits = false;
    bool initialized = false;
};

static DisplayState prevState;  // состояние для сравнения в крупном режиме

// массивы, используемые для построения больших цифр (переписаны из оригинального скетча)
static uint8_t UB[8] = {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};   // для двустрочных 7,0 и четырехстрочных 2,3,4,5,6,8,9
static uint8_t UMB[8] = {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};  // для двустрочных 2,3,5,6,8,9
static uint8_t LMB[8] = {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};  // для двустрочных 2,3,5,6,8,9
static uint8_t LM2[8] = {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};  // для двустрочной 4
static uint8_t UT[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000};   // для четырехстрочных 2,3,4,5,6,7,8,9,0

static uint8_t KU[8] = {0b00000, 0b00000, 0b00000, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000};  // для верхней части %
static uint8_t KD[8] = {0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000, 0b00000, 0b00000};  // для нижней части %
// конец дополнительной графики

void init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    pinMode(BACKLIGHT, OUTPUT);
    analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
    createCustomChars();
    if (DEBUG) {
        Serial.println("Display::init called");
        // попробуем показать тестовый текст
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Display init");
    }
}

// Таймеры для обновления яркости
static unsigned long brightTimer = 2000;
static unsigned long brightTimerD = 0;

// Текущее значение яркости LCD (0..10, 11 - авто)
static uint8_t LCD_BRIGHT = 11;

// Автоматическая/ручная настройка яркости подсветки
static void checkBrightness() {
    int photoValue = analogRead(PHOTO);
    static bool isDark = false;

    if (isDark) {
        if (photoValue > BRIGHT_THRESHOLD + BRIGHT_HYSTERESYS) {
            isDark = false;
        }
    } else {
        if (photoValue < BRIGHT_THRESHOLD - BRIGHT_HYSTERESYS) {
            isDark = true;
        }
    }

    if (LCD_BRIGHT == 11) {
        if (isDark) {
            analogWrite(BACKLIGHT, LCD_BRIGHT_MIN);
        } else {
            analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
        }
    } else {
        analogWrite(BACKLIGHT, static_cast<int>(LCD_BRIGHT * LCD_BRIGHT * 2.5));
    }
}

void tick() {
    unsigned long now = millis();
    if (now - brightTimerD >= brightTimer) {
        brightTimerD = now;
        checkBrightness();
    }
}

void resetState() {
    prevState = DisplayState();  // сброс состояния для крупного режима
}

void clear() {
    lcd.clear();
}

void createCustomChars() {
    // создать символы, которые используются в любом месте
    lcd.createChar(1, rowS);
    lcd.createChar(2, row7);
    lcd.createChar(3, row6);
    lcd.createChar(4, row5);
    lcd.createChar(5, row4);
    lcd.createChar(6, row3);
    lcd.createChar(7, row2);
    lcd.createChar(0, row1);
    // дополнительные символы для русских букв и индикаторов можно определить при необходимости
    prevState.initialized = false;  // сброс состояния при пересоздании символов
}

// главная функция, выводящая все показания на экран
// предварительные объявления вспомогательных функций, используемых ниже
static void drawClock(int hours, int minutes, int x, int y);
static void drawTemp(float dispTemp, int x, int y);
static void drawHum(int dispHum, int x, int y);
static void drawPPM(int dispCO2, int x, int y);
static void drawPres(int dispPres, int x, int y);
static void drawAlt(float dispAlt, int x, int y);
static void drawPlot(int pos, int row, int width, int height,
                     int min_val, int max_val, int* plot_array,
                     const char* label1, const char* label2, int stretch);
static void drawIndicatorMode();
static void drawBridLCD();
static void drawBridLED();

static void loadClock();
static void loadPlot();

void drawSensors() {
    // каждый раз обновляем кастомные символы для часов/цифр
    loadClock();

    float dispTemp = Sensors::getTemp();
    uint8_t dispHum = Sensors::getHumidity();
    int dispPres = Sensors::getPres();
    int dispCO2 = Sensors::getCO2();
    float dispAlt = Sensors::getAlt();
    int dispRain = Sensors::getRain();

    int hrs = Clock::getHours();
    int mins = Clock::getMinutes();

    MainDisplayMode mode0scr = static_cast<MainDisplayMode>(UI::getMode0Scr());
    bool isBig = UI::isBigDigits();

    // проверка на первую инициализацию или смену режима
    bool forceRedraw = !prevState.initialized ||
                       prevState.mode0scr != static_cast<uint8_t>(mode0scr) ||
                       prevState.isBigDigits != isBig;

    // при смене режима сбрасываем prevState для корректного сравнения
    if (forceRedraw) {
        prevState = DisplayState();
    }

    // вывод для LCD2004
    if (forceRedraw || dispTemp != prevState.temp) {
        if (mode0scr != MainDisplayMode::Temperature) {  // температура
            lcd.setCursor(0, 2);
            if (isBig) {
                if (mode0scr == MainDisplayMode::CO2) lcd.setCursor(LCD_PLOT_COLUMN, 2);
                if (mode0scr != MainDisplayMode::CO2) lcd.setCursor(LCD_PLOT_COLUMN, 0);
            }
            lcd.print(String(dispTemp, 1));
            lcd.write(239);
        } else {
            drawTemp(dispTemp, 0, 0);
        }

        prevState.temp = dispTemp;
    }

    if (forceRedraw || dispHum != prevState.humidity) {
        if (mode0scr != MainDisplayMode::Humidity) {  // влажность
            lcd.setCursor(5, 2);
            if (isBig) lcd.setCursor(LCD_PLOT_COLUMN, 1);
            lcd.print(" " + String(dispHum) + "% ");
            prevState.humidity = dispHum;
        } else {
            drawHum(dispHum, 0, 0);
        }

        prevState.humidity = dispHum;
    }

#if (CO2_SENSOR == 1)
    if (forceRedraw || dispCO2 != prevState.co2) {
        if (mode0scr != MainDisplayMode::CO2) {  // СО2
            if (isBig) {
                lcd.setCursor(LCD_PLOT_COLUMN, 2);
                lcd.print(String(dispCO2) + "p");
            } else {
                lcd.setCursor(11, 2);
                lcd.print(String(dispCO2) + "ppm ");
            }
        } else {
            drawPPM(dispCO2, 0, 0);
        }

        prevState.co2 = dispCO2;
    }
#endif

    if (forceRedraw || dispPres != prevState.pres) {
        if (mode0scr != MainDisplayMode::Pressure) {  // давление
            lcd.setCursor(0, 3);
            if (isBig && mode0scr == MainDisplayMode::Time) lcd.setCursor(LCD_PLOT_COLUMN, 3);
            if (isBig && (mode0scr == MainDisplayMode::CO2 || mode0scr == MainDisplayMode::Temperature)) lcd.setCursor(LCD_PLOT_COLUMN, 0);
            if (isBig && mode0scr == MainDisplayMode::Humidity) lcd.setCursor(LCD_PLOT_COLUMN, 1);
            if (!(isBig && mode0scr == MainDisplayMode::CO2)) lcd.print(String(dispPres) + "mm");
        } else {
            drawPres(dispPres, 0, 0);
        }

        prevState.pres = dispPres;
    }

    if (forceRedraw || dispAlt != prevState.alt) {
        if (mode0scr != MainDisplayMode::Altitude) {
            // ничего
        } else {
            drawAlt(dispAlt, 0, 0);
        }

        prevState.alt = dispAlt;
    }

    if (forceRedraw || dispRain != prevState.rain) {
        if (!isBig) {
            lcd.setCursor(5, 3);
            lcd.print(" rain     ");
            lcd.setCursor(11, 3);
            if (dispRain < 0) lcd.setCursor(10, 3);
            lcd.print(String(dispRain) + "%");
        }

        prevState.rain = dispRain;
    }

    if (hrs != prevState.hours || mins != prevState.minutes) {
        if (mode0scr != MainDisplayMode::Time) {
            lcd.setCursor(LCD_PLOT_COLUMN, 3);
            if (hrs / 10 == 0) lcd.print(" ");
            lcd.print(hrs);
            lcd.print(":");
            if (mins / 10 == 0) lcd.print("0");
            lcd.print(mins);
        } else {
            drawClock(hrs, mins, 0, 0);
        }

        prevState.hours = hrs;
        prevState.minutes = mins;
    }

    // мигающие точки между цифрами
    byte code = Clock::isDotOn() ? CHAR_DOT_BIG : CHAR_SPACE;
    if (forceRedraw || code != (prevState.dotOn ? CHAR_DOT_BIG : CHAR_SPACE)) {
        if (mode0scr == MainDisplayMode::Time) {
            if (isBig) lcd.setCursor(7, 2);
            else lcd.setCursor(7, 0);
            lcd.write(code);
            lcd.setCursor(7, 1);
            lcd.write(code);
        } else {
            if (code == CHAR_DOT_BIG) code = CHAR_COLON_ALT;
            lcd.setCursor(LCD_PLOT_COLUMN + 2, 3);
            lcd.write(code);
        }

        prevState.dotOn = Clock::isDotOn();
    }

    prevState.mode0scr = static_cast<uint8_t>(mode0scr);
    prevState.isBigDigits = isBig;
    prevState.initialized = true;
}

void redrawPlot(uint8_t mode) {
    // подготовим символы для графика
    loadPlot();
    lcd.clear();
    switch (mode) {
        case 1:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, CO2_MIN, CO2_MAX, (int*)Plot::co2Hour(), "c ", "hr", mode);
            break;
        case 2:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, CO2_MIN, CO2_MAX, (int*)Plot::co2Day(), "c ", "da", mode);
            break;
        case 3:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, HUM_MIN, HUM_MAX, (int*)Plot::humHour(), "h%", "hr", mode);
            break;
        case 4:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, HUM_MIN, HUM_MAX, (int*)Plot::humDay(), "h%", "da", mode);
            break;
        case 5:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, TEMP_MIN, TEMP_MAX, (int*)Plot::tempHour(), "t\337", "hr", mode);
            break;
        case 6:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, TEMP_MIN, TEMP_MAX, (int*)Plot::tempDay(), "t\337", "da", mode);
            break;
        case 7:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, PRESS_MIN, PRESS_MAX, (int*)Plot::pressHour(), "p ", "hr", mode);
            break;
        case 8:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, PRESS_MIN, PRESS_MAX, (int*)Plot::pressDay(), "p ", "da", mode);
            break;
        case 9:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, ALT_MIN, ALT_MAX, (int*)Plot::altHour(), "m ", "hr", mode);
            break;
        case 10:
            drawPlot(0, 3, PLOT_WIDTH, PLOT_HEIGHT, ALT_MIN, ALT_MAX, (int*)Plot::altDay(), "m ", "da", mode);
            break;
    }
}

// предыдущее значение podMode для отрисовки меню
static uint8_t menuPrevPodMode = 255;
static uint8_t menuPrevMode = 255;  // для отслеживания входа в меню

void drawMenu(uint8_t mode, uint8_t podMode, int visOnData) {
    // создаём русские символы при входе в меню или смене podMode
    if (menuPrevMode != mode || menuPrevPodMode != podMode) {
        lcd.clear();
        menuPrevPodMode = podMode;
        menuPrevMode = mode;
    }

    // главное меню (mode == Mode::Menu)
    if (mode == 255) {
        lcd.setCursor(0, 0);
        // НАСТРОЙКИ
        lcd.print("HACTPO");
        lcd.write(RUS.J);
        lcd.print("K");
        lcd.write(RUS.I);

        if (podMode >= 6 && podMode <= 17) {
            lcd.print(" ");
        } else {
            lcd.print(":");
        }

        lcd.setCursor(0, 1);
        switch (podMode) {
            case 1:
                // СОХРАНИТЬ
                lcd.print("COXPAH");
                lcd.write(RUS.I);
                lcd.print("T");
                lcd.write(RUS.sft);
                break;
            case 2:
                // ВЫХОД
                lcd.print("B");
                lcd.write(RUS.Y);
                lcd.print("XO");
                lcd.write(RUS.D);
                break;
            case 3:
                // ЯРК.ИНДИКАТОРА
                drawBridLED();
                break;
            case 4:
                // ЯРК.ЭКРАНА
                drawBridLCD();
                break;
            case 5:
                // РЕЖ.ИНДИКАТОРА
                drawIndicatorMode();
                break;
        }

        if (podMode >= 6 && podMode <= 17) {
            lcd.setCursor(10, 0);

            // ГРАФИКОВ
            lcd.write(RUS.G);
            lcd.print("PA");
            lcd.write(RUS.F);
            lcd.write(RUS.I);
            lcd.print("KOB:");

            lcd.setCursor(0, 1);
            // CO2
            if ((3 & (1 << (podMode - 6))) != 0) {
                lcd.print("CO2,ppm");
            }

            // Влажность
            if ((12 & (1 << (podMode - 6))) != 0) {
                // ВЛ,%
                lcd.print("B");
                lcd.write(RUS.L);
                lcd.print(",% ");
            }

            // Температура
            if ((48 & (1 << (podMode - 6))) != 0) {
                lcd.print("t,");
                lcd.write(239);
                lcd.print("C");
            }

            // Давление
            if ((192 & (1 << (podMode - 6))) != 0) {
#if CO2_SENSOR == 1
                lcd.print("p,rain ");
#else
                lcd.print("p,mmPT ");
#endif
            }

            // Высота
            if ((768 & (1 << (podMode - 6))) != 0) {
                // ВЫС,м
                lcd.print("B");
                lcd.write(RUS.Y);
                lcd.print("C,");
                lcd.write(RUS.m);
                lcd.print("  ");
            }

            // Часы/дни
            if ((1365 & (1 << (podMode - 6))) != 0) {
                lcd.setCursor(9, 1);
                // ЧАС:
                lcd.write(RUS.CH);
                lcd.print("AC:");
            } else {
                // ДЕНЬ:
                lcd.setCursor(9, 1);
                lcd.write(RUS.D);
                lcd.print("EH");
                lcd.write(RUS.sft);
                lcd.print(":");
            }

            // Вкл/выкл
            if ((visOnData & (1 << (podMode - 6))) != 0) {
                // ВКЛ
                lcd.print("BK");
                lcd.write(RUS.L);
                lcd.print("  ");
            } else {
                // ОТКЛ
                lcd.print("OTK");
                lcd.write(RUS.L);
            }
        }
    }

    // режим индикатора (mode == Mode::LED_Mode)
    if (mode == 252) {
        lcd.setCursor(0, 0);
        drawIndicatorMode();
        lcd.print(":");
        lcd.setCursor(0, 1);
        switch (podMode) {
            case 0:
                lcd.print("CO2   ");
                break;
            case 1:
                // ВЛАЖНОСТЬ
                lcd.print("B");
                lcd.write(RUS.L);
                lcd.print("A");
                lcd.write(RUS.ZH);
                lcd.print("HOCT");
                lcd.write(RUS.sft);
                break;
            case 2:
                // ТЕМПЕРАТУРА
                lcd.print("TEM");
                lcd.write(RUS.P);
                lcd.print("EPAT");
                lcd.write(RUS.U);
                lcd.print("PA");
                break;
            case 3:
                // ОСАДКИ
                lcd.print("OCA");
                lcd.write(RUS.D);
                lcd.print("K");
                lcd.write(RUS.I);
                break;
            case 4:
                // ДАВЛЕНИЕ
                lcd.write(RUS.D);
                lcd.print("AB");
                lcd.write(RUS.L);
                lcd.print("EH");
                lcd.write(RUS.I);
                lcd.print("E");
                break;
        }
    }

    // яркость экрана (mode == Mode::LED_Bright)
    if (mode == 253) {
        lcd.setCursor(0, 0);
        drawBridLCD();
        lcd.print(":");
        // значение яркости нужно получить из LED модуля
        uint8_t lcdBright = 11;  // заглушка, нужно передавать параметром
        lcd.setCursor(0, 1);
        if (lcdBright == 11) {
            lcd.print("ABTO");
        } else {
            lcd.print(String(lcdBright * 10) + "%");
        }
    }

    // яркость индикатора (mode == Mode::LED_Manual)
    if (mode == 254) {
        lcd.setCursor(0, 0);
        drawBridLED();
        lcd.print(":");
        uint8_t ledBright = LED::getBrightness();
        lcd.setCursor(0, 1);
        if (ledBright == 11) {
            lcd.print("ABTO");
        } else {
            lcd.print(String(ledBright * 10) + "%");
        }
    }
}

// вспомогательные функции, перенесённые из оригинального скетча
// котируются static, потому что используются только внутри Display.cpp

static void digSeg(int x, int y, int z1, int z2, int z3, int z4, int z5, int z6) {
    lcd.setCursor(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
    lcd.write(static_cast<uint8_t>(z1));
    lcd.write(static_cast<uint8_t>(z2));
    lcd.write(static_cast<uint8_t>(z3));
    if (x <= 11) lcd.print(" ");
    lcd.setCursor(static_cast<uint8_t>(x), static_cast<uint8_t>(y + 1));
    lcd.write(static_cast<uint8_t>(z4));
    lcd.write(static_cast<uint8_t>(z5));
    lcd.write(static_cast<uint8_t>(z6));
    if (x <= 11) lcd.print(" ");
}

static void drawDig(int dig, int x, int y) {
    bool isBig = UI::isBigDigits();
    if (isBig) {
        switch (dig) {
            case 0:
                digSeg(x, y, 255, 0, 255, 255, 32, 255);
                digSeg(x, y + 2, 255, 32, 255, 255, 3, 255);
                break;
            case 1:
                digSeg(x, y, 32, 255, 32, 32, 255, 32);
                digSeg(x, y + 2, 32, 255, 32, 32, 255, 32);
                break;
            case 2:
                digSeg(x, y, 0, 0, 255, 1, 1, 255);
                digSeg(x, y + 2, 255, 2, 2, 255, 3, 3);
                break;
            case 3:
                digSeg(x, y, 0, 0, 255, 1, 1, 255);
                digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
                break;
            case 4:
                digSeg(x, y, 255, 32, 255, 255, 1, 255);
                digSeg(x, y + 2, 2, 2, 255, 32, 32, 255);
                break;
            case 5:
                digSeg(x, y, 255, 0, 0, 255, 1, 1);
                digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
                break;
            case 6:
                digSeg(x, y, 255, 0, 0, 255, 1, 1);
                digSeg(x, y + 2, 255, 2, 255, 255, 3, 255);
                break;
            case 7:
                digSeg(x, y, 0, 0, 255, 32, 32, 255);
                digSeg(x, y + 2, 32, 255, 32, 32, 255, 32);
                break;
            case 8:
                digSeg(x, y, 255, 0, 255, 255, 1, 255);
                digSeg(x, y + 2, 255, 2, 255, 255, 3, 255);
                break;
            case 9:
                digSeg(x, y, 255, 0, 255, 255, 1, 255);
                digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
                break;
            case 10:  // blank
                digSeg(x, y, 32, 32, 32, 32, 32, 32);
                digSeg(x, y + 2, 32, 32, 32, 32, 32, 32);
                break;
        }
    } else {
        // двухстрочные цифры (как в оригинальном скетче)
        switch (dig) {
            case 0:
                digSeg(x, y, 255, 1, 255, 255, 2, 255);
                break;
            case 1:
                digSeg(x, y, 32, 255, 32, 32, 255, 32);
                break;
            case 2:
                digSeg(x, y, 3, 3, 255, 255, 4, 4);
                break;
            case 3:
                digSeg(x, y, 3, 3, 255, 4, 4, 255);
                break;
            case 4:
                digSeg(x, y, 255, 0, 255, 5, 5, 255);
                break;
            case 5:
                digSeg(x, y, 255, 3, 3, 4, 4, 255);
                break;
            case 6:
                digSeg(x, y, 255, 3, 3, 255, 4, 255);
                break;
            case 7:
                digSeg(x, y, 1, 1, 255, 32, 255, 32);
                break;
            case 8:
                digSeg(x, y, 255, 3, 255, 255, 4, 255);
                break;
            case 9:
                digSeg(x, y, 255, 3, 255, 4, 4, 255);
                break;
            case 10:
                digSeg(x, y, 32, 32, 32, 32, 32, 32);
                break;
        }
    }
}

static void drawPlot(int pos, int row, int width, int height, int min_val, int max_val, int* plot_array, const char* label1, const char* label2, int stretch) {
    int max_value = -32000;
    int min_value = 32000;
    for (byte i = 0; i < PLOT_SAMPLES; i++) {
        max_value = max(plot_array[i], max_value);
        min_value = min(plot_array[i], min_value);
    }

    // меняем пределы графиков на предельные/фактические значения, одновременно рисуем указатель пределов
    lcd.setCursor(LCD_PLOT_COLUMN, 0);
    if ((UI::getMaxOnData() & (1 << (stretch - 1))) > 0) {
        max_val = max_value;
        min_val = min_value;
        lcd.write(0b01011110);
        lcd.setCursor(LCD_PLOT_COLUMN, 3);
        lcd.write(0);
    } else {
        lcd.write(0);
        lcd.setCursor(LCD_PLOT_COLUMN, 3);
        lcd.write(0b01011110);
    }

    if (min_val >= max_val) max_val = min_val + 1;
    lcd.setCursor(LCD_PLOT_COLUMN, 1);
    lcd.write(209);
    lcd.setCursor(LCD_PLOT_COLUMN, 2);
    lcd.write(209);

    lcd.setCursor(LCD_VALUE_COLUMN, 0);
    lcd.print(max_value);
    lcd.setCursor(LCD_VALUE_COLUMN, 1);
    lcd.print(label1);
    lcd.print(label2);
    lcd.setCursor(LCD_VALUE_COLUMN, 2);
    lcd.print(plot_array[PLOT_SAMPLES - 1]);
    lcd.setCursor(LCD_VALUE_COLUMN, 3);
    lcd.print(min_value);

    for (byte i = 0; i < static_cast<byte>(width); i++) {
        int fill_val = plot_array[i];
        fill_val = constrain(fill_val, min_val, max_val);
        byte infill, fract;
        if ((plot_array[i]) > min_val)
            infill = static_cast<byte>(floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10));
        else
            infill = 0;
        fract = static_cast<byte>((float)(infill % 10) * 8 / 10);
        infill = static_cast<byte>(infill / 10);

        for (byte n = 0; n < height; n++) {
            if (n < infill && infill > 0) {
                lcd.setCursor(static_cast<uint8_t>(i), static_cast<uint8_t>(row - n));
                lcd.write(static_cast<uint8_t>(255));
            }
            if (n >= infill) {
                lcd.setCursor(static_cast<uint8_t>(i), static_cast<uint8_t>(row - n));
                if (n == 0 && fract == 0) fract++;
                if (fract > 0)
                    lcd.write(static_cast<uint8_t>(fract));
                else
                    lcd.write(static_cast<uint8_t>(16));
                for (int k = n + 1; k < height; k++) {
                    lcd.setCursor(static_cast<uint8_t>(i), static_cast<uint8_t>(row - k));
                    lcd.write(static_cast<uint8_t>(16));
                }
                break;
            }
        }
    }
}

static void drawClock(int hours, int minutes, int x, int y) {
    if (hours > 23 || minutes > 59) return;
    if (hours / 10 == 0) drawDig(10, x, y);
    else drawDig(hours / 10, x, y);
    drawDig(hours % 10, x + 4, y);
    drawDig(minutes / 10, x + 8, y);
    drawDig(minutes % 10, x + 12, y);
}

static void drawTemp(float dispTemp, int x, int y) {
    int tens = static_cast<int>(dispTemp / 10.0f);
    if (tens == 0) drawDig(10, x, y);
    else drawDig(tens, x, y);
    drawDig(static_cast<int>(dispTemp) % 10, x + 4, y);
    drawDig(static_cast<int>(dispTemp * 10.0f) % 10, x + 9, y);

    if (UI::isBigDigits()) {
        lcd.setCursor(static_cast<uint8_t>(x + 7), static_cast<uint8_t>(y + 3));
        lcd.write(1);  // десятичная точка
    } else {
        lcd.setCursor(static_cast<uint8_t>(x + 7), static_cast<uint8_t>(y + 1));
        lcd.write(46);
    }
    lcd.setCursor(static_cast<uint8_t>(x + 13), static_cast<uint8_t>(y));
    lcd.write(239);
}

static void drawHum(int dispHum, int x, int y) {
    if (dispHum / 100 == 0) drawDig(10, x, y);
    else drawDig(dispHum / 100, x, y);
    if ((dispHum % 100) / 10 == 0) drawDig(0, x + 4, y);
    else drawDig(dispHum / 10, x + 4, y);
    drawDig(static_cast<int>(dispHum) % 10, x + 8, y);
    if (UI::isBigDigits()) {
        lcd.setCursor(static_cast<uint8_t>(x + 12), static_cast<uint8_t>(y + 1));
        lcd.write(239);
        lcd.print("\4");
        lcd.setCursor(static_cast<uint8_t>(x + 12), static_cast<uint8_t>(y + 2));
        lcd.print("\5");
        lcd.write(239);
    } else {
        lcd.setCursor(static_cast<uint8_t>(x + 12), static_cast<uint8_t>(y + 1));
        lcd.print("%");
    }
}

static void drawPPM(int dispCO2, int x, int y) {
    if (dispCO2 / 1000 == 0) drawDig(10, x, y);
    else drawDig(dispCO2 / 1000, x, y);
    drawDig((dispCO2 % 1000) / 100, x + 4, y);
    drawDig((dispCO2 % 100) / 10, x + 8, y);
    drawDig(dispCO2 % 10, x + 12, y);
    lcd.setCursor(static_cast<uint8_t>(LCD_PLOT_COLUMN), 0);
    lcd.print("ppm");
}

static void drawPres(int dispPres, int x, int y) {
    drawDig((dispPres % 1000) / 100, x, y);
    drawDig((dispPres % 100) / 10, x + 4, y);
    drawDig(dispPres % 10, x + 8, y);
    lcd.setCursor(static_cast<uint8_t>(x + 11), static_cast<uint8_t>(1 + (UI::isBigDigits()) * 2));
    lcd.print("mm");
}

static void drawAlt(float dispAlt, int x, int y) {
    if (dispAlt >= 1000) {
        drawDig((static_cast<int>(dispAlt) % 10000) / 1000, x, y);
        x += 4;
    }
    drawDig((static_cast<int>(dispAlt) % 1000) / 100, x, y);
    drawDig((static_cast<int>(dispAlt) % 100) / 10, x + 4, y);
    drawDig(static_cast<int>(dispAlt) % 10, x + 8, y);
    if (dispAlt < 1000) {
        lcd.setCursor(static_cast<uint8_t>(x + 12), static_cast<uint8_t>(y + 1 + (UI::isBigDigits()) * 2));
        lcd.print(static_cast<int>(dispAlt * 10.0f) % 10);
        if (UI::isBigDigits()) lcd.setCursor(static_cast<uint8_t>(x + 11), static_cast<uint8_t>(y + 3));
        else lcd.setCursor(static_cast<uint8_t>(x + 11), static_cast<uint8_t>(y + 1));
        lcd.print(".");
        x -= 1;
    } else {
        x -= 4;
    }
    if (UI::isBigDigits()) lcd.setCursor(static_cast<uint8_t>(x + 14), 3);
    else lcd.setCursor(static_cast<uint8_t>(x + 14), 1);
    lcd.print("m");
}

static void loadClock() {
    bool isBig = UI::isBigDigits();
    if (isBig) {
        lcd.createChar(0, UT);
        lcd.createChar(1, row3);
        lcd.createChar(2, UB);
        lcd.createChar(3, row5);
        lcd.createChar(4, KU);
        lcd.createChar(5, KD);
    } else {
        lcd.createChar(0, row2);
        lcd.createChar(1, UB);
        lcd.createChar(2, row3);
        lcd.createChar(3, UMB);
        lcd.createChar(4, LMB);
        lcd.createChar(5, LM2);
    }
    // буква дня недели (П/Ч/Б)
    // этот фрагмент требует доступа к часам, но мы можем не создавать его,
    // либо делать в UI, оставлю как заглушку
}

static void loadPlot() {
    lcd.createChar(0, rowS);
    lcd.createChar(1, row1);
    lcd.createChar(2, row2);
    lcd.createChar(3, row3);
    lcd.createChar(4, row4);
    lcd.createChar(5, row5);
    lcd.createChar(6, row6);
    lcd.createChar(7, row7);
}

// РЕЖ.ИНДИКАТОРА
static void drawIndicatorMode() {
    lcd.print("PE");
    lcd.write(RUS.ZH);
    lcd.print(".");
    lcd.write(RUS.I);
    lcd.print("H");
    lcd.write(RUS.D);
    lcd.write(RUS.I);
    lcd.print("KATOPA");
}

static void drawBridLCD() {
    lcd.write(RUS.YA);
    lcd.print("PK.");
    lcd.write(RUS.E);
    lcd.print("KPAHA");
}

static void drawBridLED() {
    lcd.write(RUS.YA);
    lcd.print("PK.");
    lcd.write(RUS.I);
    lcd.print("H");
    lcd.write(RUS.D);
    lcd.write(RUS.I);
    lcd.print("KATOPA");
}
}  // namespace Display
