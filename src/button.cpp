#include "button.h"

#include "config.h"

static GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);

void updateButton() {
    btn.tick();
}

bool isSingleButton() {
    return btn.isSingle();
}

bool isDoubleButton() {
    return btn.isDouble();
}

bool isTripleButton() {
    return btn.isTriple();
}

bool isHoldedButton() {
    return btn.isHolded();
}
