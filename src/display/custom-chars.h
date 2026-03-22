#pragma once
#include <Arduino.h>

struct RUS_CHARS {
    // Заглавные буквы
    const byte B = 160;    // Б
    const byte G = 161;    // Г
    const byte D = 224;    // Д
    const byte YO = 162;   // Ё
    const byte ZH = 163;   // Ж
    const byte Z = 164;    // З
    const byte I = 165;    // И
    const byte J = 166;    // Й
    const byte L = 167;    // Л
    const byte P = 168;    // П
    const byte U = 169;    // У
    const byte F = 170;    // Ф
    const byte TS = 225;   // Ц
    const byte CH = 171;   // Ч
    const byte SH = 172;   // Ш
    const byte SHH = 226;  // Щ
    const byte SLD = 173;  // Ъ
    const byte Y = 174;    // Ы
    const byte E = 175;    // Э
    const byte YU = 176;   // Ю
    const byte YA = 177;   // Я

    // Строчные буквы
    const byte b = 178;    // б
    const byte v = 179;    // в
    const byte g = 180;    // г
    const byte d = 227;    // д
    const byte yo = 181;   // ё
    const byte zh = 182;   // ж
    const byte z = 183;    // з
    const byte i = 184;    // и
    const byte j = 185;    // й
    const byte k = 186;    // к
    const byte l = 187;    // л
    const byte m = 188;    // м
    const byte n = 189;    // н
    const byte p = 190;    // п
    const byte t = 191;    // т
    const byte f = 228;    // ф
    const byte ts = 229;   // ц
    const byte ch = 192;   // ч
    const byte sh = 193;   // ш
    const byte shh = 230;  // щ
    const byte sld = 194;  // ъ
    const byte y = 195;    // ы
    const byte sft = 196;  // ь
    const byte e = 197;    // э
    const byte yu = 198;   // ю
    const byte ya = 199;   // я
};

extern RUS_CHARS RUS;
