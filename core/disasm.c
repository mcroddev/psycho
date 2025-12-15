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

#include <stdio.h>
#include <string.h>

#include "core/cpu-defs.h"
#include "cpu-defs.h"

#include "disasm.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_DISASM);

static const char *const gpr[CPU_GPR_NUM] = {
	// clang-format off

	[CPU_GPR_ZERO]	= "$zero",
	[CPU_GPR_AT]	= "$at",
	[CPU_GPR_V0]	= "$v0",
	[CPU_GPR_V1]	= "$v1",
	[CPU_GPR_A0]	= "$a0",
	[CPU_GPR_A1]	= "$a1",
	[CPU_GPR_A2]	= "$a2",
	[CPU_GPR_A3]	= "$a3",
	[CPU_GPR_T0]	= "$t0",
	[CPU_GPR_T1]	= "$t1",
	[CPU_GPR_T2]	= "$t2",
	[CPU_GPR_T3]	= "$t3",
	[CPU_GPR_T4]	= "$t4",
	[CPU_GPR_T5]	= "$t5",
	[CPU_GPR_T6]	= "$t6",
	[CPU_GPR_T7]	= "$t7",
	[CPU_GPR_S0]	= "$s0",
	[CPU_GPR_S1]	= "$s1",
	[CPU_GPR_S2]	= "$s2",
	[CPU_GPR_S3]	= "$s3",
	[CPU_GPR_S4]	= "$s4",
	[CPU_GPR_S5]	= "$s5",
	[CPU_GPR_S6]	= "$s6",
	[CPU_GPR_S7]	= "$s7",
	[CPU_GPR_T8]	= "$t8",
	[CPU_GPR_T9]	= "$t9",
	[CPU_GPR_K0]	= "$k0",
	[CPU_GPR_K1]	= "$k1",
	[CPU_GPR_GP]	= "$gp",
	[CPU_GPR_SP]	= "$sp",
	[CPU_GPR_FP]	= "$fp",
	[CPU_GPR_RA]	= "$ra"

	// clang-format on
};

static const char *const cop0_cpr[CPU_COP0_NUM] = {
	// clang-format off

	[CPU_COP0_BPC]		= "BPC",
	[CPU_COP0_BDA]		= "BDA",
	[CPU_COP0_TAR]		= "TAR",
	[CPU_COP0_DCIC]		= "DCIC",
	[CPU_COP0_BADA]		= "BadA",
	[CPU_COP0_BDAM]		= "BDAM",
	[CPU_COP0_BPCM]		= "BPCM",
	[CPU_COP0_SR]		= "SR",
	[CPU_COP0_CAUSE]	= "CAUSE",
	[CPU_COP0_PRID]		= "PRID",

	// clang-format on
};

void psycho_disasm_trace_instruction_enable(struct psycho_ctx *const ctx,
					    const bool enable)
{
	ctx->disasm.trace_instruction = enable;
}

void psycho_disasm_instr(struct psycho_ctx *const ctx, const u32 instr,
			 const u32 pc)
{
#define set_result(n, ...)       \
	ctx->disasm.result.len = \
		sprintf(ctx->disasm.result.str, (n), ##__VA_ARGS__)

#define op (psycho_cpu_instr_get_op(instr))
#define rt (psycho_cpu_instr_get_rt(instr))
#define rd (psycho_cpu_instr_get_rd(instr))
#define rs (psycho_cpu_instr_get_rs(instr))
#define funct (psycho_cpu_instr_get_funct(instr))
#define shamt (psycho_cpu_instr_get_shamt(instr))
#define base (rs)
#define imm (psycho_cpu_instr_get_immediate(instr))

	switch (op) {
	case CPU_INSTR_GROUP_SPECIAL:
		switch (funct) {
		case CPU_INSTR_SLL:
			if (instr == 0x00000000)
				set_result("nop");
			else
				set_result("sll %s, %s, %u", gpr[rd], gpr[rt],
					   shamt);
			return;

		case CPU_INSTR_SRL:
			set_result("srl %s, %s, %u", gpr[rd], gpr[rt], shamt);
			return;

		case CPU_INSTR_SRA:
			set_result("sra %s, %s, %u", gpr[rd], gpr[rt], shamt);
			return;

		case CPU_INSTR_JR:
			set_result("jr %s", gpr[rs]);
			return;

		case CPU_INSTR_JALR:
			set_result("jalr %s, %s", gpr[rd], gpr[rs]);
			return;

		case CPU_INSTR_MFHI:
			set_result("mfhi %s", gpr[rd]);
			return;

		case CPU_INSTR_MFLO:
			set_result("mflo %s", gpr[rd]);
			return;

		case CPU_INSTR_DIV:
			set_result("div %s, %s", gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_DIVU:
			set_result("div %s, %s", gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_ADD:
			set_result("add %s, %s, %s", gpr[rd], gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_ADDU:
			set_result("addu %s, %s, %s", gpr[rd], gpr[rs],
				   gpr[rt]);
			return;

		case CPU_INSTR_SUBU:
			set_result("subu %s, %s, %s", gpr[rd], gpr[rs],
				   gpr[rt]);
			return;

		case CPU_INSTR_AND:
			set_result("and %s, %s, %s", gpr[rd], gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_OR:
			set_result("or %s, %s, %s", gpr[rd], gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_SLT:
			set_result("slt %s, %s, %s", gpr[rd], gpr[rs], gpr[rt]);
			return;

		case CPU_INSTR_SLTU:
			set_result("sltu %s, %s, %s", gpr[rd], gpr[rs],
				   gpr[rt]);
			return;

		default:
			break;
		}
		break;

	case CPU_INSTR_GROUP_BCOND:
		switch (rt) {
		case CPU_INSTR_BLTZ:
			set_result("bltz %s, 0x%08X", gpr[rs],
				   psycho_cpu_calc_branch_addr(instr, pc));
			return;

		case CPU_INSTR_BGEZ:
			set_result("bgez %s, 0x%08X", gpr[rs],
				   psycho_cpu_calc_branch_addr(instr, pc));
			return;

		default:
			break;
		}
		break;

	case CPU_INSTR_GROUP_COP0:
		switch (rs) {
		case CPU_INSTR_COP_MF:
			set_result("mfc0 %s, %s", gpr[rt], cop0_cpr[rd]);
			return;

		case CPU_INSTR_COP_MT:
			set_result("mtc0 %s, %s", cop0_cpr[rd], gpr[rt]);
			return;

		default:
			break;
		}
		break;

	case CPU_INSTR_J:
		set_result("j 0x%08X", psycho_cpu_calc_jmp_addr(instr, pc));
		return;

	case CPU_INSTR_JAL:
		set_result("jal 0x%08X", psycho_cpu_calc_jmp_addr(instr, pc));
		return;

	case CPU_INSTR_BEQ:
		set_result("beq %s, %s, 0x%08X", gpr[rs], gpr[rt],
			   psycho_cpu_calc_branch_addr(instr, pc));
		return;

	case CPU_INSTR_BNE:
		set_result("bne %s, %s, 0x%08X", gpr[rs], gpr[rt],
			   psycho_cpu_calc_branch_addr(instr, pc));
		return;

	case CPU_INSTR_BLEZ:
		set_result("blez %s, %s, 0x%08X", gpr[rs], gpr[rt],
			   psycho_cpu_calc_branch_addr(instr, pc));
		return;

	case CPU_INSTR_BGTZ:
		set_result("bgtz %s, %s, 0x%08X", gpr[rs], gpr[rt],
			   psycho_cpu_calc_branch_addr(instr, pc));
		return;

	case CPU_INSTR_ADDI:
		set_result("addi %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_ADDIU:
		set_result("addiu %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_SLTI:
		set_result("slti %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_SLTIU:
		set_result("sltiu %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_ANDI:
		set_result("andi %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_ORI:
		set_result("ori %s, %s, 0x%04X", gpr[rt], gpr[rs], imm);
		return;

	case CPU_INSTR_LUI:
		set_result("lui %s, 0x%04X", gpr[rt], imm);
		return;

	case CPU_INSTR_LB:
		set_result("lb %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	case CPU_INSTR_LW:
		set_result("lw %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	case CPU_INSTR_LBU:
		set_result("lbu %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	case CPU_INSTR_SB:
		set_result("sb %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	case CPU_INSTR_SH:
		set_result("sh %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	case CPU_INSTR_SW:
		set_result("sw %s, 0x%04X(%s)", gpr[rt], imm, gpr[base]);
		return;

	default:
		break;
	}
	set_result("illegal 0x%08X", instr);

#undef set_result
}

void psycho_disasm_trace_begin(struct psycho_ctx *const ctx)
{
	memset(&ctx->disasm.result, 0, sizeof(ctx->disasm.result));

	ctx->disasm.result.pc = ctx->cpu.pc;
	const u32 paddr = psycho_cpu_translate_vaddr_to_paddr(ctx->cpu.pc);
	const u32 instr = psycho_bus_peek_word(ctx, paddr);
	psycho_disasm_instr(ctx, instr, ctx->cpu.pc);
}

void psycho_disasm_trace_end(struct psycho_ctx *const ctx)
{
	LOG_INFO(ctx, "0x%08X: %s", ctx->disasm.result.pc,
		 ctx->disasm.result.str);
}
