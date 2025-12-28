#pragma once

#include <Arduino.h>

// Внешние переменные сенсоров (определены в sensors.cpp)
extern float dispTemp;
extern byte  dispHum;
extern int   dispPres;
extern int   dispCO2;
extern int   dispRain;
extern float dispAlt;

// Прогноз погоды
extern int delta;
extern uint32_t pressure_array[6];

// Функции
void sensorsInit();
void sensorsRead();
void sensorsUpdatePlots();
void sensorsUpdatePrediction();