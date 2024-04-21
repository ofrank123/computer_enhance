#include "print.h"

#include <ctype.h>

static inline const char *getRegStr(Register reg) {
    switch (reg) {
        case REG_AL: return "al";
        case REG_AX: return "ax";
        case REG_CL: return "cl"; 
        case REG_CX: return "cx";
        case REG_DL: return "dl"; 
        case REG_DX: return "dx";
        case REG_BL: return "bl"; 
        case REG_BX: return "bx";
        case REG_AH: return "ah"; 
        case REG_SP: return "sp";
        case REG_CH: return "ch"; 
        case REG_BP: return "bp";
        case REG_DH: return "dh";
        case REG_SI: return "si";
        case REG_BH: return "bh";
        case REG_DI: return "di";
        case REG_ES: return "es";
        case REG_CS: return "cs";
        case REG_SS: return "ss";
        case REG_DS: return "ds";
        case REG_NONE:
            fprintf(stderr, "Attempted to print REG_NONE!");
            exit(1);
    }
}

static inline const char *getEABStr(EffectiveAddrBase eab) {
    switch(eab) {
        case EAB_BX_SI: return "bx + si";
        case EAB_BX_DI: return "bx + di";
        case EAB_BP_SI: return "bp + si";
        case EAB_BP_DI: return "bp + di";
        case EAB_SI: return "si";
        case EAB_DI: return "di";
        case EAB_BP: return "bp";
        case EAB_BX: return "bx";
        default:
            fprintf(stderr, "Cannot print EAB!\n");
            exit(1);
    }
}

static inline const char *getSegmentPrefix(Register segment) {
    switch(segment) {
        case REG_ES: return "es:";
        case REG_CS: return "cs:";
        case REG_SS: return "ss:";
        case REG_DS: return "ds:";
        default: return "";
    }
}

static inline const char *getMemoryPrefix(Register segment, bool wide, bool isDst) {
    if (isDst) {
        char *ret = allocStr(16);
        sprintf(ret, "%s%s",
                wide ? "word " : "byte ",
                getSegmentPrefix(segment));
        return ret;
    } else {
        return getSegmentPrefix(segment);
    }
}

static const char *getArgStr(Arg arg, bool wide, bool isDst) {
    char *ret = allocStr(32);

    switch (arg.type) {
        case ARG_IMM:
            sprintf(ret, "%d", arg.imm);
            break;
        case ARG_REL_IMM:
            sprintf(ret, "%d", arg.relImm);
            break;
        case ARG_REG:
            sprintf(ret, "%s", getRegStr(arg.reg));
            break;
        case ARG_MEM:
            if (arg.eac.base == EAB_DIRECT) {
                sprintf(ret, "%s[%d]",
                        getMemoryPrefix(arg.eac.segment, wide, isDst),
                        arg.eac.disp);
            } else {
                if (arg.eac.disp == 0) {
                    sprintf(ret, "%s[%s]",
                            getMemoryPrefix(arg.eac.segment, wide, isDst),
                            getEABStr(arg.eac.base));
                } else {
                    sprintf(ret, "%s[%s %s %d]",
                            getMemoryPrefix(arg.eac.segment, wide, isDst),
                            getEABStr(arg.eac.base),
                            arg.eac.disp < 0 ? "-" : "+",
                            abs(arg.eac.disp));
                }
            }
            break;
        case ARG_NONE:
            fprintf(stderr, "Attempted to print ARG_NONE!\n");
            exit(1);
    }

    return ret;
}

static inline void strToLower(char *p) {
    for ( ; *p; ++p) *p = tolower(*p);
}

void printInstr(Instr instr) {
    char opName[8];
    strcpy(opName, OP_STRINGS[instr.op]);
    strToLower(opName);

    // Need to switch XCHG args if not accumulator for nasm to be happy
    if (instr.op == OP_XCHG &&
        ((instr.dst.type == ARG_REG && instr.dst.reg != REG_AX) ||
         (instr.src.type == ARG_REG && instr.src.reg == REG_AX))) {
        Arg tmp = instr.dst;
        instr.dst = instr.src;
        instr.src = tmp;
    }

    if (instr.op == OP_SEGMENT) {
        return;
    }

    if (instr.op == OP_REP || instr.op == OP_LOCK) {
        printf("%s ", opName);
        return;
    }

    if (instr.dst.type == ARG_NONE) {
        printf("%s", opName);
        // These require w/b appended depending on the instr width
        switch (instr.op) {
            case OP_MOVS:
            case OP_CMPS:
            case OP_SCAS:
            case OP_LODS:
            case OP_STOS:
                printf("%s", instr.wide ? "w" : "b");
            default: break;
        }
        printf("\n");
    } else if (instr.src.type == ARG_NONE) {
        printf("%s %s\n", opName, getArgStr(instr.dst, instr.wide, true));
    } else {
        printf("%s %s, %s\n",
               opName,
               getArgStr(instr.dst, instr.wide, true),
               getArgStr(instr.src, instr.wide, false));
    }
}
