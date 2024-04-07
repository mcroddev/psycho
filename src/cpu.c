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
#include "util.h"

// clang-format off

#define zero	(CPU_GPR_zero)
#define ra	(CPU_GPR_ra)

#define GROUP_BCOND	(CPU_OP_GROUP_BCOND)
#define GROUP_COP0	(CPU_OP_GROUP_COP0)
#define GROUP_COP2	(CPU_OP_GROUP_COP2)
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
#define CC	(CPU_OP_CC)
#define CDP	(CPU_OP_CDP)
#define CF	(CPU_OP_CF)
#define CT	(CPU_OP_CT)
#define DIV	(CPU_OP_DIV)
#define DIVU	(CPU_OP_DIVU)
#define DPCS	(CPU_OP_DPCS)
#define INTPL	(CPU_OP_INTPL)
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
#define MVMVA	(CPU_OP_MVMVA)
#define NCS	(CPU_OP_NCS)
#define NCT	(CPU_OP_NCT)
#define NCCS	(CPU_OP_NCCS)
#define NCDS	(CPU_OP_NCDS)
#define NCDT	(CPU_OP_NCDT)
#define NCLIP	(CPU_OP_NCLIP)
#define NOR	(CPU_OP_NOR)
#define OP	(CPU_OP_OP)
#define OR	(CPU_OP_OR)
#define ORI	(CPU_OP_ORI)
#define RFE	(CPU_OP_RFE)
#define RTPS	(CPU_OP_RTPS)
#define SB	(CPU_OP_SB)
#define SH	(CPU_OP_SH)
#define SLL	(CPU_OP_SLL)
#define SLLV	(CPU_OP_SLLV)
#define SLT	(CPU_OP_SLT)
#define SLTI	(CPU_OP_SLTI)
#define SLTIU	(CPU_OP_SLTIU)
#define SLTU	(CPU_OP_SLTU)
#define SQR	(CPU_OP_SQR)
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

#define CP2_CPR	(ctx->cpu.cp2.cpr.regs)
#define IR1	(ctx->cpu.cp2.cpr.IR1)
#define IR2	(ctx->cpu.cp2.cpr.IR2)
#define IR3	(ctx->cpu.cp2.cpr.IR3)
#define OTZ	(ctx->cpu.cp2.cpr.OTZ)
#define VXY0	(ctx->cpu.cp2.cpr.VXY0)
#define VZ0	(ctx->cpu.cp2.cpr.VZ0)
#define IR0	(ctx->cpu.cp2.cpr.IR0)
#define SZ0	(ctx->cpu.cp2.cpr.SZ0)
#define SZ1	(ctx->cpu.cp2.cpr.SZ1)
#define SZ2	(ctx->cpu.cp2.cpr.SZ2)
#define SZ3	(ctx->cpu.cp2.cpr.SZ3)
#define SXY0	(ctx->cpu.cp2.cpr.SXY0)
#define SXY1	(ctx->cpu.cp2.cpr.SXY1)
#define SXY2	(ctx->cpu.cp2.cpr.SXY2)
#define LZCS	(ctx->cpu.cp2.cpr.LZCS)
#define LZCR	(ctx->cpu.cp2.cpr.LZCR)
#define MAC0	(ctx->cpu.cp2.cpr.MAC0)
#define MAC1	(ctx->cpu.cp2.cpr.MAC1)
#define MAC2	(ctx->cpu.cp2.cpr.MAC2)
#define MAC3	(ctx->cpu.cp2.cpr.MAC3)
#define SZ3	(ctx->cpu.cp2.cpr.SZ3)
#define VX0	((s16)(VXY0 & 0xFFFF))
#define VY0	((s16)(VXY0 >> 16))
#define VZ0	(ctx->cpu.cp2.cpr.VZ0)
#define SX0	((s16)(SXY0 & 0xFFFF))
#define SY0	((s16)(SXY0 >> 16))
#define SX1	((s16)(SXY1 & 0xFFFF))
#define SY1	((s16)(SXY1 >> 16))
#define SX2	((s16)(SXY2 & 0xFFFF))
#define SY2	((s16)(SXY2 >> 16))
#define RFC	(ctx->cpu.cp2.ccr.RFC)
#define GFC	(ctx->cpu.cp2.ccr.GFC)
#define BFC	(ctx->cpu.cp2.ccr.BFC)
#define RGB0	(ctx->cpu.cp2.cpr.RGB0)
#define RGB1	(ctx->cpu.cp2.cpr.RGB1)
#define RGB2	(ctx->cpu.cp2.cpr.RGB2)
#define RGBC	(ctx->cpu.cp2.cpr.RGBC)
#define VX0	((s16)(VXY0 & 0xFFFF))
#define VX1	((s16)(ctx->cpu.cp2.cpr.VXY1 & 0xFFFF))
#define VX2	((s16)(ctx->cpu.cp2.cpr.VXY2 & 0xFFFF))
#define VY0	((s16)(VXY0 >> 16))
#define VY1	((s16)(ctx->cpu.cp2.cpr.VXY1 >> 16))
#define VY2	((s16)(ctx->cpu.cp2.cpr.VXY2 >> 16))
#define VZ0	(ctx->cpu.cp2.cpr.VZ0)
#define VZ1	(ctx->cpu.cp2.cpr.VZ1)
#define VZ2	(ctx->cpu.cp2.cpr.VZ2)

#define CP2_CCR	(ctx->cpu.cp2.ccr.regs)
#define R11R12	(ctx->cpu.cp2.ccr.R11R12)
#define R22R23	(ctx->cpu.cp2.ccr.R22R23)
#define DQA	(ctx->cpu.cp2.ccr.DQA)
#define DQB	(ctx->cpu.cp2.ccr.DQB)
#define FLAG	(ctx->cpu.cp2.ccr.FLAG)
#define D1	((s16)(R11R12 & 0xFFFF))
#define D2	((s16)(R22R23 & 0xFFFF))
#define D3	(ctx->cpu.cp2.ccr.R33)
#define RT11	(D1)
#define RT12	((s16)(R11R12 >> 16))
#define RT13	((s16)(ctx->cpu.cp2.ccr.R13R21 & 0xFFFF))
#define RT21	((s16)(ctx->cpu.cp2.ccr.R13R21 >> 16))
#define RT22	(D2)
#define RT23	((s16)(R22R23 >> 16))
#define RT31	((s16)(ctx->cpu.cp2.ccr.R31R32 & 0xFFFF))
#define RT32	((s16)(ctx->cpu.cp2.ccr.R31R32 >> 16))
#define RT33	(ctx->cpu.cp2.ccr.R33)
#define TRX	(ctx->cpu.cp2.ccr.TRX)
#define TRY	(ctx->cpu.cp2.ccr.TRY)
#define TRZ	(ctx->cpu.cp2.ccr.TRZ)
#define H	(ctx->cpu.cp2.ccr.H)
#define OFX	(ctx->cpu.cp2.ccr.OFX)
#define OFY	(ctx->cpu.cp2.ccr.OFY)
#define RBK	(ctx->cpu.cp2.ccr.RBK)
#define GBK	(ctx->cpu.cp2.ccr.GBK)
#define BBK	(ctx->cpu.cp2.ccr.BBK)
#define L11L12	(ctx->cpu.cp2.ccr.L11L12)
#define L13L21	(ctx->cpu.cp2.ccr.L13L21)
#define L22L23	(ctx->cpu.cp2.ccr.L22L23)
#define L31L32	(ctx->cpu.cp2.ccr.L31L32)
#define L11	((s16)(L11L12 & 0xFFFF))
#define L12	((s16)(L11L12 >> 16))
#define L13	((s16)(L13L21 & 0xFFFF))
#define L21	((s16)(L13L21 >> 16))
#define L22	((s16)(L22L23 & 0xFFFF))
#define L23	((s16)(L22L23 >> 16))
#define L31	((s16)(L31L32 & 0xFFFF))
#define L32	((s16)(L31L32 >> 16))
#define L33	(ctx->cpu.cp2.ccr.L33)
#define LR1LR2	(ctx->cpu.cp2.ccr.LR1LR2)
#define LR3LG1	(ctx->cpu.cp2.ccr.LR3LG1)
#define LG2LG3	(ctx->cpu.cp2.ccr.LG2LG3)
#define LB1LB2	(ctx->cpu.cp2.ccr.LB1LB2)
#define LB3	(ctx->cpu.cp2.ccr.LB3)
#define LR1	((s16)(LR1LR2 & 0xFFFF))
#define LR2	((s16)(LR1LR2 >> 16))
#define LR3	((s16)(LR3LG1 & 0xFFFF))
#define LG1	((s16)(LR3LG1 >> 16))
#define LG2	((s16)(LG2LG3 & 0xFFFF))
#define LG3	((s16)(LG2LG3 >> 16))
#define LB1	((s16)(LB1LB2 & 0xFFFF))
#define LB2	((s16)(LB1LB2 >> 16))

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

static ALWAYS_INLINE void gte_flag_update(struct psycho_ctx *const ctx)
{
	if (FLAG & CPU_CP2_CCR_FLAG_MASK_ERR) {
		FLAG |= CPU_CP2_CCR_FLAG_ERR;
	}
}

static ALWAYS_INLINE void gte_ovf_chk(struct psycho_ctx *const ctx,
				      const s64 sum, const s64 min,
				      const s64 max, const uint neg_flag,
				      const uint pos_flag)
{
	if (sum > max) {
		FLAG |= pos_flag;
	} else if (sum < min) {
		FLAG |= neg_flag;
	}
}

static ALWAYS_INLINE NODISCARD s64 gte_mac0_add(struct psycho_ctx *const ctx,
						const s64 sum)
{
	gte_ovf_chk(ctx, sum, CPU_CP2_CPR_MAC0_MIN, CPU_CP2_CPR_MAC0_MAX,
		    CPU_CP2_CCR_FLAG_MAC0_NEG_OVF,
		    CPU_CP2_CCR_FLAG_MAC0_POS_OVF);
	return sum;
}

static ALWAYS_INLINE NODISCARD s64 gte_mac123_add(struct psycho_ctx *const ctx,
						  const s64 mac,
						  const s64 addend,
						  const uint neg_flag,
						  const uint pos_flag)
{
	const s64 sum = mac + addend;

	gte_ovf_chk(ctx, sum, CPU_CP2_CPR_MAC123_MIN, CPU_CP2_CPR_MAC123_MAX,
		    neg_flag, pos_flag);

	// Sign-extend result to 44 bits (64-20 = 44).
	return (s64)((u64)sum << 20) >> 20;
}

static NODISCARD s64 gte_mac1_add(struct psycho_ctx *const ctx, const s64 mac,
				  const s64 addend)
{
	return gte_mac123_add(ctx, mac, addend, CPU_CP2_CCR_FLAG_MAC1_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC1_POS_OVF);
}

static NODISCARD s64 gte_mac2_add(struct psycho_ctx *const ctx, const s64 mac,
				  const s64 addend)
{
	return gte_mac123_add(ctx, mac, addend, CPU_CP2_CCR_FLAG_MAC2_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC2_POS_OVF);
}

static NODISCARD s64 gte_mac3_add(struct psycho_ctx *const ctx, const s64 mac,
				  const s64 addend)
{
	return gte_mac123_add(ctx, mac, addend, CPU_CP2_CCR_FLAG_MAC3_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC3_POS_OVF);
}

static ALWAYS_INLINE u16 gte_chk_sz3_otz(struct psycho_ctx *const ctx,
					 const s32 value)
{
	if (value < CPU_CP2_CPR_SZ3_OTZ_MIN) {
		FLAG |= CPU_CP2_CCR_FLAG_SZ3_OR_OTZ_SATURATED;
		return CPU_CP2_CPR_SZ3_OTZ_MIN;
	} else if (value > CPU_CP2_CPR_SZ3_OTZ_MAX) {
		FLAG |= CPU_CP2_CCR_FLAG_SZ3_OR_OTZ_SATURATED;
		return CPU_CP2_CPR_SZ3_OTZ_MAX;
	}
	return (u16)value;
}

static ALWAYS_INLINE void gte_sz_push(struct psycho_ctx *const ctx,
				      const s64 sum)
{
	SZ0 = SZ1;
	SZ1 = SZ2;
	SZ2 = SZ3;
	SZ3 = gte_chk_sz3_otz(ctx, (s32)(sum >> 12));
}

static ALWAYS_INLINE void gte_sxy_push(struct psycho_ctx *const ctx,
				       const s16 x, const s16 y)
{
	SXY0 = SXY1;
	SXY1 = SXY2;
	SXY2 = (s32)(((u32)x & 0xFFFF) | ((u32)y << 16));
}

static ALWAYS_INLINE s16 gte_chk_ir(struct psycho_ctx *const ctx,
				    const s64 value, const uint flag,
				    const bool lm)
{
	const s16 min = lm ? CPU_CP2_CPR_IR123_LM_MIN : CPU_CP2_CPR_IR123_MIN;

	if (value < min) {
		FLAG |= flag;
		return min;
	} else if (value > CPU_CP2_CPR_IR123_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_IR123_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_ir0(struct psycho_ctx *const ctx,
				     const s32 value)
{
	if (value < CPU_CP2_CPR_IR0_MIN) {
		FLAG |= CPU_CP2_CCR_FLAG_IR0_SATURATED;
		return CPU_CP2_CPR_IR0_MIN;
	} else if (value > CPU_CP2_CPR_IR0_MAX) {
		FLAG |= CPU_CP2_CCR_FLAG_IR0_SATURATED;
		return CPU_CP2_CPR_IR0_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_ir1(struct psycho_ctx *const ctx,
				     const s64 value, const bool lm)
{
	return gte_chk_ir(ctx, value, CPU_CP2_CCR_FLAG_IR1_SATURATED, lm);
}

static ALWAYS_INLINE s16 gte_chk_ir2(struct psycho_ctx *const ctx,
				     const s64 value, const bool lm)
{
	return gte_chk_ir(ctx, value, CPU_CP2_CCR_FLAG_IR2_SATURATED, lm);
}

static ALWAYS_INLINE s16 gte_chk_ir3(struct psycho_ctx *const ctx,
				     const s64 value, const bool lm)
{
	return gte_chk_ir(ctx, value, CPU_CP2_CCR_FLAG_IR3_SATURATED, lm);
}

static ALWAYS_INLINE s16 gte_chk_sxy(struct psycho_ctx *const ctx,
				     const s32 value, const uint flag)
{
	if (value < CPU_CP2_CPR_SXY2_MIN) {
		FLAG |= flag;
		return CPU_CP2_CPR_SXY2_MIN;
	} else if (value > CPU_CP2_CPR_SXY2_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_SXY2_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_sx2(struct psycho_ctx *const ctx,
				     const s32 value)
{
	return gte_chk_sxy(ctx, value, CPU_CP2_CCR_FLAG_SX2_SATURATED);
}

static ALWAYS_INLINE s16 gte_chk_sy2(struct psycho_ctx *const ctx,
				     const s32 value)
{
	return gte_chk_sxy(ctx, value, CPU_CP2_CCR_FLAG_SY2_SATURATED);
}

static ALWAYS_INLINE u32 gte_chk_rgb(struct psycho_ctx *const ctx,
				     const s32 value, const uint flag)
{
	if (value < CPU_CP2_CPR_RGB_MIN) {
		FLAG |= flag;
		return CPU_CP2_CPR_RGB_MIN;
	} else if (value > CPU_CP2_CPR_RGB_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_RGB_MAX;
	}
	return (u32)(u8)value;
}

static ALWAYS_INLINE u32 gte_chk_rgb_r(struct psycho_ctx *const ctx,
				       const s32 value)
{
	return gte_chk_rgb(ctx, value, CPU_CP2_CCR_FLAG_RGB_R_SATURATED);
}

static ALWAYS_INLINE u32 gte_chk_rgb_g(struct psycho_ctx *const ctx,
				       const s32 value)
{
	return gte_chk_rgb(ctx, value, CPU_CP2_CCR_FLAG_RGB_G_SATURATED);
}

static ALWAYS_INLINE u32 gte_chk_rgb_b(struct psycho_ctx *const ctx,
				       const s32 value)
{
	return gte_chk_rgb(ctx, value, CPU_CP2_CCR_FLAG_RGB_B_SATURATED);
}

static ALWAYS_INLINE void gte_rgb_push(struct psycho_ctx *const ctx)
{
	const u32 b = (gte_chk_rgb_b(ctx, MAC3 >> 4)) << 16;
	const u32 g = (gte_chk_rgb_g(ctx, MAC2 >> 4)) << 8;
	const u32 r = gte_chk_rgb_r(ctx, MAC1 >> 4);

	RGB0 = RGB1;
	RGB1 = RGB2;
	RGB2 = ((RGBC >> 24) << 24) | b | g | r;

	const bool lm = ctx->cpu.instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);
}

static void gte_intpl_color(struct psycho_ctx *const ctx)
{
	const uint SHIFT_FRAC = cpu_instr_shift_frac_get(ctx->cpu.instr);

	s64 x = (s64)(((u64)RFC << 12) - (u64)MAC1);
	s64 y = (s64)(((u64)GFC << 12) - (u64)MAC2);
	s64 z = (s64)(((u64)BFC << 12) - (u64)MAC3);

	(void)gte_mac1_add(ctx, 0, x);
	(void)gte_mac2_add(ctx, 0, y);
	(void)gte_mac3_add(ctx, 0, z);

	IR1 = gte_chk_ir1(ctx, (s32)(x >> SHIFT_FRAC), false);
	IR2 = gte_chk_ir2(ctx, (s32)(y >> SHIFT_FRAC), false);
	IR3 = gte_chk_ir3(ctx, (s32)(z >> SHIFT_FRAC), false);

	s64 sum = gte_mac1_add(ctx, 0, (IR1 * IR0) + MAC1);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = gte_mac2_add(ctx, 0, (IR2 * IR0) + MAC2);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = gte_mac3_add(ctx, 0, (IR3 * IR0) + MAC3);
	MAC3 = (s32)(sum >> SHIFT_FRAC);
}

static void gte_dpc(struct psycho_ctx *const ctx, const u32 rgb)
{
	MAC1 = (s32)((rgb & 0xFF) << 16);
	MAC2 = (s32)(((rgb >> 8) & 0xFF) << 16);
	MAC3 = (s32)(((rgb >> 16) & 0xFF) << 16);

	gte_intpl_color(ctx);
	gte_rgb_push(ctx);
	gte_flag_update(ctx);
}

static void gte_nc(struct psycho_ctx *const ctx, const s16 x, const s16 y,
		   const s16 z)
{
	const uint SHIFT_FRAC = cpu_instr_shift_frac_get(ctx->cpu.instr);

	s64 sum = 0;
	sum = gte_mac1_add(ctx, sum, L11 * x);
	sum = gte_mac1_add(ctx, sum, L12 * y);
	sum = gte_mac1_add(ctx, sum, L13 * z);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, L21 * x);
	sum = gte_mac2_add(ctx, sum, L22 * y);
	sum = gte_mac2_add(ctx, sum, L23 * z);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, L31 * x);
	sum = gte_mac3_add(ctx, sum, L32 * y);
	sum = gte_mac3_add(ctx, sum, L33 * z);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	const bool lm = ctx->cpu.instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	sum = 0;
	sum = gte_mac1_add(ctx, sum, (s64)((u64)RBK << 12));
	sum = gte_mac1_add(ctx, sum, LR1 * IR1);
	sum = gte_mac1_add(ctx, sum, LR2 * IR2);
	sum = gte_mac1_add(ctx, sum, LR3 * IR3);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (s64)((u64)GBK << 12));
	sum = gte_mac2_add(ctx, sum, LG1 * IR1);
	sum = gte_mac2_add(ctx, sum, LG2 * IR2);
	sum = gte_mac2_add(ctx, sum, LG3 * IR3);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (s64)((u64)BBK << 12));
	sum = gte_mac3_add(ctx, sum, LB1 * IR1);
	sum = gte_mac3_add(ctx, sum, LB2 * IR2);
	sum = gte_mac3_add(ctx, sum, LB3 * IR3);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	gte_rgb_push(ctx);
	gte_flag_update(ctx);
}

static void gte_ncd(struct psycho_ctx *const ctx, const s16 x, const s16 y,
		    const s16 z)
{
	const uint SHIFT_FRAC = cpu_instr_shift_frac_get(ctx->cpu.instr);

	s64 sum = 0;
	sum = gte_mac1_add(ctx, sum, L11 * x);
	sum = gte_mac1_add(ctx, sum, L12 * y);
	sum = gte_mac1_add(ctx, sum, L13 * z);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, L21 * x);
	sum = gte_mac2_add(ctx, sum, L22 * y);
	sum = gte_mac2_add(ctx, sum, L23 * z);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, L31 * x);
	sum = gte_mac3_add(ctx, sum, L32 * y);
	sum = gte_mac3_add(ctx, sum, L33 * z);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	const bool lm = ctx->cpu.instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	sum = 0;
	sum = gte_mac1_add(ctx, sum, (s64)((u64)RBK << 12));
	sum = gte_mac1_add(ctx, sum, LR1 * IR1);
	sum = gte_mac1_add(ctx, sum, LR2 * IR2);
	sum = gte_mac1_add(ctx, sum, LR3 * IR3);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (s64)((u64)GBK << 12));
	sum = gte_mac2_add(ctx, sum, LG1 * IR1);
	sum = gte_mac2_add(ctx, sum, LG2 * IR2);
	sum = gte_mac2_add(ctx, sum, LG3 * IR3);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (s64)((u64)BBK << 12));
	sum = gte_mac3_add(ctx, sum, LB1 * IR1);
	sum = gte_mac3_add(ctx, sum, LB2 * IR2);
	sum = gte_mac3_add(ctx, sum, LB3 * IR3);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	sum = 0;
	sum = gte_mac1_add(ctx, sum, ((RGBC & 0xFF) * (u32)IR1) << 4);
	MAC1 = (s32)sum;

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (((RGBC >> 8) & 0xFF) * (u32)IR2) << 4);
	MAC2 = (s32)sum;

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (((RGBC >> 16) & 0xFF) * (u32)IR3) << 4);
	MAC3 = (s32)sum;

	gte_intpl_color(ctx);
	gte_rgb_push(ctx);
	gte_flag_update(ctx);
}

static void gte_rtp(struct psycho_ctx *const ctx, const s16 x, const s16 y,
		    const s16 z)
{
	static const u8 unr_table[] = {
		0xFF, 0xFD, 0xFB, 0xF9, 0xF7, 0xF5, 0xF3, 0xF1, 0xEF, 0xEE,
		0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE3, 0xE1, 0xDF, 0xDD, 0xDC,
		0xDA, 0xD8, 0xD6, 0xD5, 0xD3, 0xD1, 0xD0, 0xCE, 0xCD, 0xCB,
		0xC9, 0xC8, 0xC6, 0xC5, 0xC3, 0xC1, 0xC0, 0xBE, 0xBD, 0xBB,
		0xBA, 0xB8, 0xB7, 0xB5, 0xB4, 0xB2, 0xB1, 0xB0, 0xAE, 0xAD,
		0xAB, 0xAA, 0xA9, 0xA7, 0xA6, 0xA4, 0xA3, 0xA2, 0xA0, 0x9F,
		0x9E, 0x9C, 0x9B, 0x9A, 0x99, 0x97, 0x96, 0x95, 0x94, 0x92,
		0x91, 0x90, 0x8F, 0x8D, 0x8C, 0x8B, 0x8A, 0x89, 0x87, 0x86,
		0x85, 0x84, 0x83, 0x82, 0x81, 0x7F, 0x7E, 0x7D, 0x7C, 0x7B,
		0x7A, 0x79, 0x78, 0x77, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
		0x6F, 0x6E, 0x6D, 0x6C, 0x6B, 0x6A, 0x69, 0x68, 0x67, 0x66,
		0x65, 0x64, 0x63, 0x62, 0x61, 0x60, 0x5F, 0x5E, 0x5D, 0x5D,
		0x5C, 0x5B, 0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53,
		0x53, 0x52, 0x51, 0x50, 0x4F, 0x4E, 0x4D, 0x4D, 0x4C, 0x4B,
		0x4A, 0x49, 0x48, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x43,
		0x42, 0x41, 0x40, 0x3F, 0x3F, 0x3E, 0x3D, 0x3C, 0x3C, 0x3B,
		0x3A, 0x39, 0x39, 0x38, 0x37, 0x36, 0x36, 0x35, 0x34, 0x33,
		0x33, 0x32, 0x31, 0x31, 0x30, 0x2F, 0x2E, 0x2E, 0x2D, 0x2C,
		0x2C, 0x2B, 0x2A, 0x2A, 0x29, 0x28, 0x28, 0x27, 0x26, 0x26,
		0x25, 0x24, 0x24, 0x23, 0x22, 0x22, 0x21, 0x20, 0x20, 0x1F,
		0x1E, 0x1E, 0x1D, 0x1D, 0x1C, 0x1B, 0x1B, 0x1A, 0x19, 0x19,
		0x18, 0x18, 0x17, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14, 0x13,
		0x12, 0x12, 0x11, 0x11, 0x10, 0x0F, 0x0F, 0x0E, 0x0E, 0x0D,
		0x0D, 0x0C, 0x0C, 0x0B, 0x0A, 0x0A, 0x09, 0x09, 0x08, 0x08,
		0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03,
		0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00
	};

	const uint SHIFT_FRAC = cpu_instr_shift_frac_get(ctx->cpu.instr);

	s64 sum = 0;
	sum = gte_mac1_add(ctx, sum, (s64)((u64)TRX << 12));
	sum = gte_mac1_add(ctx, sum, RT11 * x);
	sum = gte_mac1_add(ctx, sum, RT12 * y);
	sum = gte_mac1_add(ctx, sum, RT13 * z);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (s64)((u64)TRY << 12));
	sum = gte_mac2_add(ctx, sum, RT21 * x);
	sum = gte_mac2_add(ctx, sum, RT22 * y);
	sum = gte_mac2_add(ctx, sum, RT23 * z);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (s64)((u64)TRZ << 12));
	sum = gte_mac3_add(ctx, sum, RT31 * x);
	sum = gte_mac3_add(ctx, sum, RT32 * y);
	sum = gte_mac3_add(ctx, sum, RT33 * z);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	gte_sz_push(ctx, sum);

	const bool lm = ctx->cpu.instr & CPU_INSTR_LM_FLAG;
	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, (s32)(sum >> 12), false);
	IR3 = (s16)clamp(MAC3,
			 lm ? CPU_CP2_CPR_IR123_LM_MIN : CPU_CP2_CPR_IR123_MIN,
			 CPU_CP2_CPR_IR123_MAX);

	s64 quot;

	if (H < (SZ3 * 2)) {
		const int i = SZ3 ? __builtin_clz(SZ3) - 16 : 16;
		quot = H << i;
		s64 d = SZ3 << i;
		const uint u = unr_table[(d - 0x7FC0) >> 7] + 0x101;
		d = (0x2000080 - (d * u)) >> 8;
		d = (0x0000080 + (d * u)) >> 8;
		quot = min(0x1FFFF, ((quot * d) + 0x8000) >> 16);
	} else {
		quot = 0x1FFFF;
		FLAG |= CPU_CP2_CCR_FLAG_DIV_OVF;
	}

	sum = gte_mac0_add(ctx, (quot * IR1) + OFX);
	const s16 sx = gte_chk_sx2(ctx, (s32)(sum >> 16));

	sum = gte_mac0_add(ctx, (quot * IR2) + OFY);
	const s16 sy = gte_chk_sy2(ctx, (s32)(sum >> 16));

	gte_sxy_push(ctx, sx, sy);

	sum = gte_mac0_add(ctx, (quot * DQA) + DQB);
	MAC0 = (s32)sum;
	IR0 = gte_chk_ir0(ctx, (s32)(sum >> 12));

	gte_flag_update(ctx);
}

static void gte_ncc(struct psycho_ctx *const ctx, const s16 x, const s16 y,
		    const s16 z)
{
	const uint SHIFT_FRAC = cpu_instr_shift_frac_get(ctx->cpu.instr);

	s64 sum = 0;
	sum = gte_mac1_add(ctx, sum, L11 * x);
	sum = gte_mac1_add(ctx, sum, L12 * y);
	sum = gte_mac1_add(ctx, sum, L13 * z);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, L21 * x);
	sum = gte_mac2_add(ctx, sum, L22 * y);
	sum = gte_mac2_add(ctx, sum, L23 * z);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, L31 * x);
	sum = gte_mac3_add(ctx, sum, L32 * y);
	sum = gte_mac3_add(ctx, sum, L33 * z);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	const bool lm = ctx->cpu.instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	sum = 0;
	sum = gte_mac1_add(ctx, sum, (s64)((u64)RBK << 12));
	sum = gte_mac1_add(ctx, sum, LR1 * IR1);
	sum = gte_mac1_add(ctx, sum, LR2 * IR2);
	sum = gte_mac1_add(ctx, sum, LR3 * IR3);
	MAC1 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (s64)((u64)GBK << 12));
	sum = gte_mac2_add(ctx, sum, LG1 * IR1);
	sum = gte_mac2_add(ctx, sum, LG2 * IR2);
	sum = gte_mac2_add(ctx, sum, LG3 * IR3);
	MAC2 = (s32)(sum >> SHIFT_FRAC);

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (s64)((u64)BBK << 12));
	sum = gte_mac3_add(ctx, sum, LB1 * IR1);
	sum = gte_mac3_add(ctx, sum, LB2 * IR2);
	sum = gte_mac3_add(ctx, sum, LB3 * IR3);
	MAC3 = (s32)(sum >> SHIFT_FRAC);

	IR1 = gte_chk_ir1(ctx, MAC1, lm);
	IR2 = gte_chk_ir2(ctx, MAC2, lm);
	IR3 = gte_chk_ir3(ctx, MAC3, lm);

	sum = 0;
	sum = gte_mac1_add(ctx, sum, ((RGBC & 0xFF) * (u32)IR1) << 4);
	MAC1 = (s32)sum;

	sum = 0;
	sum = gte_mac2_add(ctx, sum, (((RGBC >> 8) & 0xFF) * (u32)IR2) << 4);
	MAC2 = (s32)sum;

	sum = 0;
	sum = gte_mac3_add(ctx, sum, (((RGBC >> 16) & 0xFF) * (u32)IR3) << 4);
	MAC3 = (s32)sum;

	sum = 0;
	sum = gte_mac1_add(ctx, sum, MAC1 >> SHIFT_FRAC);
	MAC1 = (s32)sum;

	sum = 0;
	sum = gte_mac2_add(ctx, sum, MAC2 >> SHIFT_FRAC);
	MAC2 = (s32)sum;

	sum = 0;
	sum = gte_mac3_add(ctx, sum, MAC3 >> SHIFT_FRAC);
	MAC3 = (s32)sum;

	gte_rgb_push(ctx);
	gte_flag_update(ctx);
}

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

	case GROUP_COP2:
		switch (rs) {
		case MF:
			switch (rd) {
			case CPU_CP2_CPR_IR0:
			case CPU_CP2_CPR_IR1:
			case CPU_CP2_CPR_IR2:
			case CPU_CP2_CPR_IR3:
				GPR[rt] = (u32)(s16)CP2_CPR[rd];
				break;

			case CPU_CP2_CPR_SXYP:
				GPR[rt] = (u32)SXY2;
				break;

			case CPU_CP2_CPR_IRGB:
			case CPU_CP2_CPR_ORGB: {
				s32 b = IR3 >> 7;
				s32 g = IR2 >> 7;
				s32 r = IR1 >> 7;

				b = clamp(b, 0x00, 0x1F) << 10;
				g = clamp(g, 0x00, 0x1F) << 5;
				r = clamp(r, 0x00, 0x1F);

				GPR[rt] = (u32)(b | g | r);
				break;
			}

			case CPU_CP2_CPR_LZCR: {
				if (LZCS > 0) {
					GPR[rt] =
						(u32)__builtin_clz((uint)LZCS);
				} else if (LZCS < 0) {
					const uint n = (uint)~LZCS;
					GPR[rt] = n ? (u32)__builtin_clz(n) :
						      32;
				} else {
					GPR[rt] = 32;
				}
				break;
			}

			default:
				GPR[rt] = CP2_CPR[rd];
				break;
			}
			break;

		case CF:
			switch (rd) {
			case CPU_CP2_CCR_H:
				GPR[rt] = (u32)(s16)CP2_CCR[rd];
				break;

			default:
				GPR[rt] = CP2_CCR[rd];
				break;
			}
			break;

		case MT:
			switch (rd) {
			case CPU_CP2_CPR_VZ0:
			case CPU_CP2_CPR_VZ1:
			case CPU_CP2_CPR_VZ2:
			case CPU_CP2_CPR_IR0:
				CP2_CPR[rd] = (u32)(s16)GPR[rt];
				break;

			case CPU_CP2_CPR_OTZ:
			case CPU_CP2_CPR_SZ0:
			case CPU_CP2_CPR_SZ1:
			case CPU_CP2_CPR_SZ2:
			case CPU_CP2_CPR_SZ3:
				CP2_CPR[rd] = (u16)GPR[rt];
				break;

			case CPU_CP2_CPR_SXYP:
				SXY0 = SXY1;
				SXY1 = SXY2;
				SXY2 = (s32)GPR[rt];

				break;

			case CPU_CP2_CPR_IRGB:
				IR1 = (s16)((GPR[rt] & 0x1F) << 7);
				IR2 = (s16)(((GPR[rt] >> 5) & 0x1F) << 7);
				IR3 = (s16)(((GPR[rt] >> 10) & 0x1F) << 7);

				break;

			default:
				CP2_CPR[rd] = GPR[rt];
				break;
			}
			break;

		case CT:
			switch (rd) {
			case CPU_CP2_CCR_R33:
			case CPU_CP2_CCR_L33:
			case CPU_CP2_CCR_LB3:
			case CPU_CP2_CCR_DQA:
			case CPU_CP2_CCR_ZSF3:
			case CPU_CP2_CCR_ZSF4:
				CP2_CCR[rd] = (u32)(s16)GPR[rt];
				break;

			case CPU_CP2_CCR_FLAG:
				FLAG = GPR[rt] & CPU_CP2_CCR_FLAG_MASK_WRITE;
				gte_flag_update(ctx);

				break;

			default:
				CP2_CCR[rd] = GPR[rt];
				break;
			}
			break;

		default:
			switch (funct) {
			case RTPS:
				FLAG = 0;

				gte_rtp(ctx, VX0, VY0, VZ0);
				break;

			case NCLIP: {
				FLAG = 0;

				MAC0 = gte_mac0_add(
					ctx, (s64)(SX0 * (SY1 - SY2)) +
						     (SX1 * (SY2 - SY0)) +
						     (SX2 * (SY0 - SY1)));

				gte_flag_update(ctx);
				break;
			}

			case OP: {
				FLAG = 0;

				const uint SHIFT_FRAC =
					cpu_instr_shift_frac_get(
						ctx->cpu.instr);

				s64 sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   (IR3 * D2) - (IR2 * D3));
				MAC1 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac2_add(ctx, sum,
						   (IR1 * D3) - (IR3 * D1));
				MAC2 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac3_add(ctx, sum,
						   (IR2 * D1) - (IR1 * D2));
				MAC3 = (s32)(sum >> SHIFT_FRAC);

				const bool lm = ctx->cpu.instr &
						CPU_INSTR_LM_FLAG;

				IR1 = gte_chk_ir1(ctx, MAC1, lm);
				IR2 = gte_chk_ir2(ctx, MAC2, lm);
				IR3 = gte_chk_ir3(ctx, MAC3, lm);

				gte_flag_update(ctx);
				break;
			}

			case DPCS:
				FLAG = 0;

				gte_dpc(ctx, RGBC);
				break;

			case INTPL:
				FLAG = 0;

				MAC1 = IR1 << 12;
				MAC2 = IR2 << 12;
				MAC3 = IR3 << 12;

				gte_intpl_color(ctx);
				gte_rgb_push(ctx);
				gte_flag_update(ctx);

				break;

			case MVMVA: {
				FLAG = 0;

				s32 Tx1;
				s32 Tx2;
				s32 Tx3;
				s16 Vx1;
				s16 Vx2;
				s16 Vx3;
				s16 Mx11;
				s16 Mx12;
				s16 Mx13;
				s16 Mx21;
				s16 Mx22;
				s16 Mx23;
				s16 Mx31;
				s16 Mx32;
				s16 Mx33;

				const uint mx =
					cpu_instr_mx_get(ctx->cpu.instr);
				const uint vx =
					cpu_instr_vx_get(ctx->cpu.instr);
				const uint tx =
					cpu_instr_tx_get(ctx->cpu.instr);

				switch (mx) {
				case 0:
					Mx11 = RT11;
					Mx12 = RT12;
					Mx13 = RT13;
					Mx21 = RT21;
					Mx22 = RT22;
					Mx23 = RT23;
					Mx31 = RT31;
					Mx32 = RT32;
					Mx33 = RT33;

					break;

				case 1:
					Mx11 = L11;
					Mx12 = L12;
					Mx13 = L13;
					Mx21 = L21;
					Mx22 = L22;
					Mx23 = L23;
					Mx31 = L31;
					Mx32 = L32;
					Mx33 = L33;

					break;

				case 2:
					Mx11 = LR1;
					Mx12 = LR2;
					Mx13 = LR3;
					Mx21 = LG1;
					Mx22 = LG2;
					Mx23 = LG3;
					Mx31 = LB1;
					Mx32 = LB2;
					Mx33 = LB3;

					break;

				case 3:
					Mx11 = (s16) - ((RGBC & 0xFF) << 4);
					Mx12 = (s16)((RGBC & 0xFF) << 4);
					Mx13 = IR0;
					Mx21 = RT13;
					Mx22 = RT13;
					Mx23 = RT13;
					Mx31 = RT22;
					Mx32 = RT22;
					Mx33 = RT22;

					break;

				default:
					UNREACHABLE;
				}

				switch (vx) {
				case 0:
					Vx1 = VX0;
					Vx2 = VY0;
					Vx3 = VZ0;
					break;

				case 1:
					Vx1 = VX1;
					Vx2 = VY1;
					Vx3 = VZ1;
					break;

				case 2:
					Vx1 = VX2;
					Vx2 = VY2;
					Vx3 = VZ2;
					break;

				case 3:
					Vx1 = IR1;
					Vx2 = IR2;
					Vx3 = IR3;
					break;

				default:
					UNREACHABLE;
				}

				switch (tx) {
				case 0:
					Tx1 = TRX;
					Tx2 = TRY;
					Tx3 = TRZ;
					break;

				case 1:
					Tx1 = RBK;
					Tx2 = GBK;
					Tx3 = BBK;
					break;

				case 2:
					Tx1 = RFC;
					Tx2 = GFC;
					Tx3 = BFC;
					break;

				case 3:
					Tx1 = 0;
					Tx2 = 0;
					Tx3 = 0;
					break;

				default:
					UNREACHABLE;
				}

				s64 sum;
				const uint SHIFT_FRAC =
					cpu_instr_shift_frac_get(
						ctx->cpu.instr);
				const bool lm = ctx->cpu.instr &
						CPU_INSTR_LM_FLAG;

				if (tx == 2) {
					sum = gte_mac1_add(
						ctx, 0, (s64)((u64)Tx1 << 12));
					sum = gte_mac1_add(ctx, sum,
							   Mx11 * Vx1);
					MAC1 = (s32)(sum >> SHIFT_FRAC);
					IR1 = gte_chk_ir1(ctx, MAC1, false);

					sum = gte_mac1_add(ctx, 0, Mx12 * Vx2);
					sum = gte_mac1_add(ctx, sum,
							   Mx13 * Vx3);
					MAC1 = (s32)(sum >> SHIFT_FRAC);
					IR1 = gte_chk_ir1(ctx, MAC1, lm);

					sum = gte_mac2_add(
						ctx, 0, (s64)((u64)Tx2 << 12));
					sum = gte_mac2_add(ctx, sum,
							   Mx21 * Vx1);
					MAC2 = (s32)(sum >> SHIFT_FRAC);
					IR2 = gte_chk_ir2(ctx, MAC2, false);

					sum = gte_mac2_add(ctx, 0, Mx22 * Vx2);
					sum = gte_mac2_add(ctx, sum,
							   Mx23 * Vx3);
					MAC2 = (s32)(sum >> SHIFT_FRAC);
					IR2 = gte_chk_ir2(ctx, MAC2, lm);

					sum = gte_mac3_add(
						ctx, 0, (s64)((u64)Tx3 << 12));
					sum = gte_mac3_add(ctx, sum,
							   Mx31 * Vx1);
					MAC3 = (s32)(sum >> SHIFT_FRAC);
					IR3 = gte_chk_ir3(ctx, MAC3, false);

					sum = gte_mac3_add(ctx, 0, Mx32 * Vx2);
					sum = gte_mac3_add(ctx, sum,
							   Mx33 * Vx3);
					MAC3 = (s32)(sum >> SHIFT_FRAC);
					IR3 = gte_chk_ir3(ctx, MAC3, lm);
				} else {
					sum = gte_mac1_add(
						ctx, 0, (s64)((u64)Tx1 << 12));
					sum = gte_mac1_add(ctx, sum,
							   Mx11 * Vx1);
					sum = gte_mac1_add(ctx, sum,
							   Mx12 * Vx2);
					sum = gte_mac1_add(ctx, sum,
							   Mx13 * Vx3);
					MAC1 = (s32)(sum >> SHIFT_FRAC);

					sum = gte_mac2_add(
						ctx, 0, (s64)((u64)Tx2 << 12));
					sum = gte_mac2_add(ctx, sum,
							   Mx21 * Vx1);
					sum = gte_mac2_add(ctx, sum,
							   Mx22 * Vx2);
					sum = gte_mac2_add(ctx, sum,
							   Mx23 * Vx3);
					MAC2 = (s32)(sum >> SHIFT_FRAC);

					sum = gte_mac3_add(
						ctx, 0, (s64)((u64)Tx3 << 12));
					sum = gte_mac3_add(ctx, sum,
							   Mx31 * Vx1);
					sum = gte_mac3_add(ctx, sum,
							   Mx32 * Vx2);
					sum = gte_mac3_add(ctx, sum,
							   Mx33 * Vx3);
					MAC3 = (s32)(sum >> SHIFT_FRAC);

					IR1 = gte_chk_ir1(ctx, MAC1, lm);
					IR2 = gte_chk_ir2(ctx, MAC2, lm);
					IR3 = gte_chk_ir3(ctx, MAC3, lm);
				}
				gte_flag_update(ctx);
				break;
			}

			case NCDS:
				FLAG = 0;

				gte_ncd(ctx, VX0, VY0, VZ0);
				break;

			case CDP: {
				FLAG = 0;

				const uint SHIFT_FRAC =
					cpu_instr_shift_frac_get(
						ctx->cpu.instr);

				s64 sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   (s64)((u64)RBK << 12));
				sum = gte_mac1_add(ctx, sum, LR1 * IR1);
				sum = gte_mac1_add(ctx, sum, LR2 * IR2);
				sum = gte_mac1_add(ctx, sum, LR3 * IR3);
				MAC1 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac2_add(ctx, sum,
						   (s64)((u64)GBK << 12));
				sum = gte_mac2_add(ctx, sum, LG1 * IR1);
				sum = gte_mac2_add(ctx, sum, LG2 * IR2);
				sum = gte_mac2_add(ctx, sum, LG3 * IR3);
				MAC2 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac3_add(ctx, sum,
						   (s64)((u64)BBK << 12));
				sum = gte_mac3_add(ctx, sum, LB1 * IR1);
				sum = gte_mac3_add(ctx, sum, LB2 * IR2);
				sum = gte_mac3_add(ctx, sum, LB3 * IR3);
				MAC3 = (s32)(sum >> SHIFT_FRAC);

				const bool lm = ctx->cpu.instr &
						CPU_INSTR_LM_FLAG;

				IR1 = gte_chk_ir1(ctx, MAC1, lm);
				IR2 = gte_chk_ir2(ctx, MAC2, lm);
				IR3 = gte_chk_ir3(ctx, MAC3, lm);

				sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   ((RGBC & 0xFF) * (u32)IR1)
							   << 4);
				MAC1 = (s32)sum;

				sum = 0;
				sum = gte_mac2_add(
					ctx, sum,
					(((RGBC >> 8) & 0xFF) * (u32)IR2) << 4);
				MAC2 = (s32)sum;

				sum = 0;
				sum = gte_mac3_add(
					ctx, sum,
					(((RGBC >> 16) & 0xFF) * (u32)IR3)
						<< 4);
				MAC3 = (s32)sum;

				gte_intpl_color(ctx);
				gte_rgb_push(ctx);
				gte_flag_update(ctx);

				break;
			}

			case NCDT:
				FLAG = 0;

				gte_ncd(ctx, VX0, VY0, VZ0);
				gte_ncd(ctx, VX1, VY1, VZ1);
				gte_ncd(ctx, VX2, VY2, VZ2);

				break;

			case NCCS:
				FLAG = 0;

				gte_ncc(ctx, VX0, VY0, VZ0);
				break;

			case CC: {
				FLAG = 0;

				const uint SHIFT_FRAC =
					cpu_instr_shift_frac_get(
						ctx->cpu.instr);

				s64 sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   (s64)((u64)RBK << 12));
				sum = gte_mac1_add(ctx, sum, LR1 * IR1);
				sum = gte_mac1_add(ctx, sum, LR2 * IR2);
				sum = gte_mac1_add(ctx, sum, LR3 * IR3);
				MAC1 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac2_add(ctx, sum,
						   (s64)((u64)GBK << 12));
				sum = gte_mac2_add(ctx, sum, LG1 * IR1);
				sum = gte_mac2_add(ctx, sum, LG2 * IR2);
				sum = gte_mac2_add(ctx, sum, LG3 * IR3);
				MAC2 = (s32)(sum >> SHIFT_FRAC);

				sum = 0;
				sum = gte_mac3_add(ctx, sum,
						   (s64)((u64)BBK << 12));
				sum = gte_mac3_add(ctx, sum, LB1 * IR1);
				sum = gte_mac3_add(ctx, sum, LB2 * IR2);
				sum = gte_mac3_add(ctx, sum, LB3 * IR3);
				MAC3 = (s32)(sum >> SHIFT_FRAC);

				const bool lm = ctx->cpu.instr &
						CPU_INSTR_LM_FLAG;

				IR1 = gte_chk_ir1(ctx, MAC1, lm);
				IR2 = gte_chk_ir2(ctx, MAC2, lm);
				IR3 = gte_chk_ir3(ctx, MAC3, lm);

				sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   ((RGBC & 0xFF) * (u32)IR1)
							   << 4);
				MAC1 = (s32)sum;

				sum = 0;
				sum = gte_mac2_add(
					ctx, sum,
					(((RGBC >> 8) & 0xFF) * (u32)IR2) << 4);
				MAC2 = (s32)sum;

				sum = 0;
				sum = gte_mac3_add(
					ctx, sum,
					(((RGBC >> 16) & 0xFF) * (u32)IR3)
						<< 4);
				MAC3 = (s32)sum;

				sum = 0;
				sum = gte_mac1_add(ctx, sum,
						   MAC1 >> SHIFT_FRAC);
				MAC1 = (s32)sum;

				sum = 0;
				sum = gte_mac2_add(ctx, sum,
						   MAC2 >> SHIFT_FRAC);
				MAC2 = (s32)sum;

				sum = 0;
				sum = gte_mac3_add(ctx, sum,
						   MAC3 >> SHIFT_FRAC);
				MAC3 = (s32)sum;

				gte_rgb_push(ctx);
				gte_flag_update(ctx);

				break;
			}

			case NCS:
				FLAG = 0;

				gte_nc(ctx, VX0, VY0, VZ0);
				break;

			case NCT:
				FLAG = 0;

				gte_nc(ctx, VX0, VY0, VZ0);
				gte_nc(ctx, VX1, VY1, VZ1);
				gte_nc(ctx, VX2, VY2, VZ2);

				break;

			case SQR: {
				FLAG = 0;

				const uint SHIFT_FRAC =
					cpu_instr_shift_frac_get(
						ctx->cpu.instr);

				s64 sum;

				sum = gte_mac1_add(ctx, 0, IR1 * IR1);
				MAC1 = (s32)(sum >> SHIFT_FRAC);

				sum = gte_mac2_add(ctx, 0, IR2 * IR2);
				MAC2 = (s32)(sum >> SHIFT_FRAC);

				sum = gte_mac3_add(ctx, 0, IR3 * IR3);
				MAC3 = (s32)(sum >> SHIFT_FRAC);

				IR1 = gte_chk_ir1(ctx, MAC1, true);
				IR2 = gte_chk_ir2(ctx, MAC2, true);
				IR3 = gte_chk_ir3(ctx, MAC3, true);

				gte_flag_update(ctx);
				break;
			}

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
