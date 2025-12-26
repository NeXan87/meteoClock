#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Укажите адрес I2C вашего LCD (обычно 0x27 или 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);  // 20 символов, 4 строки

RTC_DS3231 rtc;

// Раскомментируйте эту строку ТОЛЬКО при первой загрузке, чтобы установить время!
// После загрузки — закомментируйте снова.
// #define SET_TIME_ONCE

void setup() {
  Serial.begin(9600);
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.print("Init RTC...");
  delay(1000);

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("No RTC found!");
    while (1);
  }

#ifdef SET_TIME_ONCE
  // Установка времени с компилятора (__DATE__, __TIME__)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  lcd.clear();
  lcd.print("Time set!");
  delay(1500);
#endif

  if (rtc.lostPower()) {
    // Это срабатывает, если RTC потерял питание (ионистор/батарейка села)
    // Но если вы уже установили время выше — это не нужно
  }
}

void loop() {
  DateTime now = rtc.now();

  lcd.clear();
  // Формат: ГГГГ-ММ-ДД
  lcd.setCursor(0, 0);
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);

  // Формат: День недели
  lcd.setCursor(0, 1);
  lcd.print(rtc.now().dayOfTheWeek() == 0 ? "Sunday" :
             rtc.now().dayOfTheWeek() == 1 ? "Monday" :
             rtc.now().dayOfTheWeek() == 2 ? "Tuesday" :
             rtc.now().dayOfTheWeek() == 3 ? "Wednesday" :
             rtc.now().dayOfTheWeek() == 4 ? "Thursday" :
             rtc.now().dayOfTheWeek() == 5 ? "Friday" : "Saturday");

  // Формат: ЧЧ:ММ:СС
  lcd.setCursor(0, 2);
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second(), DEC);

  // Температура (DS3231 имеет встроенный датчик)
  lcd.setCursor(0, 3);
  lcd.print("Temp: ");
  lcd.print(rtc.getTemperature());
  lcd.print(" C");

  delay(1000);
}