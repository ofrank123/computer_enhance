#pragma once
#include "../common.h"

#define FOREACH_OP(OP) \
    OP(MOV) \
    OP(PUSH) \
    OP(POP) \
    OP(XCHG) \
    OP(IN) \
    OP(OUT) \
    OP(XLAT) \
    OP(LEA) \
    OP(LDS) \
    OP(LES) \
    OP(LAHF) \
    OP(SAHF) \
    OP(PUSHF) \
    OP(POPF) \
    OP(ADD) \
    OP(ADC) \
    OP(INC) \
    OP(AAA) \
    OP(DAA) \
    OP(SUB) \
    OP(SBB) \
    OP(DEC) \
    OP(NEG) \
    OP(CMP) \
    OP(AAS) \
    OP(DAS) \
    OP(MUL) \
    OP(IMUL) \
    OP(AAM) \
    OP(DIV) \
    OP(IDIV) \
    OP(AAD) \
    OP(CBW) \
    OP(CWD) \
    OP(NOT) \
    OP(SHL) \
    OP(SHR) \
    OP(SAR) \
    OP(ROL) \
    OP(ROR) \
    OP(RCL) \
    OP(RCR) \
    OP(AND) \
    OP(TEST) \
    OP(OR) \
    OP(XOR) \
    OP(REP) \
    OP(MOVS) \
    OP(CMPS) \
    OP(SCAS) \
    OP(LODS) \
    OP(STOS) \
    OP(CALL) \
    OP(JMP) \
    OP(RET) \
    OP(JE) \
    OP(JL) \
    OP(JLE) \
    OP(JB) \
    OP(JBE) \
    OP(JP) \
    OP(JO) \
    OP(JS) \
    OP(JNE) \
    OP(JNL) \
    OP(JG) \
    OP(JNB) \
    OP(JA) \
    OP(JNP) \
    OP(JNO) \
    OP(JNS) \
    OP(LOOP) \
    OP(LOOPZ) \
    OP(LOOPNZ) \
    OP(JCXZ) \
    OP(INT) \
    OP(INT3) \
    OP(INTO) \
    OP(IRET) \
    OP(CLC) \
    OP(CMC) \
    OP(STC) \
    OP(CLD) \
    OP(STD) \
    OP(CLI) \
    OP(STI) \
    OP(HLT) \
    OP(WAIT) \
    OP(ESC) \
    OP(LOCK) \
    OP(SEGMENT)

#define GENERATE_ENUM(ENUM) OP_##ENUM, 
#define GENERATE_STRING(STRING) #STRING,
#define DECODE_OP_STR(OP) if (strcmp(str, #OP) == 0) return OP_##OP;

enum Op {
    FOREACH_OP(GENERATE_ENUM)
    OP_NONE,
};

static const char *OP_STRINGS[] = {
    FOREACH_OP(GENERATE_STRING)
};

static inline Op decodeOpStr(char *str) {
    FOREACH_OP(DECODE_OP_STR)
    return OP_NONE;
}

typedef u32 sim_ptr;

enum Mod {
    MOD_NONE = 0,
    MOD_REG,
    MOD_DISP,
    MOD_DISP8,
    MOD_DISP16,
};

enum Register {
    REG_NONE = 0,
    REG_AL,
    REG_AX,
    REG_CL, 
    REG_CX,
    REG_DL, 
    REG_DX,
    REG_BL, 
    REG_BX,
    REG_AH, 
    REG_SP,
    REG_CH, 
    REG_BP,
    REG_DH,
    REG_SI,
    REG_BH,
    REG_DI,
    REG_ES,
    REG_CS,
    REG_SS,
    REG_DS
};

enum EffectiveAddrBase {
    EAB_NONE = 0,
    EAB_DIRECT,
    EAB_BX_SI,
    EAB_BX_DI,
    EAB_BP_SI,
    EAB_BP_DI,
    EAB_SI,
    EAB_DI,
    EAB_BP,
    EAB_BX,
};

struct EffectiveAddrCalc {
    Register segment;
    EffectiveAddrBase base;
    i32 disp;
};

enum ArgType {
    ARG_NONE = 0,
    ARG_REG,
    ARG_MEM,
    ARG_IMM,
    ARG_REL_IMM,
};

struct Arg {
    ArgType type;

    union {
        Register reg;
        EffectiveAddrCalc eac;
        i32 imm;
        i32 relImm;
    };
};

struct Instr {
    Op op;

    bool wide;
    u16 length;

    Arg dst;
    Arg src;
};

struct InstrFlags {
    bool rep;
    bool lock;
    Register segmentOverride;
};

/**
 * Read `size` bytes from simulation memory, into `dst`
 * starting at offset `src`
 */
void readMem(u8 *dst, sim_ptr src, u32 size);

/**
 * Write `size` bytes from `src` into simulation memory
 * at offset `src`
 */
void writeMem(sim_ptr dst, u8 *src, u32 size);

/**
 * Load program into simulation memory
 */
void loadProgram(const char *progFile);

/**
 * Update flags if instruction affects them, otherwise apply flags
 * to current instruction and clear them
 */
void handleFlags(InstrFlags *flags, Instr *instr);

/**
 * Get string from register
 */
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

