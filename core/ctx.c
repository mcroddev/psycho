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

#include "core/ctx.h"
#include "cpu.h"
#include "disasm.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_CTX);

void psycho_ctx_init(struct psycho_ctx *const ctx,
		     const struct psycho_ctx_cfg *const cfg)
{
	ctx->bus.bios = cfg->bios_data;
	ctx->bus.ram = cfg->ram_data;
	ctx->event_cb = cfg->event_cb;

	psycho_ctx_reset(ctx);
}

void psycho_ctx_reset(struct psycho_ctx *const ctx)
{
	psycho_cpu_reset(ctx);
}

void psycho_ctx_step(struct psycho_ctx *const ctx)
{
	if (ctx->disasm.trace_instruction)
		psycho_disasm_trace_begin(ctx);

	psycho_cpu_step(ctx);

	if (ctx->disasm.trace_instruction)
		psycho_disasm_trace_end(ctx);
}
