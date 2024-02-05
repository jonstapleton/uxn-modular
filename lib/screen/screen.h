#include "uxn.h"
#include <stdlib.h>
#include <Arduino.h>

#define FIXED_SIZE 1
#define BG 0;
#define FG 1;

typedef struct Layer {
    uint8_t *pixels, changed;
} Layer;

typedef struct UxnScreen {
    Uint16 palette[4];
    Uint16 width, height;
    uint16_t *pixels;
    Layer fg, bg;
} UxnScreen;

void init_update();
void clear(uint16_t* fb, uint16_t color = 0);
void screen_init();
void screen_redraw(UxnScreen *p, Uint16 *pixels);
Uint8 screen_dei(Device *d, Uint8 port);
void screen_deo(Device *d, Uint8 port);
void screen_evaluate(Uxn *u, Device *d);

// https://github.com/max22-/uxn/blob/33ec3794386b484007c896cec6a55aa5f320a64a/src/devices/screen.h
void screen_palette(UxnScreen *p, Uint8 *addr);
void screen_resize(UxnScreen *p, Uint16 width, Uint16 height);
void screen_clear(UxnScreen *p, Layer *layer);
void screen_debug(UxnScreen *p, Uint8 *stack, Uint8 wptr, Uint8 rptr, Uint8 *memory);
void system_deo_special(Device *d, Uint8 port);