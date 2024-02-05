#include "uxn.h"
#include "console.h"
#include <Arduino.h>

/*
Copyright (c) 2022 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

void console_deo (Device *d, Uint8 port) {
    if (port == 0x8) {
        Serial.print((char)d->dat[port]);
    } else if (port == 0x9) {
        Serial.print((char)d->dat[port]);
    }
}
