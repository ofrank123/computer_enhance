// Computer Enhance -- Exercise 01

#include "../common.h"

#define OPCODE_MOV_MASK      0b11111100
#define OPCODE_MOV           0b10001000

#define MOV_D_MASK           0b00000010
#define MOV_D_DEST           0b00000010
#define MOV_D_SRC            0b00000000

#define MOV_W_MASK           0b00000001
#define MOV_W_BYTE           0b00000000
#define MOV_W_WORD           0b00000001

#define MOV_MOD_MASK         0b11000000
#define MOV_MOD_MEM          0b00000000
#define MOV_MOD_MEM8         0b01000000
#define MOV_MOD_MEM16        0b10000000
#define MOV_MOD_REG          0b11000000

#define MOV_REG_MASK         0b00111000

#define MOV_REG_AL_AX        0b00000000
#define MOV_REG_CL_CX        0b00001000
#define MOV_REG_DL_DX        0b00010000
#define MOV_REG_BL_BX        0b00011000
#define MOV_REG_AH_SP        0b00100000
#define MOV_REG_CH_BP        0b00101000
#define MOV_REG_DH_SI        0b00110000
#define MOV_REG_BH_DI        0b00111000

#define MOV_RM_MASK          0b00000111
#define MOV_RM_AL_AX         0b00000000
#define MOV_RM_CL_CX         0b00000001
#define MOV_RM_DL_DX         0b00000010
#define MOV_RM_BL_BX         0b00000011
#define MOV_RM_AH_SP         0b00000100
#define MOV_RM_CH_BP         0b00000101
#define MOV_RM_DH_SI         0b00000110
#define MOV_RM_BH_DI         0b00000111

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

void printReg(Reg reg) {
	switch (reg) {
		case REG_AX: printf("ax"); break;
		case REG_BX: printf("bx"); break;
		case REG_CX: printf("cx"); break;
		case REG_DX: printf("dx"); break;
		case REG_AL: printf("al"); break;
		case REG_BL: printf("bl"); break;
		case REG_CL: printf("cl"); break;
		case REG_DL: printf("dl"); break;
		case REG_AH: printf("ah"); break;
		case REG_BH: printf("bh"); break;
		case REG_CH: printf("ch"); break;
		case REG_DH: printf("dh"); break;
		case REG_SP: printf("sp"); break;
		case REG_BP: printf("bp"); break;
		case REG_SI: printf("si"); break;
		case REG_DI: printf("di"); break;
	}
}

u32 decodeNextOp(FILE *fp) {
	u32 bytesRead = 0;
	u8 byte;
	fread(&byte, 1, 1, fp);
	bytesRead++;

	if ((byte & OPCODE_MOV_MASK) == OPCODE_MOV) { // MOV - Reg/mem to/from reg
			u8 regDestOrSrc = byte & MOV_D_MASK;
		    u8 width      = byte & MOV_W_MASK;

			fread(&byte, 1, 1, fp);
			bytesRead++;

			u8 mode = byte & MOV_MOD_MASK;
			u8 reg = byte & MOV_REG_MASK;
			u8 regMem = byte & MOV_RM_MASK;

			if (mode == MOV_MOD_REG) { // Register to Register move
				Reg regReg; // Register from "reg" field in mov
				switch (reg) {
					case MOV_REG_AL_AX: regReg = width == MOV_W_BYTE ? REG_AL : REG_AX; break;
					case MOV_REG_CL_CX: regReg = width == MOV_W_BYTE ? REG_CL : REG_CX; break;
					case MOV_REG_DL_DX: regReg = width == MOV_W_BYTE ? REG_DL : REG_DX; break;
					case MOV_REG_BL_BX: regReg = width == MOV_W_BYTE ? REG_BL : REG_BX; break;
					case MOV_REG_AH_SP: regReg = width == MOV_W_BYTE ? REG_AH : REG_SP; break;
					case MOV_REG_CH_BP: regReg = width == MOV_W_BYTE ? REG_CH : REG_BP; break;
					case MOV_REG_DH_SI: regReg = width == MOV_W_BYTE ? REG_DH : REG_SI; break;
					case MOV_REG_BH_DI: regReg = width == MOV_W_BYTE ? REG_BH : REG_DI; break;
				}

				Reg rmReg;
				switch (regMem) {
					case MOV_RM_AL_AX: rmReg = width == MOV_W_BYTE ? REG_AL : REG_AX; break;
					case MOV_RM_CL_CX: rmReg = width == MOV_W_BYTE ? REG_CL : REG_CX; break;
					case MOV_RM_DL_DX: rmReg = width == MOV_W_BYTE ? REG_DL : REG_DX; break;
					case MOV_RM_BL_BX: rmReg = width == MOV_W_BYTE ? REG_BL : REG_BX; break;
					case MOV_RM_AH_SP: rmReg = width == MOV_W_BYTE ? REG_AH : REG_SP; break;
					case MOV_RM_CH_BP: rmReg = width == MOV_W_BYTE ? REG_CH : REG_BP; break;
					case MOV_RM_DH_SI: rmReg = width == MOV_W_BYTE ? REG_DH : REG_SI; break;
					case MOV_RM_BH_DI: rmReg = width == MOV_W_BYTE ? REG_BH : REG_DI; break;
				}

				Reg dstReg;
				Reg srcReg;
				if (regDestOrSrc == MOV_D_SRC) {
					srcReg = regReg;
					dstReg = rmReg;
				} else {
					srcReg = rmReg;
					dstReg = regReg;
				}

				printf("\tmov ");
				printReg(dstReg);
				printf(", ");
				printReg(srcReg);
				printf("\r\n");
			} else {
				printf("Unsupported mov mode!\n");
				exit(1);
			}
	} else {
		printf("Unsupported Opcode!\n");
		exit(1);
	}

	return bytesRead;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("No input specified!\n");
		exit(1);
	}

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
}
