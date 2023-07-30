#pragma once

#include "../common.h"

#include "sim86.h"
#include "instTable.h"

/**
 * Attempt to decode an instruction starting at `offset` in program memory,
 * and return the number of bytes consumed.
 */
u32 decodeNextInstr(Instr *instr, sim_ptr offset);
void dissassemble(char *inputPath);
