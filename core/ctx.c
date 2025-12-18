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

#include "bios-trace.h"
#include "cpu-defs.h"
#include "cpu.h"
#include "disasm.h"
#include "log.h"

enum {
	// clang-format off

	EXE_OFF_INITIAL_PC		= 0x010,
	EXE_OFF_INITIAL_GP		= 0x014,
	EXE_OFF_DEST_ADDR		= 0x018,
	EXE_OFF_FILE_SIZE		= 0x01C,
	EXE_OFF_INITIAL_SP_FP_BASE	= 0x030,
	EXE_OFF_INITIAL_SP_FP_OFF	= 0x034,
	EXE_OFF_CODE			= 0x800

	// clang-format on
};

LOG_MODULE(PSYCHO_LOG_MODULE_ID_CTX);

void psycho_init(struct psycho_ctx *const ctx,
		 const struct psycho_ctx_cfg *const cfg)
{
	ctx->bus.bios = cfg->bios_data;
	ctx->bus.ram = cfg->ram_data;
	ctx->event_cb = cfg->event_cb;

	psycho_reset(ctx);
}

void psycho_reset(struct psycho_ctx *const ctx)
{
	psycho_cpu_reset(ctx);
}

void psycho_step(struct psycho_ctx *const ctx)
{
	if (ctx->disasm.trace_instruction)
		psycho_disasm_trace_begin(ctx);

	psycho_bios_trace_begin(ctx);

	psycho_cpu_step(ctx);

	if (ctx->disasm.trace_instruction)
		psycho_disasm_trace_end(ctx);

	psycho_bios_trace_end(ctx);
}

void psycho_tty_stdout_enable(struct psycho_ctx *const ctx, const bool enable)
{
	ctx->bios_trace.enable_tty_output = enable;
}

enum psycho_return_code psycho_exe_load(struct psycho_ctx *const ctx,
					const u8 *const exe_data,
					const size_t exe_size)
{
#define extract_u32(addr)                                     \
	({                                                    \
		u32 res;                                      \
		memcpy(&res, &exe_data[(addr)], sizeof(u32)); \
		res;                                          \
	})

	if (exe_size < EXE_MIN_SIZE)
		return PSYCHO_EXE_SIZE_BAD;

	if (memcmp(&exe_data[0], "PS-X EXE", sizeof("PS-X EXE") - 1) != 0)
		return PSYCHO_EXE_ID_BAD;

	ctx->cpu.pc = extract_u32(EXE_OFF_INITIAL_PC);
	ctx->cpu.next_pc = ctx->cpu.pc + sizeof(u32);

	ctx->cpu.gpr[CPU_GPR_GP] = extract_u32(EXE_OFF_INITIAL_GP);

	u32 dst_addr = extract_u32(EXE_OFF_DEST_ADDR);
	dst_addr = vaddr_to_paddr(dst_addr);

	const u32 file_size = extract_u32(EXE_OFF_FILE_SIZE);
	memcpy(&ctx->bus.ram[dst_addr], &exe_data[EXE_OFF_CODE], file_size);

	ctx->cpu.gpr[CPU_GPR_FP] = extract_u32(EXE_OFF_INITIAL_SP_FP_BASE);

	if (ctx->cpu.gpr[CPU_GPR_FP] != 0)
		ctx->cpu.gpr[CPU_GPR_SP] = ctx->cpu.gpr[CPU_GPR_FP];

	const u32 sp_fp_off = extract_u32(EXE_OFF_INITIAL_SP_FP_OFF);

	ctx->cpu.gpr[CPU_GPR_SP] += sp_fp_off;
	ctx->cpu.gpr[CPU_GPR_FP] += sp_fp_off;

	LOG_INFO(ctx, "EXE loaded", exe_size);
	return PSYCHO_OK;

#undef extract_u32
}
