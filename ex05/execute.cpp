#include "execute.h"

#include "sim86.h"
#include "instTable.h"
#include "disasm.h"
#include "decode.h"

RegisterFile regFile;

char *getFlagsStr() {
    char *ret = allocStr(10);
    u8 offset = 0;
    if (regFile.flags & FLAG_CF) ret[offset++] = 'C';
    if (regFile.flags & FLAG_PF) ret[offset++] = 'P';
    if (regFile.flags & FLAG_AF) ret[offset++] = 'A';
    if (regFile.flags & FLAG_ZF) ret[offset++] = 'Z';
    if (regFile.flags & FLAG_SF) ret[offset++] = 'S';
    if (regFile.flags & FLAG_TF) ret[offset++] = 'T';
    if (regFile.flags & FLAG_IF) ret[offset++] = 'I';
    if (regFile.flags & FLAG_DF) ret[offset++] = 'D';
    if (regFile.flags & FLAG_OF) ret[offset++] = 'O';
    ret[offset] = 0;
    return ret;
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

    printf("flags: ");
    printf("%s", getFlagsStr());
    printf("\n");
}

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
            printf("%s:0x%04X->0x%04X", getRegStr(arg.reg), readReg(arg.reg), value);
            writeReg(arg.reg, value);
            break;
        default:
            PANIC("Unsupported arg type!");
    }
}

void setOverflowFlag(i32 overflowCheck, bool wide) {
    if (wide) {
        if (overflowCheck > INT16_MAX || overflowCheck < INT16_MIN) {
            regFile.flags |= FLAG_OF;
        } else {
            regFile.flags &= ~FLAG_OF;
        }
    } else {
        if (overflowCheck > INT8_MAX || overflowCheck < INT8_MIN) {
            regFile.flags |= FLAG_OF;
        } else {
            regFile.flags &= ~FLAG_OF;
        }
    }
}

void setParityFlag(u16 res) {
    if (__popcnt16(res & 0x00FF) % 2 == 0) {
        regFile.flags |= FLAG_PF;
    } else {
        regFile.flags &= ~FLAG_PF;
    }
}

void setSignFlag(u16 res, bool wide) {
    if (res & (1 << (wide ? 15 : 7))) {
        regFile.flags |= FLAG_SF;
    } else {
        regFile.flags &= ~FLAG_SF;
    }
}

void setZeroFlag(u16 res) {
    if (res == 0) {
        regFile.flags |= FLAG_ZF;
    } else {
        regFile.flags &= ~FLAG_ZF;
    }
}

u16 sub_calcAndSetFlags(u16 dst, u16 src, bool wide) {
    // AF
    if ((dst & 0xF) < (src & 0xF)) {
        regFile.flags |= FLAG_AF;
    } else {
        regFile.flags &= ~FLAG_AF;
    }

    // CF
    if (src > dst) {
        regFile.flags |= FLAG_CF;
    } else {
        regFile.flags &= ~FLAG_CF;
    }

    i32 overflowCheck = wide ? (i16) dst - (i16) src : (i8) dst - (i8) src;
    setOverflowFlag(overflowCheck, wide);

    u16 res = wide ? dst - src : (u8) dst - (u8) src;
    setParityFlag(res);
    setSignFlag(res, wide);
    setZeroFlag(res);

    return res;
}

void executeInstr(Instr instr) {

    switch (instr.op) {
        case OP_MOV: {
            writeArg(instr.dst, readArg(instr.src));
            break;
        }
        case OP_ADD: {
            u16 dst = readArg(instr.dst);
            u16 src = readArg(instr.src);

            char *preOpFlags = getFlagsStr();

            // AF
            if ((dst & 0xF) + (src & 0xF) > 0xF) {
                regFile.flags |= FLAG_AF;
            } else {
                regFile.flags &= ~FLAG_AF;
            }

            // CF
            u32 carryCheck = (u32) src + (u32) dst;
            if (carryCheck > (instr.wide ? 0xFFFF : 0xFF)) {
                regFile.flags |= FLAG_CF;
            } else {
                regFile.flags &= ~FLAG_CF;
            }

            // OF
            i32 overflowCheck = instr.wide ? (i16) src + (i16) dst : (i8) src + (i8) dst;
            setOverflowFlag(overflowCheck, instr.wide);

            u16 res = instr.wide ? dst + src : (u8) dst + (u8) src;
            setParityFlag(res);
            setSignFlag(res, instr.wide);
            setZeroFlag(res);

            writeArg(instr.dst, res);

            char *postOpFlags = getFlagsStr();
            printf(" flags %s->%s", preOpFlags, postOpFlags);

            break;
        }
        case OP_SUB: {
            u16 dst = readArg(instr.dst);
            u16 src = readArg(instr.src);

            char *preOpFlags = getFlagsStr();

            u16 res = sub_calcAndSetFlags(dst, src, instr.wide);
            writeArg(instr.dst, res);

            char *postOpFlags = getFlagsStr();
            printf(" flags %s->%s", preOpFlags, postOpFlags);
            break;
        }
        case OP_CMP: {
            u16 dst = readArg(instr.dst);
            u16 src = readArg(instr.src);

            char *preOpFlags = getFlagsStr();

            sub_calcAndSetFlags(dst, src, instr.wide);

            char *postOpFlags = getFlagsStr();
            printf(" flags %s->%s", preOpFlags, postOpFlags);
            break;
        }
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
