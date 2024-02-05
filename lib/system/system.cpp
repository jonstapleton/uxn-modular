#include <stdio.h>
#include "uxn.h"
#include "system.h"
#include "varvara.h"
#include "screen.h"

/*
Copyright (c) 2022 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

Uint8 system_dei (Device *d, Uint8 port) {
    switch(port) {
        case 0x2: return d->u->wst.ptr;
        case 0x3: return d->u->rst.ptr;
        default: return d->dat[port];
    }
}

void system_deo (Device *d, Uint8 port) {
    switch(port) {
        case 0x2: d->u->wst.ptr = d->dat[port]; break;
        case 0x3: d->u->rst.ptr = d->dat[port]; break;
        default: system_deo_special(d, port);
    }
}


