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

#include "core/cpu-defs.h"
#include "cpu-defs.h"

#include "cpu.h"
#include "bus.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_CPU);

static void illegal(struct psycho_ctx *const ctx)
{
	LOG_ERROR(ctx, "Illegal instruction trapped: 0x%08X", ctx->cpu.instr);
	ctx->event_cb(ctx, PSYCHO_CTX_EVENT_CPU_ILLEGAL, NULL);
}

static void branch_if(struct psycho_ctx *const ctx, const bool condition_met)
{
	if (condition_met)
		ctx->cpu.next_pc = psycho_cpu_calc_branch_addr(ctx->cpu.instr,
							       ctx->cpu.pc);
}

static u32 get_physical_address(struct psycho_ctx *const ctx)
{
	const u16 offset = psycho_cpu_instr_get_offset(ctx->cpu.instr);
	const uint base = psycho_cpu_instr_get_rs(ctx->cpu.instr);
	const u32 vaddr = psycho_cpu_get_vaddr(offset, ctx->cpu.gpr[base]);

	return psycho_cpu_translate_vaddr_to_paddr(vaddr);
}

void psycho_cpu_reset(struct psycho_ctx *const ctx)
{
	ctx->cpu.pc = RESET_PC;
	ctx->cpu.next_pc = RESET_PC;
}

void psycho_cpu_step(struct psycho_ctx *const ctx)
{
#define op (psycho_cpu_instr_get_op(ctx->cpu.instr))
#define rt (psycho_cpu_instr_get_rt(ctx->cpu.instr))
#define rd (psycho_cpu_instr_get_rd(ctx->cpu.instr))
#define rs (psycho_cpu_instr_get_rs(ctx->cpu.instr))
#define funct (psycho_cpu_instr_get_funct(ctx->cpu.instr))
#define shamt (psycho_cpu_instr_get_shamt(ctx->cpu.instr))
#define imm (psycho_cpu_instr_get_immediate(ctx->cpu.instr))
#define gpr (ctx->cpu.gpr)

	const u32 paddr = psycho_cpu_translate_vaddr_to_paddr(ctx->cpu.pc);
	ctx->cpu.instr = psycho_bus_load_word(ctx, paddr);

	ctx->cpu.pc = ctx->cpu.next_pc;
	ctx->cpu.next_pc += sizeof(u32);

	switch (op) {
	case CPU_INSTR_GROUP_SPECIAL:
		switch (funct) {
		case CPU_INSTR_SLL:
			gpr[rd] = gpr[rt] << shamt;
			break;

		case CPU_INSTR_SRL:
			gpr[rd] = gpr[rt] >> shamt;
			break;

		case CPU_INSTR_SRA:
			gpr[rd] = (s32)gpr[rt] >> shamt;
			break;

		case CPU_INSTR_JR:
			ctx->cpu.next_pc = gpr[rs];
			break;

		case CPU_INSTR_JALR:
			gpr[rd] = ctx->cpu.pc + 4;
			ctx->cpu.next_pc = gpr[rs];

			break;

		case CPU_INSTR_MFHI:
			gpr[rd] = ctx->cpu.alu_hi;
			break;

		case CPU_INSTR_MFLO:
			gpr[rd] = ctx->cpu.alu_lo;
			break;

		case CPU_INSTR_DIV:
			ctx->cpu.alu_lo = (s32)gpr[rs] / (s32)gpr[rt];
			ctx->cpu.alu_hi = (s32)gpr[rs] % (s32)gpr[rt];

			break;

		case CPU_INSTR_DIVU:
			ctx->cpu.alu_lo = gpr[rs] / gpr[rt];
			ctx->cpu.alu_hi = gpr[rs] % gpr[rt];

			break;

		case CPU_INSTR_ADD:
		case CPU_INSTR_ADDU:
			gpr[rd] = gpr[rs] + gpr[rt];
			break;

		case CPU_INSTR_SUBU:
			gpr[rd] = gpr[rs] - gpr[rt];
			break;

		case CPU_INSTR_AND:
			gpr[rd] = gpr[rs] & gpr[rt];
			break;

		case CPU_INSTR_OR:
			gpr[rd] = gpr[rs] | gpr[rt];
			break;

		case CPU_INSTR_SLT:
			gpr[rd] = (s32)gpr[rs] < (s32)gpr[rt];
			break;

		case CPU_INSTR_SLTU:
			gpr[rd] = gpr[rs] < gpr[rt];
			break;

		default:
			illegal(ctx);
			return;
		}
		break;

	case CPU_INSTR_GROUP_BCOND:
		switch (rt) {
		case CPU_INSTR_BLTZ:
			branch_if(ctx, (s32)gpr[rs] < 0);
			break;

		case CPU_INSTR_BGEZ:
			branch_if(ctx, (s32)gpr[rs] >= 0);
			break;

		default:
			illegal(ctx);
			return;
		}
		break;

	case CPU_INSTR_GROUP_COP0:
		switch (rs) {
		case CPU_INSTR_COP_MF:
			gpr[rt] = ctx->cpu.cop0[rd];
			break;

		case CPU_INSTR_COP_MT:
			ctx->cpu.cop0[rd] = gpr[rt];
			break;

		default:
			switch (funct) {
			default:
				illegal(ctx);
				return;
			}
			break;
		}
		break;

	case CPU_INSTR_J:
		ctx->cpu.next_pc =
			psycho_cpu_calc_jmp_addr(ctx->cpu.instr, ctx->cpu.pc);
		break;

	case CPU_INSTR_JAL:
		gpr[PSYCHO_CPU_GPR_RA] = ctx->cpu.pc + 4;

		ctx->cpu.next_pc =
			psycho_cpu_calc_jmp_addr(ctx->cpu.instr, ctx->cpu.pc);
		break;

	case CPU_INSTR_BEQ:
		branch_if(ctx, gpr[rs] == gpr[rt]);
		break;

	case CPU_INSTR_BNE:
		branch_if(ctx, gpr[rs] != gpr[rt]);
		break;

	case CPU_INSTR_BLEZ:
		branch_if(ctx, (s32)gpr[rs] <= 0);
		break;

	case CPU_INSTR_BGTZ:
		branch_if(ctx, (s32)gpr[rs] > 0);
		break;

	case CPU_INSTR_ADDI:
	case CPU_INSTR_ADDIU:
		gpr[rt] = psycho_sign_extend_16_32(imm) + gpr[rs];
		break;

	case CPU_INSTR_SLTI:
		gpr[rt] = (s32)gpr[rs] < (s32)psycho_sign_extend_16_32(imm);
		break;

	case CPU_INSTR_SLTIU:
		gpr[rt] = gpr[rs] < psycho_sign_extend_16_32(imm);
		break;

	case CPU_INSTR_ANDI:
		gpr[rt] = gpr[rs] & imm;
		break;

	case CPU_INSTR_ORI:
		gpr[rt] = gpr[rs] | imm;
		break;

	case CPU_INSTR_LUI:
		gpr[rt] = imm << 16;
		break;

	case CPU_INSTR_LB: {
		const u32 m_paddr = get_physical_address(ctx);

		gpr[rt] = psycho_bus_load_byte(ctx, m_paddr);
		gpr[rt] = psycho_sign_extend_8_32(gpr[rt]);

		break;
	}

	case CPU_INSTR_LW: {
		const u32 m_paddr = get_physical_address(ctx);

		gpr[rt] = psycho_bus_load_word(ctx, m_paddr);
		break;
	}

	case CPU_INSTR_LBU: {
		const u32 m_paddr = get_physical_address(ctx);

		gpr[rt] = psycho_bus_load_byte(ctx, m_paddr);
		break;
	}

	case CPU_INSTR_SB: {
		const u32 m_paddr = get_physical_address(ctx);

		psycho_bus_store_byte(ctx, m_paddr, gpr[rt] & UINT8_MAX);
		break;
	}

	case CPU_INSTR_SH: {
		const u32 m_paddr = get_physical_address(ctx);

		psycho_bus_store_halfword(ctx, m_paddr, gpr[rt] & UINT16_MAX);
		break;
	}

	case CPU_INSTR_SW: {
		if (!(ctx->cpu.cop0[PSYCHO_CPU_COP0_REG_SR] & CPU_SR_ISC)) {
			const u32 m_paddr = get_physical_address(ctx);
			psycho_bus_store_word(ctx, m_paddr, gpr[rt]);
		}
		break;
	}

	default:
		illegal(ctx);
		return;
	}

#undef op
#undef rt
#undef immediate
}
