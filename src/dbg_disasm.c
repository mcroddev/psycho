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

/// @file dbg_disasm.c Defines the implementation of the disassembler.
///
/// The disassembler simply takes an LR33300 instruction and converts it into
/// the equivalent assembly language. It has the ability to output the contents
/// of the affected register(s) pre- or post- instruction execution, assuming
/// the instruction is the next one to execute.
///
/// There are no provisions at this time to support custom output formats; this
/// would be quite slow, difficult to maintain, and non-trivial to handle.
/// Unfortunately, a common output format does not exist among emulators.
///
/// Counterintuitively, the disassembler has an important role to play with
/// respect to speed; if a full system trace is executing, we want to format
/// instructions as fast as possible to reduce the impact these operations have
/// on emulation itself. Indeed, we could probably get better performance with a
/// custom sprintf() function that supports exactly the operations we need and
/// nothing more. This does not address the performance concern of I/O
/// contention on disk, as that would be the responsibility of the frontend.
///
/// Development is much too early to worry about this to such an extent.
///
/// Note that since this file has a "dbg_" prefixed to it, this means the
/// functionality provided here may be compiled out entirely.

#include "psycho/ctx.h"
#include "psycho/cpu_defs.h"
#include "psycho/dbg_disasm.h"
#include "cpu_defs.h"
#include <stdio.h>
#include <string.h>

// clang-format off
#define GROUP_SPECIAL	(CPU_OP_GROUP_SPECIAL)
#define GROUP_BCOND	(CPU_OP_GROUP_BCOND)
#define GROUP_COP0	(CPU_OP_GROUP_COP0)
#define GROUP_COP2	(CPU_OP_GROUP_COP2)

#define ADD 	(CPU_OP_ADD)
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

#define zero	(CPU_GPR_zero)
#define at	(CPU_GPR_at)
#define v0	(CPU_GPR_v0)
#define v1	(CPU_GPR_v1)
#define a0	(CPU_GPR_a0)
#define a1	(CPU_GPR_a1)
#define a2	(CPU_GPR_a2)
#define a3	(CPU_GPR_a3)
#define t0	(CPU_GPR_t0)
#define t1	(CPU_GPR_t1)
#define t2	(CPU_GPR_t2)
#define t3	(CPU_GPR_t3)
#define t4	(CPU_GPR_t4)
#define t5	(CPU_GPR_t5)
#define t6	(CPU_GPR_t6)
#define t7	(CPU_GPR_t7)
#define s0	(CPU_GPR_s0)
#define s1	(CPU_GPR_s1)
#define s2	(CPU_GPR_s2)
#define s3	(CPU_GPR_s3)
#define s4	(CPU_GPR_s4)
#define s5	(CPU_GPR_s5)
#define s6	(CPU_GPR_s6)
#define s7	(CPU_GPR_s7)
#define t8	(CPU_GPR_t8)
#define t9	(CPU_GPR_t9)
#define k0	(CPU_GPR_k0)
#define k1	(CPU_GPR_k1)
#define gp	(CPU_GPR_gp)
#define sp	(CPU_GPR_sp)
#define fp	(CPU_GPR_fp)
#define ra	(CPU_GPR_ra)

#define Index		(CPU_CP0_CPR_Index)
#define Random		(CPU_CP0_CPR_Random)
#define EntryLo		(CPU_CP0_CPR_EntryLo)
#define BPC		(CPU_CP0_CPR_BPC)
#define Context		(CPU_CP0_CPR_Context)
#define BDA		(CPU_CP0_CPR_BDA)
#define TAR		(CPU_CP0_CPR_TAR)
#define DCIC		(CPU_CP0_CPR_DCIC)
#define BadA		(CPU_CP0_CPR_BadA)
#define BDAM		(CPU_CP0_CPR_BDAM)
#define EntryHi		(CPU_CP0_CPR_EntryHi)
#define BPCM		(CPU_CP0_CPR_BPCM)
#define SR		(CPU_CP0_CPR_SR)
#define Cause		(CPU_CP0_CPR_Cause)
#define EPC		(CPU_CP0_CPR_EPC)
#define PRId		(CPU_CP0_CPR_PRId)

#define VXY0	(CPU_CP2_CPR_VXY0)
#define VZ0	(CPU_CP2_CPR_VZ0)
#define VXY1	(CPU_CP2_CPR_VXY1)
#define VZ1	(CPU_CP2_CPR_VZ1)
#define VXY2	(CPU_CP2_CPR_VXY2)
#define VZ2	(CPU_CP2_CPR_VZ2)
#define RGB	(CPU_CP2_CPR_RGB)
#define OTZ	(CPU_CP2_CPR_OTZ)
#define IR0	(CPU_CP2_CPR_IR0)
#define IR1	(CPU_CP2_CPR_IR1)
#define IR2	(CPU_CP2_CPR_IR2)
#define IR3	(CPU_CP2_CPR_IR3)
#define SXY0	(CPU_CP2_CPR_SXY0)
#define SXY1	(CPU_CP2_CPR_SXY1)
#define SXY2	(CPU_CP2_CPR_SXY2)
#define SXYP	(CPU_CP2_CPR_SXYP)
#define SZ0	(CPU_CP2_CPR_SZ0)
#define SZ1	(CPU_CP2_CPR_SZ1)
#define SZ2	(CPU_CP2_CPR_SZ2)
#define SZ3	(CPU_CP2_CPR_SZ3)
#define RGB0	(CPU_CP2_CPR_RGB0)
#define RGB1	(CPU_CP2_CPR_RGB1)
#define RGB2	(CPU_CP2_CPR_RGB2)
#define RES1	(CPU_CP2_CPR_RES1)
#define MAC0	(CPU_CP2_CPR_MAC0)
#define MAC1	(CPU_CP2_CPR_MAC1)
#define MAC2	(CPU_CP2_CPR_MAC2)
#define MAC3	(CPU_CP2_CPR_MAC3)
#define IRGB	(CPU_CP2_CPR_IRGB)
#define ORGB	(CPU_CP2_CPR_ORGB)
#define LZCS	(CPU_CP2_CPR_LZCS)
#define LZCR	(CPU_CP2_CPR_LZCR)

#define R11R12	(CPU_CP2_CCR_R11R12)
#define R13R21	(CPU_CP2_CCR_R13R21)
#define R22R23	(CPU_CP2_CCR_R22R23)
#define R31R32	(CPU_CP2_CCR_R31R32)
#define R33	(CPU_CP2_CCR_R33)
#define TRX	(CPU_CP2_CCR_TRX)
#define TRY	(CPU_CP2_CCR_TRY)
#define TRZ	(CPU_CP2_CCR_TRZ)
#define L11L12	(CPU_CP2_CCR_L11L12)
#define L13L21	(CPU_CP2_CCR_L13L21)
#define L22L23	(CPU_CP2_CCR_L22L23)
#define L31L32	(CPU_CP2_CCR_L31L32)
#define L33	(CPU_CP2_CCR_L33)
#define RBK	(CPU_CP2_CCR_RBK)
#define GBK	(CPU_CP2_CCR_GBK)
#define BBK	(CPU_CP2_CCR_BBK)
#define LR1LR2	(CPU_CP2_CCR_LR1LR2)
#define LR3LG1	(CPU_CP2_CCR_LR3LG1)
#define LG2LG3	(CPU_CP2_CCR_LG2LG3)
#define LB1LB2	(CPU_CP2_CCR_LB1LB2)
#define LB3	(CPU_CP2_CCR_LB3)
#define RFC	(CPU_CP2_CCR_RFC)
#define GFC	(CPU_CP2_CCR_GFC)
#define BFC	(CPU_CP2_CCR_BFC)
#define OFX	(CPU_CP2_CCR_OFX)
#define OFY	(CPU_CP2_CCR_OFY)
#define H	(CPU_CP2_CCR_H)
#define DQA	(CPU_CP2_CCR_DQA)
#define DQB	(CPU_CP2_CCR_DQB)
#define ZSF3	(CPU_CP2_CCR_ZSF3)
#define ZSF4	(CPU_CP2_CCR_ZSF4)
#define FLAG	(CPU_CP2_CCR_FLAG)

#define COMMENT_GPR_RD	(0)
#define COMMENT_GPR_RT	(1)
#define COMMENT_LO	(2)
#define COMMENT_HI	(3)

/// @brief Resolve branch offsets to a branch target address.
#define COMMENT_BRANCH	(4)

/// @brief Resolve jump offsets to a jump target address.
#define COMMENT_JUMP	(5)

/// @brief Resolve virtual addresses and convert them to physical addresses.
#define COMMENT_PADDR	(6)

#define COMMENT_CP0_CPR_RD	(7)

/// @brief The number of spaces relative to the end of the disassembly result to
/// append for comments.
#define TRACE_NUM_SPACES (35)

/// @brief The character to use to start a comment section.
#define COMMENT_START_CHAR (';')

/// @brief The character to use to delimit comments.
#define COMMENT_DELIM (',')

const char *const psycho_cpu_gpr_names[PSYCHO_CPU_GPR_REGS_NUM] = {
	[zero] = "zero", [at] = "at", [v0] = "v0", [v1] = "v1", [a0] = "a0",
	[a1]   = "a1",	 [a2] = "a2", [a3] = "a3", [t0] = "t0", [t1] = "t1",
	[t2]   = "t2",	 [t3] = "t3", [t4] = "t4", [t5] = "t5", [t6] = "t6",
	[t7]   = "t7",	 [s0] = "s0", [s1] = "s1", [s2] = "s2", [s3] = "s3",
	[s4]   = "s4",	 [s5] = "s5", [s6] = "s6", [s7] = "s7", [t8] = "t8",
	[t9]   = "t9",	 [k0] = "k0", [k1] = "k1", [gp] = "gp", [sp] = "sp",
	[fp]   = "fp",	 [ra] = "ra"
};

const char *const psycho_cpu_cp0_cpr_names[PSYCHO_CPU_CP0_CPR_REGS_NUM] = {
	[Index]   = "C0_Index",	  [Random] = "C0_Random",
	[EntryLo] = "C0_EntryLo", [BPC]    = "C0_BPC",
	[Context] = "C0_Context", [BDA]	   = "C0_BDA",
	[TAR] 	  = "C0_TAR",	  [DCIC]   = "C0_DCIC",
	[BadA]	  = "C0_BadA",	  [BDAM]   = "C0_BDAM",
	[EntryHi] = "C0_EntryHi", [BPCM]   = "C0_BPCM",
	[SR]	  = "C0_SR",	  [Cause]  = "C0_Cause",
	[EPC]	  = "C0_EPC",	  [PRId]   = "C0_PRId",
	[16]	  = "C0_REG16",	  [17]	   = "C0_REG17",
	[18]	  = "C0_REG18",	  [19] 	   = "C0_REG19",
	[20]	  = "C0_REG20",	  [21]	   = "C0_REG21",
	[22]	  = "C0_REG22",	  [23]	   = "C0_REG23",
	[24]	  = "C0_REG24",	  [25]	   = "C0_REG25",
	[26]	  = "C0_REG26",	  [27]	   = "C0_REG27",
	[28]	  = "C0_REG28",	  [29]	   = "C0_REG29",
	[30]	  = "C0_REG30",	  [31]	   = "C0_REG31"
};

const char *const psycho_cpu_cp2_cpr_names[PSYCHO_CPU_CP2_CPR_REGS_NUM] = {
	[VXY0] = "C2_VXY0", [VZ0]  = "C2_VZ0",	[VXY1] = "C2_VXY1",
	[VZ1]  = "C2_VZ1",  [VXY2] = "C2_VXY2", [VZ2]  = "C2_VZ2",
	[RGB]  = "C2_RGB",  [OTZ]  = "C2_OTZ",	[IR0]  = "C2_IR0",
	[IR1]  = "C2_IR1",  [IR2]  = "C2_IR2",	[IR3]  = "C2_IR3",
	[SXY0] = "C2_SXY0", [SXY1] = "C2_SXY1", [SXY2] = "C2_SYX2",
	[SXYP] = "C2_SXYP", [SZ0]  = "C2_SZ0",	[SZ1]  = "C2_SZ1",
	[SZ2]  = "C2_SZ2",  [SZ3]  = "C2_SZ3",	[RGB0] = "C2_RGB0",
	[RGB1] = "C2_RGB1", [RGB2] = "C2_RGB2", [RES1] = "C2_RES1",
	[MAC0] = "C2_MAC0", [MAC1] = "C2_MAC1", [MAC2] = "C2_MAC2",
	[MAC3] = "C2_MAC3", [IRGB] = "C2_IRGB", [ORGB] = "C2_ORGB",
	[LZCS] = "C2_LZCS", [LZCR] = "C2_LZCR"
};

const char *const psycho_cpu_cp2_ccr_names[PSYCHO_CPU_CP2_CCR_REGS_NUM] = {
	[R11R12] = "C2_R11R12", [R13R21] = "C2_R13R21", [R22R23] = "C2_R22R23",
	[R31R32] = "C2_R31R23", [R33] 	 = "C2_R33",	[TRX] 	 = "C2_TRX",
	[TRY] 	 = "C2_TRY",	[TRZ] 	 = "C2_TRZ",	[L11L12] = "C2_L11L12",
	[L13L21] = "C2_L13L21", [L22L23] = "C2_L22L23", [L31L32] = "C2_L31L32",
	[L33] 	 = "C2_L33",	[RBK] 	 = "C2_RBK",	[GBK] 	 = "C2_GBK",
	[BBK] 	 = "C2_BBK",	[LR1LR2] = "C2_LR1LR2", [LR3LG1] = "C2_LR3LG1",
	[LG2LG3] = "C2_LG2LG3", [LB1LB2] = "C2_LB1LB2", [LB3] 	 = "C2_LB3",
	[RFC] 	 = "C2_RFC",	[GFC] 	 = "C2_GFC",	[BFC] 	 = "C2_BFC",
	[OFX] 	 = "C2_OFX",	[OFY] 	 = "C2_OFY",	[H] 	 = "C2_H",
	[DQA] 	 = "C2_DQA",	[DQB] 	 = "C2_DQB",	[ZSF3] 	 = "C2_ZSF3",
	[ZSF4] 	 = "C2_ZSF4",	[FLAG] 	 = "C2_FLAG"
};

#define GPR	(psycho_cpu_gpr_names)
#define CP0_CPR	(psycho_cpu_cp0_cpr_names)
#define CP2_CPR	(psycho_cpu_cp2_cpr_names)
#define CP2_CCR	(psycho_cpu_cp2_ccr_names)
// clang-format on

static ALWAYS_INLINE void res_set(struct psycho_ctx *const ctx,
				  const char *const str, const size_t len)
{
	memcpy(ctx->disasm.result, str, len);
	ctx->disasm.len = (int)len - 1;
}

static void output_comment(struct psycho_ctx *const ctx, const uint comment)
{
#define FORMAT(args...) \
	ctx->disasm.len += sprintf(&ctx->disasm.result[ctx->disasm.len], args)

#define rt (cpu_instr_rt_get(ctx->disasm.instr))
#define rd (cpu_instr_rd_get(ctx->disasm.instr))
#define base (cpu_instr_base_get(ctx->disasm.instr))
#define offset (cpu_instr_offset_get(ctx->disasm.instr))

	switch (comment) {
	case COMMENT_GPR_RT:
		FORMAT("%s=0x%08X", GPR[rt], ctx->cpu.gpr[rt]);
		break;

	case COMMENT_GPR_RD:
		FORMAT("%s=0x%08X", GPR[rd], ctx->cpu.gpr[rd]);
		break;

	case COMMENT_PADDR: {
		const u32 vaddr = ctx->cpu.gpr[base] + offset;
		const u32 paddr = cpu_vaddr_to_paddr(vaddr);

		FORMAT("paddr=0x%08X", paddr);
		break;
	}

	case COMMENT_JUMP: {
		const u32 addr =
			cpu_jmp_tgt_get(ctx->disasm.instr, ctx->disasm.pc);

		FORMAT("addr=0x%08X", addr);
		break;
	}

	case COMMENT_CP0_CPR_RD:
		FORMAT("%s=0x%08X", CP0_CPR[rd], ctx->cpu.cp0_cpr[rd]);
		break;

	case COMMENT_BRANCH: {
		const u32 addr =
			cpu_branch_tgt_get(ctx->disasm.instr, ctx->disasm.pc);
		FORMAT("addr=0x%08X", addr);

		break;
	}

	case COMMENT_LO:
		FORMAT("LO=0x%08X", ctx->cpu.lo);
		break;

	case COMMENT_HI:
		FORMAT("HI=0x%08X", ctx->cpu.hi);
		break;

	default:
		break;
	}

#undef FORMAT
#undef rt
#undef rd
#undef base
#undef offset
}

void psycho_dbg_disasm_instr(struct psycho_ctx *const ctx, const u32 instr,
			     const u32 pc)
{
#define FORMAT(args...) (ctx->disasm.len = sprintf(ctx->disasm.result, args))

#define RES_SET(str) (res_set(ctx, (str), sizeof(str)))

#define base (cpu_instr_base_get(instr))
#define op (cpu_instr_op_get(instr))
#define rd (cpu_instr_rd_get(instr))
#define rt (cpu_instr_rt_get(instr))
#define rs (cpu_instr_rs_get(instr))
#define offset ((s16)cpu_instr_offset_get(instr))
#define shamt (cpu_instr_shamt_get(instr))
#define funct (cpu_instr_funct_get(instr))
#define target (cpu_instr_target_get(instr))
#define ZEXT_IMM (cpu_instr_zext_imm_get(instr))
#define SEXT_IMM (offset)

#define COMMENT_ADD(comment) \
	(ctx->disasm.comments[ctx->disasm.num_comments++] = comment)

#define FORMAT_SHIFT_VAR(op_name)                                     \
	({                                                            \
		FORMAT(op_name " %s,%s,%u", GPR[rd], GPR[rt], shamt); \
		COMMENT_ADD(COMMENT_GPR_RD);                          \
	})

#define FORMAT_SHIFT_REG(op_name)                                       \
	({                                                              \
		FORMAT(op_name " %s,%s,%s", GPR[rd], GPR[rt], GPR[rs]); \
		COMMENT_ADD(COMMENT_GPR_RD);                            \
	})

#define FORMAT_MULT_DIV(op_name)                            \
	({                                                  \
		FORMAT(op_name " %s,%s", GPR[rs], GPR[rt]); \
		COMMENT_ADD(COMMENT_LO);                    \
		COMMENT_ADD(COMMENT_HI);                    \
	})

#define FORMAT_ARITH_REG(op_name)                                       \
	({                                                              \
		FORMAT(op_name " %s,%s,%s", GPR[rd], GPR[rs], GPR[rt]); \
		COMMENT_ADD(COMMENT_GPR_RD);                            \
	})

#define FORMAT_BRANCH_REG(op_name)                                   \
	({                                                           \
		FORMAT(op_name " %s,%s,%s0x%04hX", GPR[rs], GPR[rt], \
		       (offset < 0) ? "-" : "", offset);             \
		COMMENT_ADD(COMMENT_BRANCH);                         \
	})

#define FORMAT_BRANCH(op_name)                           \
	({                                               \
		FORMAT(op_name " %s,%s0x%04hX", GPR[rs], \
		       (offset < 0) ? "-" : "", offset); \
		COMMENT_ADD(COMMENT_BRANCH);             \
	})

#define FORMAT_LOAD_STORE(op_name)                                            \
	FORMAT(op_name " %s,%s0x%04hX(%s)", GPR[rt], (offset < 0) ? "-" : "", \
	       offset, GPR[base]);

#define FORMAT_LOAD(op_name)                 \
	({                                   \
		FORMAT_LOAD_STORE(op_name);  \
		COMMENT_ADD(COMMENT_GPR_RT); \
		COMMENT_ADD(COMMENT_PADDR);  \
	})

#define FORMAT_STORE(op_name)               \
	({                                  \
		FORMAT_LOAD_STORE(op_name); \
		COMMENT_ADD(COMMENT_PADDR); \
	})

#define FORMAT_ARITH_ZEXT_IMM(op_name)                                       \
	({                                                                   \
		FORMAT(op_name " %s,%s,0x%04X", GPR[rt], GPR[rs], ZEXT_IMM); \
		COMMENT_ADD(COMMENT_GPR_RT);                                 \
	})

#define FORMAT_ARITH_SEXT_IMM(op_name)                               \
	({                                                           \
		FORMAT(op_name " %s,%s,%s0x%04hX", GPR[rt], GPR[rs], \
		       (SEXT_IMM < 0) ? "-" : "", SEXT_IMM);         \
		COMMENT_ADD(COMMENT_GPR_RT);                         \
	})

#define ILLEGAL (FORMAT("illegal 0x%08X", instr))

	ctx->disasm.instr = instr;
	ctx->disasm.pc = pc;

	ctx->disasm.num_comments = 0;
	ctx->disasm.len = 0;

	switch (op) {
	case GROUP_SPECIAL:
		switch (funct) {
		case SLL:
			FORMAT_SHIFT_VAR("sll");
			return;

		case SRL:
			FORMAT_SHIFT_VAR("srl");
			return;

		case SRA:
			FORMAT_SHIFT_VAR("sra");
			return;

		case SLLV:
			FORMAT_SHIFT_REG("sllv");
			return;

		case SRLV:
			FORMAT_SHIFT_REG("srlv");
			return;

		case SRAV:
			FORMAT_SHIFT_REG("srav");
			return;

		case JR:
			FORMAT("jr %s", GPR[rs]);
			return;

		case JALR:
			FORMAT("jalr %s,%s", GPR[rd], GPR[rs]);
			COMMENT_ADD(COMMENT_GPR_RD);

			return;

		case SYSCALL:
			RES_SET("syscall");
			return;

		case BREAK:
			RES_SET("break");
			return;

		case MFHI:
			FORMAT("mfhi %s", GPR[rd]);
			COMMENT_ADD(COMMENT_GPR_RD);

			return;

		case MTHI:
			FORMAT("mthi %s", GPR[rs]);
			return;

		case MFLO:
			FORMAT("mflo %s", GPR[rd]);
			COMMENT_ADD(COMMENT_GPR_RD);

			return;

		case MTLO:
			FORMAT("mtlo %s", GPR[rs]);
			return;

		case MULT:
			FORMAT_MULT_DIV("mult");
			return;

		case MULTU:
			FORMAT_MULT_DIV("multu");
			return;

		case DIV:
			FORMAT_MULT_DIV("div");
			return;

		case DIVU:
			FORMAT_MULT_DIV("divu");
			return;

		case ADD:
			FORMAT_ARITH_REG("add");
			return;

		case ADDU:
			FORMAT_ARITH_REG("addu");
			return;

		case SUB:
			FORMAT_ARITH_REG("sub");
			return;

		case SUBU:
			FORMAT_ARITH_REG("subu");
			return;

		case AND:
			FORMAT_ARITH_REG("and");
			return;

		case OR:
			FORMAT_ARITH_REG("or");
			return;

		case XOR:
			FORMAT_ARITH_REG("xor");
			return;

		case NOR:
			FORMAT_ARITH_REG("nor");
			return;

		case SLT:
			FORMAT_ARITH_REG("slt");
			return;

		case SLTU:
			FORMAT_ARITH_REG("sltu");
			return;

		default:
			ILLEGAL;
			return;
		}

	case GROUP_BCOND: {
		const char *const opcode = (rt & 1) ? "bgez" : "bltz";
		const char *const link = ((rt & 0x1E) == 0x10) ? "al" : "";

		FORMAT("%s%s %s,%s0x%04hX", opcode, link, GPR[rs],
		       (offset < 0) ? "-" : "", offset);
		return;
	}

	case J:
		FORMAT("j 0x%08X", target);
		COMMENT_ADD(COMMENT_JUMP);

		return;

	case JAL:
		FORMAT("jal 0x%08X", target);
		COMMENT_ADD(COMMENT_JUMP);

		return;

	case BEQ:
		FORMAT_BRANCH_REG("beq");
		return;

	case BNE:
		FORMAT_BRANCH_REG("bne");
		return;

	case BLEZ:
		FORMAT_BRANCH("blez");
		return;

	case BGTZ:
		FORMAT_BRANCH("bgtz");
		return;

	case ADDI:
		FORMAT_ARITH_SEXT_IMM("addi");
		return;

	case ADDIU:
		FORMAT_ARITH_SEXT_IMM("addiu");
		return;

	case SLTI:
		FORMAT_ARITH_SEXT_IMM("slti");
		return;

	case SLTIU:
		FORMAT_ARITH_SEXT_IMM("sltiu");
		return;

	case ANDI:
		FORMAT_ARITH_ZEXT_IMM("andi");
		return;

	case ORI:
		FORMAT_ARITH_ZEXT_IMM("ori");
		return;

	case XORI:
		FORMAT_ARITH_ZEXT_IMM("xori");
		return;

	case LUI:
		FORMAT("lui %s,0x%04X", GPR[rt], ZEXT_IMM);
		COMMENT_ADD(COMMENT_GPR_RT);

		return;

	case GROUP_COP0:
		switch (rs) {
		case MF:
			FORMAT("mfc0 %s,%s", GPR[rt], CP0_CPR[rd]);
			return;

		case MT:
			FORMAT("mtc0 %s,%s", GPR[rt], CP0_CPR[rd]);
			COMMENT_ADD(COMMENT_CP0_CPR_RD);

			return;

		default:
			switch (funct) {
			case RFE:
				RES_SET("rfe");
				return;

			default:
				ILLEGAL;
				return;
			}
		}

	case GROUP_COP2:
		switch (rs) {
		case MF:
			FORMAT("mfc2 %s,%s", GPR[rt], CP2_CPR[rd]);
			return;

		case CF:
			FORMAT("cfc2 %s,%s", GPR[rt], CP2_CCR[rd]);
			return;

		case MT:
			FORMAT("mtc2 %s,%s", GPR[rt], CP2_CPR[rd]);
			return;

		case CT:
			FORMAT("ctc2 %s,%s", GPR[rd], CP2_CCR[rd]);
			return;

		default:
			switch (funct) {
			case RTPS:
				RES_SET("rtps");
				return;

			case NCLIP:
				RES_SET("nclip");
				return;

			case OP:
				RES_SET("op");
				return;

			case DPCS:
				RES_SET("dpcs");
				return;

			case INTPL:
				RES_SET("intpl");
				return;

			case MVMVA:
				RES_SET("mvmva");
				return;

			case NCDS:
				RES_SET("ncds");
				return;

			case CDP:
				RES_SET("cdp");
				return;

			case NCDT:
				RES_SET("ncdt");
				return;

			case NCCS:
				RES_SET("nccs");
				return;

			case CC:
				RES_SET("cc");
				return;

			case NCS:
				RES_SET("ncs");
				return;

			case NCT:
				RES_SET("nct");
				return;

			case SQR:
				RES_SET("sqr");
				return;

			case DCPL:
				RES_SET("dcpl");
				return;

			case DPCT:
				RES_SET("dpct");
				return;

			case AVSZ3:
				RES_SET("avsz3");
				return;

			case AVSZ4:
				RES_SET("avsz4");
				return;

			case RTPT:
				RES_SET("rtpt");
				return;

			case GPF:
				RES_SET("gpf");
				return;

			case GPL:
				RES_SET("gpl");
				return;

			case NCCT:
				RES_SET("ncct");
				return;

			default:
				ILLEGAL;
				return;
			}
		}

	case LB:
		FORMAT_LOAD("lb");
		return;

	case LH:
		FORMAT_LOAD("lh");
		return;

	case LWL:
		FORMAT_LOAD("lwl");
		return;

	case LW:
		FORMAT_LOAD("lw");
		return;

	case LBU:
		FORMAT_LOAD("lbu");
		return;

	case LHU:
		FORMAT_LOAD("lhu");
		return;

	case LWR:
		FORMAT_LOAD("lwr");
		return;

	case SB:
		FORMAT_STORE("sb");
		return;

	case SH:
		FORMAT_STORE("sh");
		return;

	case SWL:
		FORMAT_STORE("swl");
		return;

	case SW:
		FORMAT_STORE("sw");
		return;

	case SWR:
		FORMAT_STORE("swr");
		return;

	case LWC2:
		return;

	case SWC2:
		return;

	default:
		ILLEGAL;
		return;
	}

#undef rt
#undef FORMAT
#undef FORMAT_SHIFT_VAR
#undef FORMAT_SHIFT_REG
#undef FORMAT_MULT_DIV
#undef FORMAT_BRANCH
#undef FORMAT_LOAD_STORE
#undef FORMAT_ARITH_REG
#undef FORMAT_ARITH_ZEXT_IMM
#undef FORMAT_ARITH_SEXT_IMM
}

void psycho_dbg_disasm_trace(struct psycho_ctx *const ctx)
{
#define OUTPUT_DELIM        \
	(ctx->disasm.len += \
	 sprintf(&ctx->disasm.result[ctx->disasm.len], "%c ", COMMENT_DELIM))

	if (!ctx->disasm.num_comments) {
		return;
	}

	const int num_spaces = TRACE_NUM_SPACES - ctx->disasm.len;

	memset(&ctx->disasm.result[ctx->disasm.len], ' ', (ulong)num_spaces);
	ctx->disasm.len += num_spaces;

	ctx->disasm.result[ctx->disasm.len++] = COMMENT_START_CHAR;
	ctx->disasm.result[ctx->disasm.len++] = ' ';

	output_comment(ctx, ctx->disasm.comments[0]);
	ctx->disasm.num_comments--;

	if (ctx->disasm.num_comments) {
		for (uint i = 1; i <= ctx->disasm.num_comments; ++i) {
			OUTPUT_DELIM;
			output_comment(ctx, ctx->disasm.comments[i]);
		}
	}
}
