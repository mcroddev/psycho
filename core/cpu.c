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

#include "cpu.h"
#include "cpu-defs.h"
#include "bus.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_CPU);

static u32 translate_vaddr_to_paddr(const u32 vaddr)
{
	return vaddr & 0x1FFFFFFF;
}

void psycho_cpu_reset(struct psycho_ctx *const ctx)
{
	ctx->cpu.reg.pc = RESET_PC;
}

bool psycho_cpu_step(struct psycho_ctx *ctx)
{
	const u32 paddr = translate_vaddr_to_paddr(ctx->cpu.reg.pc);
	const u32 instr = psycho_bus_load_word(ctx, paddr);

	switch (instr) {
	default:
		LOG_ERROR(ctx, "Unhandled instruction: 0x%08X", instr);
		return false;
	}
	return true;
}
