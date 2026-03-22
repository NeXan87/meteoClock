#pragma once

namespace RTC {

void init();
void tick();

int getHours();
int getMinutes();
int getSeconds();

bool isDotOn();

}  // namespace RTC
