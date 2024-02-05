#include <Arduino.h>
#include "uxn.h"

#ifndef VARVARA
#define VARVARA

#include "uxn.h"

void init(Uxn *u, char *rom);
void evaluate(Uxn *u);
int uxn_start(Uxn *u);
int uxn_halt(Uxn *u, Uint8 error, Uint16 addr);
void uxn_load(Uxn *u, char *rom_name);
int uxn_error(String msg, String err);
int uxn_interrupt(Uxn *u);

#endif