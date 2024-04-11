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

/// @file dbg_log_bios.c Defines the implementation of the BIOS call logger.
///
/// The BIOS call logger follows the execution paths of BIOS calls and outputs
/// the function prototype, arguments, and return value of the BIOS call. This
/// can be especially useful for debugging as it allows for deeper inspection of
/// guest software.
///
/// The result of each BIOS call is logged at the "debug" log level.
///
/// Note that since this file has a "dbg_" prefixed to it, this means the
/// functionality provided here may be compiled out entirely.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "psycho/ctx.h"
#include "cpu_defs.h"
#include "dbg_log_bios.h"
#include "dbg_log.h"

#define a0 (CPU_GPR_a0)
#define a1 (CPU_GPR_a1)
#define a2 (CPU_GPR_a2)
#define a3 (CPU_GPR_a3)

#define JR_RA (0x03E00008)

#define RETVAL_TYPE_NONE (0)
#define RETVAL_TYPE_PTR (1)
#define RETVAL_TYPE_INT (2)

// clang-format off
#define PROTOTYPE(str)			\
	.prototype = (str), 		\
	.prototype_len = sizeof((str)) - 1
// clang-format on

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct bios_fn {
	/// @brief The full prototype of the BIOS call.
	///
	/// @note If the function call is a standard libc function, it should
	/// match as close as possible to the prototype in the ANSI C standard
	/// (X3.159-1989 or ISO/IEC 9899:1990).
	const char *const prototype;

	/// @brief The length of the prototype string not counting the NULL
	/// terminator.
	const size_t prototype_len;

	/// @brief Return type of the function, if any.
	const uint retval_type;

	/// @brief Should we run this BIOS call through the formatter?
	const bool format_args;
};
#pragma GCC diagnostic pop

// clang-format off
static const struct bios_fn a0_table[] = {
	[0x17] = {
		PROTOTYPE("int strcmp(const char *s1=%ps, const char *s2=%ps)"),
		.retval_type	= RETVAL_TYPE_INT,
		.format_args	= true
	},

	[0x25] = {
		PROTOTYPE("int toupper(int c=%d)"),
		.retval_type	= RETVAL_TYPE_INT,
		.format_args	= true
	},

	[0x2A] = {
		PROTOTYPE("void *memcpy(void *s1=%p, const void *s2=%p, size_t n=%d)"),
		.retval_type	= RETVAL_TYPE_PTR,
		.format_args	= true
	},

	[0x3F] = {
		PROTOTYPE("void printf(const char *format=%ps, ...)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x44] = {
		PROTOTYPE("void FlushCache(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	},

	[0x72] = {
		PROTOTYPE("void CdRemove(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	},

	[0x96] = {
		PROTOTYPE("void AddCDROMDevice(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	},

	[0x97] = {
		PROTOTYPE("void AddMemCardDevice(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false,
	},

	[0x99] = {
		PROTOTYPE("void AddDummyTtyDevice(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	},

	[0xA3] = {
		PROTOTYPE("void DequeueCdIntr(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	}
};

static const struct bios_fn b0_table[] = {
	[0x00] = {
		PROTOTYPE("void alloc_kernel_memory(size_t size=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x09] = {
		PROTOTYPE("int CloseEvent(struct ev *ev=%p)"),
		.retval_type	= RETVAL_TYPE_INT,
		.format_args	= true
	},

	[0x18] = {
		PROTOTYPE("void *SetDefaultExitFromException(void)"),
		.retval_type	= RETVAL_TYPE_PTR,
		.format_args	= false
	},

	[0x19] = {
		PROTOTYPE("void SetCustomExitFromException(void *buf=%p)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x3D] = {
		PROTOTYPE("void std_out_putchar(char c=%c)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x47] = {
		PROTOTYPE("void AddDevice(struct device_info *dev=%p)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x5B] = {
		PROTOTYPE("void ChangeClearPad(int n=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	}
};

static const struct bios_fn c0_table[] = {
	[0x00] = {
		PROTOTYPE("void EnqueueTimerAndVblankIrqs(int prio=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x01] = {
		PROTOTYPE("void EnqueueSyscallHandler(int prio=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x03] = {
		PROTOTYPE("void *SysDeqIntRP(int prio=%d, int struc=%d"),
		.retval_type	= RETVAL_TYPE_PTR,
		.format_args	= true
	},

	[0x07] = {
		PROTOTYPE("void InstallExceptionHandlers(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	},

	[0x08] = {
		PROTOTYPE("void SysInitMemory(u32 *addr=%p, size_t size=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x0A] = {
		PROTOTYPE("int ChangeClearRCnt(int t=%d, int flag=%d)"),
		.retval_type	= RETVAL_TYPE_INT,
		.format_args	= true
	},

	[0x0C] = {
		PROTOTYPE("void InitDefInt(int prio=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x12] = {
		PROTOTYPE("void InstallDevices(int ttyflag=%d)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= true
	},

	[0x1C] = {
		PROTOTYPE("void AdjustA0Table(void)"),
		.retval_type	= RETVAL_TYPE_NONE,
		.format_args	= false
	}
};
// clang-format on

static const char *esc_seq_conv(const char c)
{
	switch (c) {
	case '\n':
		return "\\n";

	case '\t':
		return "\\t";

	default:
		return NULL;
	}
}

static ALWAYS_INLINE RETURNS_NONNULL u8 *
addr_resolve(struct psycho_ctx *const ctx, u32 addr)
{
	addr = cpu_vaddr_to_paddr(addr);

	switch (addr) {
	case PSYCHO_BUS_RAM_BEG ... PSYCHO_BUS_RAM_END:
		return &ctx->bus.ram[addr];

	case PSYCHO_BUS_BIOS_BEG ... PSYCHO_BUS_BIOS_END:
		return &ctx->bus.bios[addr & 0x000FFFFF];

	default:
		UNREACHABLE;
	}
}

static void fmt_process(struct psycho_ctx *const ctx,
			const struct bios_fn *const fn, uint *pos_src,
			int *const pos_dst, const uint arg)
{
#define FORMAT(args...) \
	(*pos_dst += sprintf(&ctx->log_bios.str[*pos_dst], args))

	switch (fn->prototype[*pos_src]) {
	case 'c': {
		const char c = (char)ctx->cpu.gpr[arg];
		const char *const esc_seq = esc_seq_conv(c);

		if (esc_seq) {
			FORMAT("'%s'", esc_seq);
		} else {
			FORMAT("'%c'", c);
		}
		(*pos_src)++;
		break;
	}

	case 'd':
		FORMAT("%d", (int)ctx->cpu.gpr[arg]);
		(*pos_src)++;
		break;

	case 'p':
		if (ctx->log_bios.ptr_deref) {
			(*pos_src)++;

			switch (fn->prototype[*pos_src]) {
			case 's': {
				(*pos_src)++;

				const u8 *addr =
					addr_resolve(ctx, ctx->cpu.gpr[arg]);

				FORMAT("\"%s\"", addr);
				break;
			}

			default:
				FORMAT("0x%08X", ctx->cpu.gpr[arg]);
				break;
			}
		} else {
			(*pos_src)++;

			switch (fn->prototype[*pos_src]) {
			case 's':
				(*pos_src)++;
				break;

			default:
				break;
			}
			FORMAT("0x%08X", ctx->cpu.gpr[arg]);
			break;
		}
		break;

	default:
		UNREACHABLE;
	}
#undef FORMAT
}

static void stracef(struct psycho_ctx *const ctx,
		    const struct bios_fn *const fn)
{
	uint pos_src = 0;
	int pos_dst = 0;

	uint arg = CPU_GPR_a0;

	memset(ctx->log_bios.str, 0, sizeof(ctx->log_bios.str));

	while (pos_src < fn->prototype_len) {
		if (fn->prototype[pos_src] == '%') {
			pos_src++;
			fmt_process(ctx, fn, &pos_src, &pos_dst, arg);

			arg++;
		} else {
			ctx->log_bios.str[pos_dst++] = fn->prototype[pos_src++];
		}
	}
}

static ALWAYS_INLINE void state_reset(struct psycho_ctx *const ctx)
{
	ctx->log_bios.waiting = false;
	memset(&ctx->log_bios.str, 0, sizeof(ctx->log_bios.str));
}

static ALWAYS_INLINE void tty_intercept_handle(struct psycho_ctx *const ctx)
{
	const char c = (char)ctx->cpu.gpr[CPU_GPR_a0];

	if (c == '\n') {
		LOG_DBG(&ctx->log, "TTY: %s", ctx->log_bios.tty_buf);

		memset(ctx->log_bios.tty_buf, 0, sizeof(ctx->log_bios.tty_buf));
		ctx->log_bios.tty_buf_len = 0;
	} else {
		ctx->log_bios.tty_buf[ctx->log_bios.tty_buf_len++] = c;
	}
}

void dbg_log_bios_chk(struct psycho_ctx *const ctx)
{
	if ((ctx->log_bios.waiting) && ctx->cpu.instr == JR_RA) {
		LOG_DBG(&ctx->log, "BIOS call: %s -> 0x%08X", ctx->log_bios.str,
			ctx->cpu.gpr[CPU_GPR_v0]);

		state_reset(ctx);
		return;
	}

	const struct bios_fn *fn;

	if (ctx->cpu.pc == 0xA0) {
		fn = &a0_table[ctx->cpu.gpr[CPU_GPR_t1]];
	} else if (ctx->cpu.pc == 0xB0) {
		fn = &b0_table[ctx->cpu.gpr[CPU_GPR_t1]];
	} else if (ctx->cpu.pc == 0xC0) {
		fn = &c0_table[ctx->cpu.gpr[CPU_GPR_t1]];
	} else {
		return;
	}

	if (!fn->prototype) {
		printf("PC=0x%08X   fn=0x%02X\n", ctx->cpu.pc,
		       ctx->cpu.gpr[CPU_GPR_t1]);
		fflush(stdout);

		abort();
	}

	if (!fn->format_args) {
		if (fn->retval_type == RETVAL_TYPE_NONE) {
			LOG_DBG(&ctx->log, "BIOS call: %s", fn->prototype);
		} else {
			memcpy(ctx->log_bios.str, fn->prototype,
			       fn->prototype_len);
			ctx->log_bios.waiting = true;
		}
	} else {
		if ((ctx->log_bios.tty_intercept) &&
		    ((fn == &a0_table[0x3C]) || (fn == &b0_table[0x3D]))) {
			tty_intercept_handle(ctx);
			return;
		}

		stracef(ctx, fn);

		if (fn->retval_type == RETVAL_TYPE_NONE) {
			LOG_DBG(&ctx->log, "BIOS call: %s", ctx->log_bios.str);
		} else {
			ctx->log_bios.waiting = true;
		}
	}
}
