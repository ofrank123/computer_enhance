#pragma once

#include "../common.h"

struct RegisterFile {
    u8 a[2];
    u8 c[2];
    u8 d[2];
    u8 b[2];
    u16 sp;
    u16 bp;
    u16 si;
    u16 di;
    u16 es;
    u16 cs;
    u16 ss;
    u16 ds;
};

/**
 * Execute loaded program
 */
void executeProgram();
