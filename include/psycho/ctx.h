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

/// @file ctx.h Provides the public interface for the psycho context.

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "bus.h"
#include "cpu.h"
#include "dbg_log_bios.h"
#include "dbg_disasm.h"
#include "dbg_log.h"

/// @brief Defines the emulator context.
struct psycho_ctx {
	struct psycho_bus bus;
	struct psycho_dbg_log_bios log_bios;
	struct psycho_dbg_disasm disasm;
	struct psycho_cpu cpu;
	struct psycho_dbg_log log;

	/// @brief The PS-X EXE which will be injected.
	const u8 *ps_x_exe;
};

void psycho_ctx_init(struct psycho_ctx *ctx, u8 *ram);

void psycho_ctx_reset(struct psycho_ctx *ctx);
void psycho_ctx_step(struct psycho_ctx *ctx);

bool psycho_ctx_ps_x_exe_run(struct psycho_ctx *ctx, const u8 *data,
			     size_t len);
