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

/** @file ctx.h Defines the interface to a psycho emulator context. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>

#include "bios-trace.h"
#include "bus.h"
#include "cpu.h"
#include "disasm.h"
#include "log.h"

enum psycho_event {
	/** @brief The CPU has executed an illegal instruction. */
	PSYCHO_EVENT_CPU_ILLEGAL,

	/** @brief A log message has been dispatched. */
	PSYCHO_EVENT_LOG_MESSAGE,

	/**
	 * @brief A TTY message has been dispatched.
	 *
	 * Note that this event will not be raised until a newline is seen.
	 */
	PSYCHO_EVENT_TTY_MESSAGE
};

typedef void (*psycho_event_cb)(struct psycho_ctx *, enum psycho_event, void *);

struct psycho_ctx_cfg {
	psycho_event_cb event_cb;
	u8 *ram_data;
	u8 *bios_data;
};

struct psycho_ctx {
	struct psycho_bus bus;
	struct psycho_cpu cpu;
	struct psycho_disasm disasm;
	struct psycho_log log;
	struct psycho_bios_trace bios_trace;

	psycho_event_cb event_cb;
};

enum psycho_return_code {
	PSYCHO_EXE_SIZE_BAD = -1,
	PSYCHO_EXE_ID_BAD = -2,
	PSYCHO_OK = 1
};

enum {
	EXE_MIN_SIZE = 0x800,
};

void psycho_init(struct psycho_ctx *ctx, const struct psycho_ctx_cfg *cfg);
void psycho_reset(struct psycho_ctx *ctx);
void psycho_step(struct psycho_ctx *ctx);

void psycho_tty_stdout_enable(struct psycho_ctx *ctx, bool enable);

enum psycho_return_code psycho_exe_load(struct psycho_ctx *ctx,
					const u8 *exe_data, size_t exe_size);

#ifdef __cplusplus
}
#endif // __cplusplus
