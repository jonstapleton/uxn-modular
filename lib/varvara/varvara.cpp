#include "../uxn/uxn.h"
#include <Arduino.h>
#include <SD.h>
#include "system.h"
#include "console.h"
#include "nil_device.h"
#include "controller.h"
#include "./varvara.h"
#include "screen.h"

Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;
const int chipSelect = BUILTIN_SDCARD;

static Device *devsys, *devconsole, *devctrl, *devscreen;

void init(Uxn *u, char *rom) {
    uxn_start(u);
    // spin up controller driver
    controller_setup(devctrl);

    // initialize screen
    screen_init();

    Serial.println("uxn started...");

    uxn_load(u, rom);
    uxn_eval(u, PAGE_PROGRAM);
    init_update(); // screen
}

void evaluate(Uxn *u) {
    controller_update(devctrl); // evaluate controller inputs
    
    // TODO: Screen stuff
    screen_evaluate(u, devscreen);
}

int uxn_start(Uxn *u)
{
    Serial.println("uxn starting");

    if(!uxn_boot(u, (Uint8 *)calloc(0x10000, sizeof(Uint8))))
        return uxn_error("Boot", "Failed");
    /* system   */ devsys = uxn_port(u, 0x0, system_dei, system_deo);       // ✅
    /* console  */ devconsole = uxn_port(u, 0x1, nil_dei, console_deo);     // ✅
    // TODO: the screen isn't calling the DEI or DEO functions
    /* screen   */ devscreen = uxn_port(u, 0x2, screen_dei, screen_deo);    // ✅
    /* empty    */ uxn_port(u, 0x3, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0x4, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0x5, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0x6, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0x7, nil_dei, nil_deo);
    /* ctrlr    */ devctrl = uxn_port(u, 0x8, nil_dei, nil_deo);            // ✅
    /* empty    */ uxn_port(u, 0x9, nil_dei, nil_deo);
    /* file     */ uxn_port(u, 0xa, nil_dei, nil_deo); // file_deo);
    /* datetime */ uxn_port(u, 0xb, nil_dei, nil_deo); // datetime_dei, nil_deo);
    /* onboardio*/ uxn_port(u, 0xc, nil_dei, nil_deo); // onboardio_deo);
    /* empty    */ uxn_port(u, 0xd, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0xe, nil_dei, nil_deo);
    /* empty    */ uxn_port(u, 0xf, nil_dei, nil_deo);
    return 1;
}

/* uxn stuff */
int uxn_error(String msg, String err)
{
    Serial.println("uxn error");
    Serial.println(msg);
    Serial.println(err);
    while (1);
}

int uxn_interrupt(Uxn *u)
{
    (void)u;
    return 0;
}

int uxn_halt(Uxn *u, Uint8 error, Uint16 addr)
{
    Device *d = &u->dev[0];
    Uint16 vec = GETVECTOR(d);
    DEVPOKE16(0x4, addr);
    d->dat[0x6] = error;
    if(vec) {
        /* need to rearm to run System/vector again */
        d->dat[0] = 0;
        d->dat[1] = 0;
        if(error != 2) /* working stack overflow has special treatment */
            Serial.println("Working stack overflow!");
            vec += 0x0004;
        return uxn_eval(u, vec);
    }
    Serial.print("uxn halted. error code: ");
    Serial.println(error);
    return 0;
}

void uxn_load(Uxn *u, char *rom_name)
{
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  Serial.println("loading rom...");

    // File readFile = arcada.open(rom_name, FILE_READ);
    File readFile = SD.open(rom_name, FILE_READ);
    if (!readFile) {
        Serial.println("Error, failed to open ROM for reading!");
        while(1);
    }
    int r_size = readFile.size();
    Serial.printf("rom size: %d bytes\n", r_size);
    Uint8 rom[r_size];

    // chance to optimization? get rid of this rom variable
    for (int i = 0; i < r_size; i++) {
        rom[i] = (Uint8)readFile.read();
    }

    memcpy(u->ram + PAGE_PROGRAM, rom, r_size);
    // Serial.println(u->ram);
    Serial.println("rom loaded");
}


