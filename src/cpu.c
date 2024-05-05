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
#define GROUP_COP1	(CPU_OP_GROUP_COP1)
#define GROUP_COP2	(CPU_OP_GROUP_COP2)
#define GROUP_COP3	(CPU_OP_GROUP_COP3)
#define GROUP_SPECIAL	(CPU_OP_GROUP_SPECIAL)

#define ADD	(CPU_OP_ADD)
#define ADDI	(CPU_OP_ADDI)
#define ADDIU	(CPU_OP_ADDIU)
#define ADDU	(CPU_OP_ADDU)
#define AND	(CPU_OP_AND)
#define ANDI	(CPU_OP_ANDI)
#define AVSZ3	(CPU_OP_AVSZ3)
#define AVSZ4	(CPU_OP_AVSZ4)
#define BEQ	(CPU_OP_BEQ)
#define BGTZ	(CPU_OP_BGTZ)
#define BLEZ	(CPU_OP_BLEZ)
#define BNE	(CPU_OP_BNE)
#define BREAK	(CPU_OP_BREAK)
#define CC	(CPU_OP_CC)
#define CDP	(CPU_OP_CDP)
#define CF	(CPU_OP_CF)
#define CT	(CPU_OP_CT)
#define DCPL	(CPU_OP_DCPL)
#define DIV	(CPU_OP_DIV)
#define DIVU	(CPU_OP_DIVU)
#define DPCS	(CPU_OP_DPCS)
#define DPCT	(CPU_OP_DPCT)
#define GPF	(CPU_OP_GPF)
#define GPL	(CPU_OP_GPL)
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
#define LWC2	(CPU_OP_LWC2)
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
#define NCCS	(CPU_OP_NCCS)
#define NCCT	(CPU_OP_NCCT)
#define NCDS	(CPU_OP_NCDS)
#define NCDT	(CPU_OP_NCDT)
#define NCLIP	(CPU_OP_NCLIP)
#define NCS	(CPU_OP_NCS)
#define NCT	(CPU_OP_NCT)
#define NOR	(CPU_OP_NOR)
#define OP	(CPU_OP_OP)
#define OR	(CPU_OP_OR)
#define ORI	(CPU_OP_ORI)
#define RFE	(CPU_OP_RFE)
#define RTPS	(CPU_OP_RTPS)
#define RTPT	(CPU_OP_RTPT)
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
#define SWC2	(CPU_OP_SWC2)
#define SWL	(CPU_OP_SWL)
#define SWR	(CPU_OP_SWR)
#define SYSCALL	(CPU_OP_SYSCALL)
#define XOR	(CPU_OP_XOR)
#define XORI	(CPU_OP_XORI)

#define GPR	(cpu->gpr)
#define NPC	(cpu->npc)
#define PC 	(cpu->pc)

#define CP0_CPR	(cpu->cp0_cpr)
#define BadA	(CP0_CPR[CPU_CP0_CPR_BadA])
#define Cause	(CP0_CPR[CPU_CP0_CPR_Cause])
#define EPC	(CP0_CPR[CPU_CP0_CPR_EPC])
#define SR	(CP0_CPR[CPU_CP0_CPR_SR])

#define CP2_CPR	(cpu->cp2.cpr.regs)
#define IR0	(cpu->cp2.cpr.IR0)
#define IR1	(cpu->cp2.cpr.IR1)
#define IR2	(cpu->cp2.cpr.IR2)
#define IR3	(cpu->cp2.cpr.IR3)

#define LZCR	(cpu->cp2.cpr.LZCR)
#define LZCS	(cpu->cp2.cpr.LZCS)
#define MAC0	(cpu->cp2.MAC0)
#define MAC1	(cpu->cp2.MAC1)
#define MAC2	(cpu->cp2.MAC2)
#define MAC3	(cpu->cp2.MAC3)
#define OTZ	(cpu->cp2.cpr.OTZ)
#define RGB0	(cpu->cp2.cpr.RGB0)
#define RGBC	(cpu->cp2.cpr.RGBC)
#define SX0	((s16)(SXY0 & 0xFFFF))
#define SX1	((s16)(SXY1 & 0xFFFF))
#define SX2	((s16)(SXY2 & 0xFFFF))
#define SXY0	(cpu->cp2.cpr.SXY0)
#define SXY1	(cpu->cp2.cpr.SXY1)
#define SXY2	(cpu->cp2.cpr.SXY2)
#define SY0	((s16)(SXY0 >> 16))
#define SY1	((s16)(SXY1 >> 16))
#define SY2	((s16)(SXY2 >> 16))
#define SZ0	(cpu->cp2.cpr.SZ0)
#define SZ1	(cpu->cp2.cpr.SZ1)
#define SZ2	(cpu->cp2.cpr.SZ2)
#define SZ3	(cpu->cp2.cpr.SZ3)
#define V	(cpu->cp2.cpr.V)
#define V0	(V[0])
#define V1	(V[1])
#define V2	(V[2])

#define CP2_CCR	(cpu->cp2.ccr.regs)
#define BK	(cpu->cp2.ccr.BK)
#define FC	(cpu->cp2.ccr.FC)
#define RFC	(FC[0])
#define GFC	(FC[1])
#define BFC	(FC[2])
#define D1	(RT[0][0])
#define D2	(RT[1][1])
#define D3	(RT[2][2])
#define DQA	(cpu->cp2.ccr.DQA)
#define DQB	(cpu->cp2.ccr.DQB)
#define FLAG	(cpu->cp2.ccr.FLAG)
#define H	(cpu->cp2.ccr.H)
#define LCM	(cpu->cp2.ccr.LCM)
#define LLM	(cpu->cp2.ccr.LLM)
#define OFX	(cpu->cp2.ccr.OFX)
#define OFY	(cpu->cp2.ccr.OFY)
#define RT	(cpu->cp2.ccr.RT)
#define TR	(cpu->cp2.ccr.TR)
#define ZSF3	(cpu->cp2.ccr.ZSF3)
#define ZSF4	(cpu->cp2.ccr.ZSF4)

#define AdEL	(PSYCHO_CPU_EXC_CODE_AdEL)
#define AdES	(PSYCHO_CPU_EXC_CODE_AdES)
#define Bp	(PSYCHO_CPU_EXC_CODE_Bp)
#define Ovf	(PSYCHO_CPU_EXC_CODE_Ovf)
#define Sys	(PSYCHO_CPU_EXC_CODE_Sys)
#define RI	(PSYCHO_CPU_EXC_CODE_RI)

#define EXC_RAISE(exc_code)	(exc_raise(cpu, (exc_code)))
#define BRANCH_IF(cond)		(branch_if(cpu, (cond)))

#define LDS_NEXT	(cpu->lds_next)
#define LDS_PEND	(cpu->lds_pend)

#define HI	(cpu->hi)
#define LO	(cpu->lo)

#define IsC	(CPU_CP0_CPR_SR_IsC)

// clang-format on

const char *const exc_code_names[] = { [AdEL] = "Address error on load",
				       [AdES] = "Address error on store",
				       [Sys] = "System call",
				       [Bp] = "Breakpoint",
				       [RI] = "Reserved instruction",
				       [Ovf] = "Arithmetic overflow" };

static ALWAYS_INLINE void gte_flag_update(struct psycho_cpu *const cpu)
{
	if (FLAG & CPU_CP2_CCR_FLAG_MASK_ERR) {
		FLAG |= CPU_CP2_CCR_FLAG_ERR;
	}
}

static ALWAYS_INLINE s16 gte_chk_sxy(struct psycho_cpu *const cpu,
				     const s64 value, const uint flag)
{
	if (value < CPU_CP2_CPR_SXY2_MIN) {
		FLAG |= flag;
		return CPU_CP2_CPR_SXY2_MIN;
	}

	if (value > CPU_CP2_CPR_SXY2_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_SXY2_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_sx2(struct psycho_cpu *const cpu,
				     const s64 value)
{
	return gte_chk_sxy(cpu, value, CPU_CP2_CCR_FLAG_SX2_SATURATED);
}

static ALWAYS_INLINE s16 gte_chk_sy2(struct psycho_cpu *const cpu,
				     const s64 value)
{
	return gte_chk_sxy(cpu, value, CPU_CP2_CCR_FLAG_SY2_SATURATED);
}

static ALWAYS_INLINE u16 gte_chk_sz3_otz(struct psycho_cpu *const cpu,
					 const s64 value)
{
	if (value < CPU_CP2_CPR_SZ3_OTZ_MIN) {
		FLAG |= CPU_CP2_CCR_FLAG_SZ3_OR_OTZ_SATURATED;
		return CPU_CP2_CPR_SZ3_OTZ_MIN;
	}

	if (value > CPU_CP2_CPR_SZ3_OTZ_MAX) {
		FLAG |= CPU_CP2_CCR_FLAG_SZ3_OR_OTZ_SATURATED;
		return CPU_CP2_CPR_SZ3_OTZ_MAX;
	}
	return (u16)value;
}

static ALWAYS_INLINE s16 gte_chk_ir123(struct psycho_cpu *const cpu,
				       const s64 value, const uint flag,
				       const bool lm)
{
	const s16 min = lm ? CPU_CP2_CPR_IR123_LM_MIN : CPU_CP2_CPR_IR123_MIN;

	if (value < min) {
		FLAG |= flag;
		return min;
	}

	if (value > CPU_CP2_CPR_IR123_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_IR123_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_ir0(struct psycho_cpu *const cpu,
				     const s64 value)
{
	if (value < CPU_CP2_CPR_IR0_MIN) {
		FLAG |= CPU_CP2_CCR_FLAG_IR0_SATURATED;
		return CPU_CP2_CPR_IR0_MIN;
	}

	if (value > CPU_CP2_CPR_IR0_MAX) {
		FLAG |= CPU_CP2_CCR_FLAG_IR0_SATURATED;
		return CPU_CP2_CPR_IR0_MAX;
	}
	return (s16)value;
}

static ALWAYS_INLINE s16 gte_chk_ir1(struct psycho_cpu *const cpu,
				     const s64 value, const bool lm)
{
	return gte_chk_ir123(cpu, value, CPU_CP2_CCR_FLAG_IR1_SATURATED, lm);
}

static ALWAYS_INLINE s16 gte_chk_ir2(struct psycho_cpu *const cpu,
				     const s64 value, const bool lm)
{
	return gte_chk_ir123(cpu, value, CPU_CP2_CCR_FLAG_IR2_SATURATED, lm);
}

static ALWAYS_INLINE s16 gte_chk_ir3(struct psycho_cpu *const cpu,
				     const s64 value, const bool lm)
{
	return gte_chk_ir123(cpu, value, CPU_CP2_CCR_FLAG_IR3_SATURATED, lm);
}

static ALWAYS_INLINE void gte_ovf_chk(struct psycho_cpu *const cpu,
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

static ALWAYS_INLINE NODISCARD s64 gte_mac123_add(struct psycho_cpu *const cpu,
						  const s64 mac,
						  const s64 addend,
						  const uint neg_flag,
						  const uint pos_flag)
{
	const s64 sum = mac + addend;

	gte_ovf_chk(cpu, sum, CPU_CP2_CPR_MAC123_MIN, CPU_CP2_CPR_MAC123_MAX,
		    neg_flag, pos_flag);

	// Sign-extend result to 44 bits (64-20 = 44).
	return (s64)((u64)sum << 20) >> 20;
}

static ALWAYS_INLINE NODISCARD s64 gte_mac0_add(struct psycho_cpu *const cpu,
						const s64 sum)
{
	gte_ovf_chk(cpu, sum, CPU_CP2_CPR_MAC0_MIN, CPU_CP2_CPR_MAC0_MAX,
		    CPU_CP2_CCR_FLAG_MAC0_NEG_OVF,
		    CPU_CP2_CCR_FLAG_MAC0_POS_OVF);

	return sum;
}

static NODISCARD s64 gte_mac1_add(struct psycho_cpu *const cpu,
				  const s64 addend)
{
	return gte_mac123_add(cpu, MAC1, addend, CPU_CP2_CCR_FLAG_MAC1_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC1_POS_OVF);
}

static NODISCARD s64 gte_mac2_add(struct psycho_cpu *const cpu,
				  const s64 addend)
{
	return gte_mac123_add(cpu, MAC2, addend, CPU_CP2_CCR_FLAG_MAC2_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC2_POS_OVF);
}

static NODISCARD s64 gte_mac3_add(struct psycho_cpu *const cpu,
				  const s64 addend)
{
	return gte_mac123_add(cpu, MAC3, addend, CPU_CP2_CCR_FLAG_MAC3_NEG_OVF,
			      CPU_CP2_CCR_FLAG_MAC3_POS_OVF);
}

static NODISCARD ALWAYS_INLINE s64 gte_mac1_chk(struct psycho_cpu *const cpu,
						const s64 sum)
{
	gte_ovf_chk(cpu, sum, CPU_CP2_CPR_MAC123_MIN, CPU_CP2_CPR_MAC123_MAX,
		    CPU_CP2_CCR_FLAG_MAC1_NEG_OVF,
		    CPU_CP2_CCR_FLAG_MAC1_POS_OVF);
	return sum;
}

static NODISCARD ALWAYS_INLINE s64 gte_mac2_chk(struct psycho_cpu *const cpu,
						const s64 sum)
{
	gte_ovf_chk(cpu, sum, CPU_CP2_CPR_MAC123_MIN, CPU_CP2_CPR_MAC123_MAX,
		    CPU_CP2_CCR_FLAG_MAC2_NEG_OVF,
		    CPU_CP2_CCR_FLAG_MAC2_POS_OVF);
	return sum;
}

static NODISCARD ALWAYS_INLINE s64 gte_mac3_chk(struct psycho_cpu *const cpu,
						const s64 sum)
{
	gte_ovf_chk(cpu, sum, CPU_CP2_CPR_MAC123_MIN, CPU_CP2_CPR_MAC123_MAX,
		    CPU_CP2_CCR_FLAG_MAC3_NEG_OVF,
		    CPU_CP2_CCR_FLAG_MAC3_POS_OVF);
	return sum;
}

static void gte_matmul(struct psycho_cpu *const cpu, const s32 *const v0,
		       const s16 v1[3][3], const s16 *const v2)
{
#define iter(n)                                                              \
	({                                                                   \
		MAC##n = 0;                                                  \
		MAC##n = gte_mac##n##_add(cpu, (s64)((u64)v0[n - 1] << 12)); \
		MAC##n = gte_mac##n##_add(cpu, v1[n - 1][0] * v2[0]);        \
		MAC##n = gte_mac##n##_add(cpu, v1[n - 1][1] * v2[1]);        \
		MAC##n = gte_mac##n##_add(cpu, v1[n - 1][2] * v2[2]);        \
	})

	iter(1);
	iter(2);
	iter(3);
#undef iter
}

static void
gte_matmul_llm_vec(struct psycho_cpu *const cpu, const s16 *const vec)
{
	const uint sf = cpu_instr_shift_frac_get(cpu->instr);
	const bool lm = cpu->instr & CPU_INSTR_LM_FLAG;

#define iter(n)                                                         \
	({                                                              \
		MAC##n = 0;                                             \
		MAC##n = gte_mac##n##_add(cpu, LLM[n - 1][0] * vec[0]); \
		MAC##n = gte_mac##n##_add(cpu, LLM[n - 1][1] * vec[1]); \
		MAC##n = gte_mac##n##_add(cpu, LLM[n - 1][2] * vec[2]); \
		MAC##n >>= sf;                                          \
		IR##n = gte_chk_ir##n(cpu, (s32)MAC##n, lm);            \
	})

	iter(1);
	iter(2);
	iter(3);
#undef iter
}

static void gte_matmul_ir(struct psycho_cpu *const cpu, const s32 *const v0,
			  const s16 v1[3][3], const s16 *const v2)
{
	gte_matmul(cpu, v0, v1, v2);

	const uint sf = cpu_instr_shift_frac_get(cpu->instr);
	const bool lm = cpu->instr & CPU_INSTR_LM_FLAG;

#define iter(n)                                              \
	({                                                   \
		MAC##n >>= sf;                               \
		IR##n = gte_chk_ir##n(cpu, (s32)MAC##n, lm); \
	})

	iter(1);
	iter(2);
	iter(3);

#undef iter
}

static void gte_intpl_bk_lcm(struct psycho_cpu *const cpu)
{
	const uint sf = cpu_instr_shift_frac_get(cpu->instr);
	const bool lm = cpu->instr & CPU_INSTR_LM_FLAG;

#define iter(n)                                                              \
	({                                                                   \
		MAC##n = 0;                                                  \
		MAC##n = gte_mac##n##_add(cpu, (s64)((u64)BK[n - 1] << 12)); \
		MAC##n = gte_mac##n##_add(cpu, LCM[n - 1][0] * IR1);         \
		MAC##n = gte_mac##n##_add(cpu, LCM[n - 1][1] * IR2);         \
		MAC##n = gte_mac##n##_add(cpu, LCM[n - 1][2] * IR3);         \
		MAC##n >>= sf;                                               \
	})

	iter(1);
	iter(2);
	iter(3);
#undef iter

	IR1 = gte_chk_ir1(cpu, (s32)MAC1, lm);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, lm);
	IR3 = gte_chk_ir3(cpu, (s32)MAC3, lm);
}

static void gte_rtp(struct psycho_cpu *const cpu, const s16 *const vec,
		    const bool last_vertex)
{
	// XXX: This LUT is inefficient.
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

	const uint sf = cpu_instr_shift_frac_get(cpu->instr);

	gte_matmul(cpu, TR, RT, vec);
	MAC1 >>= sf;
	MAC2 >>= sf;

	SZ0 = SZ1;
	SZ1 = SZ2;
	SZ2 = SZ3;
	SZ3 = gte_chk_sz3_otz(cpu, MAC3 >> 12);

	const bool lm = cpu->instr & CPU_INSTR_LM_FLAG;
	IR1 = gte_chk_ir1(cpu, (s32)MAC1, lm);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, lm);
	IR3 = gte_chk_ir3(cpu, (s32)(MAC3 >> 12), false);

	MAC3 >>= sf;

	IR3 = (s16)clamp((s32)MAC3,
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

	MAC0 = gte_mac0_add(cpu, (quot * IR1) + OFX);
	const s16 sx = gte_chk_sx2(cpu, MAC0 >> 16);

	MAC0 = gte_mac0_add(cpu, (quot * IR2) + OFY);
	const s16 sy = gte_chk_sy2(cpu, MAC0 >> 16);

	SXY0 = SXY1;
	SXY1 = SXY2;
	SXY2 = (s32)(((u32)sx & 0xFFFF) | ((u32)sy << 16));

	if (last_vertex) {
		MAC0 = gte_mac0_add(cpu, (quot * DQA) + DQB);
		IR0 = gte_chk_ir0(cpu, MAC0 >> 12);
	}
	gte_flag_update(cpu);
}

static ALWAYS_INLINE u32 gte_chk_rgb(struct psycho_cpu *const cpu,
				     const s32 value, const uint flag)
{
	if (value < CPU_CP2_CPR_RGB_MIN) {
		FLAG |= flag;
		return CPU_CP2_CPR_RGB_MIN;
	}

	if (value > CPU_CP2_CPR_RGB_MAX) {
		FLAG |= flag;
		return CPU_CP2_CPR_RGB_MAX;
	}
	return (u32)(u8)value;
}

static ALWAYS_INLINE u32 gte_chk_rgb_r(struct psycho_cpu *const cpu,
				       const s32 value)
{
	return gte_chk_rgb(cpu, value, CPU_CP2_CCR_FLAG_RGB_R_SATURATED);
}

static ALWAYS_INLINE u32 gte_chk_rgb_g(struct psycho_cpu *const cpu,
				       const s32 value)
{
	return gte_chk_rgb(cpu, value, CPU_CP2_CCR_FLAG_RGB_G_SATURATED);
}

static ALWAYS_INLINE u32 gte_chk_rgb_b(struct psycho_cpu *const cpu,
				       const s32 value)
{
	return gte_chk_rgb(cpu, value, CPU_CP2_CCR_FLAG_RGB_B_SATURATED);
}

static ALWAYS_INLINE void gte_rgb_push(struct psycho_cpu *const cpu)
{
	const u32 b = gte_chk_rgb_b(cpu, (s32)MAC3 >> 4) << 16;
	const u32 g = gte_chk_rgb_g(cpu, (s32)MAC2 >> 4) << 8;
	const u32 r = gte_chk_rgb_r(cpu, (s32)MAC1 >> 4);

	CP2_CPR[CPU_CP2_CPR_RGB0] = CP2_CPR[CPU_CP2_CPR_RGB1];
	CP2_CPR[CPU_CP2_CPR_RGB1] = CP2_CPR[CPU_CP2_CPR_RGB2];
	CP2_CPR[CPU_CP2_CPR_RGB2] = (((u32)RGBC[3] << 24)) | b | g | r;

	const bool lm = cpu->instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(cpu, (s32)MAC1, lm);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, lm);
	IR3 = gte_chk_ir3(cpu, (s32)MAC3, lm);
}

static void gte_intpl_color(struct psycho_cpu *const cpu)
{
	const uint sf = cpu_instr_shift_frac_get(cpu->instr);

	const s32 m1 = (s32)MAC1;
	const s32 m2 = (s32)MAC2;
	const s32 m3 = (s32)MAC3;

	MAC1 = gte_mac1_chk(cpu, (s64)(((u64)RFC << 12) - (u64)m1)) >> sf;
	MAC2 = gte_mac2_chk(cpu, (s64)(((u64)GFC << 12) - (u64)m2)) >> sf;
	MAC3 = gte_mac3_chk(cpu, (s64)(((u64)BFC << 12) - (u64)m3)) >> sf;

	IR1 = gte_chk_ir1(cpu, (s32)MAC1, false);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, false);
	IR3 = gte_chk_ir3(cpu, (s32)MAC3, false);

	MAC1 = ((IR1 * IR0) + m1) >> sf;
	MAC2 = ((IR2 * IR0) + m2) >> sf;
	MAC3 = ((IR3 * IR0) + m3) >> sf;
}

static void gte_intpl_rgb(struct psycho_cpu *const cpu)
{
	MAC1 = (RGBC[0] * (u32)IR1) << 4;
	MAC2 = (RGBC[1] * (u32)IR2) << 4;
	MAC3 = (RGBC[2] * (u32)IR3) << 4;
}

static void gte_dpc(struct psycho_cpu *const cpu, const u8 *const rgb)
{
	MAC1 = rgb[0] << 16;
	MAC2 = rgb[1] << 16;
	MAC3 = rgb[2] << 16;

	gte_intpl_color(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);
}

static void gte_ncd(struct psycho_cpu *const cpu, const s16 *const vec)
{
	gte_matmul_llm_vec(cpu, vec);
	gte_intpl_bk_lcm(cpu);
	gte_intpl_rgb(cpu);
	gte_intpl_color(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);
}

static void gte_ncc(struct psycho_cpu *const cpu, const s16 *const vec)
{
	const uint sf = cpu_instr_shift_frac_get(cpu->instr);

	gte_matmul_llm_vec(cpu, vec);
	gte_intpl_bk_lcm(cpu);
	gte_intpl_rgb(cpu);

	MAC1 = (s32)MAC1 >> sf;
	MAC2 = (s32)MAC2 >> sf;
	MAC3 = (s32)MAC3 >> sf;

	gte_rgb_push(cpu);
	gte_flag_update(cpu);
}

static void gte_nc(struct psycho_cpu *const cpu, const s16 *const vec)
{
	gte_matmul_llm_vec(cpu, vec);
	gte_intpl_bk_lcm(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);
}

static void gte_avsz(struct psycho_cpu *const cpu, const s16 zsf, const u16 sz0)
{
	FLAG = 0;

	MAC0 = gte_mac0_add(cpu, (s64)(zsf) * (sz0 + SZ1 + SZ2 + SZ3));
	OTZ = gte_chk_sz3_otz(cpu, MAC0 >> 12);

	gte_flag_update(cpu);
}

/// @brief Branches to the target address if a condition was met.
/// @param ctx The psycho_ctx instance.
/// @param condition_met `true` if the branch condition was met, or `false`
/// otherwise.
static ALWAYS_INLINE void branch_if(struct psycho_cpu *const cpu,
				    const bool condition_met)
{
	if (condition_met) {
		NPC = cpu_branch_tgt_get(cpu->instr, PC);
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
static ALWAYS_INLINE void load_delay(struct psycho_cpu *const cpu,
				     const uint dst, const u32 val)
{
	if (unlikely(dst == CPU_GPR_zero)) {
		// $zero must always indeed remain $zero; psxtest_cpu exercises
		// this case.
		return;
	}

	LDS_PEND.dst = dst;
	LDS_PEND.val = val;

	LOG_TRACE(cpu->log, "Load delay now pending (dst=%d, val=0x%08X)", dst,
		  val);

	if (LDS_NEXT.dst == dst) {
		LOG_TRACE(cpu->log, "Evicting next load delay slot");
		memset(&LDS_NEXT, 0, sizeof(LDS_NEXT));
	}
}

/// @brief Raises an exception.
/// @param ctx The psycho_ctx instance.
/// @param exc_code The exception to raise.
static void exc_raise(struct psycho_cpu *const cpu, const uint exc_code)
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

	if (cpu->exc_halt & (1 << exc_code)) {
		LOG_ERR(cpu->log, "%s exception raised!", exc_name);
		return;
	}

	LOG_WARN(cpu->log, "%s exception raised!", exc_name);

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

	// 4) transfers control to the exception entry point.
	PC = CPU_VEC_EXC - sizeof(u32);
	NPC = CPU_VEC_EXC + sizeof(u32);
}

/// @brief Retrieves the virtual address for a load/store operation.
/// @param ctx The psycho_ctx instance.
/// @returns The virtual address.
static ALWAYS_INLINE NODISCARD u32 vaddr_get(const struct psycho_cpu *const cpu)
{
	const u32 base = cpu_instr_base_get(cpu->instr);
	const u32 offset = cpu_instr_offset_get(cpu->instr);

	return GPR[base] + offset;
}

/// @brief Retrieves the physical address for a load/store operation.
/// @param ctx The psycho_ctx instance.
/// @returns The physical address.
static ALWAYS_INLINE NODISCARD u32 paddr_get(const struct psycho_cpu *const cpu)
{
	const u32 vaddr = vaddr_get(cpu);
	return cpu_vaddr_to_paddr(vaddr);
}

/// @brief Fetches the instruction pointed to by the program counter.
///
/// This corresponds to the IF (Instruction Fetch) stage of the MIPS-I pipeline.
///
/// @param ctx The psycho_ctx instance.
/// @returns The instruction that was fetched.
static ALWAYS_INLINE NODISCARD u32 instr_fetch(struct psycho_cpu *const cpu)
{
	const u32 paddr = cpu_vaddr_to_paddr(PC);
	const u32 instr = bus_lw(cpu->bus, paddr);

	if (LDS_NEXT.dst) {
		LOG_TRACE(cpu->log,
			  "Flushing load delay slot (dest=%d, value=0x%08X)",
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
void cpu_reset(struct psycho_cpu *const cpu)
{
	memset(GPR, 0, sizeof(GPR));
	PC = CPU_VEC_RST;
	NPC = PC + sizeof(u32);

	cpu->instr = instr_fetch(cpu);

	memset(&LDS_NEXT, 0, sizeof(LDS_NEXT));
	memset(&LDS_PEND, 0, sizeof(LDS_PEND));

	LOG_INFO(cpu->log, "CPU reset!");
}

/// @brief Executes the current instruction.
/// @param cpu The psycho_cpu instance.
void cpu_step(struct psycho_cpu *const cpu)
{
	// clang-format off
#define op		(cpu_instr_op_get(cpu->instr))
#define rt		(cpu_instr_rt_get(cpu->instr))
#define rs		(cpu_instr_rs_get(cpu->instr))
#define rd		(cpu_instr_rd_get(cpu->instr))
#define funct		(cpu_instr_funct_get(cpu->instr))
#define shamt		(cpu_instr_shamt_get(cpu->instr))
#define ZEXT_IMM	(cpu_instr_zext_imm_get(cpu->instr))
#define SEXT_IMM	(cpu_instr_sext_imm_get(cpu->instr))
	// clang-format on

	// clang-format off
	static const void *const op_tbl[] = {
		[GROUP_SPECIAL]	= &&special,
		[GROUP_BCOND]	= &&bcond,
		[J]		= &&op_j,
		[JAL]		= &&op_jal,
		[BEQ]		= &&op_beq,
		[BNE]		= &&op_bne,
		[BLEZ]		= &&op_blez,
		[BGTZ]		= &&op_bgtz,
		[ADDI]		= &&op_addi,
		[ADDIU]		= &&op_addiu,
		[SLTI]		= &&op_slti,
		[SLTIU]		= &&op_sltiu,
		[ANDI]		= &&op_andi,
		[ORI]		= &&op_ori,
		[XORI]		= &&op_xori,
		[LUI]		= &&op_lui,
		[GROUP_COP0]	= &&cp0,
		[GROUP_COP1]	= &&illegal,
		[GROUP_COP2]	= &&cp2,
		[GROUP_COP3]	= &&illegal,
		[0x14 ... 0x1F]	= &&illegal,
		[LB]		= &&op_lb,
		[LH]		= &&op_lh,
		[LWL]		= &&op_lwl,
		[LW]		= &&op_lw,
		[LBU]		= &&op_lbu,
		[LHU]		= &&op_lhu,
		[LWR]		= &&op_lwr,
		[0x27]		= &&illegal,
		[SB]		= &&op_sb,
		[SH]		= &&op_sh,
		[SWL]		= &&op_swl,
		[SW]		= &&op_sw,
		[0x2C ... 0x2D]	= &&illegal,
		[SWR]		= &&op_swr,
		[0x2F ... 0x31]	= &&illegal,
		[LWC2]		= &&op_lwc2,
		[0x33 ... 0x39]	= &&illegal,
		[SWC2]		= &&op_swc2,
		[0x3B ... 0x3F]	= &&illegal
	};

	// clang-format off
	static const void *const special_tbl[] = {
		[SLL] 		= &&op_sll,
		[0x01]		= &&illegal,
		[SRL]		= &&op_srl,
		[SRA]		= &&op_sra,
		[SLLV]		= &&op_sllv,
		[0x05]		= &&illegal,
		[SRLV]		= &&op_srlv,
		[SRAV]		= &&op_srav,
		[JR]		= &&op_jr,
		[JALR]		= &&op_jalr,
		[0x0A ... 0x0B]	= &&illegal,
		[SYSCALL]	= &&op_syscall,
		[BREAK]		= &&op_break,
		[0x0E ... 0x0F]	= &&illegal,
		[MFHI]		= &&op_mfhi,
		[MTHI]		= &&op_mthi,
		[MFLO]		= &&op_mflo,
		[MTLO]		= &&op_mtlo,
		[0x14 ... 0x17]	= &&illegal,
		[MULT]		= &&op_mult,
		[MULTU]		= &&op_multu,
		[DIV]		= &&op_div,
		[DIVU]		= &&op_divu,
		[0x1C ... 0x1F]	= &&illegal,
		[ADD]		= &&op_add,
		[ADDU]		= &&op_addu,
		[SUB]		= &&op_sub,
		[SUBU]		= &&op_subu,
		[AND]		= &&op_and,
		[OR]		= &&op_or,
		[XOR]		= &&op_xor,
		[NOR]		= &&op_nor,
		[0x28 ... 0x29]	= &&illegal,
		[SLT]		= &&op_slt,
		[SLTU]		= &&op_sltu,
		[0x2C ... 0x3F]	= &&illegal
	};

	static const void *const cp0_tbl[] = {
		[MF]		= &&op_cp0_mf,
		[0x01 ... 0x03]	= &&op_cp0_funct,
		[MT]		= &&op_cp0_mt,
		[0x05 ... 0x1F] = &&op_cp0_funct
	};

	static const void *const cp0_instr_tbl[] = {
		[0x00 ... 0x0F] = &&illegal,
		[RFE]		= &&op_rfe,
		[0x11 ... 0x3F] = &&illegal
	};

	static const void *const cp2_tbl[] = {
		[MF]		= &&op_cp2_mf,
		[0x01]		= &&op_cp2_funct,
		[CF]		= &&op_cp2_cf,
		[0x03]		= &&op_cp2_funct,
		[MT]		= &&op_cp2_mt,
		[0x05]		= &&op_cp2_funct,
		[CT]		= &&op_cp2_ct,
		[0x07 ... 0x1F] = &&op_cp2_funct,
	};

	static const void *const cp2_instr_tbl[] = {
		[0x00]		= &&illegal,
		[RTPS]		= &&op_rtps,
		[0x02 ... 0x05]	= &&illegal,
		[NCLIP]		= &&op_nclip,
		[0x07 ... 0x0B] = &&illegal,
		[OP]		= &&op_op,
		[0x0D ... 0x0F] = &&illegal,
		[DPCS]		= &&op_dpcs,
		[INTPL]		= &&op_intpl,
		[MVMVA]		= &&op_mvmva,
		[NCDS]		= &&op_ncds,
		[CDP]		= &&op_cdp,
		[0x15]		= &&illegal,
		[NCDT]		= &&op_ncdt,
		[0x17 ... 0x1A]	= &&illegal,
		[NCCS]		= &&op_nccs,
		[CC]		= &&op_cc,
		[0x1D]		= &&illegal,
		[NCS]		= &&op_ncs,
		[0x1F]		= &&illegal,
		[NCT]		= &&op_nct,
		[0x21 ... 0x27]	= &&illegal,
		[SQR]		= &&op_sqr,
		[DCPL]		= &&op_dcpl,
		[DPCT]		= &&op_dpct,
		[0x2B ... 0x2C] = &&illegal,
		[AVSZ3]		= &&op_avsz3,
		[AVSZ4]		= &&op_avsz4,
		[0x2F]		= &&illegal,
		[RTPT]		= &&op_rtpt,
		[0x31 ... 0x3C]	= &&illegal,
		[GPF]		= &&op_gpf,
		[GPL]		= &&op_gpl,
		[NCCT]		= &&op_ncct
	};

	static const void *const cp2_mf_tbl[] = {
		[CPU_CP2_CPR_VXY0 ... CPU_CP2_CPR_OTZ]	= &&cp2_mf_default,
		[CPU_CP2_CPR_IR0 ... CPU_CP2_CPR_IR3]	= &&cp2_mf_sext,
		[CPU_CP2_CPR_SXY0 ... CPU_CP2_CPR_SXY2]	= &&cp2_mf_default,
		[CPU_CP2_CPR_SXYP]			= &&cp2_mf_sxyp,
		[CPU_CP2_CPR_SZ0 ... CPU_CP2_CPR_RES1]	= &&cp2_mf_default,
		[CPU_CP2_CPR_MAC0]			= &&cp2_mf_mac0,
		[CPU_CP2_CPR_MAC1]			= &&cp2_mf_mac1,
		[CPU_CP2_CPR_MAC2]			= &&cp2_mf_mac2,
		[CPU_CP2_CPR_MAC3]			= &&cp2_mf_mac3,
		[CPU_CP2_CPR_IRGB ... CPU_CP2_CPR_ORGB]	= &&cp2_mf_iorgb,
		[CPU_CP2_CPR_LZCS]			= &&cp2_mf_default,
		[CPU_CP2_CPR_LZCR]			= &&cp2_mf_lzcr
	};

	static const void *const cp2_cf_tbl[] = {
		[CPU_CP2_CCR_R11R12 ... CPU_CP2_CCR_OFY]	= &&cp2_cf_default,
		[CPU_CP2_CCR_H]					= &&cp2_cf_zext,
		[CPU_CP2_CCR_DQA ... CPU_CP2_CCR_FLAG]		= &&cp2_cf_default
	};

	static const void *const cp2_mt_tbl[] = {
		[CPU_CP2_CPR_VXY0]			= &&cp2_mt_default,
		[CPU_CP2_CPR_VZ0]			= &&cp2_mt_sext,
		[CPU_CP2_CPR_VXY1]			= &&cp2_mt_default,
		[CPU_CP2_CPR_VZ1]			= &&cp2_mt_sext,
		[CPU_CP2_CPR_VXY2]			= &&cp2_mt_default,
		[CPU_CP2_CPR_VZ2]			= &&cp2_mt_sext,
		[CPU_CP2_CPR_RGB]			= &&cp2_mt_default,
		[CPU_CP2_CPR_OTZ]			= &&cp2_mt_zext,
		[CPU_CP2_CPR_IR0]			= &&cp2_mt_sext,
		[CPU_CP2_CPR_IR1 ... CPU_CP2_CPR_SXY2]	= &&cp2_mt_default,
		[CPU_CP2_CPR_SXYP]			= &&cp2_mt_sxyp,
		[CPU_CP2_CPR_SZ0 ... CPU_CP2_CPR_SZ3]	= &&cp2_mt_zext,
		[CPU_CP2_CPR_RGB0 ... CPU_CP2_CPR_RES1]	= &&cp2_mt_default,
		[CPU_CP2_CPR_MAC0]			= &&cp2_mt_mac0,
		[CPU_CP2_CPR_MAC1]			= &&cp2_mt_mac1,
		[CPU_CP2_CPR_MAC2]			= &&cp2_mt_mac2,
		[CPU_CP2_CPR_MAC3]			= &&cp2_mt_mac3,
		[CPU_CP2_CPR_IRGB]			= &&cp2_mt_irgb,
		[CPU_CP2_CPR_ORGB ... CPU_CP2_CPR_LZCR]	= &&cp2_mt_default
	};

	static const void *const cp2_ct_tbl[] = {
		[CPU_CP2_CCR_R11R12 ... CPU_CP2_CCR_R31R32]	= &&cp2_ct_default,
		[CPU_CP2_CCR_R33]				= &&cp2_ct_sext,
		[CPU_CP2_CCR_TRX ... CPU_CP2_CCR_L31L32]	= &&cp2_ct_default,
		[CPU_CP2_CCR_L33]				= &&cp2_ct_sext,
		[CPU_CP2_CCR_RBK ... CPU_CP2_CCR_LB1LB2]	= &&cp2_ct_default,
		[CPU_CP2_CCR_LB3]				= &&cp2_ct_sext,
		[CPU_CP2_CCR_RFC ... CPU_CP2_CCR_H]		= &&cp2_ct_default,
		[CPU_CP2_CCR_DQA]				= &&cp2_ct_sext,
		[CPU_CP2_CCR_DQB]				= &&cp2_ct_default,
		[CPU_CP2_CCR_ZSF3 ... CPU_CP2_CCR_ZSF4]		= &&cp2_ct_sext,
		[CPU_CP2_CCR_FLAG]				= &&cp2_ct_flag
	};

	static const void *const mvmva_mx_tbl[] = {
		[0]	= &&op_mvmva_mx_rt,
		[1]	= &&op_mvmva_mx_llm,
		[2]	= &&op_mvmva_mx_lcm,
		[3]	= &&op_mvmva_mx_bugged
	};

	static const void *const mvmva_vx_tbl[] = {
		[0]	= &&op_mvmva_vx_v0,
		[1]	= &&op_mvmva_vx_v1,
		[2]	= &&op_mvmva_vx_v2,
		[3]	= &&op_mvmva_vx_ir
	};

	static const void *const mvmva_tx_tbl[] = {
		[0]	= &&op_mvmva_tx_tr,
		[1]	= &&op_mvmva_tx_bk,
		[2]	= &&op_mvmva_tx_fc,
		[3]	= &&op_mvmva_tx_null
	};

	static const void *const mvmva_impl_tbl[] = {
		[0 ... 1]	= &&op_mvmva_impl,
		[2]		= &&op_mvmva_impl_bugged,
		[3]		= &&op_mvmva_impl
	};
	// clang-format on

	u64 prod;
	uint shift, mask, sf;
	u32 jmp_tgt, sum, paddr, vaddr, data, word, aligned_paddr;
	s32 b, g, r;
	s16 signed_hword;
	u16 hword;
	bool cond_met;
	u8 ubyte;
	s8 sbyte;
	bool lm;

	uint tx = 0;
	s16 Vx[3];
	s32 Tx[3];
	s16 Mx[3][3];

	PC = (u32)(NPC - sizeof(u32));
	NPC += sizeof(u32);

	goto *op_tbl[op];

special:
	goto *special_tbl[funct];

op_sll:
	GPR[rd] = GPR[rt] << shamt;
	goto end;

op_srl:
	GPR[rd] = GPR[rt] >> shamt;
	goto end;

op_sra:
	GPR[rd] = (u32)((s32)GPR[rt] >> shamt);
	goto end;

op_sllv:
	GPR[rd] = GPR[rt] << (GPR[rs] & 0x1F);
	goto end;

op_srlv:
	GPR[rd] = GPR[rt] >> (GPR[rs] & 0x1F);
	goto end;

op_srav:
	GPR[rd] = (u32)((s32)GPR[rt] >> (GPR[rs] & 0x1F));
	goto end;

op_jr:
	if (unlikely((GPR[rs] & 3) != 0)) {
		EXC_RAISE(AdEL);
		goto end;
	}
	NPC = GPR[rs];
	goto end;

op_jalr:
	jmp_tgt = GPR[rs];

	GPR[rd] = PC + 8;

	if (unlikely((jmp_tgt & 3) != 0)) {
		EXC_RAISE(AdEL);
		goto end;
	}
	NPC = jmp_tgt;
	goto end;

op_syscall:
	EXC_RAISE(Sys);
	goto end;

op_break:
	EXC_RAISE(Bp);
	goto end;

op_mfhi:
	GPR[rd] = HI;
	goto end;

op_mflo:
	GPR[rd] = LO;
	goto end;

op_mthi:
	HI = GPR[rs];
	goto end;

op_mtlo:
	LO = GPR[rs];
	goto end;

op_mult:
	prod = (u64)((s64)(s32)GPR[rs] * (s64)(s32)GPR[rt]);

	LO = (u32)(prod & 0xFFFFFFFF);
	HI = (u32)(prod >> 32);

	goto end;

op_multu:
	prod = (u64)GPR[rs] * (u64)GPR[rt];

	LO = prod & 0xFFFFFFFF;
	HI = (u32)(prod >> 32);

	goto end;

op_div:
	// The result of a division by zero is consistent with a simple
	// radix-2 ("one bit at a time") implementation.
	if ((s32)GPR[rt] == 0) {
		LO = ((s32)GPR[rs] < 0) ? 1 : 0xFFFFFFFF;
		HI = (u32)(s32)GPR[rs];
	} else if ((GPR[rs] == 0x80000000) && GPR[rt] == 0xFFFFFFFF) {
		LO = 0x80000000;
		HI = 0;
	} else {
		LO = (u32)((s32)GPR[rs] / (s32)GPR[rt]);
		HI = (u32)((s32)GPR[rs] % (s32)GPR[rt]);
	}
	goto end;

op_divu:
	if (GPR[rt] == 0) {
		LO = 0xFFFFFFFF;
		HI = GPR[rs];
	} else {
		LO = GPR[rs] / GPR[rt];
		HI = GPR[rs] % GPR[rt];
	}
	goto end;

op_add:
	sum = GPR[rs] + GPR[rt];

	if (ovf_add(GPR[rs], GPR[rt], sum)) {
		EXC_RAISE(Ovf);
		goto end;
	}
	GPR[rd] = sum;
	goto end;

op_addu:
	GPR[rd] = GPR[rs] + GPR[rt];
	goto end;

op_sub:
	sum = GPR[rs] - GPR[rt];

	if (ovf_sub(GPR[rs], GPR[rt], sum)) {
		EXC_RAISE(Ovf);
		goto end;
	}
	GPR[rd] = sum;
	goto end;

op_subu:
	GPR[rd] = GPR[rs] - GPR[rt];
	goto end;

op_and:
	GPR[rd] = GPR[rs] & GPR[rt];
	goto end;

op_or:
	GPR[rd] = GPR[rs] | GPR[rt];
	goto end;

op_xor:
	GPR[rd] = GPR[rs] ^ GPR[rt];
	goto end;

op_nor:
	GPR[rd] = ~(GPR[rs] | GPR[rt]);
	goto end;

op_slt:
	GPR[rd] = (s32)GPR[rs] < (s32)GPR[rt];
	goto end;

op_sltu:
	GPR[rd] = GPR[rs] < GPR[rt];
	goto end;

bcond:
	cond_met = ((s32)GPR[rs] < 0) ^ (rt & 1);

	if ((rt & 0x1E) == 0x10) {
		GPR[ra] = PC + 8;
	}

	BRANCH_IF(cond_met);
	goto end;

op_j:
	NPC = cpu_jmp_tgt_get(cpu->instr, cpu->pc);
	goto end;

op_jal:
	// Delayed load is "cancelled" by the JAL due to the interlock between
	// $ra in the LW and the implicit $ra store in the JAL.
	if (LDS_NEXT.dst == ra) {
		memset(&LDS_NEXT, 0, sizeof(LDS_NEXT));
	}

	GPR[ra] = PC + 8;
	NPC = cpu_jmp_tgt_get(cpu->instr, cpu->pc);

	goto end;

op_beq:
	BRANCH_IF(GPR[rs] == GPR[rt]);
	goto end;

op_bne:
	BRANCH_IF(GPR[rs] != GPR[rt]);
	goto end;

op_blez:
	BRANCH_IF((s32)GPR[rs] <= 0);
	goto end;

op_bgtz:
	BRANCH_IF((s32)GPR[rs] > 0);
	goto end;

op_ori:
	GPR[rt] = GPR[rs] | ZEXT_IMM;
	goto end;

op_xori:
	GPR[rt] = GPR[rs] ^ ZEXT_IMM;
	goto end;

op_addi:
	sum = GPR[rs] + SEXT_IMM;

	if (ovf_add(GPR[rs], SEXT_IMM, sum)) {
		EXC_RAISE(Ovf);
		goto end;
	}
	GPR[rt] = sum;
	goto end;

op_addiu:
	GPR[rt] = GPR[rs] + SEXT_IMM;
	goto end;

op_slti:
	GPR[rt] = (s32)GPR[rs] < (s32)SEXT_IMM;
	goto end;

op_sltiu:
	GPR[rt] = GPR[rs] < SEXT_IMM;
	goto end;

op_andi:
	GPR[rt] = GPR[rs] & ZEXT_IMM;
	goto end;

op_lui:
	GPR[rt] = ZEXT_IMM << 16;
	goto end;

cp0:
	goto *cp0_tbl[rs];

op_cp0_mf:
	GPR[rt] = CP0_CPR[rd];
	goto end;

op_cp0_mt:
	CP0_CPR[rd] = GPR[rt];
	goto end;

op_cp0_funct:
	goto *cp0_instr_tbl[funct];

op_rfe:
	SR = (SR & 0xFFFFFFF0) | ((SR & 0x3C) >> 2);
	goto end;

cp2:
	goto *cp2_tbl[rs];

op_cp2_mf:
	goto *cp2_mf_tbl[rd];

cp2_mf_default:
	GPR[rt] = CP2_CPR[rd];
	goto end;

cp2_mf_sext:
	GPR[rt] = (u32)(s16)CP2_CPR[rd];
	goto end;

cp2_mf_sxyp:
	GPR[rt] = (u32)SXY2;
	goto end;

cp2_mf_mac0:
	GPR[rt] = (u32)(s32)MAC0;
	goto end;

cp2_mf_mac1:
	GPR[rt] = (u32)(s32)MAC1;
	goto end;

cp2_mf_mac2:
	GPR[rt] = (u32)(s32)MAC2;
	goto end;

cp2_mf_mac3:
	GPR[rt] = (u32)(s32)MAC3;
	goto end;

cp2_mf_iorgb:
	b = clamp(IR3 >> 7, 0x00, 0x1F) << 10;
	g = clamp(IR2 >> 7, 0x00, 0x1F) << 5;
	r = clamp(IR1 >> 7, 0x00, 0x1F);

	GPR[rt] = (u32)(b | g | r);
	goto end;

op_cp2_cf:
	goto *cp2_cf_tbl[rd];

cp2_cf_default:
	GPR[rt] = CP2_CCR[rd];
	goto end;

cp2_cf_zext:
	GPR[rt] = (u32)(s16)CP2_CCR[rd];
	goto end;

op_cp2_mt:
	goto *cp2_mt_tbl[rd];

cp2_mt_default:
	CP2_CPR[rd] = GPR[rt];
	goto end;

cp2_mt_irgb:
	IR1 = (s16)((GPR[rt] & 0x1F) << 7);
	IR2 = (s16)(((GPR[rt] >> 5) & 0x1F) << 7);
	IR3 = (s16)(((GPR[rt] >> 10) & 0x1F) << 7);

	goto end;

cp2_mt_mac0:
	MAC0 = (s32)GPR[rt];
	goto end;

cp2_mt_mac1:
	MAC1 = (s32)GPR[rt];
	goto end;

cp2_mt_mac2:
	MAC2 = (s32)GPR[rt];
	goto end;

cp2_mt_mac3:
	MAC3 = (s32)GPR[rt];
	goto end;

cp2_mt_sxyp:
	SXY0 = SXY1;
	SXY1 = SXY2;
	SXY2 = (s32)GPR[rt];

	goto end;

cp2_mt_sext:
	CP2_CPR[rd] = (u32)(s16)GPR[rt];
	goto end;

cp2_mt_zext:
	CP2_CPR[rd] = (u16)GPR[rt];
	goto end;

op_cp2_ct:
	goto *cp2_ct_tbl[rd];

cp2_ct_default:
	CP2_CCR[rd] = GPR[rt];
	goto end;

cp2_ct_sext:
	CP2_CCR[rd] = (u32)(s16)GPR[rt];
	goto end;

cp2_ct_flag:
	FLAG = GPR[rt] & CPU_CP2_CCR_FLAG_MASK_WRITE;
	gte_flag_update(cpu);

	goto end;

op_cp2_funct:
	goto *cp2_instr_tbl[funct];

op_rtps:
	FLAG = 0;

	gte_rtp(cpu, V0, true);
	goto end;

op_rtpt:
	FLAG = 0;

	gte_rtp(cpu, V0, false);
	gte_rtp(cpu, V1, false);
	gte_rtp(cpu, V2, true);

	goto end;

op_gpf:
	MAC1 = 0;
	MAC2 = 0;
	MAC3 = 0;

	goto op_gpl;

op_gpl:
	sf = cpu_instr_shift_frac_get(cpu->instr);

	MAC1 = gte_mac1_chk(cpu, (s64)((u64)MAC1 << sf));
	MAC2 = gte_mac2_chk(cpu, (s64)((u64)MAC2 << sf));
	MAC3 = gte_mac3_chk(cpu, (s64)((u64)MAC3 << sf));

	goto op_gp;

op_gp:
	FLAG = 0;

	MAC1 = gte_mac1_add(cpu, IR1 * IR0) >> sf;
	MAC2 = gte_mac2_add(cpu, IR2 * IR0) >> sf;
	MAC3 = gte_mac3_add(cpu, IR3 * IR0) >> sf;

	gte_rgb_push(cpu);
	gte_flag_update(cpu);

	goto end;

op_nclip:
	FLAG = 0;

	MAC0 = gte_mac0_add(cpu, (s64)(SX0 * (SY1 - SY2)) +
					 (SX1 * (SY2 - SY0)) +
					 (SX2 * (SY0 - SY1)));

	gte_flag_update(cpu);
	goto end;

op_op:
	FLAG = 0;

	sf = cpu_instr_shift_frac_get(cpu->instr);

	MAC1 = gte_mac1_chk(cpu, (IR3 * D2) - (IR2 * D3)) >> sf;
	MAC2 = gte_mac2_chk(cpu, (IR1 * D3) - (IR3 * D1)) >> sf;
	MAC3 = gte_mac3_chk(cpu, (IR2 * D1) - (IR1 * D2)) >> sf;

	lm = cpu->instr & CPU_INSTR_LM_FLAG;

	IR1 = gte_chk_ir1(cpu, (s32)MAC1, lm);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, lm);
	IR3 = gte_chk_ir3(cpu, (s32)MAC3, lm);

	gte_flag_update(cpu);
	goto end;

op_dpcs:
	FLAG = 0;

	gte_dpc(cpu, RGBC);
	goto end;

op_intpl:
	FLAG = 0;

	MAC1 = IR1 << 12;
	MAC2 = IR2 << 12;
	MAC3 = IR3 << 12;

	gte_intpl_color(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);

	goto end;

op_mvmva:
	FLAG = 0;

	tx = cpu_instr_tx_get(cpu->instr);
	goto *mvmva_mx_tbl[cpu_instr_mx_get(cpu->instr)];

op_mvmva_mx_rt:
	memcpy(Mx, RT, sizeof(Mx));
	goto *mvmva_vx_tbl[cpu_instr_vx_get(cpu->instr)];

op_mvmva_mx_llm:
	memcpy(Mx, LLM, sizeof(Mx));
	goto *mvmva_vx_tbl[cpu_instr_vx_get(cpu->instr)];

op_mvmva_mx_lcm:
	memcpy(Mx, LCM, sizeof(Mx));
	goto *mvmva_vx_tbl[cpu_instr_vx_get(cpu->instr)];

op_mvmva_mx_bugged:
	Mx[0][0] = (s16) - ((RGBC[0]) << 4);
	Mx[0][1] = (s16)((RGBC[0]) << 4);
	Mx[0][2] = IR0;
	Mx[1][0] = RT[0][2];
	Mx[1][1] = RT[0][2];
	Mx[1][2] = RT[0][2];
	Mx[2][0] = RT[1][1];
	Mx[2][1] = RT[1][1];
	Mx[2][2] = RT[1][1];

	goto *mvmva_vx_tbl[cpu_instr_vx_get(cpu->instr)];

op_mvmva_vx_v0:
	memcpy(Vx, V0, sizeof(Vx));
	goto *mvmva_tx_tbl[tx];

op_mvmva_vx_v1:
	memcpy(Vx, V1, sizeof(Vx));
	goto *mvmva_tx_tbl[tx];

op_mvmva_vx_v2:
	memcpy(Vx, V2, sizeof(Vx));
	goto *mvmva_tx_tbl[tx];

op_mvmva_vx_ir:
	Vx[0] = IR1;
	Vx[1] = IR2;
	Vx[2] = IR3;

	goto *mvmva_tx_tbl[tx];

op_mvmva_tx_tr:
	memcpy(Tx, TR, sizeof(Tx));
	goto *mvmva_impl_tbl[tx];

op_mvmva_tx_bk:
	memcpy(Tx, BK, sizeof(Tx));
	goto *mvmva_impl_tbl[tx];

op_mvmva_tx_fc:
	memcpy(Tx, FC, sizeof(Tx));
	goto *mvmva_impl_tbl[tx];

op_mvmva_tx_null:
	memset(Tx, 0, sizeof(Tx));
	goto *mvmva_impl_tbl[tx];

op_mvmva_impl:
	gte_matmul_ir(cpu, Tx, Mx, Vx);
	gte_flag_update(cpu);
	goto end;

op_mvmva_impl_bugged:
	lm = cpu->instr & CPU_INSTR_LM_FLAG;
	sf = cpu_instr_shift_frac_get(cpu->instr);

#define iter(n)                                                              \
	({                                                                   \
		MAC##n = 0;                                                  \
		MAC##n = gte_mac##n##_add(cpu, (s64)((u64)Tx[n - 1] << 12)); \
		MAC##n = gte_mac##n##_add(cpu, Mx[n - 1][0] * Vx[0]);        \
		MAC##n >>= sf;                                               \
		IR##n = gte_chk_ir##n(cpu, (s32)MAC##n, false);              \
                                                                             \
		MAC##n = 0;                                                  \
		MAC##n = gte_mac##n##_add(cpu, Mx[n - 1][1] * Vx[1]);        \
		MAC##n = gte_mac##n##_add(cpu, Mx[n - 1][2] * Vx[2]);        \
		MAC##n >>= sf;                                               \
		IR##n = gte_chk_ir##n(cpu, (s32)MAC##n, lm);                 \
	})
	iter(1);
	iter(2);
	iter(3);
#undef iter
	gte_flag_update(cpu);
	goto end;

op_nccs:
	FLAG = 0;

	gte_ncc(cpu, V0);
	goto end;

op_cc:
	sf = cpu_instr_shift_frac_get(cpu->instr);

	FLAG = 0;

	gte_intpl_bk_lcm(cpu);
	gte_intpl_rgb(cpu);

	MAC1 = (s32)MAC1 >> sf;
	MAC2 = (s32)MAC2 >> sf;
	MAC3 = (s32)MAC3 >> sf;

	gte_rgb_push(cpu);
	gte_flag_update(cpu);

	goto end;

op_ncds:
	FLAG = 0;

	gte_ncd(cpu, V0);
	goto end;

op_cdp:
	FLAG = 0;

	gte_intpl_bk_lcm(cpu);
	gte_intpl_rgb(cpu);
	gte_intpl_color(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);

	goto end;

op_ncdt:
	FLAG = 0;

	gte_ncd(cpu, V0);
	gte_ncd(cpu, V1);
	gte_ncd(cpu, V2);

	goto end;

op_ncs:
	FLAG = 0;

	gte_nc(cpu, V0);
	goto end;

op_nct:
	FLAG = 0;

	gte_nc(cpu, V0);
	gte_nc(cpu, V1);
	gte_nc(cpu, V2);

	goto end;

op_sqr:
	FLAG = 0;

	sf = cpu_instr_shift_frac_get(cpu->instr);

	MAC1 = gte_mac1_chk(cpu, IR1 * IR1) >> sf;
	MAC2 = gte_mac2_chk(cpu, IR2 * IR2) >> sf;
	MAC3 = gte_mac3_chk(cpu, IR3 * IR3) >> sf;

	IR1 = gte_chk_ir1(cpu, (s32)MAC1, true);
	IR2 = gte_chk_ir2(cpu, (s32)MAC2, true);
	IR3 = gte_chk_ir3(cpu, (s32)MAC3, true);

	gte_flag_update(cpu);
	goto end;

op_dcpl:
	FLAG = 0;

	gte_intpl_rgb(cpu);
	gte_intpl_color(cpu);
	gte_rgb_push(cpu);
	gte_flag_update(cpu);

	goto end;

op_ncct:
	FLAG = 0;

	gte_ncc(cpu, V0);
	gte_ncc(cpu, V1);
	gte_ncc(cpu, V2);

	goto end;

op_dpct:
	FLAG = 0;

	gte_dpc(cpu, RGB0);
	gte_dpc(cpu, RGB0);
	gte_dpc(cpu, RGB0);

	goto end;

op_avsz3:
	FLAG = 0;

	gte_avsz(cpu, ZSF3, 0);
	goto end;

op_avsz4:
	FLAG = 0;

	gte_avsz(cpu, ZSF4, SZ0);
	goto end;

cp2_mf_lzcr:
	if (LZCS > 0) {
		GPR[rt] = (u32)__builtin_clz((uint)LZCS);
	} else if (LZCS < 0) {
		const uint n = (uint)~LZCS;
		GPR[rt] = n ? (u32)__builtin_clz(n) : 32;
	} else {
		GPR[rt] = 32;
	}
	goto end;

op_lb:
	paddr = paddr_get(cpu);
	sbyte = (s8)bus_lb(cpu->bus, paddr);

	load_delay(cpu, rt, (u32)sbyte);
	goto end;

op_lh:
	vaddr = vaddr_get(cpu);

	if (unlikely((vaddr & 1) != 0)) {
		EXC_RAISE(AdEL);
		goto end;
	}

	paddr = cpu_vaddr_to_paddr(vaddr);
	signed_hword = (s16)bus_lh(cpu->bus, paddr);

	load_delay(cpu, rt, (u32)signed_hword);
	goto end;

op_lwl:
	paddr = paddr_get(cpu);
	data = bus_lw(cpu->bus, paddr & (u32)~3);

	word = (LDS_NEXT.dst == rt) ? LDS_NEXT.val : GPR[rt];

	shift = (paddr & 3) * 8;
	mask = 0x00FFFFFF >> shift;

	word = (word & mask) | (data << (24 - shift));

	load_delay(cpu, rt, word);
	goto end;

op_lw:
	vaddr = vaddr_get(cpu);

	if (unlikely((vaddr & 3) != 0)) {
		EXC_RAISE(AdEL);
		goto end;
	}

	paddr = cpu_vaddr_to_paddr(vaddr);
	word = bus_lw(cpu->bus, paddr);

	load_delay(cpu, rt, word);
	goto end;

op_lbu:
	paddr = paddr_get(cpu);
	ubyte = bus_lb(cpu->bus, paddr);

	load_delay(cpu, rt, ubyte);
	goto end;

op_lhu:
	vaddr = vaddr_get(cpu);

	if (unlikely((vaddr & 1) != 0)) {
		EXC_RAISE(AdEL);
		goto end;
	}
	paddr = cpu_vaddr_to_paddr(vaddr);
	hword = bus_lh(cpu->bus, paddr);

	load_delay(cpu, rt, hword);
	goto end;

op_lwr:
	paddr = paddr_get(cpu);
	data = bus_lw(cpu->bus, paddr & (u32)~3);

	word = LDS_NEXT.dst == rt ? LDS_NEXT.val : GPR[rt];

	shift = (paddr & 3) * 8;
	mask = 0xFFFFFF00 << (24 - shift);

	word = (word & mask) | (data >> shift);

	load_delay(cpu, rt, word);
	goto end;

op_sb:
	paddr = paddr_get(cpu);

	bus_sb(cpu->bus, paddr, (u8)GPR[rt]);
	goto end;

op_sh:
	vaddr = vaddr_get(cpu);

	if (unlikely((vaddr & 1) != 0)) {
		EXC_RAISE(AdES);
		goto end;
	}
	paddr = cpu_vaddr_to_paddr(vaddr);
	bus_sh(cpu->bus, paddr, (u16)GPR[rt]);

	goto end;

op_swl:
	paddr = paddr_get(cpu);
	aligned_paddr = paddr & (u32)~3;

	word = bus_lw(cpu->bus, aligned_paddr);

	shift = (paddr & 3) * 8;
	mask = 0xFFFFFF00 << shift;

	word = (word & mask) | (GPR[rt] >> (24 - shift));

	bus_sw(cpu->bus, aligned_paddr, word);
	goto end;

op_sw:
	if (SR & IsC) {
		goto end;
	}

	vaddr = vaddr_get(cpu);

	if (unlikely((vaddr & 3) != 0)) {
		EXC_RAISE(AdES);
		goto end;
	}

	paddr = cpu_vaddr_to_paddr(vaddr);

	bus_sw(cpu->bus, paddr, GPR[rt]);
	goto end;

op_swr:
	paddr = paddr_get(cpu);
	aligned_paddr = paddr & (u32)~3;

	word = bus_lw(cpu->bus, aligned_paddr);

	shift = (paddr & 3) * 8;
	mask = 0x00FFFFFF >> (24 - shift);

	word = (word & mask) | (GPR[rt] << shift);

	bus_sw(cpu->bus, aligned_paddr, word);
	goto end;

op_lwc2:
	goto end;

op_swc2:
	goto end;

illegal:
	EXC_RAISE(RI);
	goto end;

end:
	GPR[zero] = 0;
	PC += sizeof(u32);
	cpu->instr = instr_fetch(cpu);
}
