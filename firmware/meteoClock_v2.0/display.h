#pragma once

#include <Arduino.h>

// Внешние переменные (определены в globals.cpp)
extern float dispTemp;
extern byte  dispHum;
extern int   dispPres;
extern int   dispCO2;
extern int   dispRain;
extern float dispAlt;
extern int8_t hrs, mins, secs;
extern byte mode0scr;
extern boolean bigDig;
class DateTime;  // forward declaration
extern DateTime now;

// Функции отрисовки
void displayInitChars();
void displayLoadClock();
void displayLoadPlot();
void displayDrawData();
void displayDrawSensors();
void displayDrawClock(byte hours, byte minutes, byte x, byte y);
void displayDrawPPM(int co2, byte x, byte y);
void displayDrawTemp(float temp, byte x, byte y);
void displayDrawHum(byte hum, byte x, byte y);
void displayDrawPres(int pres, byte x, byte y);
void displayDrawAlt(float alt, byte x, byte y);
void displayDigSeg(byte x, byte y, byte z1, byte z2, byte z3, byte z4, byte z5, byte z6);
void displayDrawDig(byte dig, byte x, byte y);
void drawSensors();
void drawData();
void loadClock(); 
void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int* plot_array, const char* label1, const char* label2, int stretch)