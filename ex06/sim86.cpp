#include "../common.h"
#include "sim86.h"
#include "instTable.h"
#include "decode.h"
#include "disasm.h"
#include "execute.h"

#include "instTable.cpp"
#include "decode.cpp"
#include "disasm.cpp"
#include "execute.cpp"

#include <stdio.h>

#define MEMORY_SIZE (64 * 1024 * 1024)

u8 memory[MEMORY_SIZE];

void readMem(u8 *dst, sim_ptr src, u32 size) {
    for (u32 i = 0; i < size; i++) {
        dst[i] = memory[(src + i) % MEMORY_SIZE];
    }
}

void writeMem(sim_ptr dst, u8 *src, u32 size) {
    for (u32 i = 0; i < size; i++) {
        memory[(dst + i) % MEMORY_SIZE] = src[i];
    }
}

void loadProgram(const char *progFile) {
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

void handleFlags(InstrFlags *flags, Instr *instr) {
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

void parseArgs(char **path, bool *execute, int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: .\\sim8086.exe [--exec] program\n");
        exit(1);
    }

    if (argc == 2) {
        *path = argv[1];
        *execute = false;
        return;
    }

    if (argc == 3) {
        if (strcmp(argv[1], "--exec") == 0) {
            *execute = true;
            *path = argv[2];
            return;
        }
    }

    fprintf(stderr, "Usage: .\\sim8086.exe [--exec] program\n");
    exit(1);
}

int main(int argc, char **argv) {
    char *inputPath;
    bool execute;
    parseArgs(&inputPath, &execute, argc, argv);

    initStrArena();
    loadProgram(inputPath);
    loadInstrTable();

    if (execute) {
        executeProgram();
    } else {
        disasmProgram();
    }

    destroyStrArena();
}
