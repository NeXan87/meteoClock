#pragma once

// Конфигурационный заголовок для проекта "Домашняя метеостанция"
// Все константы описаны комментариями и используют директиву #define,
// как требуется по задаче.

// Сброс часов при загрузке (1 - сбросить, 0 - не сбрасывать)
#define RESET_CLOCK 0  // при первом запуске сбросить, затем установить 0 и прошить снова

// Временные интервалы (в миллисекундах)
#define SENS_TIME 10000  // частота опроса датчиков

// Яркость и автоматическое управление
#define BRIGHT_CONTROL 1      // 0 - запретить, 1 - разрешить автоматическую регулировку
#define BRIGHT_THRESHOLD 350  // порог для фоторезистора (0-1023)
#define LED_BRIGHT_MAX 255    // максимальная яркость rgb-светодиода (0-255)
#define LED_BRIGHT_MIN 10     // минимальная яркость rgb-светодиода (0-255)
#define LCD_BRIGHT_MAX 255    // максимальная яркость подсветки LCD
#define LCD_BRIGHT_MIN 10     // минимальная яркость подсветки LCD

// Тип светодиода
#define LED_MODE 0  // 0 - общий катод, 1 - общий анод

// Отображение
#define DISP_MODE 1  // 0 - год, 1 - день недели, 2 - секунды
#define WEEK_LANG 1  // 0 - английский, 1 - русский
#define DEBUG 0      // вывод отладочных сообщений на экран

// Датчики и дисплей
#define CO2_SENSOR 1       // 1 - поддержка MH-Z19, 0 - без CO2
#define DISPLAY_TYPE 1     // 1 - LCD2004, 0 - LCD1602
#define DISPLAY_ADDR 0x27  // адрес I2C адаптера экрана

// Границы для индикации
#define normCO2 800       // нормальный CO2 (ppm)
#define maxCO2 1200       // высокий CO2
#define blinkLEDCO2 1500  // при превышении мигает

#define minTemp 21       // температура ниже этого - синий
#define normTemp 26      // температура выше этого - желтый
#define maxTemp 28       // температура выше этого - красный
#define blinkLEDTemp 35  // температура для мигания

#define maxHum 90       // влажность выше этого - синий
#define normHum 30      // влажность ниже этого - желтый
#define minHum 20       // влажность ниже этого - красный
#define blinkLEDHum 15  // влажность для мигания

#define normPress 733  // давление ниже - желтый
#define minPress 728   // давление ниже - красный (можно синее)

#define minRain -50   // прогноз дождя ниже - красный
#define normRain -20  // прогноз дождя ниже - желтый
#define maxRain 50    // прогноз дождя выше - синий

// Маски по умолчанию для графиков
#define DEFAULT_MAX_ONDATA 4095  // если EEPROM пуст, все графики масштабируются автоматически
#define DEFAULT_VIS_ONDATA 4095  // если EEPROM пуст, все графики отображаются

// Пределы графиков
#define TEMP_MIN 15
#define TEMP_MAX 35
#define HUM_MIN 0
#define HUM_MAX 100
#define PRESS_MIN 720
#define PRESS_MAX 760
#define CO2_MIN 400
#define CO2_MAX 2000
#define ALT_MIN 0
#define ALT_MAX 1000

// Адрес BME280 в библиотеке уже задан, если используете другой модуль
// не забудьте поменять в библиотеке.

// Аппаратные пины
#define BACKLIGHT 10
#define PHOTO A3

#define MHZ_RX 2
#define MHZ_TX 3

#define LED_COM 7
#define LED_R 9
#define LED_G 6
#define LED_B 5
#define BTN_PIN 4

// Высота над уровнем моря, необходимая для расчета давления
#define SEALEVELPRESSURE_HPA (1013.25)
