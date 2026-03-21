#pragma once

// Конфигурационный заголовок для проекта "Домашняя метеостанция"
// Все константы описаны комментариями и используют директиву #define,
// как требуется по задаче.

// Сброс часов при загрузке (1 - сбросить, 0 - не сбрасывать)
#define RESET_CLOCK 0  // при первом запуске сбросить, затем установить 0 и прошить снова

// Временные интервалы (в миллисекундах)
#define SENS_TIME 10000  // частота опроса датчиков
// Интервал тика часов (мс) — используется для мигания точек и подсчёта секунд
#define CLOCK_TICK_INTERVAL_MS 500

// Яркость и автоматическое управление
#define BRIGHT_CONTROL 1      // 0 - запретить, 1 - разрешить автоматическую регулировку
#define BRIGHT_THRESHOLD 350  // порог для фоторезистора (0-1023)
#define BRIGHT_HYSTERESYS 50  // гистерезис для автоматической яркости
#define LED_BRIGHT_MAX 10     // максимальная яркость rgb-светодиода (0-255)
#define LED_BRIGHT_MIN 0      // минимальная яркость rgb-светодиода (0-255)
#define LCD_BRIGHT_MAX 255    // максимальная яркость подсветки LCD
#define LCD_BRIGHT_MIN 10     // минимальная яркость подсветки LCD

// Тип светодиода
#define LED_MODE 0  // 0 - общий катод, 1 - общий анод

// Отображение
#define DISP_MODE 1  // 0 - год, 1 - день недели, 2 - секунды
#define DEBUG 0      // вывод отладочных сообщений на экран

// Датчики и дисплей
#define CO2_SENSOR 1        // 1 - поддержка MH-Z19, 0 - без CO2
#define TEMP_OFFSET (-5.0)  // поправка на температуру (реальная = измеренная + TEMP_OFFSET)
// дисплей жёстко выбирается LCD2004
#define DISPLAY_ADDR 0x27  // адрес I2C адаптера экрана

// Границы для индикации
#define NORM_CO2 800        // нормальный CO2 (ppm)
#define MAX_CO2 1200        // высокий CO2
#define BLINK_LED_CO2 1500  // при превышении мигает

#define MIN_TEMP 21        // температура ниже этого - синий
#define NORM_TEMP 26       // температура выше этого - желтый
#define MAX_TEMP 28        // температура выше этого - красный
#define BLINK_LED_TEMP 35  // температура для мигания

#define MAX_HUM 90        // влажность выше этого - синий
#define NORM_HUM 30       // влажность ниже этого - желтый
#define MIN_HUM 20        // влажность ниже этого - красный
#define BLINK_LED_HUM 15  // влажность для мигания

#define NORM_PRESS 733  // давление ниже - желтый
#define MIN_PRESS 728   // давление ниже - красный (можно синее)

#define MIN_RAIN -50   // прогноз дождя ниже - красный
#define NORM_RAIN -20  // прогноз дождя ниже - желтый
#define MAX_RAIN 50    // прогноз дождя выше - синий

// Маски по умолчанию для графиков
#define DEFAULT_MAX_ONDATA 4095  // если EEPROM пуст, все графики масштабируются автоматически
#define DEFAULT_VIS_ONDATA 4095  // если EEPROM пуст, все графики отображаются

// Пределы графиков
#define TEMP_MIN 15    // минимальное значение шкалы температуры (°C)
#define TEMP_MAX 35    // максимальное значение шкалы температуры (°C)
#define HUM_MIN 0      // минимальное значение шкалы влажности (%)
#define HUM_MAX 100    // максимальное значение шкалы влажности (%)
#define PRESS_MIN 720  // минимальное значение шкалы давления (мм рт.ст. или используемая единица)
#define PRESS_MAX 760  // максимальное значение шкалы давления
#define CO2_MIN 400    // минимальное значение шкалы CO2 (ppm)
#define CO2_MAX 2000   // максимальное значение шкалы CO2 (ppm)
#define ALT_MIN 0      // минимальное значение шкалы высоты (м)
#define ALT_MAX 1000   // максимальное значение шкалы высоты (м)

// Адрес BME280 в библиотеке уже задан, если используете другой модуль
// не забудьте поменять в библиотеке.

// Аппаратные пины
#define BACKLIGHT 10  // пин управления подсветкой LCD (может быть PWM)
#define PHOTO A3      // аналоговый вход фоторезистора (A3)

#define MHZ_RX 2  // RX для MH-Z19 (SoftwareSerial)
#define MHZ_TX 3  // TX для MH-Z19 (SoftwareSerial)

#define LED_COM 7  // общий вывод RGB (анод/катод в зависимости от LED_MODE)
#define LED_R 9    // PWM-пин канала красного
#define LED_G 6    // PWM-пин канала зелёного
#define LED_B 5    // PWM-пин канала синего
#define BTN_PIN 4  // пин кнопки

// Стандартное атмосферное давление на уровне моря (в гПа), используется для расчёта
#define SEALEVELPRESSURE_HPA (1013.25)

// Разметка EEPROM
#define EEPROM_MAGIC_ADDR 0         // адрес ячейки с магическим маркером настроек
#define EEPROM_MAGIC_VALUE 122      // значение-маркер, подтверждающее сохранённые настройки
#define EEPROM_MAX_ONDATA_ADDR 2    // адрес младшего байта MAX_ONDATA в EEPROM
#define EEPROM_VIS_ONDATA_ADDR 4    // адрес младшего байта VIS_ONDATA в EEPROM
#define EEPROM_MAIN_DISPLAY_ADDR 6  // адрес байта, где хранится MainDisplayMode
#define EEPROM_BIGDIG_ADDR 7        // адрес байта, где хранится флаг больших цифр (0/1)
#define EEPROM_LED_BRIGHT_ADDR 8    // адрес байта, где хранится яркость LED (0-10, 11=auto)
#define EEPROM_LCD_BRIGHT_ADDR 9    // адрес байта, где хранится яркость LCD (0-10, 11=auto)
#define EEPROM_LED_TYPE_ADDR 10     // адрес байта, где хранится режим привязки LED

// Параметры LCD
#define LCD_COLS 20          // количество столбцов LCD
#define LCD_ROWS 4           // количество строк LCD
#define LCD_PLOT_COLUMN 15   // колонка, где рисуется указатель/правая область графика
#define LCD_VALUE_COLUMN 16  // колонка, где выводятся числовые значения справа

// Константы графиков
#define PLOT_SAMPLES 15                                                     // число точек в одном наборе графика (часовой/дневной)
#define PLOT_WIDTH 15                                                       // ширина графика в символах
#define PLOT_HEIGHT 4                                                       // высота графика в символах
#define PLOT_HOUR_INTERVAL_MS (4UL * 60UL * 1000UL)                         // интервал обновления часового графика (мс)
#define PLOT_DAY_INTERVAL_MS ((unsigned long)(1.6 * 60.0 * 60.0 * 1000.0))  // интервал обновления суточного графика (мс)
#define PREDICT_INTERVAL_MS (10UL * 60UL * 1000UL)                          // интервал предсказания давления (мс)
#define PREDICT_SAMPLE_COUNT 10                                             // число считываний для усреднения при предсказании
#define PREDICT_READ_DELAY_MS 1                                             // задержка между быстрыми чтениями для предсказания (мс)
#define PRESSURE_SAMPLE_COUNT 6                                             // количество значений давления для регрессии

// Предсказание и сопоставление дождя
#define PRESSURE_DELTA_MIN -250  // минимальный дельта-давления для map()
#define PRESSURE_DELTA_MAX 250   // максимальный дельта-давления для map()
#define RAIN_MAP_OUT_MIN 100     // верхняя граница выходного процента дождя
#define RAIN_MAP_OUT_MAX -100    // нижняя граница выходного процента дождя

// Константы LED
#define LED_BRIGHT_AUTO 11         // значение яркости, означающее "авто"
#define LED_PRESET_MAX 11          // максимальный индекс пресета яркости
#define LED_BIND_MODE_COUNT 4      // число режимов привязки LED к датчикам
#define LED_BLINK_INTERVAL_MS 500  // интервал мигания индикатора (мс)

// Последовательный порт
#define SERIAL_BAUD 115200  // скорость последовательного порта

#define MENU_PODMODE_MAX 15  // максимальное значение podMode для меню

// Границы карты графиков (по умолчанию заданы, повторно экспортируются)
#define DEFAULT_PLOT_WIDTH PLOT_WIDTH  // переопределение для совместимости
