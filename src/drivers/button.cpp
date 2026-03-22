#include "drivers/button.h"

#include <Arduino.h>
#include <GyverButton.h>

namespace Button {

static GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);

void tick() {
    button.tick();
}

bool isSingle() { return button.isSingle(); }
bool isDouble() { return button.isDouble(); }
bool isTriple() { return button.isTriple(); }
bool isHolded() { return button.isHolded(); }

}  // namespace Button
