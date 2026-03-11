#include "display/display.h"

#include <Arduino.h>
#include <Wire.h>

#include "clock/clock.h"  // для времени
#include "config.h"
#include "plot/plot.h"        // для доступа к массивам графиков
#include "sensors/sensors.h"  // для получения данных
#include "ui/ui.h"

namespace Display {

static LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);  // только LCD2004

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

// массивы, используемые для построения больших цифр (переписаны из оригинального скетча)
static uint8_t UB[8] = {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};   // для двустрочных 7,0 и четырехстрочных 2,3,4,5,6,8,9
static uint8_t UMB[8] = {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};  // для двустрочных 2,3,5,6,8,9
static uint8_t LMB[8] = {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};  // для двустрочных 2,3,5,6,8,9
static uint8_t LM2[8] = {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};  // для двустрочной 4
static uint8_t UT[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000};   // для четырехстрочных 2,3,4,5,6,7,8,9,0

static uint8_t KU[8] = {0b00000, 0b00000, 0b00000, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000};  // для верхней части %
static uint8_t KD[8] = {0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000, 0b00000, 0b00000};  // для нижней части %

// русские буквы, встречающиеся в меню
[[maybe_unused]] static uint8_t PP[8] = {0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000};  // П
[[maybe_unused]] static uint8_t BB[8] = {0b11111, 0b10000, 0b10000, 0b11111, 0b10001, 0b10001, 0b11111, 0b00000};  // Б
[[maybe_unused]] static uint8_t CH[8] = {0b10001, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b00001, 0b00000};  // Ч
[[maybe_unused]] static uint8_t II[8] = {0b10001, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000};  // И
[[maybe_unused]] static uint8_t BM[8] = {0b10000, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000};  // Ь
[[maybe_unused]] static uint8_t IY[8] = {0b01100, 0b00001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000};  // Й
[[maybe_unused]] static uint8_t DD[8] = {0b01110, 0b01010, 0b01010, 0b01010, 0b01010, 0b01010, 0b11111, 0b10001};  // Д
[[maybe_unused]] static uint8_t AA[8] = {0b11100, 0b00010, 0b00001, 0b00111, 0b00001, 0b00010, 0b11100, 0b00000};  // Э
[[maybe_unused]] static uint8_t IA[8] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001, 0b00000};  // Я
[[maybe_unused]] static uint8_t YY[8] = {0b10001, 0b10001, 0b10001, 0b11101, 0b10011, 0b10011, 0b11101, 0b00000};  // Ы
[[maybe_unused]] static uint8_t GG[8] = {0b11110, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b00000};  // Г
// конец дополнительной графики

void init() {
    lcd.init();
    lcd.backlight();
    createCustomChars();
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

    uint8_t mode0scr = UI::getMode0Scr();
    bool isBig = UI::isBigDigits();

    // вывод для LCD2004
    if (mode0scr != 2) {  // температура
        lcd.setCursor(0, 2);
        if (isBig) {
            if (mode0scr == 1) lcd.setCursor(15, 2);
            if (mode0scr != 1) lcd.setCursor(15, 0);
        }
        lcd.print(String(dispTemp, 1));
        lcd.write(223);
    } else {
        drawTemp(dispTemp, 0, 0);
    }

    if (mode0scr != 4) {  // влажность
        lcd.setCursor(5, 2);
        if (isBig) lcd.setCursor(15, 1);
        lcd.print(" " + String(dispHum) + "% ");
    } else {
        drawHum(dispHum, 0, 0);
    }

#if (CO2_SENSOR == 1)
    if (mode0scr != 1) {  // СО2
        if (isBig) {
            lcd.setCursor(15, 2);
            lcd.print(String(dispCO2) + "p");
        } else {
            lcd.setCursor(11, 2);
            lcd.print(String(dispCO2) + "ppm ");
        }
    } else {
        drawPPM(dispCO2, 0, 0);
    }
#endif

    if (mode0scr != 3) {  // давление
        lcd.setCursor(0, 3);
        if (isBig && mode0scr == 0) lcd.setCursor(15, 3);
        if (isBig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
        if (isBig && mode0scr == 4) lcd.setCursor(15, 1);
        if (!(isBig && mode0scr == 1)) lcd.print(String(dispPres) + "mm");
    } else {
        drawPres(dispPres, 0, 0);
    }

    if (mode0scr != 5) {
        // ничего
    } else {
        drawAlt(dispAlt, 0, 0);
    }

    if (!isBig) {
        lcd.setCursor(5, 3);
        lcd.print(" rain     ");
        lcd.setCursor(11, 3);
        if (dispRain < 0) lcd.setCursor(10, 3);
        lcd.print(String(dispRain) + "%");
    }

    if (mode0scr != 0) {
        lcd.setCursor(15, 3);
        if (hrs / 10 == 0) lcd.print(" ");
        lcd.print(hrs);
        lcd.print(":");
        if (mins / 10 == 0) lcd.print("0");
        lcd.print(mins);
    } else {
        drawClock(hrs, mins, 0, 0);
        // мигающие точки между цифрами
        byte code = Clock::isDotOn() ? 165 : 32;
        if (mode0scr == 0) {
            if (isBig) lcd.setCursor(7, 2);
            else lcd.setCursor(7, 0);
            lcd.write(code);
            lcd.setCursor(7, 1);
            lcd.write(code);
        } else {
            if (code == 165) code = 58;
            lcd.setCursor(17, 3);
            lcd.write(code);
        }
    }
}

void redrawPlot(uint8_t mode) {
    // подготовим символы для графика
    loadPlot();
    lcd.clear();
    switch (mode) {
        case 1:
            drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)Plot::co2Hour(), "c ", "hr", mode);
            break;
        case 2:
            drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)Plot::co2Day(), "c ", "da", mode);
            break;
        case 3:
            drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)Plot::humHour(), "h%", "hr", mode);
            break;
        case 4:
            drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)Plot::humDay(), "h%", "da", mode);
            break;
        case 5:
            drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)Plot::tempHour(), "t\337", "hr", mode);
            break;
        case 6:
            drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)Plot::tempDay(), "t\337", "da", mode);
            break;
        case 7:
            drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)Plot::pressHour(), "p ", "hr", mode);
            break;
        case 8:
            drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)Plot::pressDay(), "p ", "da", mode);
            break;
        case 9:
            drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, (int*)Plot::altHour(), "m ", "hr", mode);
            break;
        case 10:
            drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, (int*)Plot::altDay(), "m ", "da", mode);
            break;
    }
}

// вспомогательные функции, перенесённые из оригинального скетча
// котируются static, потому что используются только внутри Display.cpp

static void digSeg(int x, int y, int z1, int z2, int z3, int z4, int z5, int z6) {
    lcd.setCursor((uint8_t)x, (uint8_t)y);
    lcd.write(z1);
    lcd.write(z2);
    lcd.write(z3);
    if (x <= 11) lcd.print(" ");
    lcd.setCursor((uint8_t)x, (uint8_t)(y + 1));
    lcd.write(z4);
    lcd.write(z5);
    lcd.write(z6);
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
        // простые цифры: обычный вывод
        lcd.setCursor(x, y);
        if (dig == 10) lcd.print(' ');
        else lcd.print(dig);
    }
}

static void drawPlot(int pos, int row, int width, int height, int min_val, int max_val, int* plot_array, const char* label1, const char* label2, int stretch) {
    int max_value = -32000;
    int min_value = 32000;
    for (byte i = 0; i < 15; i++) {
        max_value = max(plot_array[i], max_value);
        min_value = min(plot_array[i], min_value);
    }

    // меняем пределы графиков на предельные/фактические значения, одновременно рисуем указатель пределов
    lcd.setCursor(15, 0);
    if ((UI::getMaxOnData() & (1 << (stretch - 1))) > 0) {
        max_val = max_value;
        min_val = min_value;
        lcd.write(0b01011110);
        lcd.setCursor(15, 3);
        lcd.write(0);
    } else {
        lcd.write(0);
        lcd.setCursor(15, 3);
        lcd.write(0b01011110);
    }

    if (min_val >= max_val) max_val = min_val + 1;
    lcd.setCursor(15, 1);
    lcd.write(0b01111100);
    lcd.setCursor(15, 2);
    lcd.write(0b01111100);

    lcd.setCursor(16, 0);
    lcd.print(max_value);
    lcd.setCursor(16, 1);
    lcd.print(label1);
    lcd.print(label2);
    lcd.setCursor(16, 2);
    lcd.print(plot_array[14]);
    lcd.setCursor(16, 3);
    lcd.print(min_value);

    for (byte i = 0; i < width; i++) {
        int fill_val = plot_array[i];
        fill_val = constrain(fill_val, min_val, max_val);
        byte infill, fract;
        if ((plot_array[i]) > min_val)
            infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10);
        else infill = 0;
        fract = (float)(infill % 10) * 8 / 10;
        infill = infill / 10;

        for (byte n = 0; n < height; n++) {
            if (n < infill && infill > 0) {
                lcd.setCursor(i, (row - n));
                lcd.write(255);
            }
            if (n >= infill) {
                lcd.setCursor(i, (row - n));
                if (n == 0 && fract == 0) fract++;
                if (fract > 0) lcd.write(fract);
                else lcd.write(16);
                for (byte k = n + 1; k < height; k++) {
                    lcd.setCursor(i, (row - k));
                    lcd.write(16);
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
    if (dispTemp / 10 == 0) drawDig(10, x, y);
    else drawDig(dispTemp / 10, x, y);
    drawDig(int(dispTemp) % 10, x + 4, y);
    drawDig(int(dispTemp * 10.0) % 10, x + 9, y);

    if (UI::isBigDigits()) {
        lcd.setCursor(x + 7, y + 3);
        lcd.write(1);  // десятичная точка
    } else {
        lcd.setCursor(x + 7, y + 1);
        lcd.write(0B10100001);
    }
    lcd.setCursor(x + 13, y);
    lcd.write(223);
}

static void drawHum(int dispHum, int x, int y) {
    if (dispHum / 100 == 0) drawDig(10, x, y);
    else drawDig(dispHum / 100, x, y);
    if ((dispHum % 100) / 10 == 0) drawDig(0, x + 4, y);
    else drawDig(dispHum / 10, x + 4, y);
    drawDig(int(dispHum) % 10, x + 8, y);
    if (UI::isBigDigits()) {
        lcd.setCursor(x + 12, y + 1);
        lcd.print("\245\4");
        lcd.setCursor(x + 12, y + 2);
        lcd.print("\5\245");
    } else {
        lcd.setCursor(x + 12, y + 1);
        lcd.print("%");
    }
}

static void drawPPM(int dispCO2, int x, int y) {
    if (dispCO2 / 1000 == 0) drawDig(10, x, y);
    else drawDig(dispCO2 / 1000, x, y);
    drawDig((dispCO2 % 1000) / 100, x + 4, y);
    drawDig((dispCO2 % 100) / 10, x + 8, y);
    drawDig(dispCO2 % 10, x + 12, y);
    lcd.setCursor(15, 0);
    lcd.print("ppm");
}

static void drawPres(int dispPres, int x, int y) {
    drawDig((dispPres % 1000) / 100, x, y);
    drawDig((dispPres % 100) / 10, x + 4, y);
    drawDig(dispPres % 10, x + 8, y);
    lcd.setCursor(x + 11, 1 + (UI::isBigDigits()) * 2);
    lcd.print("mm");
}

static void drawAlt(float dispAlt, int x, int y) {
    if (dispAlt >= 1000) {
        drawDig((int(dispAlt) % 10000) / 1000, x, y);
        x += 4;
    }
    drawDig((int(dispAlt) % 1000) / 100, x, y);
    drawDig((int(dispAlt) % 100) / 10, x + 4, y);
    drawDig(int(dispAlt) % 10, x + 8, y);
    if (dispAlt < 1000) {
        lcd.setCursor(x + 12, y + 1 + (UI::isBigDigits()) * 2);
        lcd.print((int(dispAlt * 10.0)) % 10);
        if (UI::isBigDigits()) lcd.setCursor(x + 11, y + 3);
        else lcd.setCursor(x + 11, y + 1);
        lcd.print(".");
        x -= 1;
    } else {
        x -= 4;
    }
    if (UI::isBigDigits()) lcd.setCursor(x + 14, 3);
    else lcd.setCursor(x + 14, 1);
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

}  // namespace Display
