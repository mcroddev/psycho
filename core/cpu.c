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

#include <string.h>

#include "core/cpu-defs.h"
#include "cpu-defs.h"

#include "cpu.h"
#include "bus.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_CPU);

static void illegal(struct psycho_ctx *const ctx)
{
	LOG_ERROR(ctx, "Illegal instruction trapped: 0x%08X", ctx->cpu.instr);
	ctx->event_cb(ctx, PSYCHO_EVENT_CPU_ILLEGAL, NULL);
}

static void branch_if(struct psycho_ctx *const ctx, const bool cond_met)
{
	if (cond_met)
		ctx->cpu.next_pc =
			calc_branch_addr(ctx->cpu.instr, ctx->cpu.curr_pc);
}

static void load_delay(struct psycho_ctx *const ctx, const size_t dst,
		       const u32 val)
{
	if (unlikely(!dst)) {
		LOG_WARN(ctx, "Load delay rejected - destination was $zero");
		return;
	}

	ctx->cpu.ld_next.dst = dst;
	ctx->cpu.ld_next.val = val;

	if (ctx->cpu.ld_curr.dst == dst)
		memset(&ctx->cpu.ld_curr, 0, sizeof(ctx->cpu.ld_curr));
}

static void load_delay_process(struct psycho_ctx *const ctx)
{
	ctx->cpu.gpr[ctx->cpu.ld_curr.dst] = ctx->cpu.ld_curr.val;
	memset(&ctx->cpu.ld_curr, 0, sizeof(ctx->cpu.ld_curr));
	swap(&ctx->cpu.ld_curr, &ctx->cpu.ld_next);
}

static u32 get_phys_addr(struct psycho_ctx *const ctx)
{
	const u16 off = instr_off(ctx->cpu.instr);
	const uint base = instr_rs(ctx->cpu.instr);
	const u32 vaddr = get_vaddr(off, ctx->cpu.gpr[base]);

	return vaddr_to_paddr(vaddr);
}

static void raise_exception(struct psycho_ctx *const ctx,
			    const enum cpu_exc_code exc)
{
	// On an exception, the CPU:

	static const char *const exc_name[] = {
		// clang-format off

		[EXCEPTION_ADEL]	= "Address error on load",
		[EXCEPTION_ADES]	= "Address error on store",
		[EXCEPTION_BP]		= "Breakpoint",
		[EXCEPTION_OV]		= "Arithmetic overflow"

		// clang-format on
	};

	LOG_WARN(ctx, "%s exception raised", exc_name[exc]);

	// 1) sets up EPC to point to the restart location.
	ctx->cpu.cop0[CPU_COP0_EPC] = ctx->cpu.curr_pc;

	// 2) the pre-existing user-mode and interrupt-enable flags in SR are
	//    saved by pushing the 3-entry stack inside SR, and changing to
	//    kernel mode with interrupts disabled.
	ctx->cpu.cop0[CPU_COP0_SR] =
		(ctx->cpu.cop0[CPU_COP0_SR] & 0xFFFFFFC0) |
		((ctx->cpu.cop0[CPU_COP0_SR] & 0x0000000F) << 2);

	// 3) Cause is setup so that software can see the reason for the
	//    exception.
	ctx->cpu.cop0[CPU_COP0_CAUSE] =
		(ctx->cpu.cop0[CPU_COP0_CAUSE] & ~0xFFFF00FF) | (exc << 2);

	// 4) transfers control to the exception entry point.
	ctx->cpu.pc = 0x00000080;
	ctx->cpu.next_pc = 0x00000084;
}

void psycho_cpu_reset(struct psycho_ctx *const ctx)
{
	ctx->cpu.pc = RESET_PC;
	ctx->cpu.next_pc = RESET_PC + sizeof(u32);
	ctx->cpu.curr_pc = ctx->cpu.pc;

	memset(&ctx->cpu.ld_curr, 0, sizeof(ctx->cpu.ld_curr));
	memset(&ctx->cpu.ld_next, 0, sizeof(ctx->cpu.ld_next));
}

void psycho_cpu_step(struct psycho_ctx *const ctx)
{
#define op (instr_op(ctx->cpu.instr))
#define rt (instr_rt(ctx->cpu.instr))
#define rd (instr_rd(ctx->cpu.instr))
#define rs (instr_rs(ctx->cpu.instr))
#define funct (instr_funct(ctx->cpu.instr))
#define shamt (instr_shamt(ctx->cpu.instr))
#define imm (instr_imm(ctx->cpu.instr))
#define gpr (ctx->cpu.gpr)

	load_delay_process(ctx);

	if (unlikely(ctx->cpu.pc & 0x00000003))
		raise_exception(ctx, EXCEPTION_ADEL);

	ctx->cpu.curr_pc = ctx->cpu.pc;
	const u32 paddr = vaddr_to_paddr(ctx->cpu.curr_pc);
	ctx->cpu.instr = psycho_bus_load_word(ctx, paddr);

	ctx->cpu.pc = ctx->cpu.next_pc;
	ctx->cpu.next_pc = ctx->cpu.pc + sizeof(u32);

	switch (op) {
	case INSTR_GROUP_SPECIAL:
		switch (funct) {
		case INSTR_SLL:
			gpr[rd] = gpr[rt] << shamt;
			break;

		case INSTR_SRL:
			gpr[rd] = gpr[rt] >> shamt;
			break;

		case INSTR_SRA:
			gpr[rd] = (s32)gpr[rt] >> shamt;
			break;

		case INSTR_SLLV:
			gpr[rd] = gpr[rt] << (gpr[rs] & 0x0000001F);
			break;

		case INSTR_SRLV:
			gpr[rd] = gpr[rt] >> (gpr[rs] & 0x0000001F);
			break;

		case INSTR_SRAV:
			gpr[rd] = (s32)gpr[rt] >> (gpr[rs] & 0x0000001F);
			break;

		case INSTR_JR:
			ctx->cpu.next_pc = gpr[rs];
			break;

		case INSTR_JALR: {
			const u32 target = gpr[rs];

			gpr[rd] = ctx->cpu.curr_pc + (sizeof(u32) * 2);

			if (unlikely(target & 0x00000003)) {
				raise_exception(ctx, EXCEPTION_ADEL);
				break;
			}
			ctx->cpu.next_pc = target;
			break;
		}

		case INSTR_SYSCALL:
			raise_exception(ctx, EXCEPTION_SYS);
			break;

		case INSTR_BREAK:
			raise_exception(ctx, EXCEPTION_BP);
			break;

		case INSTR_MFHI:
			gpr[rd] = ctx->cpu.hi;
			break;

		case INSTR_MTHI:
			ctx->cpu.hi = gpr[rs];
			break;

		case INSTR_MFLO:
			gpr[rd] = ctx->cpu.lo;
			break;

		case INSTR_MTLO:
			ctx->cpu.lo = gpr[rs];
			break;

		case INSTR_MULT: {
			const u64 prod = sign_ext_32_64(gpr[rs]) *
					 sign_ext_32_64(gpr[rt]);

			ctx->cpu.lo = prod & UINT32_MAX;
			ctx->cpu.hi = prod >> 32;

			break;
		}

		case INSTR_MULTU: {
			const u64 prod = zero_ext_32_64(gpr[rs]) *
					 zero_ext_32_64(gpr[rt]);

			ctx->cpu.lo = prod & UINT32_MAX;
			ctx->cpu.hi = prod >> 32;

			break;
		}

		case INSTR_DIV: {
			// The result of a division by zero is consistent with
			// the result of a simple radix-2 (“one bit at a time”)
			// implementation.

			const s32 divisor = (s32)gpr[rt];
			const s32 dividend = (s32)gpr[rs];

			if (unlikely(!divisor)) {
				// That is, if the dividend is negative, the
				// quotient is 1 (0x00000001), and if the
				// dividend is positive or zero, the quotient is
				// -1 (0xFFFFFFFF).
				ctx->cpu.lo = (dividend < 0) ? 0x000000001 :
							       UINT32_MAX;

				// In both cases the remainder equals the
				// dividend.
				ctx->cpu.hi = dividend;
			} else if (unlikely(((u32)dividend == 0x80000000) &&
					    ((u32)divisor == 0xFFFFFFFF))) {
				ctx->cpu.lo = dividend;
				ctx->cpu.hi = 0x00000000;
			} else {
				ctx->cpu.lo = dividend / divisor;
				ctx->cpu.hi = dividend % divisor;
			}
			break;
		}

		case INSTR_DIVU: {
			if (unlikely(!gpr[rt])) {
				// In the case of unsigned division, the
				// dividend can't be negative and thus the
				// quotient is always -1 (0xFFFFFFFF) and the
				// remainder equals the dividend.
				ctx->cpu.lo = UINT32_MAX;
				ctx->cpu.hi = gpr[rs];
			} else {
				ctx->cpu.lo = gpr[rs] / gpr[rt];
				ctx->cpu.hi = gpr[rs] % gpr[rt];
			}
			break;
		}

		case INSTR_ADD: {
			int sum;

			if (unlikely(__builtin_sadd_overflow(gpr[rs], gpr[rt],
							     &sum))) {
				raise_exception(ctx, EXCEPTION_OV);
				break;
			}
			gpr[rd] = sum;
			break;
		}

		case INSTR_ADDU:
			gpr[rd] = gpr[rs] + gpr[rt];
			break;

		case INSTR_SUB: {
			int diff;

			if (unlikely(__builtin_ssub_overflow(gpr[rs], gpr[rt],
							     &diff))) {
				raise_exception(ctx, EXCEPTION_OV);
				break;
			}
			gpr[rd] = diff;
			break;
		}

		case INSTR_SUBU:
			gpr[rd] = gpr[rs] - gpr[rt];
			break;

		case INSTR_AND:
			gpr[rd] = gpr[rs] & gpr[rt];
			break;

		case INSTR_OR:
			gpr[rd] = gpr[rs] | gpr[rt];
			break;

		case INSTR_XOR:
			gpr[rd] = gpr[rs] ^ gpr[rt];
			break;

		case INSTR_NOR:
			gpr[rd] = ~(gpr[rs] | gpr[rt]);
			break;

		case INSTR_SLT:
			gpr[rd] = (s32)gpr[rs] < (s32)gpr[rt];
			break;

		case INSTR_SLTU:
			gpr[rd] = gpr[rs] < gpr[rt];
			break;

		default:
			illegal(ctx);
			return;
		}
		break;

	case INSTR_GROUP_BCOND: {
		const bool link = (rt & 0x1E) == 0x10;
		const bool branch = (s32)(gpr[rs] ^ (rt << 31)) < 0;

		if (link)
			gpr[CPU_GPR_RA] = ctx->cpu.curr_pc + (sizeof(u32) * 2);

		branch_if(ctx, branch);
		break;
	}

	case INSTR_GROUP_COP0:
		switch (rs) {
		case INSTR_COP_MF:
			gpr[rt] = ctx->cpu.cop0[rd];
			break;

		case INSTR_COP_MT:
			ctx->cpu.cop0[rd] = gpr[rt];
			break;

		default:
			switch (funct) {
			case INSTR_RFE:
				ctx->cpu.cop0[CPU_COP0_SR] =
					(ctx->cpu.cop0[CPU_COP0_SR] &
					 0xFFFFFFF0) |
					((ctx->cpu.cop0[CPU_COP0_SR] &
					  0x0000003C) >>
					 2);

				break;

			default:
				illegal(ctx);
				return;
			}
			break;
		}
		break;

	case INSTR_J:
		ctx->cpu.next_pc =
			calc_jmp_addr(ctx->cpu.instr, ctx->cpu.curr_pc);
		break;

	case INSTR_JAL:
		if (unlikely(ctx->cpu.ld_curr.dst == CPU_GPR_RA))
			memset(&ctx->cpu.ld_curr.dst, 0,
			       sizeof(ctx->cpu.ld_curr));

		gpr[CPU_GPR_RA] = ctx->cpu.curr_pc + (sizeof(u32) * 2);

		ctx->cpu.next_pc =
			calc_jmp_addr(ctx->cpu.instr, ctx->cpu.curr_pc);

		break;

	case INSTR_BEQ:
		branch_if(ctx, gpr[rs] == gpr[rt]);
		break;

	case INSTR_BNE:
		branch_if(ctx, gpr[rs] != gpr[rt]);
		break;

	case INSTR_BLEZ:
		branch_if(ctx, (s32)gpr[rs] <= 0);
		break;

	case INSTR_BGTZ:
		branch_if(ctx, (s32)gpr[rs] > 0);
		break;

	case INSTR_ADDI: {
		int sum;

		if (unlikely(__builtin_sadd_overflow(sign_ext_16_32(imm),
						     gpr[rs], &sum))) {
			raise_exception(ctx, EXCEPTION_OV);
			break;
		}
		gpr[rt] = sum;
		break;
	}

	case INSTR_ADDIU:
		gpr[rt] = sign_ext_16_32(imm) + gpr[rs];
		break;

	case INSTR_SLTI:
		gpr[rt] = (s32)gpr[rs] < (s32)sign_ext_16_32(imm);
		break;

	case INSTR_SLTIU:
		gpr[rt] = gpr[rs] < sign_ext_16_32(imm);
		break;

	case INSTR_ANDI:
		gpr[rt] = gpr[rs] & imm;
		break;

	case INSTR_ORI:
		gpr[rt] = gpr[rs] | imm;
		break;

	case INSTR_XORI:
		gpr[rt] = gpr[rs] ^ imm;
		break;

	case INSTR_LUI:
		gpr[rt] = imm << 16;
		break;

	case INSTR_LB: {
		const u32 m_paddr = get_phys_addr(ctx);

		const u32 val =
			sign_ext_8_32(psycho_bus_load_byte(ctx, m_paddr));

		load_delay(ctx, rt, val);
		break;
	}

	case INSTR_LH: {
		const u32 m_paddr = get_phys_addr(ctx);

		if (unlikely(m_paddr & 1)) {
			raise_exception(ctx, EXCEPTION_ADEL);
			break;
		}

		const u32 val =
			sign_ext_16_32(psycho_bus_load_halfword(ctx, m_paddr));

		load_delay(ctx, rt, val);
		break;
	}

	case INSTR_LWL: {
		const u32 m_paddr = get_phys_addr(ctx);
		const u32 aligned_paddr = m_paddr & ~3;

		const u32 word = psycho_bus_load_word(ctx, aligned_paddr);

		const uint shift = (m_paddr & 3) * 8;
		const uint mask = 0x00FFFFFF >> shift;

		const u32 val = (ctx->cpu.ld_curr.dst == rt) ?
					ctx->cpu.ld_curr.val :
					gpr[rt];

		const u32 res = (val & mask) | (word << (24 - shift));

		load_delay(ctx, rt, res);
		break;
	}

	case INSTR_LW: {
		const u32 m_paddr = get_phys_addr(ctx);

		if (unlikely(m_paddr & 0x3)) {
			raise_exception(ctx, EXCEPTION_ADEL);
			break;
		}

		const u32 val = psycho_bus_load_word(ctx, m_paddr);

		load_delay(ctx, rt, val);
		break;
	}

	case INSTR_LBU: {
		const u32 m_paddr = get_phys_addr(ctx);
		const u32 val = psycho_bus_load_byte(ctx, m_paddr);

		load_delay(ctx, rt, val);
		break;
	}

	case INSTR_LHU: {
		const u32 m_paddr = get_phys_addr(ctx);

		if (unlikely(m_paddr & 1)) {
			raise_exception(ctx, EXCEPTION_ADEL);
			break;
		}

		const u16 val = psycho_bus_load_halfword(ctx, m_paddr);
		load_delay(ctx, rt, val);

		break;
	}

	case INSTR_LWR: {
		const u32 m_paddr = get_phys_addr(ctx);
		const u32 aligned_paddr = m_paddr & ~3;

		const u32 word = psycho_bus_load_word(ctx, aligned_paddr);

		const uint shift = (m_paddr & 3) * 8;
		const uint mask = 0xFFFFFF00 << (24 - shift);

		const u32 val = (ctx->cpu.ld_curr.dst == rt) ?
					ctx->cpu.ld_curr.val :
					gpr[rt];

		const u32 res = (val & mask) | (word >> shift);

		load_delay(ctx, rt, res);
		break;
	}

	case INSTR_SB: {
		const u32 m_paddr = get_phys_addr(ctx);

		psycho_bus_store_byte(ctx, m_paddr, gpr[rt] & UINT8_MAX);
		break;
	}

	case INSTR_SH: {
		const u32 m_paddr = get_phys_addr(ctx);

		if (unlikely(m_paddr & 1)) {
			raise_exception(ctx, EXCEPTION_ADES);
			break;
		}

		psycho_bus_store_halfword(ctx, m_paddr, gpr[rt] & UINT16_MAX);
		break;
	}

	case INSTR_SWL: {
		const u32 m_paddr = get_phys_addr(ctx);
		const u32 aligned_paddr = m_paddr & ~3;

		const uint shift = (m_paddr & 3) * 8;
		const uint mask = 0xFFFFFF00 << shift;

		u32 word = psycho_bus_load_word(ctx, aligned_paddr);
		word = (word & mask) | (gpr[rt] >> (24 - shift));
		psycho_bus_store_word(ctx, aligned_paddr, word);

		break;
	}

	case INSTR_SW: {
		if (!(ctx->cpu.cop0[CPU_COP0_SR] & CPU_SR_ISC)) {
			const u32 m_paddr = get_phys_addr(ctx);

			if (unlikely(m_paddr & 0x3)) {
				raise_exception(ctx, EXCEPTION_ADES);
				break;
			}
			psycho_bus_store_word(ctx, m_paddr, gpr[rt]);
		}
		break;
	}

	case INSTR_SWR: {
		const u32 m_paddr = get_phys_addr(ctx);
		const u32 aligned_paddr = m_paddr & ~3;

		const uint shift = (m_paddr & 3) * 8;
		const uint mask = 0x00FFFFFF >> (24 - shift);

		u32 word = psycho_bus_load_word(ctx, aligned_paddr);
		word = (word & mask) | (gpr[rt] << shift);
		psycho_bus_store_word(ctx, aligned_paddr, word);

		break;
	}

	default:
		illegal(ctx);
		return;
	}

	gpr[0] = 0x00000000;

#undef op
#undef rt
#undef rd
#undef rs
#undef funct
#undef shamt
#undef imm
#undef gpr
}
