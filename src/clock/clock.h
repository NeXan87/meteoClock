#pragma once

namespace Clock {

void init();
void tick();

int getHours();
int getMinutes();
int getSeconds();

bool isDotOn();

}  // namespace Clock
