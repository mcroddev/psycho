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

#include <stdio.h>
#include <string.h>

#include "core/ctx.h"
#include "bios-trace.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_BIOS);

static const struct psycho_bios_trace_func b0_funcs[] = {
	// clang-format off

	[0x3D] = {
		.prototype	= "void std_out_putchar(char c=%c)",
		.ret		= PSYCHO_BIOS_FUNC_RET_NONE
	}

	// clang-format on
};

static const char *escape_seq(const char c)
{
	switch (c) {
	case '\n':
		return "\\n";

	default:
		return NULL;
	}
}

static void handle_tty_output(struct psycho_ctx *const ctx)
{
	const char c = (char)ctx->cpu.gpr[CPU_GPR_A0];

	ctx->bios_trace.tty_stdout.data[ctx->bios_trace.tty_stdout.len++] = c;

	if (c == '\n') {
		ctx->event_cb(ctx, PSYCHO_EVENT_TTY_MESSAGE,
			      ctx->bios_trace.tty_stdout.data);

		psycho_log_message_dispatch(ctx,
					    PSYCHO_LOG_MODULE_ID_TTY_STDOUT,
					    PSYCHO_LOG_LEVEL_INFO, "%s",
					    ctx->bios_trace.tty_stdout.data);

		memset(&ctx->bios_trace.tty_stdout, 0,
		       sizeof(ctx->bios_trace.tty_stdout));
	} else {
		if (ctx->bios_trace.tty_stdout.len >=
		    sizeof(ctx->bios_trace.tty_stdout.data)) {
			LOG_WARN(
				ctx,
				"TTY stdout buffer wrapping around, corruption is expected");
			ctx->bios_trace.tty_stdout.len = 0;
		}
	}
}

static size_t process_arg_char(struct psycho_ctx *const ctx, char *dst)
{
	const char c = (char)ctx->cpu.gpr[CPU_GPR_A0];
	const char *escaped_char = escape_seq(c);

	int num_chars;

	if (escaped_char)
		num_chars = sprintf(dst, "'%s'", escaped_char);
	else
		num_chars = sprintf(dst, "'%c'", c);

	return num_chars;
}

static void process_prototype(struct psycho_ctx *const ctx)
{
#define SPECIFIER_LEN (2)

	const char *src = ctx->bios_trace.curr_func->prototype;
	char *dst = ctx->bios_trace.result;

	while (*src) {
		if (*src != '%') {
			*dst++ = *src++;
			continue;
		}

		const char next = *(src + 1);

		if (next == 'c') {
			dst += process_arg_char(ctx, dst);
			src += SPECIFIER_LEN;
		} else
			*dst++ = *src++;
	}
	*dst = '\0';

#undef SPECIFIER_LEN
}

void psycho_bios_trace_begin(struct psycho_ctx *const ctx)
{
	const u32 func = ctx->cpu.gpr[CPU_GPR_T1];

	if ((ctx->cpu.pc == 0x000000B0) && func == 0x3D) {
		ctx->bios_trace.curr_func = &b0_funcs[func];

		if (ctx->bios_trace.enable_tty_output)
			handle_tty_output(ctx);
	} else
		return;

	process_prototype(ctx);
}

void psycho_bios_trace_end(struct psycho_ctx *const ctx)
{
	if ((ctx->bios_trace.curr_func) &&
	    !ctx->bios_trace.waiting_for_return) {
		LOG_INFO(ctx, "%s", ctx->bios_trace.result);
		ctx->bios_trace.curr_func = NULL;

		return;
	}
}
