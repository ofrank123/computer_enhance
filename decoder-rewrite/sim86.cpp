#include "../common.h"
#include "sim86.h"
#include "instTable.h"
#include "decode.h"
#include "print.h"

#include "instTable.cpp"
#include "decode.cpp"
#include "print.cpp"

#include <stdio.h>

#define MEMORY_SIZE (64 * 1024 * 1024)

u8 memory[MEMORY_SIZE];

/**
 * Read `size` bytes from simulation memory, into `dst`
 * starting at offset `src`
 */
void readMem(u8 *dst, sim_ptr src, u32 size) {
    for (u32 i = 0; i < size; i++) {
        dst[i] = memory[(src + i) % MEMORY_SIZE];
    }
}

/**
 * Write `size` bytes from `src` into simulation memory
 * at offset `src`
 */
void writeMem(sim_ptr dst, u8 *src, u32 size) {
    for (u32 i = 0; i < size; i++) {
        memory[(dst + i) % MEMORY_SIZE] = src[i];
    }
}

/**
 * Load program into simulation memory
 */
static void loadProgram(const char *progFile) {
    FILE *fp;
    long lSize;
    fp = fopen(progFile, "rb");
    assert(fp && "Failed to open file");

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    u8 progData[lSize];

    int res = fread(progData, lSize, 1, fp);
    assert(res == 1);

    writeMem(0, progData, lSize);
    u8 terminator = 0x0f;
    writeMem(lSize, &terminator, 1);
    fclose(fp);
}

/**
 * Update flags if instruction affects them, otherwise apply flags
 * to current instruction and clear them
 */
static void handleFlags(InstrFlags *flags, Instr *instr) {
    switch (instr->op) {
        case OP_REP:
            flags->rep = true;
            break;
        case OP_LOCK:
            flags->lock = true;
            break;
        case OP_SEGMENT:
            flags->segmentOverride = instr->dst.reg;
            break;
        default:
            if (instr->dst.type == ARG_MEM) {
                instr->dst.eac.segment = flags->segmentOverride;
            }
            if (instr->src.type == ARG_MEM) {
                instr->src.eac.segment = flags->segmentOverride;
            }
            *flags = {};
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: .\\sim8086.exe [program]\n");
        exit(1);
    }

    initStrArena();

    InstrDefTable defTable = getInstTable();

    loadProgram(argv[1]);

    u32 programOffset = 0;
    u8 nextByte = 0;
    Instr instr;
    InstrFlags flags;
    while (nextByte != 0x0f) {
        programOffset += decodeNextInstr(&instr, programOffset, defTable);
        handleFlags(&flags, &instr);
        printInstr(instr);
        readMem(&nextByte, programOffset, 1);
    }

    destroyStrArena();
}
