// Computer Enhance -- Exercise 01

#include "../common.h"

#define READ_BYTE fread(&data, 1, 1, fp); bytesRead++

#define OPCODE_MOV_MASK      0b11111100
#define OPCODE_MOV           0b10001000

#define MOV_D_MASK           0b00000010
#define MOV_D_DEST           0b00000010
#define MOV_D_SRC            0b00000000

#define MOV_W_MASK           0b00000001
#define MOV_W_BYTE           0b00000000
#define MOV_W_WORD           0b00000001

#define MOV_MOD_MASK         0b11000000

// Mode Decoding
#define MOV_MOD_MEM          0b00000000
#define MOV_MOD_MEM8         0b01000000
#define MOV_MOD_MEM16        0b10000000
#define MOV_MOD_REG          0b11000000

#define MOV_REG_MASK         0b00111000
#define MOV_RM_MASK          0b00000111

// Register Decoding
#define MOV_REG_AL_AX        0b00000000
#define MOV_REG_CL_CX        0b00000001
#define MOV_REG_DL_DX        0b00000010
#define MOV_REG_BL_BX        0b00000011
#define MOV_REG_AH_SP        0b00000100
#define MOV_REG_CH_BP        0b00000101
#define MOV_REG_DH_SI        0b00000110
#define MOV_REG_BH_DI        0b00000111

// Effective Address Decoding
#define MOV_EAC_BX_SI        0b00000000
#define MOV_EAC_BX_DI        0b00000001
#define MOV_EAC_BP_SI        0b00000010
#define MOV_EAC_BP_DI        0b00000011
#define MOV_EAC_SI           0b00000100
#define MOV_EAC_DI           0b00000101
#define MOV_EAC_BP           0b00000110
#define MOV_EAC_BX           0b00000111

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

#define MOV_MTA_ATM_W_MASK   0b00000001

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

char *getRegStr(Reg reg) {
	char *regStr = allocStr(3);
	switch (reg) {
		case REG_AX: strcpy(regStr, "ax"); break;
		case REG_BX: strcpy(regStr, "bx"); break;
		case REG_CX: strcpy(regStr, "cx"); break;
		case REG_DX: strcpy(regStr, "dx"); break;
		case REG_AL: strcpy(regStr, "al"); break;
		case REG_BL: strcpy(regStr, "bl"); break;
		case REG_CL: strcpy(regStr, "cl"); break;
		case REG_DL: strcpy(regStr, "dl"); break;
		case REG_AH: strcpy(regStr, "ah"); break;
		case REG_BH: strcpy(regStr, "bh"); break;
		case REG_CH: strcpy(regStr, "ch"); break;
		case REG_DH: strcpy(regStr, "dh"); break;
		case REG_SP: strcpy(regStr, "sp"); break;
		case REG_BP: strcpy(regStr, "bp"); break;
		case REG_SI: strcpy(regStr, "si"); break;
		case REG_DI: strcpy(regStr, "di"); break;
	}

	return regStr;
}

Reg decodeReg(u8 regCode, bool word) {
	switch (regCode) {
		case MOV_REG_AL_AX: return word ? REG_AX : REG_AL;
		case MOV_REG_CL_CX: return word ? REG_CX : REG_CL;
		case MOV_REG_DL_DX: return word ? REG_DX : REG_DL;
		case MOV_REG_BL_BX: return word ? REG_BX : REG_BL;
		case MOV_REG_AH_SP: return word ? REG_SP : REG_AH;
		case MOV_REG_CH_BP: return word ? REG_BP : REG_CH;
		case MOV_REG_DH_SI: return word ? REG_SI : REG_DH;
		case MOV_REG_BH_DI: return word ? REG_DI : REG_BH;
	}
	printf("Invalid Reg Code\n");
	exit(1);
}

char *getEacStr(u8 eac) {
	char *eacStr = allocStr(8);
	switch (eac) {
		case MOV_EAC_BX_SI: strcpy(eacStr, "bx + si"); break;
		case MOV_EAC_BX_DI: strcpy(eacStr, "bx + di"); break;
		case MOV_EAC_BP_SI: strcpy(eacStr, "bp + si"); break;
		case MOV_EAC_BP_DI: strcpy(eacStr, "bp + di"); break;
		case MOV_EAC_SI:    strcpy(eacStr, "si"); break;
		case MOV_EAC_DI:    strcpy(eacStr, "di"); break;
		case MOV_EAC_BP:    strcpy(eacStr, "bp"); break;
		case MOV_EAC_BX:    strcpy(eacStr, "bx"); break;
	}

	return eacStr;
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

u32 decodeNextOp(FILE *fp) {
	u32 bytesRead = 0;
	u8 data;
	READ_BYTE;

	if ((data & OPCODE_MOV_MASK) == OPCODE_MOV // MOV - Reg/mem to/from reg
		|| (data & OPCODE_MOV_ITRM_MASK) == OPCODE_MOV_ITRM) {
		bool immediate = (data & OPCODE_MOV_ITRM_MASK) == OPCODE_MOV_ITRM;

		u8 regDestOrSrc, mode, reg, rm;
		bool wide;

		if (immediate) {
			wide = (data & MOV_ITRM_W_MASK) == MOV_W_WORD;

			READ_BYTE;

			mode = data & MOV_ITRM_MOD_MASK;
			rm = data & MOV_ITRM_RM_MASK;
		} else {
			regDestOrSrc = data & MOV_D_MASK;
			wide = (data & MOV_W_MASK) == MOV_W_WORD;

			READ_BYTE;

			mode = data & MOV_MOD_MASK;
			reg = (data & MOV_REG_MASK) >> 3;
			rm = data & MOV_RM_MASK;
		}

		if (mode == MOV_MOD_REG) { // Register to Register move
			if (immediate) {
				READ_BYTE;
				u16 immVal = data;
				if (wide) {
					READ_BYTE;
					immVal |= (u16) data << 8;
				}
				printf("\tmov %s, %s\r\n",
					   getRegStr(decodeReg(rm, wide)),
					   getPrefixedImmStr(immVal, wide));
			} else if (regDestOrSrc == MOV_D_SRC) {
				printf("\tmov %s, %s\r\n",
					   getRegStr(decodeReg(rm, wide)),
					   getRegStr(decodeReg(reg, wide)));
			} else {
				printf("\tmov %s, %s\r\n",
					   getRegStr(decodeReg(reg, wide)),
					   getRegStr(decodeReg(rm, wide)));
			}
		} else {
			u8 eac = rm;

			if (mode == MOV_MOD_MEM && eac == MOV_EAC_BP) { // DIRECT ADDRESSING
				READ_BYTE;
				u16 addr = data;
				READ_BYTE;
				addr |= ((u16) data) << 8;

				if (immediate) {
					READ_BYTE;
					u16 immVal = data;
					if (wide) {
						READ_BYTE;
						immVal |= (u16) data << 8;
					}
					printf("\tmov [%d], %s\r\n", addr, getPrefixedImmStr(immVal, wide));
				} else if (regDestOrSrc == MOV_D_SRC) {
					printf("\tmov [%d], %s\r\n", addr, getRegStr(decodeReg(reg, wide)));
				} else {
					printf("\tmov %s, [%d]\r\n", getRegStr(decodeReg(reg, wide)), addr);
				}
			}
			else if (mode == MOV_MOD_MEM) { // No-displacement addressing
				if (immediate) {
					READ_BYTE;
					u16 immVal = data;
					if (wide) {
						READ_BYTE;
						immVal |= (u16) data << 8;
					}
					printf("\tmov [%s], %s\r\n",
						   getEacStr(eac),
						   getPrefixedImmStr(immVal, wide));
				} else if (regDestOrSrc == MOV_D_SRC) {
					printf("\tmov [%s], %s\r\n",
						   getEacStr(eac),
						   getRegStr(decodeReg(reg, wide)));
				} else {
					printf("\tmov %s, [%s]\r\n",
						   getRegStr(decodeReg(reg, wide)),
						   getEacStr(eac));
				}
			}
			else if (mode == MOV_MOD_MEM8) { // 8 bit displacement
				READ_BYTE;
				u8 disp = data;

				if (immediate) {
					READ_BYTE;
					u16 immVal = data;
					if (wide) {
						READ_BYTE;
						immVal |= (u16) data << 8;
					}

					printf("\tmov [%s %s], %s\r\n",
						   getEacStr(eac),
						   getSignedDispStr(disp),
						   getPrefixedImmStr(immVal, wide));
				} else if (regDestOrSrc == MOV_D_SRC) {
					printf("\tmov [%s %s], %s\r\n",
						   getEacStr(eac),
						   getSignedDispStr(disp),
						   getRegStr(decodeReg(reg, wide)));
				} else {
					printf("\tmov %s, [%s %s]\r\n",
						   getRegStr(decodeReg(reg, wide)),
						   getEacStr(eac),
						   getSignedDispStr(disp));
				}
			}
			else { // 16 bit displacement
				READ_BYTE;
				u16 disp = data;
				READ_BYTE;
				disp |= ((u16) data) << 8;

				if (immediate) {
					READ_BYTE;
					u16 immVal = data;
					if (wide) {
						READ_BYTE;
						immVal |= (u16) data << 8;
					}

					printf("\tmov [%s %s], %s\r\n",
						   getEacStr(eac),
						   getSignedDispStr(disp),
						   getPrefixedImmStr(immVal, wide));
				} else if (regDestOrSrc == MOV_D_SRC) {
					printf("\tmov [%s %s], %s\r\n",
						   getEacStr(eac),
						   getSignedDispStr(disp),
						   getRegStr(decodeReg(reg, wide)));
				} else {
					printf("\tmov %s, [%s %s]\r\n",
						   getRegStr(decodeReg(reg, wide)),
						   getEacStr(eac),
						   getSignedDispStr(disp));
				}
			}
		}
	} else if ((data & OPCODE_MOV_ITR_MASK) == OPCODE_MOV_ITR) {
		bool wide = ((data & MOV_ITR_W_MASK) >> 3) == MOV_W_WORD;
		Reg dstReg = decodeReg(data & MOV_ITR_REG_MASK, wide);

		READ_BYTE;

		if (!wide) {
			u8 immVal = data;

			printf("\tmov %s, %d\r\n",
				   getRegStr(dstReg),
				   immVal);
		} else {
			u16 immVal = data;

			READ_BYTE;

			immVal |= ((u16) data) << 8;

			printf("\tmov %s, %d\r\n",
				   getRegStr(dstReg),
				   immVal);
		}
	} else if ((data & OPCODE_MOV_MTA_MASK) == OPCODE_MOV_MTA
			   || (data & OPCODE_MOV_ATM_MASK) == OPCODE_MOV_ATM) {
		bool axIsDest = (data & OPCODE_MOV_MTA_MASK) == OPCODE_MOV_MTA;
		bool wide = (data & MOV_MTA_ATM_W_MASK) == MOV_W_WORD;

		READ_BYTE;
		u16 addr = data;
		if (wide) {
			READ_BYTE;
			addr |= ((u16) data) << 8;
		}

		if (axIsDest) {
			printf("\tmov ax, [%d]\r\n", addr);
		} else {
			printf("\tmov [%d], ax\r\n", addr);
		}
	} else {
		printf("Unsupported Opcode! Byte: %X\n", data);
		exit(1);
	}

	return bytesRead;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("No input specified!\n");
		exit(1);
	}

	initStrArena();

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("Couldn't open file");
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	u32 fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	u32 bytesRead = 0;

	printf("\tbits 16\r\n\r\n");
	while (bytesRead < fileSize) {
		bytesRead += decodeNextOp(fp);
	}

	fclose(fp);
	destroyStrArena();
}
