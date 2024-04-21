#include "decode.h"

struct InstrDecode {
    bool w;
    bool d;
    bool s;
    bool v;
    bool z;

    struct {
        bool rmRegWide;
    } flags;

    bool hasV;
    bool hasData;
    bool hasReg;
    bool hasMod;
    bool hasSR;
    bool hasRelDisp;

    u8 reg;
    u8 mod;
    u8 rm;
    u8 sr;

    i32 disp;
    u32 data;
    u32 addr;
};

u32 tryInstrDef(InstrDecode *decodeData, InstrDef def, u8 *bytes) {
    u32 bitOffset = 0;

    *decodeData = {};
    bool hasLongData = false;

    for (InstrPart part : def.parts) {
        u8 testBits = bytes[bitOffset / 8];
        testBits <<= bitOffset % 8; // Align bits
        switch (part.type) {
            case IP_BITS:
                testBits >>= 8 - part.bits.size;
                if (testBits != part.bits.data) return 0;
                bitOffset += part.bits.size;
                break;
            case IP_W:
                testBits >>= 8 - 1;
                bitOffset += 1;
                decodeData->w = testBits;
                break;
            case IP_D:
                testBits >>= 8 - 1;
                bitOffset += 1;
                decodeData->d = testBits;
                break;
            case IP_S:
                testBits >>= 8 - 1;
                bitOffset += 1;
                decodeData->s = testBits;
                break;
            case IP_V:
                testBits >>= 8 - 1;
                bitOffset += 1;
                decodeData->hasV = true;
                decodeData->v = testBits;
                break;
            case IP_Z:
                testBits >>= 8 - 1;
                bitOffset += 1;
                decodeData->z = testBits;
                break;
            case IP_MOD:
                testBits >>= 8 - 2;
                bitOffset += 2;
                decodeData->hasMod = true;
                decodeData->mod = testBits;
                break;
            case IP_REG:
                testBits >>= 8 - 3;
                bitOffset += 3;
                decodeData->hasReg = true;
                decodeData->reg = testBits;
                break;
            case IP_RM:
                testBits >>= 8 - 3;
                bitOffset += 3;
                decodeData->rm = testBits;
                break;
            case IP_SR:
                testBits >>= 8 - 2;
                bitOffset += 2;
                decodeData->hasSR = true;
                decodeData->sr = testBits;
                break;
            case IP_DATA:
                if (bitOffset % 8 != 0) {
                    fprintf(stderr, "Unaligned read of DATA occurred!\n");
                    exit(1);
                }
                decodeData->hasData = true;
                break;
            case IP_DATA_IF_W:
                if (bitOffset % 8 != 0) {
                    fprintf(stderr, "Unaligned read of DATA_IF_W occurred!\n");
                    exit(1);
                }
                hasLongData = true;
                break;
            case IP_ADDR:
                if (bitOffset % 8 != 0) {
                    fprintf(stderr, "Unaligned read of ADDR occurred!\n");
                    exit(1);
                }
                bitOffset += 8;
                decodeData->addr = testBits;
                testBits = bytes[bitOffset / 8];
                bitOffset += 8;
                decodeData->addr += ((u16) testBits) << 8;
                break;
            case IP_DISP:
                if (bitOffset % 8 != 0) {
                    fprintf(stderr, "Unaligned read of DISP occurred!\n");
                    exit(1);
                }
                bitOffset += 8;
                decodeData->disp = (i8) testBits;
                decodeData->hasRelDisp = true;
                break;
            case IP_IMP_D:
                decodeData->d = part.impD;
                break;
            case IP_IMP_W:
                decodeData->w = part.impW;
                break;
            case IP_IMP_REG:
                decodeData->hasReg = true;
                decodeData->reg = part.impReg;
                break;
            case IP_IMP_MOD:
                decodeData->hasMod = true;
                decodeData->mod = part.impMod;
                break;
            case IP_IMP_RM:
                decodeData->rm = part.impRm;
                break;
            case IP_F_RM_REG_WIDE:
                decodeData->flags.rmRegWide = true;
                break;
        }
    }

    if (decodeData->hasMod && decodeData->mod == 0b01) { // 8 Bit displacement
        if (bitOffset % 8 != 0) {
            fprintf(stderr, "Unaligned read of DISP-LO occurred!\n");
            exit(1);
        }
        u8 disp = bytes[bitOffset/8];
        bitOffset += 8;
        decodeData->disp = disp & (1 << 7) ? -((u8) (~disp) + 1) : disp;
    }

    if (decodeData->hasMod && decodeData->mod == 0b10) { // 16 Bit displacement
        u16 disp = bytes[bitOffset / 8];
        bitOffset += 8;
        disp += bytes[bitOffset / 8] << 8;
        bitOffset += 8;
        decodeData->disp = disp & (1 << 15) ? -((u16) (~disp) + 1) : disp;
    }

    if (decodeData->hasMod && (decodeData->mod == 0b00 && decodeData->rm == 0b110)) { // Direct
        decodeData->disp = bytes[bitOffset / 8];
        bitOffset += 8;
        decodeData->disp += bytes[bitOffset / 8] << 8;
        bitOffset += 8;
    }

    if (decodeData->hasData) {
        if (bitOffset % 8 != 0) {
            fprintf(stderr, "Unaligned read of DATA_IF_W occurred!\n");
            exit(1);
        }
        decodeData->data = bytes[bitOffset / 8];
        bitOffset += 8;
        if (hasLongData && decodeData->w && !decodeData->s) {
            decodeData->data += ((u16) bytes[bitOffset / 8]) << 8;
            bitOffset += 8;
        }
    }

    return bitOffset / 8;
}

Register decodeRegister(u8 reg, bool wide) {
    switch (reg) {
        case 0b000: return wide ? REG_AX : REG_AL;
        case 0b001: return wide ? REG_CX : REG_CL;
        case 0b010: return wide ? REG_DX : REG_DL;
        case 0b011: return wide ? REG_BX : REG_BL;
        case 0b100: return wide ? REG_SP : REG_AH;
        case 0b101: return wide ? REG_BP : REG_CH;
        case 0b110: return wide ? REG_SI : REG_DH;
        case 0b111: return wide ? REG_DI : REG_BH;
    }
    fprintf(stderr, "Invalid register!\n");
    exit(1);
}

Register decodeSegmentRegister(u8 sr) {
    switch (sr) {
        case 0b00: return REG_ES;
        case 0b01: return REG_CS;
        case 0b10: return REG_SS;
        case 0b11: return REG_DS;
    }
    fprintf(stderr, "Invalid segment register!\n");
    exit(1);
}

EffectiveAddrBase decodeEAB(u8 eab) {
    switch (eab) {
        case 0b000: return EAB_BX_SI;
        case 0b001: return EAB_BX_DI;
        case 0b010: return EAB_BP_SI;
        case 0b011: return EAB_BP_DI;
        case 0b100: return EAB_SI;
        case 0b101: return EAB_DI;
        case 0b110: return EAB_BP;
        case 0b111: return EAB_BX;
    }
    fprintf(stderr, "Invalid EAB!\n");
    exit(1);
}

u32 decodeNextInstr(Instr *instr, sim_ptr offset, InstrDefTable defTable) {
    *instr = {};

    u8 bytes[6];
    readMem(bytes, offset, 6);

    u32 bytesConsumed;
    InstrDecode decodeData;
    InstrDef def{};
    bool defFound = false;
    for (InstrDef maybeDef : defTable) {
        if ((bytesConsumed = tryInstrDef(&decodeData, maybeDef, bytes)) > 0) {
            defFound = true;
            def = maybeDef;
            break;
        }
    }

    if (!defFound) {
        fprintf(stderr, "No definition found!\n");
        exit(1);
    }

    instr->op = def.op;
    instr->wide = decodeData.w;

    if (decodeData.hasReg || decodeData.hasSR) {
        Arg regArg = {
            .type = ARG_REG,
            .reg = decodeData.hasSR ?
                decodeSegmentRegister(decodeData.sr) :
                decodeRegister(decodeData.reg, decodeData.w)
        };
        if (decodeData.d) {
            instr->dst = regArg;
        } else {
            instr->src = regArg;
        }
    }

    if (decodeData.hasData) {
        Arg immArg = { .type = ARG_IMM, .imm = decodeData.data };
        if (instr->src.type != ARG_NONE) {
            instr->dst = immArg;
        } else {
            instr->src = immArg;
        }
    }

    if (decodeData.hasRelDisp) {
        Arg relImmArg = { .type = ARG_REL_IMM, .relImm = decodeData.disp };
        if (instr->src.type != ARG_NONE) {
            instr->dst = relImmArg;
        } else {
            instr->src = relImmArg;
        }
    }

    if (decodeData.hasV) {
        if (decodeData.v) {
            instr->src.type = ARG_REG;
            instr->src.reg = REG_CL;
        } else {
            instr->src.type = ARG_IMM;
            instr->src.imm = 1;
        }
    }

    if (decodeData.hasMod) {
        Arg rmArg;
        if (decodeData.mod == 0b11) { // Register
            rmArg.type = ARG_REG;
            rmArg.reg = decodeRegister(decodeData.rm,
                                       decodeData.w || decodeData.flags.rmRegWide);
        } else if (decodeData.mod == 0b00 && decodeData.rm == 0b110) { // Direct
            rmArg.type = ARG_MEM;
            rmArg.eac.base = EAB_DIRECT;
            rmArg.eac.disp = decodeData.disp;
        } else {
            rmArg.type = ARG_MEM;
            rmArg.eac.base = decodeEAB(decodeData.rm);
            rmArg.eac.disp = decodeData.disp;
        }

        if (instr->src.type != ARG_NONE) {
            instr->dst = rmArg;
        } else {
            instr->src = rmArg;
        }
    }

    if (instr->dst.type == ARG_NONE && instr->src.type != ARG_NONE) {
        instr->dst = instr->src;
        instr->src = {};
    }

    return bytesConsumed;
}
