#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "GyverButton.h"
#include "RTClib.h"
#include "config.h"

#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>
#endif

#if (LED_MODE == 0)
byte LED_ON = (LED_BRIGHT_MAX);
byte LED_OFF = (0);
#else
byte LED_ON = (255 - LED_BRIGHT_MAX);
byte LED_OFF = (255);
#endif

byte LEDType =
    0;  //  при отсутствии сохранения в EEPROM: привязка индикатора к датчикам:
        //  0 - СО2, 1 - Влажность, 2 - Температура, 3 - Осадки
byte LED_BRIGHT =
    10;  // при отсутствии сохранения в EEPROM: яркость светодиода СО2 (0 - 10)
         // (коэффициент настраиваемой яркости индикатора по умолчанию, если нет
         // сохранения и не автоматическая регулировка (с)НР)
byte LCD_BRIGHT =
    10;  // при отсутствии сохранения в EEPROM: яркость экрана (0 - 10)
         // (коэффициент настраиваемой яркости экрана по умолчанию, если нет
         // сохранения и не автоматическая регулировка (с)НР)
int MAX_ONDATA =
    1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 +
    2048;  // при отсутствии сохранения в EEPROM: максимальные показания
           // графиков исходя из накопленных фактических (но в пределах лимитов)
           // данных вместо указанных пределов, 0 - использовать фиксированные
           // пределы (с)НР
int VIS_ONDATA = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 +
                 2048;  // при отсутствии сохранения в EEPROM: отображение
                        // показания графиков, 0 - Не отображать (с)НР

/* 1 - для графика СО2 часового, 2 - для графика СО2 суточного (с)НР
   4 - для графика влажности часовой, 8 - для графика влажности суточной (с)НР
   16 - для графика температуры часовой, 32 - для графика температуры суточной
   (с)НР 64 - для прогноза дождя часового, 128 - для прогноза дождя суточного
   (с)НР 256 - для графика давления часового, 512 - для графика давления
   суточного (с)НР 1024 - для графика высоты часового, 2048 - для графика высоты
   суточного (с)НР для выборочных графиков значения нужно сложить (с)НР
   например: для изменения пределов у графиков суточной температуры и суточного
   СО2 складываем 2 + 32 и устанавливаем значение 34 (можно ставить сумму) (с)НР
*/

#if (DISPLAY_TYPE == 1)
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);
#else
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 16, 2);
#endif

RTC_DS3231 rtc;
DateTime now;
Adafruit_BME280 bme;

#if (CO2_SENSOR == 1)
MHZ19_uart mhz19;
#endif

unsigned long sensorsTimer = SENS_TIME;
unsigned long drawSensorsTimer = SENS_TIME;
unsigned long clockTimer = 500;

#if (DISPLAY_TYPE == 1)                                     // для дисплея 2004 график "длиннее", поэтому интервалы
                                                            // времени на сегмент короче (с)НР
unsigned long hourPlotTimer = ((long)4 * 60 * 1000);        // 4 минуты
unsigned long dayPlotTimer = ((long)1.6 * 60 * 60 * 1000);  // 1.6 часа
#else
unsigned long hourPlotTimer = ((long)5 * 60 * 1000);      // 5 минуты
unsigned long dayPlotTimer = ((long)2 * 60 * 60 * 1000);  // 2 часа
#endif

unsigned long predictTimer = ((long)10 * 60 * 1000);  // 10 минут
unsigned long plotTimer = hourPlotTimer;
unsigned long brightTimer = (2000);

unsigned long sensorsTimerD = 0;
unsigned long drawSensorsTimerD = 0;
unsigned long clockTimerD = 0;
unsigned long hourPlotTimerD = 0;
unsigned long dayPlotTimerD = 0;
unsigned long plotTimerD = 0;
unsigned long predictTimerD = 0;
unsigned long brightTimerD = 0;

GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);

int8_t hrs, mins, secs;
byte mode = 0;
/*
  0 часы и данные
  1 график углекислого за час
  2 график углекислого за сутки
  3 график влажности за час
  4 график влажности за сутки
  5 график температуры за час
  6 график температуры за сутки
  7 график дождя/давления за час
  8 график дождя/давления за сутки
  9 график высоты за час
  10 график высоты за сутки
*/

byte podMode = 1;  // подрежим меню(с)НР
byte mode0scr = 0;
/* (с)НР
  0 - Крупно время
  1 - Крупно содержание СО2
  2 - Крупно температура
  3 - Крупно давление
  4 - Крупно влажность
  5 - Крупно высота
*/
boolean bigDig = false;  // true - цифры на главном экране на все 4 строки (для
                         // LCD 2004) (с)НР

// переменные для вывода
float dispTemp;
byte dispHum;
int dispPres;
int dispCO2 = -1;
int dispRain;
float dispAlt;  // int

// массивы графиков
int tempHour[15], tempDay[15];
// #define tempK 40                //поправочный поэффициент, чтобы показания
// влезли в байт
int humHour[15], humDay[15];
int pressHour[15], pressDay[15];
// #define pressK -600             //поправочный поэффициент, чтобы показания
// влезли в байт
int rainHour[15], rainDay[15];
// #define rainK 100               //поправочный поэффициент, чтобы показания
// влезли в байт
int co2Hour[15], co2Day[15];
int altHour[15], altDay[15];  // высота
int delta;
uint32_t pressure_array[6];
uint32_t sumX, sumY, sumX2, sumXY;
float a, b;
// byte time_array[6];

/*
  Характеристики датчика BME:
  Температура: от-40 до + 85 °C
  Влажность: 0-100%
  Давление: 300-1100 hPa (225-825 ммРтСт)
  Разрешение:
  Температура: 0,01 °C
  Влажность: 0.008%
  Давление: 0,18 Pa
  Точность:
  Температура: +-1 °C
  Влажность: +-3%
  Давление: +-1 Па
*/

// символы
// график
byte rowS[8] = {0b00000, 0b00000, 0b00000, 0b00000,
                0b10001, 0b01010, 0b00100, 0b00000};  // стрелка вниз (с)НР
byte row7[8] = {0b00000, 0b11111, 0b11111, 0b11111,
                0b11111, 0b11111, 0b11111, 0b11111};
byte row6[8] = {0b00000, 0b00000, 0b11111, 0b11111,
                0b11111, 0b11111, 0b11111, 0b11111};
byte row5[8] = {0b00000, 0b00000, 0b00000, 0b11111, 0b11111,
                0b11111, 0b11111, 0b11111};  // в т.ч. для четырехстрочных цифр
                                             // 2, 3, 4, 5, 6, 8, 9, 0 (с)НР
byte row4[8] = {0b00000, 0b00000, 0b00000, 0b00000,
                0b11111, 0b11111, 0b11111, 0b11111};
byte row3[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000,
                0b11111, 0b11111, 0b11111};  // в т.ч. для двустрочной цифры 0,
                                             // для четырехстрочных цифр 2, 3,
                                             // 4, 5, 6, 8, 9 (с)НР
byte row2[8] = {
    0b00000, 0b00000, 0b00000, 0b00000, 0b00000,
    0b00000, 0b11111, 0b11111};  // в т.ч. для двустрочной цифры 4 (с)НР
byte row1[8] = {0b00000, 0b00000, 0b00000, 0b00000,
                0b00000, 0b00000, 0b00000, 0b11111};

// цифры //  (с)НР
uint8_t UB[8] = {
    0b11111, 0b11111, 0b11111, 0b00000,
    0b00000, 0b00000, 0b00000, 0b00000};  // для двустрочных 7, 0   // для
                                          // четырехстрочных 2, 3, 4, 5, 6, 8, 9
uint8_t UMB[8] = {
    0b11111, 0b11111, 0b11111, 0b00000,
    0b00000, 0b00000, 0b11111, 0b11111};  // для двустрочных 2, 3, 5, 6, 8, 9
uint8_t LMB[8] = {
    0b11111, 0b00000, 0b00000, 0b00000,
    0b00000, 0b11111, 0b11111, 0b11111};  // для двустрочных 2, 3, 5, 6, 8, 9
uint8_t LM2[8] = {0b11111, 0b00000, 0b00000, 0b00000,
                  0b00000, 0b00000, 0b00000, 0b00000};  // для двустрочной 4
uint8_t UT[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
                 0b00000, 0b00000, 0b00000};  // для четырехстрочных 2, 3, 4, 5,
                                              // 6, 7, 8, 9, 0

uint8_t KU[8] = {0b00000, 0b00000, 0b00000, 0b00001,
                 0b00010, 0b00100, 0b01000, 0b10000};  // для верхней части %
uint8_t KD[8] = {0b00001, 0b00010, 0b00100, 0b01000,
                 0b10000, 0b00000, 0b00000, 0b00000};  // для нижней части %

// русские буквы (с)НР
uint8_t PP[8] = {0b11111, 0b10001, 0b10001, 0b10001,
                 0b10001, 0b10001, 0b10001, 0b00000};  // П
uint8_t BB[8] = {0b11111, 0b10000, 0b10000, 0b11111,
                 0b10001, 0b10001, 0b11111, 0b00000};  // Б
uint8_t CH[8] = {0b10001, 0b10001, 0b10001, 0b01111,
                 0b00001, 0b00001, 0b00001, 0b00000};  // Ч
uint8_t II[8] = {0b10001, 0b10001, 0b10011, 0b10101,
                 0b11001, 0b10001, 0b10001, 0b00000};  // И
uint8_t BM[8] = {0b10000, 0b10000, 0b10000, 0b11110,
                 0b10001, 0b10001, 0b11110, 0b00000};  // Ь
uint8_t IY[8] = {0b01100, 0b00001, 0b10011, 0b10101,
                 0b11001, 0b10001, 0b10001, 0b00000};  // Й
uint8_t DD[8] = {0b01110, 0b01010, 0b01010, 0b01010,
                 0b01010, 0b01010, 0b11111, 0b10001};  // Д
uint8_t AA[8] = {0b11100, 0b00010, 0b00001, 0b00111,
                 0b00001, 0b00010, 0b11100, 0b00000};  // Э
uint8_t IA[8] = {0b01111, 0b10001, 0b10001, 0b01111,
                 0b00101, 0b01001, 0b10001, 0b00000};  // Я
uint8_t YY[8] = {0b10001, 0b10001, 0b10001, 0b11101,
                 0b10011, 0b10011, 0b11101, 0b00000};  // Ы
uint8_t GG[8] = {0b11110, 0b10000, 0b10000, 0b10000,
                 0b10000, 0b10000, 0b10000, 0b00000};  // Г
uint8_t FF[8] = {0b00100, 0b01110, 0b10101, 0b10101,
                 0b10101, 0b01110, 0b00100, 0b00000};  // Ф
uint8_t LL[8] = {0b01111, 0b01001, 0b01001, 0b01001,
                 0b01001, 0b01001, 0b10001, 0b00000};  // Л
uint8_t ZZ[8] = {0b10101, 0b10101, 0b10101, 0b01110,
                 0b10101, 0b10101, 0b10101, 0b00000};  // Ж

#if (WEEK_LANG == 0)
static const char* dayNames[] = {
    "Su",
    "Mo",
    "Tu",
    "We",
    "Th",
    "Fr",
    "Sa",
};
#else
static const char* dayNames[] = {
    // доработал дни недели на двухсимвольные русские (ПН, ВТ, СР....) (с)НР
    "BC",
    "\7H",
    "BT",
    "CP",
    "\7T",
    "\7T",
    "C\7",
};
#endif

void drawPlot(byte pos, byte row, byte width, byte height, int min_val,
              int max_val, int* plot_array, String label1, String label2,
              int stretch) {  // график ---------------------------------
    int max_value = -32000;
    int min_value = 32000;

    for (byte i = 0; i < 15; i++) {
        max_value = max(plot_array[i], max_value);
        min_value = min(plot_array[i], min_value);
    }

    // меняем пределы графиков на предельные/фактические значения, одновременно
    // рисуем указатель пределов (стрелочки вверх-вниз) (с)НР
    lcd.setCursor(15, 0);
    if ((MAX_ONDATA & (1 << (stretch - 1))) >
        0) {  // побитовое сравнение 1 - растягиваем, 0 - не растягиваем (по
              // указанным пределам) (с)НР
        //    max_val = min(max_value, max_val);
        //    min_val = max(min_value, min_val);
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

    // Serial.println(max_val);Serial.println(min_val);  // отладка (с)НР

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
    for (byte i = 0; i < width; i++) {  // каждый столбец параметров
        int fill_val = plot_array[i];
        fill_val = constrain(fill_val, min_val, max_val);
        byte infill, fract;
        // найти количество целых блоков с учётом минимума и максимума для
        // отображения на графике
        if ((plot_array[i]) > min_val)
            infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) *
                           height * 10);
        else
            infill = 0;
        fract =
            (float)(infill % 10) * 8 / 10;  // найти количество оставшихся полосок
        infill = infill / 10;

        for (byte n = 0; n < height; n++) {   // для всех строк графика
            if (n < infill && infill > 0) {   // пока мы ниже уровня
                lcd.setCursor(i, (row - n));  // заполняем полными ячейками
                lcd.write(255);
            }
            if (n >= infill) {  // если достигли уровня
                lcd.setCursor(i, (row - n));
                if (n == 0 && fract == 0)
                    fract++;  // если нижний перел графика имеет минимальное значение, то
                              // рисуем одну полоску, чтобы не было пропусков (с)НР
                if (fract > 0)
                    lcd.write(fract);  // заполняем дробные ячейки
                else
                    lcd.write(16);  // если дробные == 0, заливаем пустой
                for (byte k = n + 1; k < height;
                     k++) {  // всё что сверху заливаем пустыми
                    lcd.setCursor(i, (row - k));
                    lcd.write(16);
                }
                break;
            }
        }
    }
}

void loadPlot() {
    lcd.createChar(0, rowS);  // Стрелка вниз для индикатора пределов (с)НР
    lcd.createChar(1, row1);
    lcd.createChar(2, row2);
    lcd.createChar(3, row3);
    lcd.createChar(4, row4);
    lcd.createChar(5, row5);
    lcd.createChar(6, row6);
    lcd.createChar(7, row7);
}

void setLEDcolor(
    byte color) {  // цвет индикатора задается двумя битами на каждый цвет (с)НР
    analogWrite(LED_R,
                LED_ON + LED_ON * ((LED_MODE << 1) - 1) * (3 - (color & 3)) / 3);
    analogWrite(LED_G, LED_ON + LED_ON * ((LED_MODE << 1) - 1) *
                                    (3 - ((color & 12) >> 2)) / 3);
    analogWrite(LED_B, LED_ON + LED_ON * ((LED_MODE << 1) - 1) *
                                    (3 - ((color & 48) >> 4)) / 3);
}

void digSeg(byte x, byte y, byte z1, byte z2, byte z3, byte z4, byte z5,
            byte z6) {  // отображение двух строк по три символа с указанием
                        // кодов символов (с)НР
    lcd.setCursor(x, y);
    lcd.write(z1);
    lcd.write(z2);
    lcd.write(z3);
    if (x <= 11) lcd.print(" ");
    lcd.setCursor(x, y + 1);
    lcd.write(z4);
    lcd.write(z5);
    lcd.write(z6);
    if (x <= 11) lcd.print(" ");
}

void drawDig(
    byte dig, byte x,
    byte y) {  // рисуем цифры (с)НР ---------------------------------------
    if (bigDig && DISPLAY_TYPE == 1) {
        switch (dig) {  // четырехстрочные цифры (с)НР
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
            case 10:
                digSeg(x, y, 32, 32, 32, 32, 32, 32);
                digSeg(x, y + 2, 32, 32, 32, 32, 32, 32);
                break;
        }
    } else {
        switch (dig) {  // двухстрочные цифры
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

void drawPres(int dispPres, byte x,
              byte y) {  // Давление крупно на главном экране (с)НР
                         // ----------------------------
    drawDig((dispPres % 1000) / 100, x, y);
    drawDig((dispPres % 100) / 10, x + 4, y);
    drawDig(dispPres % 10, x + 8, y);
    lcd.setCursor(x + 11, 1);
    if (bigDig) lcd.setCursor(x + 11, 3);
    lcd.print("mm");
}

void drawAlt(float dispAlt, byte x,
             byte y) {  // Высота крупно на главном экране (с)НР
                        // -----------------------------
    if (dispAlt >= 1000) {
        drawDig((int(dispAlt) % 10000) / 1000, x, y);
        x += 4;
    }
    drawDig((int(dispAlt) % 1000) / 100, x, y);
    drawDig((int(dispAlt) % 100) / 10, x + 4, y);
    drawDig(int(dispAlt) % 10, x + 8, y);
    if (dispAlt < 1000) {  // десятые доли метра, если высота ниже 1000 м. (с)НР
        //   drawDig((int(dispAlt * 10.0)) % 10 , x + 12, y);         // десятые
        //   крупными цифрами (тогда буква m наезжает на последнюю цифру)
        lcd.setCursor(
            x + 12,
            y + 1 + (bigDig && DISPLAY_TYPE) * 2);  // десятые мелкими цифрами
        lcd.print((int(dispAlt * 10.0)) % 10);
        if (bigDig && DISPLAY_TYPE == 1)
            lcd.setCursor(x + 11, y + 3);
        else
            lcd.setCursor(x + 11, y + 1);
        lcd.print(".");
        x -= 1;  // сдвинуть букву m левее
    } else {
        x -= 4;
    }
    if (bigDig && DISPLAY_TYPE == 1)
        lcd.setCursor(x + 14, 3);
    else
        lcd.setCursor(x + 14, 1);
    lcd.print("m");
}

void drawClock(byte hours, byte minutes, byte x,
               byte y) {  // рисуем время крупными цифрами
                          // -------------------------------------------
    if (hours > 23 || minutes > 59) return;
    if (hours / 10 == 0)
        drawDig(10, x, y);
    else
        drawDig(hours / 10, x, y);
    drawDig(hours % 10, x + 4, y);
    // тут должны быть точки. Отдельной функцией
    drawDig(minutes / 10, x + 8, y);
    drawDig(minutes % 10, x + 12, y);
}

void drawTemp(float dispTemp, byte x,
              byte y) {  // Температура крупно на главном экране (с)НР
                         // ----------------------------
    if (dispTemp / 10 == 0)
        drawDig(10, x, y);
    else
        drawDig(dispTemp / 10, x, y);
    drawDig(int(dispTemp) % 10, x + 4, y);
    drawDig(int(dispTemp * 10.0) % 10, x + 9, y);

    if (bigDig && DISPLAY_TYPE == 1) {
        lcd.setCursor(x + 7, y + 3);
        lcd.write(1);  // десятичная точка
    } else {
        lcd.setCursor(x + 7, y + 1);
        lcd.write(46);  // десятичная точка
    }
    lcd.setCursor(x + 13, y);
    lcd.write(239);  // градусы
}

void drawHum(int dispHum, byte x,
             byte y) {  // Влажность крупно на главном экране (с)НР
                        // ----------------------------
    if (dispHum / 100 == 0)
        drawDig(10, x, y);
    else
        drawDig(dispHum / 100, x, y);
    if ((dispHum % 100) / 10 == 0)
        drawDig(0, x + 4, y);
    else
        drawDig(dispHum / 10, x + 4, y);
    drawDig(int(dispHum) % 10, x + 8, y);
    if (bigDig && DISPLAY_TYPE == 1) {
        lcd.setCursor(x + 12, y + 1);
        lcd.write(239);
        lcd.print("\4");
        lcd.setCursor(x + 12, y + 2);
        lcd.print("\5");
        lcd.write(239);
    } else {
        lcd.setCursor(x + 12, y + 1);
        lcd.print("%");
    }
}

void drawPPM(int dispCO2, byte x,
             byte y) {  // Уровень СО2 крупно на главном экране (с)НР
                        // ----------------------------
    if (dispCO2 / 1000 == 0)
        drawDig(10, x, y);
    else
        drawDig(dispCO2 / 1000, x, y);
    drawDig((dispCO2 % 1000) / 100, x + 4, y);
    drawDig((dispCO2 % 100) / 10, x + 8, y);
    drawDig(dispCO2 % 10, x + 12, y);
    lcd.setCursor(15, 0);
#if (DISPLAY_TYPE == 1)
    lcd.print("ppm");
#else
    lcd.print("p");
#endif
}

void drawSensors() {
#if (DISPLAY_TYPE == 1)
    // дисплей 2004 ----------------------------------

    if (mode0scr != 2) {  // Температура (с)НР ----------------------------
        lcd.setCursor(0, 2);
        if (bigDig) {
            if (mode0scr == 1) lcd.setCursor(15, 2);
            if (mode0scr != 1) lcd.setCursor(15, 0);
        }
        lcd.print((int)round(dispTemp));
        lcd.write(239);
    } else {
        drawTemp(dispTemp, 0, 0);
    }

    if (mode0scr != 4) {  // Влажность (с)НР ----------------------------
        lcd.setCursor(5, 2);
        if (bigDig) lcd.setCursor(15, 1);
        lcd.print(" " + String(dispHum) + "% ");
    } else {
        drawHum(dispHum, 0, 0);
    }

#if (CO2_SENSOR == 1)
    if (mode0scr != 1) {  // СО2 (с)НР ----------------------------

        if (bigDig) {
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

    if (mode0scr != 3) {  // Давление (с)НР ---------------------------
        lcd.setCursor(0, 3);
        if (bigDig && mode0scr == 0) lcd.setCursor(15, 3);
        if (bigDig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
        if (bigDig && mode0scr == 4) lcd.setCursor(15, 1);
        if (!(bigDig && mode0scr == 1)) lcd.print(String(dispPres) + "mm");
    } else {
        drawPres(dispPres, 0, 0);
    }

    if (mode0scr != 5) {  // Высота (с)НР ----------------------------
    } else {              // мелко высоту не выводим (с)НР
        drawAlt(dispAlt, 0, 0);
    }

    if (!bigDig) {  // дождь (с)НР -----------------------------
        lcd.setCursor(5, 3);
        lcd.print(" rain     ");
        lcd.setCursor(11, 3);
        if (dispRain < 0) lcd.setCursor(10, 3);
        lcd.print(String(dispRain) + "%");
        //  lcd.setCursor(14, 3);
        //  lcd.print(bme.readAltitude(SEALEVELPRESSURE_HPA));  // высота над
        //  уровнем моря (с)НР
    }

    if (mode0scr != 0) {  // время (с)НР ----------------------------
        lcd.setCursor(15, 3);
        if (hrs / 10 == 0) lcd.print(" ");
        lcd.print(hrs);
        lcd.print(":");
        if (mins / 10 == 0) lcd.print("0");
        lcd.print(mins);
    } else {
        drawClock(hrs, mins, 0, 0);  //, 1);
    }
#else

    // дисплей 1602 ----------------------------------
    if (!bigDig) {  // если только мелкими цифрами (с)НР
        lcd.setCursor(0, 0);
        lcd.print(String(dispTemp, 1));
        lcd.write(239);
        lcd.setCursor(6, 0);
        lcd.print(String(dispHum) + "% ");

#if (CO2_SENSOR == 1)
        lcd.print(String(dispCO2) + "ppm");
        if (dispCO2 < 1000) lcd.print(" ");
#endif

        lcd.setCursor(0, 1);
        lcd.print(String(dispPres) + " mm  rain ");
        lcd.print(String(dispRain) + "% ");
    } else {  // для крупных цифр (с)НР
        switch (mode0scr) {
            case 0:
                drawClock(hrs, mins, 0, 0);
                break;
            case 1:
#if (CO2_SENSOR == 1)
                drawPPM(dispCO2, 0, 0);
#endif
                break;
            case 2:
                drawTemp(dispTemp, 2, 0);
                break;
            case 3:
                drawPres(dispPres, 2, 0);
                break;
            case 4:
                drawHum(dispHum, 0, 0);
                break;
            case 5:
                drawHum(dispAlt, 0, 0);
                break;
        }
    }
#endif
}

void loadClock() {
    if (bigDig && (DISPLAY_TYPE == 1)) {  // для четырехстрочных цифр (с)НР
        lcd.createChar(0, UT);
        lcd.createChar(1, row3);
        lcd.createChar(2, UB);
        lcd.createChar(3, row5);
        lcd.createChar(4, KU);
        lcd.createChar(5, KD);
    } else {  // для двустрочных цифр (с)НР
        lcd.createChar(0, row2);
        lcd.createChar(1, UB);
        lcd.createChar(2, row3);
        lcd.createChar(3, UMB);
        lcd.createChar(4, LMB);
        lcd.createChar(5, LM2);
    }

    if (now.dayOfTheWeek() == 4) {  // Для четверга в ячейку запоминаем "Ч", для
                                    // субботы "Б", иначе "П" (с)НР
        lcd.createChar(7, CH);      // Ч (с)НР
    } else if (now.dayOfTheWeek() == 6) {
        lcd.createChar(7, BB);  // Б (с)НР
    } else {
        lcd.createChar(7, PP);  // П (с)НР
    }
}

void drawData() {  // выводим дату
                   // -------------------------------------------------------------
    int Y = 0;
    if (DISPLAY_TYPE == 1 && mode0scr == 1) Y = 2;
    if (!bigDig) {  // если 4-х строчные цифры, то дату, день недели (и секунды)
                    // не пишем - некуда (с)НР
        lcd.setCursor(15, 0 + Y);
        if (now.day() < 10) lcd.print(0);
        lcd.print(now.day());
        lcd.print(".");
        if (now.month() < 10) lcd.print(0);
        lcd.print(now.month());

        if (DISP_MODE == 0) {
            lcd.setCursor(16, 1);
            lcd.print(now.year());
        } else {
            loadClock();  // принудительно обновляем знаки, т.к. есть жалобы на
                          // необновление знаков в днях недели (с)НР
            lcd.setCursor(18, 1);
            int dayofweek = now.dayOfTheWeek();
            lcd.print(dayNames[dayofweek]);
            // if (hrs == 0 && mins == 0 && secs <= 1) loadClock();   // Обновляем
            // знаки, чтобы русские буквы в днях недели тоже обновились. (с)НР
        }
    }
}

void checkBrightness() {
    int photoValue = analogRead(PHOTO);
    static bool isDark = false;

    if (isDark) {
        // Сейчас темно → переключаемся в "светло", только если сигнал УВЕРЕННО выше
        // порога
        if (photoValue > BRIGHT_THRESHOLD + BRIGHT_HYSTERESYS) {
            isDark = false;
        }
    } else {
        // Сейчас светло → переключаемся в "темно", только если сигнал УВЕРЕННО ниже
        // порога
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
        analogWrite(BACKLIGHT, LCD_BRIGHT * LCD_BRIGHT * 2.5);
    }

    if (LED_BRIGHT == 11) {
        if (isDark) {
#if (LED_MODE == 0)
            LED_ON = (LED_BRIGHT_MIN);
#else
            LED_ON = (255 - LED_BRIGHT_MIN);
#endif
        } else {
#if (LED_MODE == 0)
            LED_ON = (LED_BRIGHT_MAX);
#else
            LED_ON = (255 - LED_BRIGHT_MAX);
#endif
        }
    }
}

/*
  mode:
  0 - Главный экран
  1-8 - Графики: СО2 (час, день), Влажность (час, день), Температура (час,
  день), Осадки (час, день) 252 - выбор режима индикатора (podMode от 0 до 3:
  индикация СО2, влажности, температуры, осадков) 253 - настройка яркости экрана
  (podMode от 0 до 11: выбор от 0% до 100% или автоматическое регулирование) 254
  - настройка яркости индикатора (podMode от 0 до 11: выбор от 0% до 100% или
  автоматическое регулирование) 255 - главное меню (podMode от 0 до 13: 1 -
  Сохранить, 2 - Выход, 3 - Ярк.индикатора, 4 - Ярк.экрана, 5 - Режим
  индикатора, 6-13 вкл/выкл графики: СО2 (час, день), Влажность (час, день),
  Температура (час, день), Осадки (час, день))
*/

void setLED() {
    if (LED_BRIGHT < 11) {  // если ручные установки яркости
        LED_ON = 255 / 100 * LED_BRIGHT * LED_BRIGHT;
    } else {
        checkBrightness();
    }
    if (LED_MODE != 0) LED_ON = 255 - LED_ON;

    // ниже задается цвет индикатора в зависимости от назначенного сенсора:
    // красный, желтый, зеленый, синий (с)НР

    if ((dispCO2 >= maxCO2) && LEDType == 0 ||
        (dispHum <= minHum) && LEDType == 1 ||
        (dispTemp >= maxTemp) && LEDType == 2 ||
        (dispRain <= minRain) && LEDType == 3 ||
        (dispPres <= minPress) && LEDType == 4)
        setLEDcolor(3);  // красный
    else if ((dispCO2 >= normCO2) && LEDType == 0 ||
             (dispHum <= normHum) && LEDType == 1 ||
             (dispTemp >= normTemp) && LEDType == 2 ||
             (dispRain <= normRain) && LEDType == 3 ||
             (dispPres <= normPress) && LEDType == 4)
        setLEDcolor(48);  // синий // желтый 3 + 8
    else if (LEDType == 0 || (dispHum <= maxHum) && LEDType == 1 ||
             (dispTemp >= minTemp) && LEDType == 2 ||
             (dispRain <= maxRain) && LEDType == 3 || LEDType == 4)
        setLEDcolor(12);  // зеленый
    else
        setLEDcolor(
            48);  // синий (если влажность превышает заданный максимум, температура
                  // ниже минимума, вероятность осадков выше maxRain)
}

void redrawPlot() {
    lcd.clear();
#if (DISPLAY_TYPE == 1)  // для дисплея 2004
    switch (mode) {      // добавлена переменная для "растягивания" графика до
                         // фактических максимальных и(или) минимальных значений(с)НР
        case 1:
            drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Hour, "c ", "hr", mode);
            break;
        case 2:
            drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Day, "c ", "da", mode);
            break;
        case 3:
            drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humHour, "h%", "hr", mode);
            break;
        case 4:
            drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humDay, "h%", "da", mode);
            break;
        case 5:
            drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t\337", "hr",
                     mode);
            break;
        case 6:
            drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t\337", "da",
                     mode);
            break;
        //    case 7: drawPlot(0, 3, 15, 4, RAIN_MIN, RAIN_MAX, (int*)rainHour, "r
        //    ", "hr", mode);
        //      break;
        //    case 8: drawPlot(0, 3, 15, 4, RAIN_MIN, RAIN_MAX, (int*)rainDay, "r ",
        //    "da", mode);
        //      break;
        case 7:
            drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p ", "hr",
                     mode);
            break;
        case 8:
            drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p ", "da",
                     mode);
            break;
        case 9:
            drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, (int*)altHour, "m ", "hr", mode);
            break;
        case 10:
            drawPlot(0, 3, 15, 4, ALT_MIN, ALT_MAX, (int*)altDay, "m ", "da", mode);
            break;
    }
#else  // для дисплея 1602
    switch (mode) {
        case 1:
            drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Hour, "c", "h", mode);
            break;
        case 2:
            drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Day, "c", "d", mode);
            break;
        case 3:
            drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humHour, "h", "h", mode);
            break;
        case 4:
            drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humDay, "h", "d", mode);
            break;
        case 5:
            drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t", "h", mode);
            break;
        case 6:
            drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t", "d", mode);
            break;
        //    case 7: drawPlot(0, 1, 12, 2, RAIN_MIN, RAIN_MAX, (int*)rainHour, "r",
        //    "h", mode);
        //      break;
        //    case 8: drawPlot(0, 1, 12, 2, RAIN_MIN, RAIN_MAX, (int*)rainDay, "r",
        //    "d", mode);
        //      break;
        case 7:
            drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p", "h",
                     mode);
            break;
        case 8:
            drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p", "d",
                     mode);
            break;
        case 9:
            drawPlot(0, 1, 12, 2, ALT_MIN, ALT_MAX, (int*)altHour, "m", "h", mode);
            break;
        case 10:
            drawPlot(0, 1, 12, 2, ALT_MIN, ALT_MAX, (int*)altDay, "m", "d", mode);
            break;
    }
#endif
}

void modesTick() {
    button.tick();
    boolean changeFlag = false;
    if (button.isSingle()) {  // одинарное нажатие на кнопку

        if (mode >= 240) {
            podMode++;
            switch (mode) {
                case 252:  // Перебираем все варианты режимов LED индикатора (с)НР
                    //         podMode++;
                    if (podMode > 4) podMode = 0;
                    LEDType = podMode;
                    changeFlag = true;
                    break;

                case 253:  // Перебираем все варианты яркости LCD экрана (с)НР
                    //         podMode++;
                    if (podMode > 11) podMode = 0;
                    LCD_BRIGHT = podMode;
                    checkBrightness();
                    changeFlag = true;
                    break;

                case 254:  // Перебираем все варианты яркости LED индикатора (с)НР
                    //         podMode++;
                    if (podMode > 11) podMode = 0;
                    LED_BRIGHT = podMode;
                    changeFlag = true;
                    break;

                case 255:  // Перебираем все варианты основных настроек (с)НР
                    //         podMode++;
                    if (podMode > 15) podMode = 1;
                    changeFlag = true;
                    break;
            }
        } else {
            do {
                mode++;
                if (mode > 10) mode = 0;
#if (CO2_SENSOR == 0 && mode == 1)
                mode = 3;
#endif
            } while (((VIS_ONDATA & (1 << (mode - 1))) == 0) &&
                     (mode > 0));  // проверка на отображение графиков (с)НР
            changeFlag = true;
        }
    }
    if (button
            .isDouble()) {            // двойное нажатие (с)НР ----------------------------
        if (mode > 0 && mode < 11) {  // Меняет пределы графика на
                                      // установленные/фактические максимумы (с)НР
            MAX_ONDATA = (int)MAX_ONDATA ^ (1 << (mode - 1));
        } else if (mode == 0) {
            mode0scr++;
            if (CO2_SENSOR == 0 && mode0scr == 1) mode0scr++;
            if (mode0scr > 5)
                mode0scr = 0;  // Переключение рехима работы главного экрана (с)НР
        } else if (mode > 240)
            podMode = 1;  // Переключение на меню сохранения (с)НР
        changeFlag = true;
    }

    if ((button.isTriple()) &&
        (mode ==
         0)) {  // тройное нажатие в режиме главного экрана - переход в меню (с)НР
        mode = 255;
        podMode = 3;
        changeFlag = true;
    }

    if (button.isHolded()) {  // удержание кнопки (с)НР
        //    if ((mode >=252) && (mode <= 254)) {
        //      mode = 255;
        //      podMode = 1;
        //    }
        switch (mode) {
            case 0:
                bigDig = !bigDig;
                break;
            case 252:  // реж. индикатора
                mode = 255;
                podMode = 1;
                break;
            case 253:  // ярк. экрана
                mode = 255;
                podMode = 1;
                break;
            case 254:  // ярк. индикатора
                mode = 255;
                podMode = 1;
                break;
            case 255:                                        // главное меню
                if (podMode == 2 || podMode == 1) mode = 0;  // если Выход или Сохранить
                if (podMode >= 3 && podMode <= 5)
                    mode = 255 - podMode + 2;  // если настройки яркостей, то
                                               // переключаемся в настройки пункта меню
                if (podMode >= 6 && podMode <= 17)
                    VIS_ONDATA = VIS_ONDATA ^
                                 (1 << (podMode - 6));  // вкл/выкл отображения графиков
                if (podMode == 1) {                     // если Сохранить
                    if (EEPROM.read(2) != (MAX_ONDATA & 255))
                        EEPROM.write(2, (MAX_ONDATA & 255));
                    if (EEPROM.read(3) != (MAX_ONDATA >> 8))
                        EEPROM.write(3, (MAX_ONDATA >> 8));
                    if (EEPROM.read(4) != (VIS_ONDATA & 255))
                        EEPROM.write(4, (VIS_ONDATA & 255));
                    if (EEPROM.read(5) != (VIS_ONDATA >> 8))
                        EEPROM.write(5, (VIS_ONDATA >> 8));
                    if (EEPROM.read(6) != mode0scr) EEPROM.write(6, mode0scr);
                    if (EEPROM.read(7) != bigDig) EEPROM.write(7, bigDig);
                    if (EEPROM.read(8) != LED_BRIGHT) EEPROM.write(8, LED_BRIGHT);
                    if (EEPROM.read(9) != LCD_BRIGHT) EEPROM.write(9, LCD_BRIGHT);
                    if (EEPROM.read(10) != LEDType) EEPROM.write(10, LEDType);
                    if (EEPROM.read(0) != 122) EEPROM.write(0, 122);
                }
                if (podMode < 6) podMode = 1;
                if (mode == 252)
                    podMode = LEDType;  // если выбран режим LED - устанавливаем текущее
                                        // значение (с)НР
                if (mode == 254)
                    podMode = LED_BRIGHT;  // если выбрана яркость LED - устанавливаем
                                           // текущее показание (с)НР
                if (mode == 253)
                    podMode = LCD_BRIGHT;  // если выбрана яркость LCD - устанавливаем
                                           // текущее показание (с)НР
                break;
            default:
                mode = 0;
        }
        changeFlag = true;
    }

    if (changeFlag) {
        if (mode >= 240) {
            lcd.clear();
            lcd.createChar(1, BM);  // Ь
            lcd.createChar(2, IY);  // Й
            lcd.createChar(3, DD);  // Д
            lcd.createChar(4, II);  // И
            lcd.createChar(5, IA);  // Я
            lcd.createChar(6, YY);  // Ы
            lcd.createChar(7, AA);  // Э
            lcd.createChar(0, ZZ);  // Ж
            lcd.setCursor(0, 0);
        }
        if (mode == 255) {  // Перебираем варианты в главном меню (с)НР
#if (WEEK_LANG == 1)
            lcd.print("HACTPO\2K\4:");  // ---Настройки
#else
            lcd.print("Setup:");
#endif
            lcd.setCursor(0, 1);
            switch (podMode) {
                case 1:
#if (WEEK_LANG == 1)
                    lcd.print("COXPAH\4T\1");  // ---Сохранить
#else
                    lcd.print("Save");
#endif
                    break;
                case 2:
#if (WEEK_LANG == 1)
                    lcd.print("B\6XO\3");  // --- Выход
#else
                    lcd.print("Exit");
#endif
                    break;
                case 5:
#if (WEEK_LANG == 1)
                    lcd.print("PE\10.\4H\3\4KATOPA");  // ---Реж.индик.
#else
                    lcd.print("indicator mode");
#endif
                    break;
                case 3:
#if (WEEK_LANG == 1)
                    lcd.print("\5PK.\4H\3\4KATOPA");  // ---Ярк.индик.
#else
                    lcd.print("indicator brt.");
#endif
                    break;
                case 4:
#if (WEEK_LANG == 1)
                    lcd.print("\5PK.\7KPAHA");  // ---Ярк.экрана
#else
                    lcd.print("Bright LCD");
#endif
                    break;
            }
            if (podMode >= 6 && podMode <= 17) {
                lcd.createChar(8, FF);  // ф
                lcd.createChar(7, GG);  // Г
                lcd.createChar(5, LL);  // Л
                lcd.setCursor(10, 0);
#if (WEEK_LANG == 1)
                lcd.print("\7PA\10\4KOB");  // ---графиков
#else
                lcd.print("Charts  ");
#endif
                lcd.setCursor(0, 1);
                if ((3 & (1 << (podMode - 6))) != 0) lcd.print("CO2 ");
                if ((12 & (1 << (podMode - 6))) != 0) {
#if (WEEK_LANG == 1)
                    lcd.print("B\5,% ");
#else
                    lcd.print("Hum,%");
#endif
                }
                if ((48 & (1 << (podMode - 6))) != 0) lcd.print("t\337 ");
                if ((192 & (1 << (podMode - 6))) != 0) {
                    if (PRESSURE)
                        lcd.print("p,rain ");
                    else
                        lcd.print("p,mmPT ");
                }
                if ((768 & (1 << (podMode - 6))) != 0) {
#if (WEEK_LANG == 1)
                    lcd.print("B\6C,m  ");
#else
                    lcd.print("hgt,m  ");
#endif
                }

                if ((1365 & (1 << (podMode - 6))) != 0) {
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
                if ((VIS_ONDATA & (1 << (podMode - 6))) != 0) {
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
        }
        if (mode == 252) {  // --------------------- показать  "Реж.индикатора"
            LEDType = podMode;
            lcd.createChar(6, LL);  // Л
            lcd.createChar(3, DD);  // Д
            lcd.createChar(5, II);  // И
            lcd.createChar(8, ZZ);  // Ж
            lcd.setCursor(0, 0);
#if (WEEK_LANG == 1)
            lcd.print("PE\10.\4H\3\4KATOPA:");
#else
            lcd.print("indicator mode:");
#endif
            lcd.setCursor(0, 1);
            switch (podMode) {
                case 0:
                    lcd.print("CO2   ");
                    break;
                case 1:
#if (WEEK_LANG == 1)
                    lcd.print("B\6A\10H.");  // влажн.
#else
                    lcd.print("Humid.");
#endif
                    break;
                case 2:
                    lcd.print("t\337     ");
                    break;
                case 3:
#if (WEEK_LANG == 1)
                    lcd.print("OCA\3K\5");  // осадки
#else
                    lcd.print("rain  ");
#endif
                    break;
                case 4:
#if (WEEK_LANG == 1)
                    lcd.print("\3AB\6EH\5E");  // давление
#else
                    lcd.print("pressure");
#endif
                    break;
            }
        }
        if (mode == 253) {  // --------------------- показать  "Ярк.экрана"
#if (WEEK_LANG == 1)
            lcd.print("\5PK.\7KPAHA:");  // + String(LCD_BRIGHT * 10) + "%  ");
#else
            lcd.print("Bright LCD:");
#endif
            // lcd.setCursor(11, 0);
            if (LCD_BRIGHT == 11) {
#if (WEEK_LANG == 1)
                lcd.print("ABTO ");
#else
                lcd.print("Auto ");
#endif
            } else
                lcd.print(String(LCD_BRIGHT * 10) + "%");
        }
        if (mode == 254) {  // --------------------- показать  "Ярк.индикатора"
#if (WEEK_LANG == 1)
            lcd.print("\5PK.\4H\3\4K.:");  // + String(LED_BRIGHT * 10) + "%  ");
#else
            lcd.print("indic.brt.:");
#endif
            // lcd.setCursor(15, 0);
            if (LED_BRIGHT == 11) {
#if (WEEK_LANG == 1)
                lcd.print("ABTO ");
#else
                lcd.print("Auto ");
#endif
            } else
                lcd.print(String(LED_BRIGHT * 10) + "%");
        }

        if (mode == 0) {
            lcd.clear();
            loadClock();
            drawSensors();
            if (DISPLAY_TYPE == 1) drawData();
        } else if (mode <= 10) {
            // lcd.clear();
            loadPlot();
            redrawPlot();
        }
    }
}

void readSensors() {
    bme.takeForcedMeasurement();
    dispTemp = bme.readTemperature() + TEMP_OFFSET;
    dispHum = bme.readHumidity();
    dispAlt = ((float)dispAlt * 1 + bme.readAltitude(SEALEVELPRESSURE_HPA)) /
              2;  // усреднение, чтобы не было резких скачков (с)НР
    dispPres = (float)bme.readPressure() * 0.00750062;
#if (CO2_SENSOR == 1)
    dispCO2 = mhz19.getPPM();
#else
    dispCO2 = 0;
#endif
}

bool testTimer(unsigned long& dataTimer,
               unsigned long setTimer) {  // Проверка таймеров (с)НР
    if (millis() - dataTimer >= setTimer) {
        dataTimer = millis();
        return true;
    } else {
        return false;
    }
}

void plotSensorsTick() {
    // 4 или 5 минутный таймер
    if (testTimer(hourPlotTimerD, hourPlotTimer)) {
        for (byte i = 0; i < 14; i++) {
            tempHour[i] = tempHour[i + 1];
            humHour[i] = humHour[i + 1];
            pressHour[i] = pressHour[i + 1];
            //      rainHour[i] = rainHour[i + 1];
            altHour[i] = altHour[i + 1];
            co2Hour[i] = co2Hour[i + 1];
        }
        tempHour[14] = dispTemp;
        humHour[14] = dispHum;
        pressHour[14] = dispPres;
        //    rainHour[14] = dispRain;
        altHour[14] = dispAlt;
        co2Hour[14] = dispCO2;

        if (PRESSURE)
            pressHour[14] = dispRain;
        else
            pressHour[14] = dispPres;
    }

    // 1.5 или 2 часовой таймер
    if (testTimer(dayPlotTimerD, dayPlotTimer)) {
        long averTemp = 0, averHum = 0, averPress = 0, averAlt = 0,
             averCO2 = 0;  //, averRain = 0

        for (byte i = 0; i < 15; i++) {
            averTemp += tempHour[i];
            averHum += humHour[i];
            averPress += pressHour[i];
            //      averRain += rainHour[i];
            averAlt += altHour[i];
            averCO2 += co2Hour[i];
        }
        averTemp /= 15;
        averHum /= 15;
        averPress /= 15;
        //    averRain /= 15;
        averAlt /= 15;
        averCO2 /= 15;

        for (byte i = 0; i < 14; i++) {
            tempDay[i] = tempDay[i + 1];
            humDay[i] = humDay[i + 1];
            pressDay[i] = pressDay[i + 1];
            //      rainDay[i] = rainDay[i + 1];
            altDay[i] = altDay[i + 1];
            co2Day[i] = co2Day[i + 1];
        }
        tempDay[14] = averTemp;
        humDay[14] = averHum;
        pressDay[14] = averPress;
        //    rainDay[14] = averRain;
        altDay[14] = averAlt;
        co2Day[14] = averCO2;
    }

    // 10 минутный таймер
    if (testTimer(predictTimerD, predictTimer)) {
        // тут делаем линейную аппроксимацию для предсказания погоды
        long averPress = 0;
        for (byte i = 0; i < 10; i++) {
            bme.takeForcedMeasurement();
            averPress += bme.readPressure();
            delay(1);
        }
        averPress /= 10;

        for (byte i = 0; i < 5;
             i++) {  // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
            pressure_array[i] =
                pressure_array[i + 1];  // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ
                                        // ЯЧЕЙКИ на шаг назад
        }
        pressure_array[5] =
            averPress;  // последний элемент массива теперь - новое давление
        sumX = 0;
        sumY = 0;
        sumX2 = 0;
        sumXY = 0;
        for (int i = 0; i < 6; i++) {  // для всех элементов массива
            // sumX += time_array[i];
            sumX += i;
            sumY += (long)pressure_array[i];
            // sumX2 += time_array[i] * time_array[i];
            sumX2 += i * i;
            // sumXY += (long)time_array[i] * pressure_array[i];
            sumXY += (long)i * pressure_array[i];
        }
        a = 0;
        a = (long)6 * sumXY;  // расчёт коэффициента наклона приямой
        a = a - (long)sumX * sumY;
        a = (float)a / (6 * sumX2 - sumX * sumX);
        delta = a * 6;  // расчёт изменения давления
        dispRain =
            map(delta, -250, 250, 100,
                -100);  // пересчитать в проценты
                        // Serial.println(String(pressure_array[5]) + " " +
                        // String(delta) + " " + String(dispRain));   // дебаг
    }
}

boolean dotFlag;
void clockTick() {
    dotFlag = !dotFlag;
    if (dotFlag) {  // каждую секунду пересчёт времени
        secs++;
        if (secs > 59) {  // каждую минуту
            secs = 0;
            mins++;
            if (mins <= 59 && mode == 0) {
                drawSensors();  // (с)НР
            }
        }
        if (mins > 59) {  // каждый час
            // loadClock();        // Обновляем знаки, чтобы русские буквы в днях
            // недели тоже обновились. (с)НР
            now = rtc.now();
            secs = now.second();
            mins = now.minute();
            hrs = now.hour();
            if (mode == 0) drawSensors();
            if (hrs > 23) hrs = 0;
            if (mode == 0 && DISPLAY_TYPE) drawData();
        }
        if ((DISP_MODE != 0 && mode == 0) && DISPLAY_TYPE == 1 &&
            !bigDig) {  // Если режим секунд или дни недели по-русски, и 2-х
                        // строчные цифры то показывать секунды (с)НР
            lcd.setCursor(15, 1);
            if (secs < 10) lcd.print(" ");
            lcd.print(secs);
        }
    }

    if (mode == 0) {  // Точки и статус питания (с)НР
                      // ---------------------------------------------------

        byte code;
        if (dotFlag)
            code = 223;
        else
            code = 32;
        if (mode0scr == 0 &&
            (bigDig && DISPLAY_TYPE == 0 ||
             DISPLAY_TYPE == 1)) {  // мигание большими точками только в нулевом
                                    // режиме главного экрана (с)НР
            if (bigDig && DISPLAY_TYPE == 1)
                lcd.setCursor(7, 2);
            else
                lcd.setCursor(7, 0);
            lcd.write(code);
            lcd.setCursor(7, 1);
            lcd.write(code);
        } else {
#if (DISPLAY_TYPE == 1)
            if (code == 223) code = 58;
            lcd.setCursor(17, 3);
            lcd.write(code);
#endif
        }
    }

    if ((dispCO2 >= blinkLEDCO2 && LEDType == 0 ||
         dispHum <= blinkLEDHum && LEDType == 1 ||
         dispTemp >= blinkLEDTemp && LEDType == 2) &&
        !dotFlag)
        setLEDcolor(0);  // мигание индикатора в зависимости от значения и
                         // привязанного сенсора (с)НР
    else
        setLED();
}

void setup() {
    Serial.begin(9600);

    pinMode(BACKLIGHT, OUTPUT);
    pinMode(LED_COM, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    setLEDcolor(0);

    digitalWrite(LED_COM, LED_MODE);
    analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);

    if (EEPROM.read(0) ==
        122) {  // если было сохранение настроек, то восстанавливаем их (с)НР
        MAX_ONDATA = EEPROM.read(2);
        MAX_ONDATA += (long)(EEPROM.read(3) << 8);
        VIS_ONDATA = EEPROM.read(4);
        VIS_ONDATA += (long)(EEPROM.read(5) << 8);
        mode0scr = EEPROM.read(6);
        bigDig = EEPROM.read(7);
        LED_BRIGHT = EEPROM.read(8);
        LCD_BRIGHT = EEPROM.read(9);
        LEDType = EEPROM.read(10);
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();

#if (DEBUG == 1 && DISPLAY_TYPE == 1)
    boolean status = true;

    setLEDcolor(3);

#if (CO2_SENSOR == 1)
    lcd.setCursor(0, 0);
    lcd.print(F("MHZ-19... "));
    Serial.print(F("MHZ-19... "));
    mhz19.begin(MHZ_TX, MHZ_RX);
    mhz19.setAutoCalibration(false);
    mhz19.getStatus();  // первый запрос, в любом случае возвращает -1
    delay(500);
    if (mhz19.getStatus() == 0) {
        lcd.print(F("OK"));
        Serial.println(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        Serial.println(F("ERROR"));
        status = false;
    }
#endif

    setLEDcolor(3 + 12);
    lcd.setCursor(0, 1);
    lcd.print(F("RTC... "));
    Serial.print(F("RTC... "));
    delay(50);
    if (rtc.begin()) {
        lcd.print(F("OK"));
        Serial.println(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        Serial.println(F("ERROR"));
        status = false;
    }

    setLEDcolor(12);
    lcd.setCursor(0, 2);
    lcd.print(F("BME280... "));
    Serial.print(F("BME280... "));
    delay(50);
    if (bme.begin(&Wire)) {
        lcd.print(F("OK"));
        Serial.println(F("OK"));
    } else {
        lcd.print(F("ERROR"));
        Serial.println(F("ERROR"));
        status = false;
    }

    setLEDcolor(0);
    lcd.setCursor(0, 3);
    if (status) {
        lcd.print(F("All good"));
        Serial.println(F("All good"));
    } else {
        lcd.print(F("Check wires!"));
        Serial.println(F("Check wires!"));
    }

    for (byte i = 1; i < 20;
         i++) {  // убрал бесконечный цикл, сделал 5-ти секундное ожидание (с)НР
        lcd.setCursor(14, 1);
        lcd.print("P:    ");
        lcd.setCursor(16, 1);
        lcd.print(analogRead(PHOTO), 1);
        Serial.println(analogRead(PHOTO));
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

    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,  // temperature
                    Adafruit_BME280::SAMPLING_X1,  // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_OFF);

    if (RESET_CLOCK || rtc.lostPower())
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    lcd.clear();
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();

    bme.takeForcedMeasurement();
    uint32_t Pressure = bme.readPressure();
    for (byte i = 0; i < 6; i++) {     // счётчик от 0 до 5
        pressure_array[i] = Pressure;  // забить весь массив текущим давлением
                                       // time_array[i] = i;                    //
                                       // забить массив времени числами 0 - 5
    }

    dispAlt = (float)bme.readAltitude(SEALEVELPRESSURE_HPA);

    // заполняем графики текущим значением
    readSensors();
    for (byte i = 0; i < 15; i++) {  // счётчик от 0 до 14
        tempHour[i] = dispTemp;
        tempDay[i] = dispTemp;
        humHour[i] = dispHum;
        humDay[i] = dispHum;
        //    rainHour[i] = 0;
        //    rainDay[i] = 0;
        altHour[i] = dispAlt;
        altDay[i] = dispAlt;
        if (PRESSURE) {
            pressHour[i] = 0;
            pressDay[i] = 0;
        } else {
            pressHour[i] = dispPres;
            pressDay[i] = dispPres;
        }
    }

    if (DISPLAY_TYPE == 1) drawData();
    loadClock();
    // readSensors();
    drawSensors();
}

void loop() {
    if (testTimer(brightTimerD, brightTimer)) checkBrightness();  // яркость
    if (testTimer(sensorsTimerD, sensorsTimer))
        readSensors();  // читаем показания датчиков с периодом SENS_TIME
    Serial.println(dispTemp);

    if (testTimer(clockTimerD, clockTimer))
        clockTick();    // два раза в секунду пересчитываем время и мигаем точками
    plotSensorsTick();  // тут внутри несколько таймеров для пересчёта графиков
                        // (за час, за день и прогноз)
    modesTick();        // тут ловим нажатия на кнопку и переключаем режимы
    if (mode == 0) {    // в режиме "главного экрана"
        if (testTimer(drawSensorsTimerD, drawSensorsTimer))
            drawSensors();  // обновляем показания датчиков на дисплее с периодом
                            // SENS_TIME
    } else {                // в любом из графиков
        if (testTimer(plotTimerD, plotTimer))
            redrawPlot();  // перерисовываем график
    }
}
