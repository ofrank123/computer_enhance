#pragma once

#include "common.h"
#include "sim86.h"
#include <vector>

enum InstrPartType {
    IP_BITS,
    IP_D,
    IP_W,
    IP_S,
    IP_V,
    IP_Z,
    IP_MOD,
    IP_REG,
    IP_RM,
    IP_ADDR,
    IP_DISP,
    IP_SR,
    IP_DATA,
    IP_DATA_IF_W,
    IP_IMP_D,
    IP_IMP_W,
    IP_IMP_REG,
    IP_IMP_MOD,
    IP_IMP_RM,
    IP_F_RM_REG_WIDE,
};

struct InstrPart {
    InstrPartType type;

    union {
        struct { u8 data; u8 size; } bits;
        u8 impD;
        u8 impW;
        u8 impReg;
        u8 impMod;
        u8 impRm;
    };
};

struct InstrDef {
    Op op;
    std::vector<InstrPart> parts;
};

typedef std::vector<InstrDef> InstrDefTable;

/**
 * Parse the instruction table in `./8086_inst_table.h`, and return
 * the table after loading.
 */
InstrDefTable getInstTable();
