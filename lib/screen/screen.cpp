#include "uxn.h"
#include <ILI9341_T4.h>
#include "screen.h"
#include <stdlib.h>

// define width & height of screen in portrait mode
#define S_WIDTH 240
#define S_HEIGHT 320

#define PIN_SCK     13      // mandatory
#define PIN_MISO    12      // mandatory (if the display has no MISO line, set this to 255 but then VSync will be disabled)
#define PIN_MOSI    11      // mandatory
#define PIN_DC      9      // mandatory, can be any pin but using pin 10 (or 36 or 37 on T4.1) provides greater performance

#define PIN_CS      10       // optional (but recommended), can be any pin.  
#define PIN_RESET   29       // optional (but recommended), can be any pin. 
#define PIN_BACKLIGHT 255   // optional, set this only if the screen LED pin is connected directly to the Teensy.
#define PIN_TOUCH_IRQ 255   // optional. set this only if the touchscreen is connected on the same SPI bus
#define PIN_TOUCH_CS  255   // optional. set this only if the touchscreen is connected on the same spi bus
ILI9341_T4::ILI9341Driver tft(PIN_CS, PIN_DC, PIN_SCK, PIN_MOSI, PIN_MISO, PIN_RESET, PIN_TOUCH_CS, PIN_TOUCH_IRQ);

// 2 diff buffers with about 6K memory each
ILI9341_T4::DiffBuffStatic<6000> diff1;
ILI9341_T4::DiffBuffStatic<6000> diff2;

// our framebuffers
uint16_t internal_fb[S_WIDTH * S_HEIGHT];     // used by the library for buffering
uint16_t fb[S_WIDTH * S_HEIGHT];              // the main framebuffer we draw onto.

UxnScreen uxn_screen;

static Uint8 blending[5][16] = {
    {0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
    {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
    {1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
    {2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2},
    {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0}};

void screen_init() {
    tft.output(&Serial);
    while(!tft.begin());

    // setup the display library 
    tft.setRotation(1); // landscape mode
    tft.setFramebuffer(internal_fb); // double buffering
    tft.setDiffBuffers(&diff1, &diff2); // 2 diff buffers
    tft.setDiffGap(4); // small gap
    tft.setRefreshRate(140); // refresh at 60hz
    tft.setVSyncSpacing(2);

    // set up uxn
    screen_resize(&uxn_screen, S_HEIGHT, S_WIDTH); // flipped because the display defaults to portrait mode
    uxn_screen.pixels = fb;
}

void init_update() {
    if (uxn_screen.fg.changed || uxn_screen.bg.changed) {
        screen_redraw(&uxn_screen, uxn_screen.pixels);
        // Serial.println("Got change!");
    }
}

void screen_evaluate(Uxn *u, Device *d) {
    uxn_eval(u, GETVECTOR(d));
    if (uxn_screen.fg.changed || uxn_screen.bg.changed) {
        screen_redraw(&uxn_screen, uxn_screen.pixels);
        Serial.println("Got change!");
    }
    // tft.overlayText(fb, "testing...", 3, 0, 12, ILI9341_T4_COLOR_WHITE, 1.0f, ILI9341_T4_COLOR_RED, 0.4f, 1); // draw text    
    tft.overlayFPS(fb);
    tft.update(fb);
}

/** fill a framebuffer with a given color*/
void clear(uint16_t* fb, uint16_t color = 0)
    {
    for (int i = 0; i < S_WIDTH * S_HEIGHT; i++) fb[i] = color;
    }

void screen_redraw(UxnScreen *p, Uint16 *pixels)
{
    // Serial.println("Redrawing screen...");
    // Serial.print("FG: ");
    // Serial.print(p->fg.changed);
    // Serial.print(" BG: ");
    // Serial.println(p->bg.changed);

    Uint32 size = p->width * p->height;
    Uint32 palette[16];
    for(int i=0;i<16;i++) {
        palette[i] = p->palette[(i >> 2) ? (i >> 2) : (i & 3)];
        // Serial.print(p->palette[i], HEX);
    }
    // Serial.println(palette[0], HEX);
    for(Uint32 i=0;i<size;i++) {
        // update pixels
        Uint32 temp = palette[p->fg.pixels[i] << 2 | p->bg.pixels[i]];
        // i%8==0?Serial.println(temp, HEX) : Serial.print(temp,HEX);
        pixels[i] = ((temp & 0xFF0000) >> 3) | ((temp & 0xFF00) >> 2) | ((temp & 0xFF) >> 3);
        // i%128==127?Serial.println(pixels[i], HEX) : Serial.print(pixels[i],HEX);
    }
    // set "changed" to 0
    p->fg.changed = p->bg.changed = 0;
}

void screen_resize(UxnScreen *p, Uint16 width, Uint16 height)
{
    Serial.println("Got call to resize screen!");
    Uint8
        *bg = (Uint8 *) realloc(p->bg.pixels, width * height),
        *fg = (Uint8 *) realloc(p->fg.pixels, width * height);
//    Uint16
//        *pixels = (Uint16 *) realloc(p->pixels, width * height * sizeof(Uint16));
        
    if(bg) p->bg.pixels = bg;
    if(fg) p->fg.pixels = fg;
//    if(pixels) p->pixels = pixels;
    if(bg && fg) {
        p->width = width;
        p->height = height;
        screen_clear(p, &p->bg);
        screen_clear(p, &p->fg);
    }
}

void screen_palette(UxnScreen *p, Uint8 *addr)
{
    int i, shift;
    for(i = 0, shift = 4; i < 4; ++i, shift ^= 4) {
        Uint8
            r = (addr[0 + i / 2] >> shift) & 0x0f,
              g = (addr[2 + i / 2] >> shift) & 0x0f,
              b = (addr[4 + i / 2] >> shift) & 0x0f;
        p->palette[i] = 0x0f000000 | r << 16 | g << 8 | b;
        p->palette[i] |= p->palette[i] << 4;
    }
    p->fg.changed = p->bg.changed = 1;
}

void screen_clear(UxnScreen *p, Layer *layer)
{
    Uint32 i, size = p->width * p->height;
    for(i = 0; i < size; i++)
        layer->pixels[i] = 0x00;
    layer->changed = 1;
}

static void screen_write(UxnScreen *p, Layer *layer, Uint16 x, Uint16 y, Uint8 color)
{
    if(x < p->width && y < p->height) {
        Uint32 i = x + y * p->width;
        if(color != layer->pixels[i]) {
            layer->pixels[i] = color;
            layer->changed = 1;
        }
    }
}

static void screen_blit(UxnScreen *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy, Uint8 twobpp)
{
    int v, h, opaque = blending[4][color];
    for(v = 0; v < 8; v++) {
        Uint16 c = sprite[v] | (twobpp ? sprite[v + 8] : 0) << 8;
        for(h = 7; h >= 0; --h, c >>= 1) {
            Uint8 ch = (c & 1) | ((c >> 7) & 2);
            if(opaque || ch)
                screen_write(p,
                        layer,
                        x + (flipx ? 7 - h : h),
                        y + (flipy ? 7 - v : v),
                        blending[ch][color]);
        }
    }
}

int clamp(int val, int min, int max)
{
    return (val >= min) ? (val <= max) ? val : max : min;
}

/* IO */

Uint8 screen_dei(Device *d, Uint8 port) {
    switch(port) {
        case 0x2: return uxn_screen.width >> 8;
        case 0x3: return uxn_screen.width;
        case 0x4: return uxn_screen.height >> 8;
        case 0x5: return uxn_screen.height;
        default: return d->dat[port];
    }
}

void screen_deo(Device *d, Uint8 port) {
    // Serial.print("Screen DEO: ");
    // Serial.println(port, HEX);
    switch(port) {
        case 0x3:
            if(!FIXED_SIZE) {
                Uint16 w;
                DEVPEEK16(w, 0x2);
                screen_resize(&uxn_screen, clamp(w, 1, 1024), uxn_screen.height);
            }
            break;
        case 0x5:
            if(!FIXED_SIZE) {
                Uint16 h;
                DEVPEEK16(h, 0x4);
                screen_resize(&uxn_screen, uxn_screen.width, clamp(h, 1, 1024));
            }
            break;
        case 0xe: {
                      Uint16 x, y;
                      Uint8 layer = d->dat[0xe] & 0x40;
                      DEVPEEK16(x, 0x8);
                      DEVPEEK16(y, 0xa);
                      screen_write(&uxn_screen, layer ? &uxn_screen.fg : &uxn_screen.bg, x, y, d->dat[0xe] & 0x3);
                      if(d->dat[0x6] & 0x01) DEVPOKE16(0x8, x + 1); /* auto x+1 */
                      if(d->dat[0x6] & 0x02) DEVPOKE16(0xa, y + 1); /* auto y+1 */
                      break;
                  }
        case 0xf: {
                      Uint16 x, y, addr;
                      Uint8 twobpp = !!(d->dat[0xf] & 0x80);
                      Layer *layer = (d->dat[0xf] & 0x40) ? &uxn_screen.fg : &uxn_screen.bg;
                      DEVPEEK16(x, 0x8);
                      DEVPEEK16(y, 0xa);
                      DEVPEEK16(addr, 0xc);
                      screen_blit(&uxn_screen, layer, x, y, &d->u->ram[addr], d->dat[0xf] & 0xf, d->dat[0xf] & 0x10, d->dat[0xf] & 0x20, twobpp);
                      if(d->dat[0x6] & 0x04) DEVPOKE16(0xc, addr + 8 + twobpp * 8); /* auto addr+length */
                      if(d->dat[0x6] & 0x01) DEVPOKE16(0x8, x + 8);                 /* auto x+8 */
                      if(d->dat[0x6] & 0x02) DEVPOKE16(0xa, y + 8);                 /* auto y+8 */
                      break;
                  }
    }
}

void system_deo_special (Device *d, Uint8 port) {
    if(port > 0x7 && port < 0xe)
        screen_palette(&uxn_screen, &d->dat[0x8]);
}