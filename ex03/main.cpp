// Computer Enhance -- Exercise 01

#include "../common.h"

#define OPCODE_MOV_RMTFR_MASK 0b11111100
#define OPCODE_MOV_RMTFR      0b10001000

#define MOV_D_MASK           0b00000010
#define MOV_D_DEST           0b00000010
#define MOV_D_SRC            0b00000000

#define MOV_W_MASK           0b00000001

#define MOV_MOD_MASK         0b11000000

// Mode Decoding
#define MOV_MOD_MEM          0b00000000
#define MOV_MOD_MEM8         0b01000000
#define MOV_MOD_MEM16        0b10000000
#define MOV_MOD_REG          0b11000000

#define MOV_REG_MASK         0b00111000
#define MOV_RM_MASK          0b00000111

#define MOV_EAC_BP           0b00000110

// Immediate to Register
#define OPCODE_MOV_ITR_MASK  0b11110000
#define OPCODE_MOV_ITR       0b10110000

#define MOV_ITR_W_MASK       0b00001000
#define MOV_ITR_REG_MASK     0b00000111

// Immediate to Register/Memory
#define OPCODE_MOV_ITRM_MASK 0b11111110 
#define OPCODE_MOV_ITRM      0b11000110

#define MOV_ITRM_W_MASK      0b00000001
#define MOV_ITRM_MOD_MASK    0b11000000
#define MOV_ITRM_RM_MASK     0b00000111

// Memory to Accumulator
#define OPCODE_MOV_MTA_MASK  0b11111110
#define OPCODE_MOV_MTA       0b10100000

// Accumulator to Memory
#define OPCODE_MOV_ATM_MASK  0b11111110
#define OPCODE_MOV_ATM       0b10100010

FILE *fp;
u32 bytesRead;

enum Reg {
	REG_AX = 0, REG_AL, REG_AH,
	REG_BX, REG_BL, REG_BH,
	REG_CX, REG_CL, REG_CH,
	REG_DX, REG_DL, REG_DH,
	REG_SP,
	REG_BP,
	REG_SI,
	REG_DI,
};

enum ArgTag {
	ARG_NONE = 0,
	ARG_REG,
	ARG_DIR,
	ARG_DIS,
	ARG_DIS8,
	ARG_DIS16,
	ARG_IMM
};

struct DispData {
	u8 eac;
	union {
		u8 d8;
		u16 d16;
	} disp;
};

struct ImmData {
	bool wide;
	u16 val;
};

struct Arg {
	ArgTag tag;

	union {
		Reg reg;
		u16 dir;
		DispData dis;
		ImmData imm;
	} v;
};

const char *getRegStr(Reg reg) {
	switch (reg) {
		case REG_AX: return "ax";
		case REG_BX: return "bx";
		case REG_CX: return "cx";
		case REG_DX: return "dx";
		case REG_AL: return "al";
		case REG_BL: return "bl";
		case REG_CL: return "cl";
		case REG_DL: return "dl";
		case REG_AH: return "ah";
		case REG_BH: return "bh";
		case REG_CH: return "ch";
		case REG_DH: return "dh";
		case REG_SP: return "sp";
		case REG_BP: return "bp";
		case REG_SI: return "si";
		case REG_DI: return "di";
	}
}

Reg decodeReg(u8 regCode, bool word) {
	switch (regCode) {
		case 0b00000000: return word ? REG_AX : REG_AL;
		case 0b00000001: return word ? REG_CX : REG_CL;
		case 0b00000010: return word ? REG_DX : REG_DL;
		case 0b00000011: return word ? REG_BX : REG_BL;
		case 0b00000100: return word ? REG_SP : REG_AH;
		case 0b00000101: return word ? REG_BP : REG_CH;
		case 0b00000110: return word ? REG_SI : REG_DH;
		case 0b00000111: return word ? REG_DI : REG_BH;
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

char *getSignedDispStr(u8 disp) {
	char *str = allocStr(8);
	if (disp & (1 << 7)) {
		u8 neg = (~disp) + 1;
		sprintf(str, "- %d", neg);
	} else {
		sprintf(str, "+ %d", disp);
	}

	return str;
}

char *getSignedDispStr(u16 disp) {
	char *str = allocStr(16);
	if (disp & (1 << 15)) {
		u16 neg = (~disp) + 1;
		sprintf(str, "- %d", neg);
	} else {
		sprintf(str, "+ %d", disp);
	}

	return str;
}

char *getPrefixedImmStr(u16 immVal, bool wide) {
	char *str = allocStr(16);
	if (wide) {
		sprintf(str, "word %d", immVal);
	} else {
		sprintf(str, "byte %d", immVal);
	}
	return str;
}

char *getArgStr(Arg arg) {
	char *str = allocStr(32);
	switch (arg.tag) {
		case ARG_REG: return (char *) getRegStr(arg.v.reg);
		case ARG_IMM: return getPrefixedImmStr(arg.v.imm.val, arg.v.imm.wide);
		case ARG_DIS:
			sprintf(str, "[%s]", getEacStr(arg.v.dis.eac));
			break;
		case ARG_DIS8:
			sprintf(str, "[%s %s]",
					getEacStr(arg.v.dis.eac),
					getSignedDispStr(arg.v.dis.disp.d8));
			break;
		case ARG_DIS16:
			sprintf(str, "[%s %s]",
					getEacStr(arg.v.dis.eac),
					getSignedDispStr(arg.v.dis.disp.d16));
			break;
		case ARG_DIR:
			sprintf(str, "[%d]", arg.v.dir);
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

void decodeNextOp() {
	u8 byte1 = readData(false);

	char op[5];
	Arg dst{};
	Arg src{};

	if ((byte1 & OPCODE_MOV_RMTFR_MASK) == OPCODE_MOV_RMTFR ||
		(byte1 & OPCODE_MOV_ITRM_MASK) == OPCODE_MOV_ITRM) {
		strcpy(op, "mov");
		u8 byte2 = readData(false);

		bool immediate = (byte1 & OPCODE_MOV_ITRM_MASK) == OPCODE_MOV_ITRM;

		bool wide = byte1 & MOV_ITRM_W_MASK;
		u8 mode = byte2 & MOV_MOD_MASK;
		u8 reg = (byte2 & MOV_REG_MASK) >> 3;
		u8 rm = byte2 & MOV_RM_MASK;

		if (immediate) {
			src.tag = ARG_IMM;
		} else if ((byte1 & MOV_D_MASK) == MOV_D_SRC) {
			src.tag = ARG_REG;
			src.v.reg = decodeReg(reg, wide);
			if (mode == MOV_MOD_REG) {
				dst.tag = ARG_REG;
				dst.v.reg = decodeReg(rm, wide);
			}
		}

		if ((byte1 & MOV_D_MASK) == MOV_D_DEST) {
			dst.tag = ARG_REG;
			dst.v.reg = decodeReg(reg, wide);
			if (!immediate && mode == MOV_MOD_REG) {
				src.tag = ARG_REG;
				src.v.reg = decodeReg(rm, wide);
			}
		}

		if (mode != MOV_MOD_REG) {
			Arg memArg{};
			if (mode == MOV_MOD_MEM && rm == MOV_EAC_BP) { // Direct Addressing
				memArg.tag = ARG_DIR;
				memArg.v.dir = readData(true);
			} else {
				memArg.v.dis.eac = rm;
				if (mode == MOV_MOD_MEM) {
					memArg.tag = ARG_DIS;
				} else if (mode == MOV_MOD_MEM8) {
					memArg.tag = ARG_DIS8;
					memArg.v.dis.disp.d8 = readData(false);
				} else if (mode == MOV_MOD_MEM16) {
					memArg.tag = ARG_DIS16;
					memArg.v.dis.disp.d16 = readData(true);
				}
			}
			if (immediate || dst.tag == ARG_NONE) dst = memArg;
			else src = memArg;
		}

		if (immediate) {
			src.v.imm.wide = wide;
			src.v.imm.val = readData(wide);
		}
	} else if ((byte1 & OPCODE_MOV_ITR_MASK) == OPCODE_MOV_ITR) {
		strcpy(op, "mov");
		bool wide = byte1 & MOV_ITR_W_MASK;
		dst.tag = ARG_REG;
		dst.v.reg = decodeReg(byte1 & MOV_ITR_REG_MASK, wide);
		src.tag = ARG_IMM;
		src.v.imm.wide = wide;
		src.v.imm.val = readData(wide);
	} else if ((byte1 & OPCODE_MOV_MTA_MASK) == OPCODE_MOV_MTA) {
		strcpy(op, "mov");
		dst.tag = ARG_REG;
		dst.v.reg = REG_AX;
		src.tag = ARG_DIR;
		src.v.dir = readData(byte1 & MOV_W_MASK);
	} else if ((byte1 & OPCODE_MOV_ATM_MASK) == OPCODE_MOV_ATM) {
		strcpy(op, "mov");
		dst.tag = ARG_DIR;
		dst.v.dir = readData(byte1 & MOV_W_MASK);
		src.tag = ARG_REG;
		src.v.reg = REG_AX;
	} else {
		printf("Unsupported Operation!\n");
		exit(1);
	}

	printf("\t%s %s, %s\r\n", op, getArgStr(dst), getArgStr(src));
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
	while (bytesRead < fileSize) {
		decodeNextOp();
	}

	fclose(fp);
	destroyStrArena();
}
