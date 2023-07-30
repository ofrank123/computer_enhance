#include "execute.h"

#include "sim86.h"
#include "instTable.h"
#include "disasm.h"
#include "decode.h"

RegisterFile regFile;
u16 readReg(Register reg) {
    switch (reg) {
        case REG_AL: return regFile.a[0];
        case REG_AH: return regFile.a[1];
        case REG_AX:
            return (((u16) regFile.a[1]) << 8) + regFile.a[0];
        case REG_CL: return regFile.c[0];
        case REG_CH: return regFile.c[1];
        case REG_CX:
            return (((u16) regFile.c[1]) << 8) + regFile.c[0];
        case REG_DL: return regFile.d[0];
        case REG_DH: return regFile.d[1];
        case REG_DX:
            return (((u16) regFile.d[1]) << 8) + regFile.d[0];
        case REG_BL: return regFile.b[0];
        case REG_BH: return regFile.b[1];
        case REG_BX:
            return (((u16) regFile.b[1]) << 8) + regFile.b[0];
        case REG_SP: return regFile.sp;
        case REG_BP: return regFile.bp;
        case REG_SI: return regFile.si;
        case REG_DI: return regFile.di;
        case REG_ES: return regFile.es;
        case REG_CS: return regFile.cs;
        case REG_SS: return regFile.ss;
        case REG_DS: return regFile.ds;
        case REG_NONE:
            PANIC("Cannot read REG_NONE");
    }
}

void writeReg(Register reg, u16 data) {
    switch (reg) {
        case REG_AL:
            regFile.a[0] = data;
            break;
        case REG_AH:
            regFile.a[1] = data;
            break;
        case REG_AX:
            regFile.a[0] = data & 0x00ff;
            regFile.a[1] = data >> 8;
            break;
        case REG_CL:
            regFile.c[0] = data;
            break;
        case REG_CH:
            regFile.c[1] = data;
            break;
        case REG_CX:
            regFile.c[0] = data & 0x00ff;
            regFile.c[1] = data >> 8;
            break;
        case REG_DL:
            regFile.d[0] = data;
            break;
        case REG_DH:
            regFile.d[1] = data;
            break;
        case REG_DX:
            regFile.d[0] = data & 0x00ff;
            regFile.d[1] = data >> 8;
            break;
        case REG_BL:
            regFile.b[0] = data;
            break;
        case REG_BH:
            regFile.b[1] = data;
            break;
        case REG_BX:
            regFile.b[0] = data & 0x00ff;
            regFile.b[1] = data >> 8;
            break;
        case REG_SP:
            regFile.sp = data;
            break;
        case REG_BP:
            regFile.bp = data;
            break;
        case REG_SI:
            regFile.si = data;
            break;
        case REG_DI:
            regFile.di = data;
            break;
        case REG_ES:
            regFile.es = data;
            break;
        case REG_CS:
            regFile.cs = data;
            break;
        case REG_SS:
            regFile.ss = data;
            break;
        case REG_DS:
            regFile.ds = data;
            break;
        case REG_NONE:
            PANIC("Cannot write to REG_NONE!");
    }
}

u32 readArg(Arg arg) {
    switch (arg.type) {
        case ARG_IMM:
            return arg.imm;
        case ARG_REG:
            return readReg(arg.reg);
        default:
            PANIC("Reading unsupported arg type!");
    }
}

void writeArg(Arg arg, u16 value) {
    switch (arg.type) {
        case ARG_IMM:
            PANIC("Cannot write to immediate value");
        case ARG_REG:
            printf("%s:0x%X->0x%x", getRegStr(arg.reg), readReg(arg.reg), value);
            writeReg(arg.reg, value);
            break;
        default:
            PANIC("Unsupported arg type!");
    }
}

void printRegFile() {
    printf("\nFinal Register File:\n\n");
    printf("\tax: 0x%04X (%d)\n", readReg(REG_AX), readReg(REG_AX));
    printf("\tbx: 0x%04X (%d)\n", readReg(REG_BX), readReg(REG_BX));
    printf("\tcx: 0x%04X (%d)\n", readReg(REG_CX), readReg(REG_CX));
    printf("\tdx: 0x%04X (%d)\n", readReg(REG_DX), readReg(REG_DX));
    printf("\tsp: 0x%04X (%d)\n", readReg(REG_SP), readReg(REG_SP));
    printf("\tbp: 0x%04X (%d)\n", readReg(REG_BP), readReg(REG_BP));
    printf("\tsi: 0x%04X (%d)\n", readReg(REG_SI), readReg(REG_SI));
    printf("\tdi: 0x%04X (%d)\n", readReg(REG_DI), readReg(REG_DI));
    printf("\tes: 0x%04X (%d)\n", readReg(REG_ES), readReg(REG_ES));
    printf("\tcs: 0x%04X (%d)\n", readReg(REG_CS), readReg(REG_CS));
    printf("\tss: 0x%04X (%d)\n", readReg(REG_SS), readReg(REG_SS));
    printf("\tds: 0x%04X (%d)\n", readReg(REG_DS), readReg(REG_DS));
}

void executeInstr(Instr instr) {
    switch (instr.op) {
        case OP_MOV:
            writeArg(instr.dst, readArg(instr.src));
            break;
        default:
            PANIC("Unsupported operation: %s", OP_STRINGS[instr.op]);
    }
}

void executeProgram() {
    u32 programOffset = 0;
    u8 nextByte = 0;
    Instr instr;
    InstrFlags flags;
    while (nextByte != 0x0f) {
        programOffset += decodeNextInstr(&instr, programOffset);
        handleFlags(&flags, &instr);
        printf("%s ; ", getInstrStr(instr));
        executeInstr(instr);
        printf("\n");
        readMem(&nextByte, programOffset, 1);
    }

    printRegFile();
}

