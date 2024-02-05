#include "USBHost_t36.h"
#include <EEPROM.h>
#include "uxn.h"
#include "Arduino.h"
#include "controller.h"

Uint8 pressed_buttons = 0x00;
uint8_t PIN_ASSIGNMENTS[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

USBHost myusb;
USBHub hub1(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
KeyboardController keyboard1(myusb);

static Device *devctrl;

void controller_setup(Device *d) {
    Serial.println("Setting up controller...");
    devctrl = d;

    for(int i=0;i<8;i++) {
      pinMode(PIN_ASSIGNMENTS[i], INPUT_PULLUP);
    }

    // Keyboard Setup
    myusb.begin();
    keyboard1.attachPress(OnRawPress);
}

void OnRawPress(int keycode) {
  // Serial.print("OnRawPress keycode: ");
  // Serial.println(keycode, HEX);
  devctrl->dat[3] = keycode;
  uxn_eval(devctrl->u, GETVECTOR(devctrl));
}

void controller_update (Device *d) {
  myusb.Task(); // Poll for USB activity & run attached tasks

  // check buttons
  Uint8 new_buttons = getButtons(PIN_ASSIGNMENTS);
  // only evaluate if there has been a change
  if(new_buttons != pressed_buttons) {
    // Serial.println(new_buttons);
    d->dat[2] = new_buttons;
    uxn_eval(d->u, GETVECTOR(d));
    pressed_buttons = new_buttons;
  }
}

Uint8 getButtons(uint8_t pins[8]) {
  Uint8 btns = 0x00;
  for(int i=7;i>=0;i--) {
    btns += !digitalRead(pins[i]);
    btns = btns << 1;
  }
  return btns;
}