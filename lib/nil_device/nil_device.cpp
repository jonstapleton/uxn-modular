#include "uxn.h"

Uint8 nil_dei(Device *d, Uint8 port)
{
    return d->dat[port];
}

void nil_deo(Device *d, Uint8 port)
{
    (void)d;
    (void)port;
}