// SPDX-License-Identifier: MIT
//
// Copyright 2025 Michael Rodriguez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "core/compiler.h"
#include "core/types.h"
#include "util.h"

#define CPU_DCIC_DR
#define CPU_DCIC_PC
#define CPU_DCIC_DE (1 << 23)
#define CPU_DCIC_DAE (1 << 25)

enum instr_group_op {
	INSTR_GROUP_SPECIAL = 0x00,
	INSTR_GROUP_BCOND = 0x01,
	INSTR_GROUP_COP0 = 0x10,
	INSTR_J = 0x02,
	INSTR_JAL = 0x03,
	INSTR_BEQ = 0x04,
	INSTR_BNE = 0x05,
	INSTR_BLEZ = 0x06,
	INSTR_BGTZ = 0x07,
	INSTR_ADDI = 0x08,
	INSTR_ADDIU = 0x09,
	INSTR_SLTI = 0x0A,
	INSTR_SLTIU = 0x0B,
	INSTR_ANDI = 0x0C,
	INSTR_ORI = 0x0D,
	INSTR_XORI = 0x0E,
	INSTR_LUI = 0x0F,
	INSTR_LB = 0x20,
	INSTR_LH = 0x21,
	INSTR_LWL = 0x22,
	INSTR_LW = 0x23,
	INSTR_LBU = 0x24,
	INSTR_LHU = 0x25,
	INSTR_LWR = 0x26,
	INSTR_SB = 0x28,
	INSTR_SH = 0x29,
	INSTR_SWL = 0x2A,
	INSTR_SW = 0x2B,
	INSTR_SWR = 0x2E
};

enum instr_group_special {
	INSTR_SLL = 0x00,
	INSTR_SRL = 0x02,
	INSTR_SRA = 0x03,
	INSTR_SLLV = 0x04,
	INSTR_SRLV = 0x06,
	INSTR_SRAV = 0x07,
	INSTR_JR = 0x08,
	INSTR_JALR = 0x09,
	INSTR_SYSCALL = 0x0C,
	INSTR_BREAK = 0x0D,
	INSTR_MFHI = 0x10,
	INSTR_MTHI = 0x11,
	INSTR_MFLO = 0x12,
	INSTR_MTLO = 0x13,
	INSTR_MULT = 0x18,
	INSTR_MULTU = 0x19,
	INSTR_DIV = 0x1A,
	INSTR_DIVU = 0x1B,
	INSTR_ADD = 0x20,
	INSTR_ADDU = 0x21,
	INSTR_SUB = 0x22,
	INSTR_SUBU = 0x23,
	INSTR_AND = 0x24,
	INSTR_OR = 0x25,
	INSTR_XOR = 0x26,
	INSTR_NOR = 0x27,
	INSTR_SLT = 0x2A,
	INSTR_SLTU = 0x2B
};

enum instr_group_bcond {
	INSTR_BLTZ = 0x00,
	INSTR_BGEZ = 0x01,
	INSTR_BLTZAL = 0x10,
};

enum instr_group_cop {
	INSTR_COP_MF = 0x00,
	INSTR_COP_MT = 0x04,
};

enum instr_group_cop0 {
	INSTR_RFE = 0x10,
};

enum cpu_exc_code {
	EXCEPTION_ADEL = 4,
	EXCEPTION_ADES = 5,
	EXCEPTION_SYS = 8,
	EXCEPTION_BP = 9,
	EXCEPTION_OV = 12,
};

enum {
	CPU_SR_ISC = 1 << 16,
};

enum {
	CAUSE_EXC_CODE_MASK = 0x0000007C,
};

ALWAYS_INLINE uint instr_op(const u32 instr)
{
	return instr >> 26;
}

ALWAYS_INLINE uint instr_rs(const u32 instr)
{
	return (instr >> 21) & 0x1F;
}

ALWAYS_INLINE uint instr_rt(const u32 instr)
{
	return (instr >> 16) & 0x1F;
}

ALWAYS_INLINE uint instr_rd(const u32 instr)
{
	return (instr >> 11) & 0x1F;
}

ALWAYS_INLINE uint instr_shamt(const u32 instr)
{
	return (instr >> 6) & 0x1F;
}

ALWAYS_INLINE uint instr_funct(const u32 instr)
{
	return instr & 0x3F;
}

ALWAYS_INLINE u16 instr_imm(const u32 instr)
{
	return instr & UINT16_MAX;
}

ALWAYS_INLINE u16 instr_off(const u32 instr)
{
	return instr_imm(instr);
}

ALWAYS_INLINE uint instr_target(const u32 instr)
{
	return instr & 0x03FFFFFF;
}

ALWAYS_INLINE u32 get_vaddr(const u16 off, const u32 val)
{
	return sign_ext_16_32(off) + val;
}

ALWAYS_INLINE u32 calc_jmp_addr(const u32 instr, const u32 pc)
{
	return (instr_target(instr) << 2) | (pc & 0xF0000000);
}

ALWAYS_INLINE u32 calc_branch_addr(const u32 instr, const u32 pc)
{
	const u16 imm = instr_imm(instr);
	return sign_ext_16_32(imm << 2) + pc + sizeof(u32);
}

ALWAYS_INLINE u32 vaddr_to_paddr(const u32 vaddr)
{
	if (unlikely(vaddr == 0xFFFE0130))
		return vaddr;

	return vaddr & 0x1FFFFFFF;
}
