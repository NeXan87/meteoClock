#pragma once

namespace Plot {

void init();

// обновление данных для графиков
void tick();

// доступ к массивам для отрисовки
int* tempHour();
int* tempDay();
int* humHour();
int* humDay();
int* pressHour();
int* pressDay();
int* altHour();
int* altDay();
int* co2Hour();
int* co2Day();

}  // namespace Plot
