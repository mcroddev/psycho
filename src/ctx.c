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

#include <string.h>

#include "cpu.h"
#include "cpu_defs.h"
#include "dbg_log_bios.h"
#include "dbg_log.h"
#include "ps_x_exe.h"

#include "psycho/ctx.h"

#define PS_X_EXE_INJECT_ADDR (0x80030000)

static void ps_x_exe_inject(struct psycho_ctx *const ctx)
{
	u32 dest = ps_x_exe_dest_get(ctx->ps_x_exe);
	const u32 size = ps_x_exe_size_get(ctx->ps_x_exe);

	LOG_INFO(&ctx->log, "Injecting PS-X EXE at 0x%08X (len=%d bytes)", dest,
		 size);

	const uint eof = PS_X_EXE_OFFSET_DATA + size;

	for (uint off = PS_X_EXE_OFFSET_DATA; off != eof;
	     off += sizeof(u32), dest += sizeof(u32)) {
		const u32 paddr = cpu_vaddr_to_paddr(dest);
		memcpy(&ctx->bus.ram[paddr], &ctx->ps_x_exe[off], sizeof(u32));
	}

	ctx->cpu.pc = ps_x_exe_pc_get(ctx->ps_x_exe);
	ctx->cpu.npc = ctx->cpu.pc + sizeof(u32);

	const u32 paddr = cpu_vaddr_to_paddr(ctx->cpu.pc);
	memcpy(&ctx->cpu.instr, &ctx->bus.ram[paddr], sizeof(u32));

	ctx->cpu.gpr[CPU_GPR_gp] = ps_x_exe_gp_get(ctx->ps_x_exe);

	const u32 sp_fp_base = ps_x_exe_sp_fp_base_get(ctx->ps_x_exe);
	const u32 sp_fp_offs = ps_x_exe_sp_fp_offs_get(ctx->ps_x_exe);

	if (sp_fp_base != 0) {
		ctx->cpu.gpr[CPU_GPR_sp] = sp_fp_base + sp_fp_offs;
	}
	ctx->cpu.gpr[CPU_GPR_fp] = sp_fp_base + sp_fp_offs;
	ctx->ps_x_exe = NULL;
}

void psycho_ctx_init(struct psycho_ctx *const ctx, u8 *const ram)
{
	ctx->bus.log = &ctx->log;

	ctx->cpu.bus = &ctx->bus;
	ctx->cpu.log = &ctx->log;

	ctx->bus.ram = ram;

	psycho_ctx_reset(ctx);
	LOG_INFO(&ctx->log, "System initialized!");
}

void psycho_ctx_reset(struct psycho_ctx *const ctx)
{
	cpu_reset(ctx);
	LOG_INFO(&ctx->log, "System reset!");
}

void psycho_ctx_step(struct psycho_ctx *const ctx)
{
	cpu_step(ctx);

	if ((ctx->ps_x_exe) && ctx->cpu.pc == PS_X_EXE_INJECT_ADDR) {
		ps_x_exe_inject(ctx);
	}

	if (ctx->log_bios.enabled) {
		dbg_log_bios_chk(ctx);
	}
}

NODISCARD bool psycho_ctx_ps_x_exe_run(struct psycho_ctx *const ctx,
				       const u8 *const data, const size_t len)
{
	if (!ps_x_exe_valid(data, len)) {
		return false;
	}

	psycho_ctx_reset(ctx);
	ctx->ps_x_exe = data;

	LOG_INFO(&ctx->log, "PS-X EXE will be injected!");
	return true;
}
