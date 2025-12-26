#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Настройки — подставьте СВОИ!
#define DISPLAY_ADDR 0x27   // или 0x3F
#define DISPLAY_COLS 20     // 16 для 1602, 20 для 2004
#define DISPLAY_ROWS 4      // 2 для 1602, 4 для 2004

LiquidCrystal_I2C lcd(DISPLAY_ADDR, DISPLAY_COLS, DISPLAY_ROWS);

unsigned long lastUpdate = 0;
const unsigned long delayMs = 1500; // задержка между символами
byte charIndex = 0;

void setup() {
  pinMode(10, OUTPUT);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  Serial.println("Тест символов LCD. Используйте Serial Monitor для отслеживания.");
digitalWrite(10, HIGH); 
}

void loop() {
  if (millis() - lastUpdate > delayMs) {
    lcd.clear();

    // Выводим номер символа
    lcd.setCursor(0, 0);
    lcd.print("Cod: ");
    if (charIndex < 10) lcd.print("0");
    if (charIndex < 100) lcd.print("0");
    lcd.print(charIndex);

    // Выводим сам символ (осторожно: 0 = пустой!)
    lcd.setCursor(0, 1);
    if (charIndex == 0) {
      lcd.print("[NUL]");
    } else {
      lcd.write(charIndex); // выводим байт как символ
    }

    // Дополнительно: для 2004 можно вывести больше контекста
    if (DISPLAY_ROWS >= 4) {
      lcd.setCursor(0, 2);
      lcd.print("Hex: 0x");
      if (charIndex < 16) lcd.print("0");
      lcd.print(charIndex, HEX);

      lcd.setCursor(0, 3);
      lcd.print("Next: ");
      lcd.print((charIndex + 1) % 256);
    }

    Serial.print("Символ ");
    Serial.print(charIndex);
    Serial.print(" (0x");
    Serial.print(charIndex, HEX);
    Serial.println(")");

    charIndex = (charIndex + 1) % 256;
    lastUpdate = millis();
  }
}