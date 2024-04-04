// SPDX-License-Identifier: MIT
//
// Copyright 2024 Michael Rodriguez
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

/// @file cpu.c Defines the implementation of the CPU interpreter.

#include <stdbool.h>
#include <string.h>

#include "cpu.h"
#include "cpu_defs.h"
#include "bus.h"
#include "dbg_log.h"

// clang-format off

#define zero	(CPU_GPR_zero)
#define ra	(CPU_GPR_ra)

#define GROUP_BCOND	(CPU_OP_GROUP_BCOND)
#define GROUP_COP0	(CPU_OP_GROUP_COP0)
#define GROUP_SPECIAL	(CPU_OP_GROUP_SPECIAL)

#define ADD	(CPU_OP_ADD)
#define ADDI	(CPU_OP_ADDI)
#define ADDIU	(CPU_OP_ADDIU)
#define ADDU	(CPU_OP_ADDU)
#define AND	(CPU_OP_AND)
#define ANDI	(CPU_OP_ANDI)
#define BEQ	(CPU_OP_BEQ)
#define BGTZ	(CPU_OP_BGTZ)
#define BLEZ	(CPU_OP_BLEZ)
#define BNE	(CPU_OP_BNE)
#define BREAK	(CPU_OP_BREAK)
#define DIV	(CPU_OP_DIV)
#define DIVU	(CPU_OP_DIVU)
#define J	(CPU_OP_J)
#define JAL	(CPU_OP_JAL)
#define JALR	(CPU_OP_JALR)
#define JR	(CPU_OP_JR)
#define LB	(CPU_OP_LB)
#define LBU	(CPU_OP_LBU)
#define LH	(CPU_OP_LH)
#define LHU	(CPU_OP_LHU)
#define LUI	(CPU_OP_LUI)
#define LW	(CPU_OP_LW)
#define LWL	(CPU_OP_LWL)
#define LWR	(CPU_OP_LWR)
#define MF	(CPU_OP_MF)
#define MFHI	(CPU_OP_MFHI)
#define MFLO	(CPU_OP_MFLO)
#define MT	(CPU_OP_MT)
#define MTHI	(CPU_OP_MTHI)
#define MTLO	(CPU_OP_MTLO)
#define MULT	(CPU_OP_MULT)
#define MULTU	(CPU_OP_MULTU)
#define NOR	(CPU_OP_NOR)
#define OR	(CPU_OP_OR)
#define ORI	(CPU_OP_ORI)
#define RFE	(CPU_OP_RFE)
#define SB	(CPU_OP_SB)
#define SH	(CPU_OP_SH)
#define SLL	(CPU_OP_SLL)
#define SLLV	(CPU_OP_SLLV)
#define SLT	(CPU_OP_SLT)
#define SLTI	(CPU_OP_SLTI)
#define SLTIU	(CPU_OP_SLTIU)
#define SLTU	(CPU_OP_SLTU)
#define SRA	(CPU_OP_SRA)
#define SRAV	(CPU_OP_SRAV)
#define SRL	(CPU_OP_SRL)
#define SRLV	(CPU_OP_SRLV)
#define SUB	(CPU_OP_SUB)
#define SUBU	(CPU_OP_SUBU)
#define SW	(CPU_OP_SW)
#define SWL	(CPU_OP_SWL)
#define SWR	(CPU_OP_SWR)
#define SYSCALL	(CPU_OP_SYSCALL)
#define XOR	(CPU_OP_XOR)
#define XORI	(CPU_OP_XORI)

#define GPR	(ctx->cpu.gpr)
#define NPC	(ctx->cpu.npc)
#define PC 	(ctx->cpu.pc)

#define CP0_CPR	(ctx->cpu.cp0_cpr)
#define BadA	(CP0_CPR[CPU_CP0_CPR_BadA])
#define Cause	(CP0_CPR[CPU_CP0_CPR_Cause])
#define EPC	(CP0_CPR[CPU_CP0_CPR_EPC])
#define SR	(CP0_CPR[CPU_CP0_CPR_SR])

#define AdEL	(PSYCHO_CPU_EXC_CODE_AdEL)
#define AdES	(PSYCHO_CPU_EXC_CODE_AdES)
#define Bp	(PSYCHO_CPU_EXC_CODE_Bp)
#define Ovf	(PSYCHO_CPU_EXC_CODE_Ovf)
#define Sys	(PSYCHO_CPU_EXC_CODE_Sys)
#define RI	(PSYCHO_CPU_EXC_CODE_RI)

#define EXC_RAISE(exc_code)	(exc_raise(ctx, (exc_code)))
#define BRANCH_IF(cond)		(branch_if(ctx, (cond)))

#define JMP_TGT (cpu_jmp_tgt_get(ctx->cpu.instr, PC))

#define LDS_NEXT	(ctx->cpu.lds_next)
#define LDS_PEND	(ctx->cpu.lds_pend)

#define HI	(ctx->cpu.hi)
#define LO	(ctx->cpu.lo)

#define IsC	(CPU_CP0_CPR_SR_IsC)

// clang-format on

const char *const exc_code_names[] = { [AdEL] = "Address error on load",
				       [AdES] = "Address error on store",
				       [Sys] = "System call",
				       [Bp] = "Breakpoint",
				       [RI] = "Reserved instruction",
				       [Ovf] = "Arithmetic overflow" };

/// @brief Branches to the target address if a condition was met.
/// @param ctx The psycho_ctx instance.
/// @param condition_met `true` if the branch condition was met, or `false`
/// otherwise.
static ALWAYS_INLINE void branch_if(struct psycho_ctx *const ctx,
				    const bool condition_met)
{
	if (condition_met) {
		NPC = cpu_branch_tgt_get(ctx->cpu.instr, PC);
	}
}

/// @brief Checks for overflow in a two's complement sum.
///
/// A sum has overflowed if:
///
/// * two positive numbers yields a negative result, or;
/// * two negative numbers yields a positive result.
///
/// @param first The first addend.
/// @param second The second addend.
/// @param sum The sum of both addends.
/// @returns `true` if the sum has overflowed, or `false` otherwise.
static ALWAYS_INLINE NODISCARD bool ovf_add(const u32 first, const u32 second,
					    const u32 sum)
{
	return !((first ^ second) & CPU_SIGN_BIT) &&
	       ((sum ^ first) & CPU_SIGN_BIT);
}

/// @brief Checks for overflow in a two's complement subtraction.
///
/// A difference has overflowed if:
///
/// * a positive number subtracted from a negative number yields a positive
///   result, or;
///
/// * a negative number subtracted from a positive number yields a negative
///   result.
///
/// @param first The minuend.
/// @param second The subtrahend.
/// @param diff The difference between the minuend and the subtrahend.
/// @returns `true` if the difference has overflowed, or `false` otherwise.
static ALWAYS_INLINE NODISCARD bool ovf_sub(const u32 first, const u32 second,
					    const u32 diff)
{
	return ((first ^ second) & CPU_SIGN_BIT) &&
	       ((diff ^ first) & CPU_SIGN_BIT);
}

/// @brief Handles the load delay slot.
///
/// When a load instruction is executed (with the exception of LWL/LWR), the
/// data read is not immediately available to the next instruction; a delay of
/// one instruction is necessary.
///
/// @param ctx The psycho_ctx instance.
/// @param dst The general purpose register to store the value.
/// @param val The value to store.
static ALWAYS_INLINE void load_delay(struct psycho_ctx *const ctx,
				     const uint dst, const u32 val)
{
	if (unlikely(dst == CPU_GPR_zero)) {
		// $zero must always indeed remain $zero; psxtest_cpu exercises
		// this case.
		return;
	}

	LDS_PEND.dst = dst;
	LDS_PEND.val = val;

	LOG_TRACE("Load delay now pending (dst=%d, val=0x%08X)", dst, val);

	if (LDS_NEXT.dst == dst) {
		LOG_TRACE("Evicting next load delay slot");
		memset(&LDS_NEXT, 0, sizeof(LDS_NEXT));
	}
}

/// @brief Raises an exception.
/// @param ctx The psycho_ctx instance.
/// @param exc_code The exception to raise.
static void exc_raise(struct psycho_ctx *const ctx, const uint exc_code)
{
	// Note that in an emulation context, we may not want to actually
	// service an exception. If the frontend cares about the state of the
	// system at the time of the exception, servicing the exception will
	// modify CPU registers and the program counter which may not be
	// desirable.
	//
	// On the other hand, if a test program is being executed, it is
	// possible that exceptions will be raised to test the quality of the
	// implementation; in that context, it is not an error.
	//
	// This necessitates the need at the discretion of the frontend to
	// determine what exceptions actually *halt* execution.
	const char *const exc_name = exc_code_names[exc_code];

	if (ctx->cpu.exc_halt & (1 << exc_code)) {
		LOG_ERR("%s exception raised!", exc_name);
		return;
	}

	LOG_WARN("%s exception raised!", exc_name);

	// So, on an exception, the CPU:

	// 1) sets up EPC to point to the restart location.
	EPC = PC;

	// 2) the pre-existing user-mode and interrupt-enable flags in SR are
	//    saved by pushing the 3-entry stack inside SR, and changing to
	//    kernel mode with interrupts disabled.
	SR = (SR & 0xFFFFFFC0) | ((SR & 0xF) << 2);

	// On address exceptions, BadA is also set.
	if (unlikely((exc_code == AdEL) || (exc_code == AdES))) {
		BadA = PC;
	}

	// 3) Cause is set up so that software can see the reason for the
	//    exception.
	//
	// This clears all the fields in the Cause register except for IP, and
	// sets the ExcCode field to the specified exception code.
	Cause = (Cause & ~0xFFFF00FF) | (exc_code << 2);

	/// 4) transfers control to the exception entry point.
	PC = CPU_VEC_EXC - sizeof(u32);
	NPC = CPU_VEC_EXC + sizeof(u32);
}

/// @brief Retrieves the virtual address for a load/store operation.
/// @param ctx The psycho_ctx instance.
/// @returns The virtual address.
static ALWAYS_INLINE NODISCARD u32 vaddr_get(const struct psycho_ctx *const ctx)
{
	const u32 base = cpu_instr_base_get(ctx->cpu.instr);
	const u32 offset = cpu_instr_offset_get(ctx->cpu.instr);

	return GPR[base] + offset;
}

/// @brief Retrieves the physical address for a load/store operation.
/// @param ctx The psycho_ctx instance.
/// @returns The physical address.
static ALWAYS_INLINE NODISCARD u32 paddr_get(const struct psycho_ctx *const ctx)
{
	const u32 vaddr = vaddr_get(ctx);
	return cpu_vaddr_to_paddr(vaddr);
}

/// @brief Fetches the instruction pointed to by the program counter.
///
/// This corresponds to the IF (Instruction Fetch) stage of the MIPS-I pipeline.
///
/// @param ctx The psycho_ctx instance.
/// @returns The instruction that was fetched.
static ALWAYS_INLINE NODISCARD u32 instr_fetch(struct psycho_ctx *const ctx)
{
	const u32 paddr = cpu_vaddr_to_paddr(PC);
	const u32 instr = bus_lw(ctx, paddr);

	if (LDS_NEXT.dst) {
		LOG_TRACE("Flushing load delay slot (dest=%d, value=0x%08X)",
			  LDS_NEXT.dst, LDS_NEXT.val);

		GPR[LDS_NEXT.dst] = LDS_NEXT.val;
	}
	LDS_NEXT = LDS_PEND;
	memset(&LDS_PEND, 0, sizeof(LDS_PEND));

	return instr;
}

/// @brief Resets the CPU to the startup state.
///
/// In MIPS-I terms, this is effectively a "reset exception". It does not pass
/// through normal exception handling, of course.
///
/// @param ctx The psycho_ctx instance
void cpu_reset(struct psycho_ctx *const ctx)
{
	memset(GPR, 0, sizeof(GPR));
	PC = CPU_VEC_RST;
	NPC = PC + sizeof(u32);

	ctx->cpu.instr = instr_fetch(ctx);

	memset(&LDS_NEXT, 0, sizeof(LDS_NEXT));
	memset(&LDS_PEND, 0, sizeof(LDS_PEND));

	LOG_INFO("CPU reset!");
}

/// @brief Executes the current instruction.
/// @param ctx The psycho_ctx instance.
void cpu_step(struct psycho_ctx *const ctx)
{
#define op (cpu_instr_op_get(ctx->cpu.instr))
#define rt (cpu_instr_rt_get(ctx->cpu.instr))
#define rs (cpu_instr_rs_get(ctx->cpu.instr))
#define rd (cpu_instr_rd_get(ctx->cpu.instr))
#define funct (cpu_instr_funct_get(ctx->cpu.instr))
#define shamt (cpu_instr_shamt_get(ctx->cpu.instr))
#define ZEXT_IMM (cpu_instr_zext_imm_get(ctx->cpu.instr))
#define SEXT_IMM (cpu_instr_sext_imm_get(ctx->cpu.instr))

	PC = (u32)(NPC - sizeof(u32));
	NPC += sizeof(u32);

	switch (op) {
	case GROUP_SPECIAL:
		switch (funct) {
		case SLL:
			GPR[rd] = GPR[rt] << shamt;
			break;

		case SRL:
			GPR[rd] = GPR[rt] >> shamt;
			break;

		case SRA:
			GPR[rd] = (u32)((s32)GPR[rt] >> shamt);
			break;

		case SLLV:
			GPR[rd] = GPR[rt] << (GPR[rs] & 0x1F);
			break;

		case SRLV:
			GPR[rd] = GPR[rt] >> (GPR[rs] & 0x1F);
			break;

		case SRAV:
			GPR[rd] = (u32)((s32)GPR[rt] >> (GPR[rs] & 0x1F));
			break;

		case JR:
			if (unlikely((GPR[rs] & 3) != 0)) {
				EXC_RAISE(AdEL);
				break;
			}
			NPC = GPR[rs];
			break;

		case JALR: {
			const u32 jmp_tgt = GPR[rs];

			GPR[rd] = PC + 8;

			if (unlikely((jmp_tgt & 3) != 0)) {
				EXC_RAISE(AdEL);
				break;
			}

			NPC = jmp_tgt;
			break;
		}

		case SYSCALL:
			EXC_RAISE(Sys);
			break;

		case BREAK:
			EXC_RAISE(Bp);
			break;

		case MFHI:
			GPR[rd] = HI;
			break;

		case MFLO:
			GPR[rd] = LO;
			break;

		case MTHI:
			HI = GPR[rs];
			break;

		case MTLO:
			LO = GPR[rs];
			break;

		case MULT: {
			const u64 prod =
				(u64)((s64)(s32)GPR[rs] * (s64)(s32)GPR[rt]);

			LO = (u32)(prod & 0xFFFFFFFF);
			HI = (u32)(prod >> 32);

			break;
		}

		case MULTU: {
			const u64 prod = (u64)GPR[rs] * (u64)GPR[rt];

			LO = prod & 0xFFFFFFFF;
			HI = (u32)(prod >> 32);

			break;
		}

		case DIV: {
			// The result of a division by zero is consistent with a
			// simple radix-2 ("one bit at a time") implementation.
			if ((s32)GPR[rt] == 0) {
				LO = ((s32)GPR[rs] < 0) ? 1 : 0xFFFFFFFF;
				HI = (u32)(s32)GPR[rs];
			} else if ((GPR[rs] == 0x80000000) &&
				   GPR[rt] == 0xFFFFFFFF) {
				LO = 0x80000000;
				HI = 0;
			} else {
				LO = (u32)((s32)GPR[rs] / (s32)GPR[rt]);
				HI = (u32)((s32)GPR[rs] % (s32)GPR[rt]);
			}
			break;
		}

		case DIVU: {
			if (GPR[rt] == 0) {
				LO = 0xFFFFFFFF;
				HI = GPR[rs];
			} else {
				LO = GPR[rs] / GPR[rt];
				HI = GPR[rs] % GPR[rt];
			}
			break;
		}

		case ADD: {
			const u32 sum = GPR[rs] + GPR[rt];

			if (ovf_add(GPR[rs], GPR[rt], sum)) {
				EXC_RAISE(Ovf);
				break;
			}
			GPR[rd] = sum;
			break;
		}

		case ADDU:
			GPR[rd] = GPR[rs] + GPR[rt];
			break;

		case SUB: {
			const u32 sum = GPR[rs] - GPR[rt];

			if (ovf_sub(GPR[rs], GPR[rt], sum)) {
				EXC_RAISE(Ovf);
				break;
			}
			GPR[rd] = sum;
			break;
		}

		case SUBU:
			GPR[rd] = GPR[rs] - GPR[rt];
			break;

		case AND:
			GPR[rd] = GPR[rs] & GPR[rt];
			break;

		case OR:
			GPR[rd] = GPR[rs] | GPR[rt];
			break;

		case XOR:
			GPR[rd] = GPR[rs] ^ GPR[rt];
			break;

		case NOR:
			GPR[rd] = ~(GPR[rs] | GPR[rt]);
			break;

		case SLT:
			GPR[rd] = (s32)GPR[rs] < (s32)GPR[rt];
			break;

		case SLTU:
			GPR[rd] = GPR[rs] < GPR[rt];
			break;

		default:
			EXC_RAISE(RI);
			break;
		}
		break;

	case GROUP_BCOND: {
		const bool cond_met = ((s32)GPR[rs] < 0) ^ (rt & 1);

		if ((rt & 0x1E) == 0x10) {
			GPR[ra] = PC + 8;
		}

		BRANCH_IF(cond_met);
		break;
	}

	case J:
		NPC = JMP_TGT;
		break;

	case JAL:
		GPR[ra] = PC + 8;
		NPC = JMP_TGT;

		break;

	case BEQ:
		BRANCH_IF(GPR[rs] == GPR[rt]);
		break;

	case BNE:
		BRANCH_IF(GPR[rs] != GPR[rt]);
		break;

	case BLEZ:
		BRANCH_IF((s32)GPR[rs] <= 0);
		break;

	case BGTZ:
		BRANCH_IF((s32)GPR[rs] > 0);
		break;

	case ORI:
		GPR[rt] = GPR[rs] | ZEXT_IMM;
		break;

	case XORI:
		GPR[rt] = GPR[rs] ^ ZEXT_IMM;
		break;

	case ADDI: {
		const u32 sum = GPR[rs] + SEXT_IMM;

		if (ovf_add(GPR[rs], SEXT_IMM, sum)) {
			EXC_RAISE(Ovf);
			break;
		}
		GPR[rt] = sum;
		break;
	}

	case ADDIU:
		GPR[rt] = GPR[rs] + SEXT_IMM;
		break;

	case SLTI:
		GPR[rt] = (s32)GPR[rs] < (s32)SEXT_IMM;
		break;

	case SLTIU:
		GPR[rt] = GPR[rs] < SEXT_IMM;
		break;

	case ANDI:
		GPR[rt] = GPR[rs] & ZEXT_IMM;
		break;

	case LUI:
		GPR[rt] = ZEXT_IMM << 16;
		break;

	case GROUP_COP0:
		switch (rs) {
		case MF:
			GPR[rt] = CP0_CPR[rd];
			break;

		case MT:
			CP0_CPR[rd] = GPR[rt];
			break;

		default:
			switch (funct) {
			case RFE:
				SR = (SR & 0xFFFFFFF0) | ((SR & 0x3C) >> 2);
				break;

			default:
				EXC_RAISE(RI);
				break;
			}
			break;
		}
		break;

	case LB: {
		const u32 paddr = paddr_get(ctx);
		const s8 byte = (s8)bus_lb(ctx, paddr);

		load_delay(ctx, rt, (u32)byte);
		break;
	}

	case LH: {
		const u32 vaddr = vaddr_get(ctx);

		if (unlikely((vaddr & 1) != 0)) {
			EXC_RAISE(AdEL);
			break;
		}

		const u32 paddr = cpu_vaddr_to_paddr(vaddr);
		const s16 hword = (s16)bus_lh(ctx, paddr);

		load_delay(ctx, rt, (u32)hword);
		break;
	}

	case LWL: {
		const u32 paddr = paddr_get(ctx);
		const u32 data = bus_lw(ctx, paddr & (u32)~3);

		u32 word = (LDS_NEXT.dst == rt) ? LDS_NEXT.val : GPR[rt];

		const uint shift = (paddr & 3) * 8;
		const uint mask = 0x00FFFFFF >> shift;

		word = (word & mask) | (data << (24 - shift));

		load_delay(ctx, rt, word);
		break;
	}

	case LW: {
		const u32 vaddr = vaddr_get(ctx);

		if (unlikely((vaddr & 3) != 0)) {
			EXC_RAISE(AdEL);
			break;
		}

		const u32 paddr = cpu_vaddr_to_paddr(vaddr);
		const u32 word = bus_lw(ctx, paddr);

		load_delay(ctx, rt, word);
		break;
	}

	case LBU: {
		const u32 paddr = paddr_get(ctx);
		const u8 byte = bus_lb(ctx, paddr);

		load_delay(ctx, rt, byte);
		break;
	}

	case LHU: {
		const u32 vaddr = vaddr_get(ctx);

		if (unlikely((vaddr & 1) != 0)) {
			EXC_RAISE(AdEL);
			break;
		}

		const u32 paddr = cpu_vaddr_to_paddr(vaddr);
		const u16 hword = bus_lh(ctx, paddr);

		load_delay(ctx, rt, hword);
		break;
	}

	case LWR: {
		const u32 paddr = paddr_get(ctx);
		const u32 data = bus_lw(ctx, paddr & (u32)~3);

		u32 word = LDS_NEXT.dst == rt ? LDS_NEXT.val : GPR[rt];

		const uint shift = (paddr & 3) * 8;
		const uint mask = 0xFFFFFF00 << (24 - shift);

		word = (word & mask) | (data >> shift);

		load_delay(ctx, rt, word);
		break;
	}

	case SB: {
		const u32 paddr = paddr_get(ctx);

		bus_sb(ctx, paddr, (u8)GPR[rt]);
		break;
	}

	case SH: {
		const u32 vaddr = vaddr_get(ctx);

		if (unlikely((vaddr & 1) != 0)) {
			EXC_RAISE(AdES);
			break;
		}

		const u32 paddr = cpu_vaddr_to_paddr(vaddr);

		bus_sh(ctx, paddr, (u16)GPR[rt]);
		break;
	}

	case SWL: {
		const u32 paddr = paddr_get(ctx);
		const u32 aligned_paddr = paddr & (u32)~3;

		u32 word = bus_lw(ctx, aligned_paddr);

		const uint shift = (paddr & 3) * 8;
		const uint mask = 0xFFFFFF00 << shift;

		word = (word & mask) | (GPR[rt] >> (24 - shift));

		bus_sw(ctx, aligned_paddr, word);
		break;
	}

	case SW: {
		if (SR & IsC) {
			break;
		}

		const u32 vaddr = vaddr_get(ctx);

		if (unlikely((vaddr & 3) != 0)) {
			EXC_RAISE(AdES);
			break;
		}

		const u32 paddr = cpu_vaddr_to_paddr(vaddr);

		bus_sw(ctx, paddr, GPR[rt]);
		break;
	}

	case SWR: {
		const u32 paddr = paddr_get(ctx);
		const u32 aligned_paddr = paddr & (u32)~3;

		u32 word = bus_lw(ctx, aligned_paddr);

		const uint shift = (paddr & 3) * 8;
		const uint mask = 0x00FFFFFF >> (24 - shift);

		word = (word & mask) | (GPR[rt] << shift);

		bus_sw(ctx, aligned_paddr, word);
		break;
	}

	default:
		EXC_RAISE(RI);
		break;
	}
	GPR[zero] = 0;
	PC += sizeof(u32);
	ctx->cpu.instr = instr_fetch(ctx);
}
