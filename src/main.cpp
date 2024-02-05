#include <SD.h>
#include <SPI.h>

#include "uxn.h"
#include "varvara.h"

#define S_WIDTH 320
#define S_HEIGHT 240

static Uxn u;                    // pointer to the uxn framebuffer
static char *rom = "hello-pixels.rom";

void setup() {
    Serial.begin(9600);
    while (!Serial && millis() < 2000);

    pinMode(13, OUTPUT);
    Serial.println("Hello! uxnuro");

    /* uxn start */
    Serial.println("starting uxn...");
    init(&u, rom);
}

void loop() {
  delay(12);  // add some delay so our screen doesnt flicker
  evaluate(&u); // pass device evaluation tasks to varvara
}
