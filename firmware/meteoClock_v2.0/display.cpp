#include "config.h"
#include "globals.h"
#include "hardware.h"
#include "display.h"
#include <LiquidCrystal_I2C.h>

// === Глобальные символы графика ===
static uint8_t rowS[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b00000};
static uint8_t row7[8] = {0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row6[8] = {0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row5[8] = {0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row4[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111};
static uint8_t row3[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};
static uint8_t row2[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
static uint8_t row1[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};

// === Русские символы ===
static uint8_t PP[8] = {0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000};  // П
static uint8_t BB[8] = {0b11111, 0b10000, 0b10000, 0b11111, 0b10001, 0b10001, 0b11111, 0b00000};  // Б
static uint8_t CH[8] = {0b10001, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b00001, 0b00000};  // Ч
static uint8_t II[8] = {0b10001, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000};  // И
static uint8_t BM[8] = {0b10000, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000};  // Ь
static uint8_t IY[8] = {0b01100, 0b00001, 0b10011, 0b10101, 0b11001, 0b10001, 0b10001, 0b00000};  // Й
static uint8_t DD[8] = {0b01110, 0b01010, 0b01010, 0b01010, 0b01010, 0b01010, 0b11111, 0b10001};  // Д
static uint8_t AA[8] = {0b11100, 0b00010, 0b00001, 0b00111, 0b00001, 0b00010, 0b11100, 0b00000};  // Э
static uint8_t IA[8] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001, 0b00000};  // Я
static uint8_t YY[8] = {0b10001, 0b10001, 0b10001, 0b11101, 0b10011, 0b10011, 0b11101, 0b00000};  // Ы
static uint8_t GG[8] = {0b11110, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b00000};  // Г
static uint8_t FF[8] = {0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b01110, 0b00100, 0b00000};  // Ф
static uint8_t LL[8] = {0b01111, 0b01001, 0b01001, 0b01001, 0b01001, 0b01001, 0b10001, 0b00000};  // Л
static uint8_t ZZ[8] = {0b10101, 0b10101, 0b10101, 0b01110, 0b10101, 0b10101, 0b10101, 0b00000};  // Ж

// === Символы для цифр (из оригинала) ===
static uint8_t UT[8] = {0b11111,0b11111,0b11111,0b11111,0b11111,0b00000,0b00000,0b00000};
static uint8_t UB[8] = {0b11111,0b11111,0b11111,0b00000,0b00000,0b00000,0b00000,0b00000};
static uint8_t UMB[8] = {0b11111,0b11111,0b11111,0b00000,0b00000,0b00000,0b11111,0b11111};
static uint8_t LMB[8] = {0b11111,0b00000,0b00000,0b00000,0b00000,0b11111,0b11111,0b11111};
static uint8_t LM2[8] = {0b11111,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000};
static uint8_t KU[8] = {0b00000,0b00000,0b00000,0b00000,0b00001,0b00010,0b00100,0b01000};
static uint8_t KD[8] = {0b00001,0b00010,0b00100,0b01000,0b10000,0b00000,0b00000,0b00000};

// === Вспомогательные функции ===
static void digSeg(byte x, byte y, byte z1, byte z2, byte z3, byte z4, byte z5, byte z6) {
    lcd.setCursor(x, y);
    lcd.write(z1);
    lcd.write(z2);
    lcd.write(z3);
    if (x <= 11) lcd.print(' ');
    lcd.setCursor(x, y + 1);
    lcd.write(z4);
    lcd.write(z5);
    lcd.write(z6);
    if (x <= 11) lcd.print(' ');
}

static void printInt(int val, byte len) {
    char buf[7];
    itoa(val, buf, 10);
    byte l = strlen(buf);
    while (len-- > l) lcd.print('0');
    lcd.print(buf);
}

// === Основные функции отрисовки ===

void drawDig(byte dig, byte x, byte y) {
    if (bigDig && DISPLAY_TYPE == 1) {
        switch (dig) {
            case 0: digSeg(x, y, 255, 0, 255, 255, 32, 255); digSeg(x, y + 2, 255, 32, 255, 255, 3, 255); break;
            case 1: digSeg(x, y, 32, 255, 32, 32, 255, 32); digSeg(x, y + 2, 32, 255, 32, 32, 255, 32); break;
            case 2: digSeg(x, y, 0, 0, 255, 1, 1, 255); digSeg(x, y + 2, 255, 2, 2, 255, 3, 3); break;
            case 3: digSeg(x, y, 0, 0, 255, 1, 1, 255); digSeg(x, y + 2, 2, 2, 255, 3, 3, 255); break;
            case 4: digSeg(x, y, 255, 32, 255, 255, 1, 255); digSeg(x, y + 2, 2, 2, 255, 32, 32, 255); break;
            case 5: digSeg(x, y, 255, 0, 0, 255, 1, 1); digSeg(x, y + 2, 2, 2, 255, 3, 3, 255); break;
            case 6: digSeg(x, y, 255, 0, 0, 255, 1, 1); digSeg(x, y + 2, 255, 2, 255, 255, 3, 255); break;
            case 7: digSeg(x, y, 0, 0, 255, 32, 32, 255); digSeg(x, y + 2, 32, 255, 32, 32, 255, 32); break;
            case 8: digSeg(x, y, 255, 0, 255, 255, 1, 255); digSeg(x, y + 2, 255, 2, 255, 255, 3, 255); break;
            case 9: digSeg(x, y, 255, 0, 255, 255, 1, 255); digSeg(x, y + 2, 2, 2, 255, 3, 3, 255); break;
            default: digSeg(x, y, 32, 32, 32, 32, 32, 32); digSeg(x, y + 2, 32, 32, 32, 32, 32, 32); break; // 10
        }
    } else {
        switch (dig) {
            case 0: digSeg(x, y, 255, 1, 255, 255, 2, 255); break;
            case 1: digSeg(x, y, 32, 255, 32, 32, 255, 32); break;
            case 2: digSeg(x, y, 3, 3, 255, 255, 4, 4); break;
            case 3: digSeg(x, y, 3, 3, 255, 4, 4, 255); break;
            case 4: digSeg(x, y, 255, 0, 255, 5, 5, 255); break;
            case 5: digSeg(x, y, 255, 3, 3, 4, 4, 255); break;
            case 6: digSeg(x, y, 255, 3, 3, 255, 4, 255); break;
            case 7: digSeg(x, y, 1, 1, 255, 32, 255, 32); break;
            case 8: digSeg(x, y, 255, 3, 255, 255, 4, 255); break;
            case 9: digSeg(x, y, 255, 3, 255, 4, 4, 255); break;
            default: digSeg(x, y, 32, 32, 32, 32, 32, 32); break; // 10
        }
    }
}

void drawPPM(int co2, byte x, byte y) {
    if (co2 / 1000 == 0) drawDig(10, x, y);
    else drawDig(co2 / 1000, x, y);
    drawDig((co2 % 1000) / 100, x + 4, y);
    drawDig((co2 % 100) / 10, x + 8, y);
    drawDig(co2 % 10, x + 12, y);
    lcd.setCursor(15, 0);
#if (DISPLAY_TYPE == 1)
    lcd.print("ppm");
#else
    lcd.print('p');
#endif
}

void drawTemp(float temp, byte x, byte y) {
    if (temp / 10 == 0) drawDig(10, x, y);
    else drawDig((int)temp / 10, x, y);
    drawDig((int)temp % 10, x + 4, y);
    drawDig((int)(temp * 10) % 10, x + 9, y);
    if (bigDig && DISPLAY_TYPE == 1) {
        lcd.setCursor(x + 7, y + 3);
        lcd.write(1);
    } else {
        lcd.setCursor(x + 7, y + 1);
        lcd.write('.');
    }
    lcd.setCursor(x + 13, y);
    lcd.write(239); // градус
}

void drawHum(byte hum, byte x, byte y) {
    if (hum / 100 == 0) drawDig(10, x, y);
    else drawDig(hum / 100, x, y);
    if ((hum % 100) / 10 == 0) drawDig(0, x + 4, y);
    else drawDig(hum / 10, x + 4, y);
    drawDig(hum % 10, x + 8, y);
    if (bigDig && DISPLAY_TYPE == 1) {
        lcd.setCursor(x + 12, y + 1);
        lcd.write(239);
        lcd.print("\4");
        lcd.setCursor(x + 12, y + 2);
        lcd.print("\5");
        lcd.write(239);
    } else {
        lcd.setCursor(x + 12, y + 1);
        lcd.print('%');
    }
}

void drawPres(int pres, byte x, byte y) {
    drawDig((pres % 1000) / 100, x, y);
    drawDig((pres % 100) / 10, x + 4, y);
    drawDig(pres % 10, x + 8, y);
    lcd.setCursor(x + 11, 1 + (bigDig && DISPLAY_TYPE) * 2);
    lcd.print("mm");
}

void drawAlt(float alt, byte x, byte y) {
    int ialt = (int)alt;
    byte start_x = x;
    if (ialt >= 1000) {
        drawDig(ialt / 1000, x, y);
        x += 4;
    }
    drawDig((ialt % 1000) / 100, x, y);
    drawDig((ialt % 100) / 10, x + 4, y);
    drawDig(ialt % 10, x + 8, y);
    if (alt < 1000.0f) {
        lcd.setCursor(x + 12, y + 1 + (bigDig && DISPLAY_TYPE) * 2);
        lcd.print((int)(alt * 10) % 10);
        lcd.setCursor(x + 11, y + 1 + (bigDig && DISPLAY_TYPE) * 2);
        lcd.print('.');
        start_x -= 1;
    } else {
        start_x -= 4;
    }
    lcd.setCursor(start_x + 14, 1 + (bigDig && DISPLAY_TYPE) * 2);
    lcd.print('m');
}

void drawClock(byte hours, byte minutes, byte x, byte y) {
    if (hours > 23 || minutes > 59) return;
    if (hours / 10 == 0) drawDig(10, x, y);
    else drawDig(hours / 10, x, y);
    drawDig(hours % 10, x + 4, y);
    drawDig(minutes / 10, x + 8, y);
    drawDig(minutes % 10, x + 12, y);
}

#if (WEEK_LANG == 0)
static const char* dayNames[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
#else
static const char* dayNames[] = {"BC", "\7H", "BT", "CP", "\7T", "\7T", "C\7"};
#endif

void drawData() {
    if (!bigDig && DISPLAY_TYPE == 1) {
        int Y = (mode0scr == 1) ? 2 : 0;
        lcd.setCursor(15, 0 + Y);
        if (now.day() < 10) lcd.print('0');
        lcd.print(now.day());
        lcd.print('.');
        if (now.month() < 10) lcd.print('0');
        lcd.print(now.month());

        if (DISP_MODE == 0) {
            lcd.setCursor(16, 1);
            lcd.print(now.year());
        } else {
            lcd.setCursor(18, 1);
            lcd.print(dayNames[now.dayOfTheWeek()]);
        }
    }
}

void drawSensors() {
#if (DISPLAY_TYPE == 1)
    if (mode0scr != 2) {
        lcd.setCursor(0, 2);
        if (bigDig) {
            if (mode0scr == 1) lcd.setCursor(15, 2);
            else lcd.setCursor(15, 0);
        }
        lcd.print((int)round(dispTemp));
        lcd.write(239);
    } else {
        drawTemp(dispTemp, 0, 0);
    }

    if (mode0scr != 4) {
        lcd.setCursor(5, 2);
        if (bigDig) lcd.setCursor(15, 1);
        printInt(dispHum, 0);
        lcd.print("% ");
    } else {
        drawHum(dispHum, 0, 0);
    }

#if (CO2_SENSOR == 1)
    if (mode0scr != 1) {
        if (bigDig) {
            lcd.setCursor(15, 2);
            printInt(dispCO2, 0);
            lcd.print('p');
        } else {
            lcd.setCursor(11, 2);
            printInt(dispCO2, 0);
            lcd.print("ppm ");
        }
    } else {
        drawPPM(dispCO2, 0, 0);
    }
#endif

    if (mode0scr != 3) {
        lcd.setCursor(0, 3);
        if (bigDig && mode0scr == 0) lcd.setCursor(15, 3);
        if (bigDig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
        if (bigDig && mode0scr == 4) lcd.setCursor(15, 1);
        if (!(bigDig && mode0scr == 1)) {
            printInt(dispPres, 0);
            lcd.print("mm");
        }
    } else {
        drawPres(dispPres, 0, 0);
    }

    if (mode0scr == 5) {
        drawAlt(dispAlt, 0, 0);
    }

    if (!bigDig) {
        lcd.setCursor(5, 3);
        lcd.print(" rain     ");
        lcd.setCursor(11, 3);
        if (dispRain < 0) lcd.setCursor(10, 3);
        printInt(dispRain, 0);
        lcd.print('%');
    }

    if (mode0scr != 0) {
        lcd.setCursor(15, 3);
        if (hrs / 10 == 0) lcd.print(' ');
        if (hrs < 10) lcd.print('0');
        lcd.print(hrs);
        lcd.print(':');
        if (mins < 10) lcd.print('0');
        lcd.print(mins);
    } else {
        drawClock(hrs, mins, 0, 0);
    }
#else
    if (!bigDig) {
        lcd.setCursor(0, 0);
        char buf[8];
        dtostrf(dispTemp, 1, 1, buf);
        lcd.print(buf);
        lcd.write(239);
        lcd.setCursor(6, 0);
        printInt(dispHum, 0);
        lcd.print("% ");
#if (CO2_SENSOR == 1)
        printInt(dispCO2, 0);
        lcd.print("ppm");
        if (dispCO2 < 1000) lcd.print(' ');
#endif
        lcd.setCursor(0, 1);
        printInt(dispPres, 0);
        lcd.print(" mm  rain ");
        printInt(dispRain, 0);
        lcd.print("% ");
    } else {
        switch (mode0scr) {
            case 0: drawClock(hrs, mins, 0, 0); break;
#if (CO2_SENSOR == 1)
            case 1: drawPPM(dispCO2, 0, 0); break;
#endif
            case 2: drawTemp(dispTemp, 2, 0); break;
            case 3: drawPres(dispPres, 2, 0); break;
            case 4: drawHum(dispHum, 0, 0); break;
            case 5: drawAlt(dispAlt, 0, 0); break;
        }
    }
#endif
}

void loadClock() {
    if (bigDig && DISPLAY_TYPE == 1) {
        lcd.createChar(0, (uint8_t*)UT);
        lcd.createChar(1, row3);
        lcd.createChar(2, (uint8_t*)UB);
        lcd.createChar(3, row5);
        lcd.createChar(4, (uint8_t*)KU);
        lcd.createChar(5, (uint8_t*)KD);
    } else {
        lcd.createChar(0, row2);
        lcd.createChar(1, (uint8_t*)UB);
        lcd.createChar(2, row3);
        lcd.createChar(3, (uint8_t*)UMB);
        lcd.createChar(4, (uint8_t*)LMB);
        lcd.createChar(5, (uint8_t*)LM2);
    }

    if (now.dayOfTheWeek() == 4) {
        lcd.createChar(7, CH);
    } else if (now.dayOfTheWeek() == 6) {
        lcd.createChar(7, BB);
    } else {
        lcd.createChar(7, PP);
    }
}

// === Функция drawPlot — без String ===
void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int* plot_array, const char* label1, const char* label2, int stretch) {
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
    printInt(max_value, 0);
    lcd.setCursor(16, 1);
    lcd.print(label1);
    lcd.print(label2);
    lcd.setCursor(16, 2);
    printInt(plot_array[14], 0);
    lcd.setCursor(16, 3);
    printInt(min_value, 0);
#else
    lcd.setCursor(12, 0);
    lcd.print(label1);
    lcd.setCursor(13, 0);
    printInt(max_value, 0);
    lcd.setCursor(12, 1);
    lcd.print(label2);
    lcd.setCursor(13, 1);
    printInt(min_value, 0);
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