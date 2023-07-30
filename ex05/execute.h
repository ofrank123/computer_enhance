#pragma once

#include "../common.h"
#include "sim86.h"

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

    u16 flags;
};

enum Flags {
    FLAG_CF = 1,
    FLAG_PF = (1 << 3),
    FLAG_AF = (1 << 5),
    FLAG_ZF = (1 << 7),
    FLAG_SF = (1 << 8),
    FLAG_TF = (1 << 9),
    FLAG_IF = (1 << 10),
    FLAG_DF = (1 << 11),
    FLAG_OF = (1 << 12),
};

/**
 * Read the current value of a given register
 */
u16 readReg(Register reg);

/**
 * Execute loaded program
 */
void executeProgram();
