#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Укажите ваш I2C-адрес (обычно 0x27 или 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  // Выводим символы в 4 "страницы" по 80 штук (0–79, 80–159, 160–239, 240–255 + дозаполнение)
  for (int page = 0; page < 4; page++) {
    lcd.clear();
    int start = page * 80;
    int end = (page == 3) ? 256 : start + 80;

    int idx = start;
    for (int row = 0; row < 4; row++) {
      lcd.setCursor(0, row);
      for (int col = 0; col < 20; col++) {
        if (idx < 256) {
          lcd.write((uint8_t)idx);
          idx++;
        } else {
          lcd.print(' '); // заполнение пробелами в последней странице
        }
      }
    }

    // Пауза 3 секунды на каждую страницу
    delay(10000);
  }
}