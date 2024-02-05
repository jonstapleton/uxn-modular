#include "uxn.h"

void controller_setup(Device *d);
void controller_update(Device *d);
void OnRawPress(int keycode);
Uint8 getButtons(uint8_t pins[8]);