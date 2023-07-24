// Computer Enhance -- Exercise 01

#include "../common.h"

FILE *fp;
u32 bytesRead;
u32 linenum = 1;
bool lockPrefix;
bool segmentPrefix;
u8 segPrefixVal;

enum ArgTag {
	ARG_NONE = 0,
	ARG_REG,
	ARG_REG_WIDE,
	ARG_DIR,
	ARG_DIS,
	ARG_DIS8,
	ARG_DIS16,
	ARG_IMM,
    ARG_IMM_WIDE,
	ARG_IPDIS,
	ARG_SEG,
    ARG_DATA
};

struct DispData {
	u8 eac;
    u8 d8;
    u16 d16;
};

struct Arg {
	ArgTag tag;

	union {
		u8 reg;
		u16 dir;
		DispData dis;
		u16 imm;
		u8 sdis;
		u8 seg;
		u8 data;
	} v;
};

const char *getRegStr(u8 reg, bool wide) {
    switch (reg) {
        case 0b00000000: return wide ? "ax" : "al";
        case 0b00000001: return wide ? "cx" : "cl";
        case 0b00000010: return wide ? "dx" : "dl";
        case 0b00000011: return wide ? "bx" : "bl";
        case 0b00000100: return wide ? "sp" : "ah";
        case 0b00000101: return wide ? "bp" : "ch";
        case 0b00000110: return wide ? "si" : "dh";
        case 0b00000111: return wide ? "di" : "bh";
    }
    exit(1);
}

const char *getEacStr(u8 eac) {
	switch (eac) {
		case  0b00000000: return "bx + si";
		case  0b00000001: return "bx + di";
		case  0b00000010: return "bp + si";
		case  0b00000011: return "bp + di";
		case  0b00000100: return "si";
		case  0b00000101: return "di";
		case  0b00000110: return "bp";
		case  0b00000111: return "bx";
	}
	exit(1);
}

const char *getSegRegStr(u8 segReg) {
    switch (segReg) {
        case 0b00000000: return "es";
        case 0b00000001: return "cs";
        case 0b00000010: return "ss";
        case 0b00000011: return "ds";
    }
    exit(1);
}

const char *getSegPrefix() {
	char *str = allocStr(5);
    if (segmentPrefix) {
        sprintf(str, "%s:", getSegRegStr(segPrefixVal));
        return str;
    } else {
        return "";
    }
}

const char *getSizePrefix(bool needsSizePrefix, bool wide) {
    if (needsSizePrefix) {
        return wide ? "word " : "byte ";
    }
    return "";
}

const char *getMemPrefix(bool needsSizePrefix, bool wide) {
    char *str = allocStr(12);
    sprintf(str, "%s%s", getSizePrefix(needsSizePrefix, wide), getSegPrefix());
    return str;
}

char *getArgStr(Arg arg, bool needsSizePrefix, bool wide) {
	char *str = allocStr(32);
	switch (arg.tag) {
		case ARG_REG: return (char *) getRegStr(arg.v.reg, false);
		case ARG_REG_WIDE: return (char *) getRegStr(arg.v.reg, true);
        case ARG_IMM: sprintf(str, "byte %d", arg.v.imm); break;
        case ARG_IMM_WIDE: sprintf(str, "word %d", arg.v.imm); break;
		case ARG_DIS:
			sprintf(str, "%s[%s]",
                    getMemPrefix(needsSizePrefix, wide),
                    getEacStr(arg.v.dis.eac));
			break;
		case ARG_DIS8:
			sprintf(str, "%s[%s %s %d]",
                    getMemPrefix(needsSizePrefix, wide),
					getEacStr(arg.v.dis.eac),
					arg.v.dis.d8 & (1 << 7) ? "-" : "+",
                    arg.v.dis.d8 & (1 << 7) ? (u8) (~arg.v.dis.d8) + 1 : arg.v.dis.d8);
			break;
		case ARG_DIS16:
			sprintf(str, "%s[%s %s %d]",
                    getMemPrefix(needsSizePrefix, wide),
					getEacStr(arg.v.dis.eac),
					arg.v.dis.d16 & (1 << 15) ? "-" : "+",
                    arg.v.dis.d16 & (1 << 15) ? (u16) (~arg.v.dis.d16) + 1 : arg.v.dis.d16);
			break;
		case ARG_DIR:
			sprintf(str, "%s[%d]",
                    getMemPrefix(needsSizePrefix, wide),
                    arg.v.dir);
			break;
		case ARG_IPDIS:
			sprintf(str, "%s%d", 
					arg.v.sdis & (1 << 7) ? "-" : "+",
                    arg.v.sdis & (1 << 7) ? (u8) (~arg.v.sdis) + 1 : arg.v.sdis);
			break;
        case ARG_SEG:
            sprintf(str, "%s", getSegRegStr(arg.v.seg));
            break;
        case ARG_DATA:
            sprintf(str, "%d", arg.v.data);
            break;
		case ARG_NONE:
			printf("ARG_NONE Printed!\n");
			exit(1);
	}
	return str;
}

u16 readData(bool wide) {
	u8 data;
	fread(&data, 1, 1, fp);
	bytesRead++;
	u16 ret = data;
	if (wide) {
		fread(&data, 1, 1, fp);
		bytesRead++;
		ret |= (u16) data << 8;
	}
	return ret;
}

Arg decodeRm(u8 rm, u8 mod, bool wide) {
	Arg arg{};
	if (mod == 0b11000000) { // Register
		arg.tag = wide ? ARG_REG_WIDE : ARG_REG;
		arg.v.reg = rm;
	} else { // Memory
		if (mod == 0 && rm == 0b00000110) { // Direct Addressing
			arg.tag = ARG_DIR;
			arg.v.dir = readData(true);
		} else { // Indirect Addressing
			arg.v.dis.eac = rm;
			if (mod == 0) { // No Displacement
				arg.tag = ARG_DIS;
			} else if (mod == 0b01000000) { // 8 Bit Displacement
				arg.tag = ARG_DIS8;
				arg.v.dis.d8 = readData(false);
			} else if (mod == 0b10000000) { // 16 Bit Displacement
				arg.tag = ARG_DIS16;
				arg.v.dis.d16 = readData(true);
			}
		}
	}

	return arg;
}

//  12|345|678
// mod|reg|rm
void decodeModRegRm(u8 data, bool wide, Arg *reg, Arg *rm) {
    if (reg != NULL) {
        reg->tag = wide ? ARG_REG_WIDE : ARG_REG;
        reg->v.reg = (data & 0b00111000) >> 3;
    }
    if (rm != NULL) {
        *rm = decodeRm(data & 0b00000111, data & 0b11000000, wide);
    }
}

const char *getOpStr(u8 byte1, u8 opByte) {
    if ((byte1 & 0b11111110) == 0b11000110 ||
        (byte1 & 0b11111100) == 0b10001000 ||
        (byte1 & 0b11110000) == 0b10110000 ||
        (byte1 & 0b11111110) == 0b10100000 ||
        (byte1 & 0b11111110) == 0b10100010) {
        return "mov";
    }
    // Arithmetic ops that follow sub-op pattern
    if ((byte1 & 0b11000100) == 0b00000000 ||
        (byte1 & 0b11111100) == 0b10000000 ||
        (byte1 & 0b11000110) == 0b00000100) {
        switch (opByte & 0b00111000) {
            case 0b00000000: return "add"; 
            case 0b00101000: return "sub";
            case 0b00111000: return "cmp";
            case 0b00010000: return "adc";
            case 0b00100000: return "and";
            case 0b00001000: return "or";
            case 0b00110000: return "xor";
            case 0b00011000: return "sbb";
        }
    }
    if ((byte1 & 0b11110000) == 0b01110000) {
        switch (byte1 & 0b00001111) {
            case 0b00000000: return "jo";
            case 0b00000001: return "jno";
            case 0b00000010: return "jb";
            case 0b00000011: return "jnb";
            case 0b00000100: return "je";
            case 0b00000101: return "jne";
            case 0b00000110: return "jbe";
            case 0b00000111: return "ja";
            case 0b00001000: return "js";
            case 0b00001001: return "jns";
            case 0b00001010: return "jp";
            case 0b00001011: return "jnp";
            case 0b00001100: return "jl";
            case 0b00001101: return "jnl";
            case 0b00001110: return "jle";
            case 0b00001111: return "jg";
        }
    }
    if ((byte1 & 0b11110000) == 0b11100000) {
        switch (byte1 & 0b00001111) {
            case 0b00000000: return "loopne";
            case 0b00000001: return "loopz";
            case 0b00000010: return "loop";
            case 0b00000011: return "jcxz";
        }
    }
    if ((byte1 & 0b11111111) == 0b11111111 ||
        (byte1 & 0b11111000) == 0b01010000 ||
        (byte1 & 0b11100111) == 0b00000110) {
        return "push";
    }
    if ((byte1 & 0b11111111) == 0b10001111 ||
        (byte1 & 0b11111000) == 0b01011000 ||
        (byte1 & 0b11100111) == 0b00000111) {
        return "pop";
    }
    if ((byte1 & 0b11111110) == 0b10000110 ||
        (byte1 & 0b11111000) == 0b10010000) {
        return "xchg";
    }
    if ((byte1 & 0b11111110) == 0b11100100 ||
        (byte1 & 0b11111110) == 0b11101100) {
        return "in";
    }
    if ((byte1 & 0b11111110) == 0b11100110 ||
        (byte1 & 0b11111110) == 0b11101110) {
        return "out";
    }
    if ((byte1 & 0b11111110) == 0b11110110) {
        if ((opByte & 0b00111000) == 0b00000000) return "test";
        if ((opByte & 0b00111000) == 0b00010000) return "not";
    }
    switch(byte1) {
        case 0b11010111: return "xlat";
        case 0b10001101: return "lea";
        case 0b11000101: return "lds";
        case 0b11000100: return "les";
        case 0b10011111: return "lahf";
        case 0b10011110: return "sahf";
        case 0b10011100: return "pushf";
        case 0b10011101: return "popf";
        case 0b11001101: return "int";
        case 0b11001100: return "int3";
        case 0b11001110: return "into";
        case 0b11001111: return "iret";
        case 0b11111000: return "clc";
        case 0b11110101: return "cmc";
        case 0b11111001: return "stc";
        case 0b11111100: return "cld";
        case 0b11111101: return "std";
        case 0b11111010: return "cli";
        case 0b11111011: return "sti";
        case 0b11110100: return "hlt";
        case 0b10011011: return "wait";
    }

    printf("%X ", byte1);
    return "NOP";
}

void decodeNextOp() {
	u8 byte1 = readData(false);
    bool wide = byte1 & 0b00000001;
    u8 opByte = byte1;

	char op[8];
	Arg dst{};
	Arg src{};

    if ((byte1 & 0b11111110) == 0b11000110 /* mov */ ||
        (byte1 & 0b11111100) == 0b10000000 /* arith */ ||
        (byte1 & 0b11111110) == 0b11110110 /* not, test */) {
        // Reg/Mem, Immediate

        u8 byte2 = readData(false);
		Arg rmArg;
		decodeModRegRm(byte2, wide, NULL, &rmArg);
        if (!((byte1 & 0b11111110) == 0b11110110 &&
              (byte2 & 0b00111000) == 0b00010000)) { // NOT special case, same byte1
                                                  // as test, but no immediate val
                                                  // needed
            src.tag = wide ? ARG_IMM_WIDE : ARG_IMM;
            if ((byte1 & 0b11000100) == 0b10000000) { // arith itrm special case
                opByte = byte2;
                bool wideRead = (wide && !(byte1 & 0b00000010)); // S: 0 + W: 1
                src.v.imm = readData(wideRead);
            } else {
                src.v.imm = readData(wide);
            }
        }

        opByte = byte2; // For test and not

        dst = rmArg;
    } else if ((byte1 & 0b11111100) == 0b10001000 /* mov */ ||
               (byte1 & 0b11000100) == 0b00000000 /* arith */ ||
               (byte1 & 0b11111110) == 0b10000110 /* xchg */ ||
               byte1 == 0b10001101 /* lea */ ||
               byte1 == 0b11000101 /* lds */ ||
               byte1 == 0b11000100 /* les */) {
        // Reg/Mem, Reg/Mem
        u8 byte2 = readData(false);
        bool dSet = byte1 & 0b00000010;
        Arg regArg, rmArg;

        if (byte1 == 0b10001101 /* lea */ ||
            byte1 == 0b11000101 /* lds */ ||
            byte1 == 0b11000100 /* les */) {
            wide = true;
            dSet = true;
        }

        decodeModRegRm(byte2, wide, &regArg, &rmArg);

        if (dSet) {
            src = rmArg;
            dst = regArg;
        } else {
            src = regArg;
            dst = rmArg;
        }
    } else if ((byte1 & 0b11110000) == 0b10110000 /* mov */) {
        // Reg, Immediate

        wide = byte1 & 0b00001000;

        dst.tag = wide ? ARG_REG_WIDE : ARG_REG;
        dst.v.reg = byte1 & 0b00000111;

        src.tag = wide ? ARG_IMM_WIDE : ARG_IMM;
        src.v.imm = readData(wide);
    } else if ((byte1 & 0b11111110) == 0b10100000 /* mov */) {
        // Accumulator, Direct

        dst.tag = ARG_REG_WIDE;
        dst.v.reg = 0; // AX

        src.tag = ARG_DIR;
        src.v.dir = readData(wide);
    } else if ((byte1 & 0b11111110) == 0b10100010 /* mov */) {
        // Direct, Accumulator

        dst.tag = ARG_DIR;
        dst.v.dir = readData(wide);

        src.tag = ARG_REG_WIDE;
        src.v.reg = 0; // AX

    } else if ((byte1 & 0b11000110) == 0b00000100 /* arith */) {
        // Accumulator, Immediate

        dst.tag = wide ? ARG_REG_WIDE : ARG_REG;
        dst.v.reg = 0;

        src.tag = wide ? ARG_IMM_WIDE : ARG_IMM;
        src.v.imm = readData(wide);
    } else if ((byte1 & 0b11110000) == 0b01110000 /* conditional jumps */ ||
               (byte1 & 0b11111100) == 0b11100000 /* loops/jcxz */) {
        // Signed Displacement (Jumps)

        dst.tag = ARG_IPDIS;
        dst.v.sdis = readData(false);
    } else if ((byte1 & 0b11111111) == 0b11111111 /* push */ ||
               (byte1 & 0b11111111) == 0b10001111 /* pop */) {
        // Register/Memory

        decodeModRegRm(readData(false), true, NULL, &dst);
    } else if ((byte1 & 0b11111000) == 0b01010000 /* push */ ||
               (byte1 & 0b11111000) == 0b01011000 /* pop */) {
        // Register

        dst.tag = ARG_REG_WIDE;
        dst.v.reg = byte1 & 0b00000111;
    } else if ((byte1 & 0b11100111) == 0b00000110 /* push */ ||
               (byte1 & 0b11100111) == 0b00000111 /* pop */) {
        // Segment Register

        dst.tag = ARG_SEG;
        dst.v.seg = (byte1 & 0b00011000) >> 3;
    } else if ((byte1 & 0b11111000) == 0b10010000 /* xchg */) {
        // Accumulator, Reg

        dst.tag = ARG_REG_WIDE;
        dst.v.reg = 0; // AX
        src.tag = ARG_REG_WIDE;
        src.v.reg = byte1 & 0b00000111;
    } else if ((byte1 & 0b11111110) == 0b11100100 /* in */ ||
               (byte1 & 0b11111110) == 0b11100110 /* out */) {
        // Fixed Port

        dst.tag = wide ? ARG_REG_WIDE : ARG_REG;
        dst.v.reg = 0; // ax/al
        src.tag = ARG_DATA;
        src.v.data = readData(false);

        // Swap if out
        if ((byte1 & 0b11111110) == 0b11100110) {
            Arg tmp = dst;
            dst = src;
            src = tmp;
        }
    } else if ((byte1 & 0b11111110) == 0b11101100 /* in */ ||
               (byte1 & 0b11111110) == 0b11101110 /* out */) {
        // Variable Port

        dst.tag = wide ? ARG_REG_WIDE : ARG_REG;
        dst.v.reg = 0; // ax/al
        src.tag = ARG_REG_WIDE;
        src.v.reg = 0b00000010; // dx

        // Swap if out
        if ((byte1 & 0b11111110) == 0b11101110) {
            Arg tmp = dst;
            dst = src;
            src = tmp;
        }
    } else if (byte1 == 0b11001101 /* int */) {
        // Data

        dst.tag = ARG_DATA;
        dst.v.data = readData(false);
    }

    if (byte1 == 0b11110000) { // Lock prefix
        lockPrefix = true;
        return;
    }

    if ((byte1 & 0b11100111) == 0b00100110) { // Segment prefix
        segmentPrefix = true;
        segPrefixVal = (byte1 & 0b00011000) >> 3;
        return;
    }

    // printf("%d", linenum++);
    // printf("%X", byte1);

    if (dst.tag == ARG_NONE)
        printf("\t%s%s\r\n",
               lockPrefix ? "lock " : "" ,
               getOpStr(byte1, opByte));
    else if (src.tag == ARG_NONE)
        printf("\t%s%s %s\r\n",
               lockPrefix ? "lock " : "",
               getOpStr(byte1, opByte),
               getArgStr(dst, true, wide));
    else
        printf("\t%s%s %s, %s\r\n",
               lockPrefix ? "lock " : "",
               getOpStr(byte1, opByte),
               getArgStr(dst, false, wide),
               getArgStr(src, false, wide));

    lockPrefix = false;
    segmentPrefix = false;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("No input specified!\n");
        exit(1);
    }

    initStrArena();

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Couldn't open file");
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    u32 fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    bytesRead = 0;

    printf("\tbits 16\r\n\r\n");
    linenum += 2;
    while (bytesRead < fileSize) {
        decodeNextOp();
    }

    fclose(fp);
    destroyStrArena();
}
