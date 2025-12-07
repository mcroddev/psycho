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

enum psycho_cpu_instr_op {
	CPU_INSTR_GROUP_SPECIAL = 0x00,
	CPU_INSTR_GROUP_BCOND = 0x01,
	CPU_INSTR_GROUP_COP0 = 0x10,
	CPU_INSTR_J = 0x02,
	CPU_INSTR_JAL = 0x03,
	CPU_INSTR_BEQ = 0x04,
	CPU_INSTR_BNE = 0x05,
	CPU_INSTR_BLEZ = 0x06,
	CPU_INSTR_BGTZ = 0x07,
	CPU_INSTR_ADDI = 0x08,
	CPU_INSTR_ADDIU = 0x09,
	CPU_INSTR_SLTI = 0x0A,
	CPU_INSTR_SLTIU = 0x0B,
	CPU_INSTR_ANDI = 0x0C,
	CPU_INSTR_ORI = 0x0D,
	CPU_INSTR_LUI = 0x0F,
	CPU_INSTR_LB = 0x20,
	CPU_INSTR_LW = 0x23,
	CPU_INSTR_LBU = 0x24,
	CPU_INSTR_SB = 0x28,
	CPU_INSTR_SH = 0x29,
	CPU_INSTR_SW = 0x2B
};

enum psycho_cpu_instr_special {
	CPU_INSTR_SLL = 0x00,
	CPU_INSTR_SRL = 0x02,
	CPU_INSTR_SRA = 0x03,
	CPU_INSTR_JR = 0x08,
	CPU_INSTR_JALR = 0x09,
	CPU_INSTR_MFHI = 0x10,
	CPU_INSTR_MFLO = 0x12,
	CPU_INSTR_DIV = 0x1A,
	CPU_INSTR_DIVU = 0x1B,
	CPU_INSTR_ADD = 0x20,
	CPU_INSTR_ADDU = 0x21,
	CPU_INSTR_SUBU = 0x23,
	CPU_INSTR_AND = 0x24,
	CPU_INSTR_OR = 0x25,
	CPU_INSTR_SLT = 0x2A,
	CPU_INSTR_SLTU = 0x2B
};

enum psycho_cpu_instr_bcond {
	CPU_INSTR_BLTZ = 0x00,
	CPU_INSTR_BGEZ = 0x01,
};

enum {
	CPU_INSTR_COP_MF = 0x00,
	CPU_INSTR_COP_MT = 0x04,
};

enum {
	CPU_SR_ISC = 1 << 16,
};

ALWAYS_INLINE uint psycho_cpu_instr_get_op(const u32 instr)
{
	return instr >> 26;
}

ALWAYS_INLINE uint psycho_cpu_instr_get_rs(const u32 instr)
{
	return (instr >> 21) & 0x1F;
}

ALWAYS_INLINE uint psycho_cpu_instr_get_rt(const u32 instr)
{
	return (instr >> 16) & 0x1F;
}

ALWAYS_INLINE uint psycho_cpu_instr_get_rd(const u32 instr)
{
	return (instr >> 11) & 0x1F;
}

ALWAYS_INLINE uint psycho_cpu_instr_get_shamt(const u32 instr)
{
	return (instr >> 6) & 0x1F;
}

ALWAYS_INLINE uint psycho_cpu_instr_get_funct(const u32 instr)
{
	return instr & 0x3F;
}

ALWAYS_INLINE u16 psycho_cpu_instr_get_immediate(const u32 instr)
{
	return instr & UINT16_MAX;
}

ALWAYS_INLINE u16 psycho_cpu_instr_get_offset(const u32 instr)
{
	return psycho_cpu_instr_get_immediate(instr);
}

ALWAYS_INLINE uint psycho_cpu_instr_get_target(const u32 instr)
{
	return instr & 0x03FFFFFF;
}

ALWAYS_INLINE u32 psycho_cpu_get_vaddr(const u16 offset, const u32 val)
{
	return psycho_sign_extend_16_32(offset) + val;
}

ALWAYS_INLINE u32 psycho_cpu_calc_jmp_addr(const u32 instr, const u32 pc)
{
	return (psycho_cpu_instr_get_target(instr) << 2) | (pc & 0xF0000000);
}

ALWAYS_INLINE u32 psycho_cpu_calc_branch_addr(const u32 instr, const u32 pc)
{
	const u16 imm = psycho_cpu_instr_get_immediate(instr);
	return psycho_sign_extend_16_32(imm << 2) + pc;
}

ALWAYS_INLINE u32 psycho_cpu_translate_vaddr_to_paddr(const u32 vaddr)
{
	if (unlikely(vaddr == 0xFFFE0130))
		return vaddr;

	return vaddr & 0x1FFFFFFF;
}
