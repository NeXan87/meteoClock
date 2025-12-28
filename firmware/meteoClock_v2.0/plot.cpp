#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "plot.h"
#include "display.h"

// === Русские символы ===
static uint8_t PP[8] = { 0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000 };  // П
static uint8_t BB[8] = { 0b11111, 0b10000, 0b10000, 0b11111, 0b10001, 0b10001, 0b11111, 0b00000 };  // Б
static uint8_t CH[8] = { 0b10001, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b00001, 0b00000 };  // Ч
static uint8_t II[8] = { 0b10001, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000 };  // И
static uint8_t BM[8] = { 0b10000, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000 };  // Ь
static uint8_t IY[8] = { 0b01100, 0b00001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000 };  // Й
static uint8_t DD[8] = { 0b01110, 0b01010, 0b01010, 0b01010, 0b01010, 0b01010, 0b11111, 0b10001 };  // Д
static uint8_t AA[8] = { 0b11100, 0b00010, 0b00001, 0b00111, 0b00001, 0b00010, 0b11100, 0b00000 };  // Э
static uint8_t IA[8] = { 0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001, 0b00000 };  // Я
static uint8_t YY[8] = { 0b10001, 0b10001, 0b10001, 0b11101, 0b10011, 0b10011, 0b11101, 0b00000 };  // Ы
static uint8_t GG[8] = { 0b11110, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b00000 };  // Г
static uint8_t FF[8] = { 0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b01110, 0b00100, 0b00000 };  // Ф
static uint8_t LL[8] = { 0b01111, 0b01001, 0b01001, 0b01001, 0b01001, 0b01001, 0b10001, 0b00000 };  // Л
static uint8_t ZZ[8] = { 0b10101, 0b10101, 0b10101, 0b01110, 0b10101, 0b10101, 0b10101, 0b00000 };  // Ж

void plotLoad() {
    lcd.createChar(0, rowS);
    lcd.createChar(1, row1);
    lcd.createChar(2, row2);
    lcd.createChar(3, row3);
    lcd.createChar(4, row4);
    lcd.createChar(5, row5);
    lcd.createChar(6, row6);
    lcd.createChar(7, row7);
}

void plotDraw(byte mode) {
#if (DISPLAY_TYPE == 1)
    switch (mode) {
        case 1: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, co2Hour, "c ", "hr", mode); break;
        case 2: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, co2Day, "c ", "da", mode); break;
        case 3: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, humHour, "h%", "hr", mode); break;
        case 4: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, humDay, "h%", "da", mode); break;
        case 5: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, tempHour, "t\337", "hr", mode); break;
        case 6: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, tempDay, "t\337", "da", mode); break;
        case 7: 
            if (PRESSURE)
                drawPlot(0, 3, 15, 4, minRain, maxRain, pressHour, "r ", "hr", mode);
            else
                drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, pressHour, "p ", "hr", mode);
            break;
        case 8:
            if (PRESSURE)
                drawPlot(0, 3, 15, 4, minRain, maxRain, pressDay, "r ", "da", mode);
            else
                drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, pressDay, "p ", "da", mode);
            break;
        case 9: drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, altHour, "m ", "hr", mode); break;
        case 10: drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, altDay, "m ", "da", mode); break;
    }
#else
    switch (mode) {
        case 1: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, co2Hour, "c", "h", mode); break;
        case 2: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, co2Day, "c", "d", mode); break;
        case 3: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, humHour, "h", "h", mode); break;
        case 4: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, humDay, "h", "d", mode); break;
        case 5: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, tempHour, "t", "h", mode); break;
        case 6: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, tempDay, "t", "d", mode); break;
        case 7:
            if (PRESSURE)
                drawPlot(0, 1, 12, 2, minRain, maxRain, pressHour, "r", "h", mode);
            else
                drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, pressHour, "p", "h", mode);
            break;
        case 8:
            if (PRESSURE)
                drawPlot(0, 1, 12, 2, minRain, maxRain, pressDay, "r", "d", mode);
            else
                drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, pressDay, "p", "d", mode);
            break;
        case 9: drawPlot(0, 1, 12, 2, ALT_MIN, ALT_MAX, altHour, "m", "h", mode); break;
        case 10: drawPlot(0, 1, 12, 2, ALT_MIN, ALT_MAX, altDay, "m", "d", mode); break;
    }
#endif
}

void plotDrawMenu(byte podMode) {
    lcd.createChar(8, FF);  // ф
    lcd.createChar(7, GG);  // Г
    lcd.createChar(5, LL);  // Л
    lcd.setCursor(10, 0);
#if (WEEK_LANG == 1)
    lcd.print("\7PA\10\4KOB");  // графиков
#else
    lcd.print("Charts  ");
#endif
    lcd.setCursor(0, 1);

    uint16_t mask = (1 << (podMode - 6));
    if (mask & 3) lcd.print("CO2 ");
    if (mask & 12) {
#if (WEEK_LANG == 1)
        lcd.print("B\5,% ");
#else
        lcd.print("Hum,%");
#endif
    }
    if (mask & 48) lcd.print("t\337 ");
    if (mask & 192) {
        if (PRESSURE) lcd.print("p,rain ");
        else lcd.print("p,mmPT ");
    }
    if (mask & 768) {
#if (WEEK_LANG == 1)
        lcd.print("B\6C,m  ");
#else
        lcd.print("hgt,m  ");
#endif
    }

    if (mask & 1365) {
        lcd.createChar(3, CH);  // Ч
        lcd.setCursor(8, 1);
#if (WEEK_LANG == 1)
        lcd.print("\3AC:");
#else
        lcd.print("Hour:");
#endif
    } else {
        lcd.setCursor(7, 1);
#if (WEEK_LANG == 1)
        lcd.print("\3EH\1:");
#else
        lcd.print("Day: ");
#endif
    }

    if (VIS_ONDATA & mask) {
#if (WEEK_LANG == 1)
        lcd.print("BK\5 ");
#else
        lcd.print("On  ");
#endif
    } else {
#if (WEEK_LANG == 1)
        lcd.print("B\6K\5");
#else
        lcd.print("Off ");
#endif
    }
}

// === drawPlot — копия из functions.txt с заменой String на char[] ===
void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int *plot_array, const char* label1, const char* label2, int stretch) {
    int max_value = -32000;
    int min_value = 32000;
    for (byte i = 0; i < 15; i++) {
        if (plot_array[i] > max_value) max_value = plot_array[i];
        if (plot_array[i] < min_value) min_value = plot_array[i];
    }

    lcd.setCursor(15, 0);
    if (MAX_ONDATA & (1 << (stretch - 1))) {
        max_val = max_value;
        min_val = min_value;
#if (DISPLAY_TYPE == 1)
        lcd.write(0b01011110);
        lcd.setCursor(15, 3);
        lcd.write(0);
#endif
    } else {
#if (DISPLAY_TYPE == 1)
        lcd.write(0);
        lcd.setCursor(15, 3);
        lcd.write(0b01011110);
#endif
    }

    if (min_val >= max_val) max_val = min_val + 1;

#if (DISPLAY_TYPE == 1)
    lcd.setCursor(15, 1);
    lcd.write(209);
    lcd.setCursor(15, 2);
    lcd.write(209);
    lcd.setCursor(16, 0);
    lcd.print(max_value);
    lcd.setCursor(16, 1);
    lcd.print(label1);
    lcd.print(label2);
    lcd.setCursor(16, 2);
    lcd.print(plot_array[14]);
    lcd.setCursor(16, 3);
    lcd.print(min_value);
#else
    lcd.setCursor(12, 0);
    lcd.print(label1);
    lcd.setCursor(13, 0);
    lcd.print(max_value);
    lcd.setCursor(12, 1);
    lcd.print(label2);
    lcd.setCursor(13, 1);
    lcd.print(min_value);
#endif

    for (byte i = 0; i < width; i++) {
        int val = plot_array[i];
        val = (val < min_val) ? min_val : (val > max_val ? max_val : val);
        int range = max_val - min_val;
        if (range == 0) range = 1;
        long scaled = (long)(val - min_val) * height * 10 / range;
        byte infill = scaled / 10;
        byte fract = (scaled % 10) * 8 / 10;

        for (byte n = 0; n < height; n++) {
            lcd.setCursor(i, row - n);
            if (n < infill) {
                lcd.write(255);
            } else if (n == infill) {
                if (fract == 0 && infill == 0) fract = 1;
                lcd.write(fract ? fract : 16);
            } else {
                lcd.write(16);
            }
        }
    }
}