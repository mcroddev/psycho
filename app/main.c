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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "core/ctx.h"
#include "core/types.h"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

#define COLOR_RESET "\e[0m"

struct {
	u8 ram[PSYCHO_RAM_SIZE];
	u8 bios[PSYCHO_BIOS_SIZE];
	struct psycho_ctx ctx;
} static emu;

static void handle_cpu_illegal_instr(struct psycho_ctx *const ctx)
{
	__builtin_trap();
}

static void handle_log_message(const struct psycho_log_msg *const msg)
{
	switch (msg->level) {
	case PSYCHO_LOG_LEVEL_INFO:
		printf(BWHT "%s\n" COLOR_RESET, msg->msg);
		return;

	case PSYCHO_LOG_LEVEL_WARN:
		printf(BYEL "%s\n" COLOR_RESET, msg->msg);
		return;

	case PSYCHO_LOG_LEVEL_ERROR:
		printf(BRED "%s\n" COLOR_RESET, msg->msg);
		return;

	default:
		printf("%s\n", msg->msg);
		return;
	}
}

static void ctx_event_handle(struct psycho_ctx *const ctx,
			     const enum psycho_ctx_event event,
			     void *const data)
{
	switch (event) {
	case PSYCHO_CTX_EVENT_CPU_ILLEGAL:
		handle_cpu_illegal_instr(ctx);
		return;

	case PSYCHO_CTX_EVENT_LOG_MESSAGE:
		handle_log_message(data);
		return;

	default:
		UNREACHABLE;
	}
}

static bool load_bios_file(const char *const bios_file)
{
	struct stat st;
	if (stat(bios_file, &st) < 0)
		return false;

	FILE *bios_file_handle = fopen(bios_file, "rb");

	if (!bios_file_handle)
		return false;

	const size_t bytes_read =
		fread(emu.bios, sizeof(u8), sizeof(emu.bios), bios_file_handle);

	if ((bytes_read != sizeof(emu.bios)) || (ferror(bios_file_handle))) {
		fclose(bios_file_handle);
		return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s: missing required argument.\n", argv[0]);
		fprintf(stderr, "syntax: %s <bios_file>\n", argv[0]);

		return EXIT_FAILURE;
	}

	if (!load_bios_file(argv[1])) {
		fprintf(stderr,
			"%s: error encountered loading bios file %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	static const struct psycho_ctx_cfg cfg = {
		// clang-format off

		.event_cb	= ctx_event_handle,
		.ram_data	= emu.ram,
		.bios_data	= emu.bios,

		// clang-format on
	};

	psycho_ctx_init(&emu.ctx, &cfg);
	psycho_log_level_set_global(&emu.ctx, PSYCHO_LOG_LEVEL_TRACE);

	psycho_disasm_trace_instruction_enable(&emu.ctx, true);

	for (;;) {
		if (emu.ctx.cpu.pc == 0x80030000)
			__builtin_trap();

		psycho_ctx_step(&emu.ctx);
	}
	return EXIT_FAILURE;
}
