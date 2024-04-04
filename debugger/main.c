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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "psycho/ctx.h"
#include "psycho/ps_x_exe.h"

#define RED "\e[1;91m"
#define YEL "\e[1;33m"
#define MAG "\e[1;35m"
#define WHT "\e[1;37m"
#define RESET "\x1B[0m"

static u8 exe[PSYCHO_PS_X_SIZE_MAX];

static void gpr_regs_output(const struct psycho_ctx *const ctx)
{
	for (uint reg = 0; reg < PSYCHO_CPU_GPR_REGS_NUM; ++reg) {
		printf("[%s] = 0x%08X\n", psycho_cpu_gpr_names[reg],
		       ctx->cpu.gpr[reg]);
	}
}

static void error_log_output(const struct psycho_ctx *const ctx,
			     const char *const str)
{
	printf(RED "%s\n", str);
	printf(RED "Last instruction: 0x%08X\t 0x%08X\t %s\n", ctx->cpu.pc,
	       ctx->cpu.instr, ctx->disasm.result);

	printf("=============== CPU registers ===============\n");
	gpr_regs_output(ctx);
	printf(RED "Emulation halted.\n" RESET);
}

static void ctx_log_msg(void *udata, const uint level, char *const str)
{
	struct psycho_ctx *ctx = (struct psycho_ctx *)udata;

	switch (level) {
	case PSYCHO_DBG_LOG_LEVEL_INFO:
		printf(WHT "%s\n" RESET, str);
		return;

	case PSYCHO_DBG_LOG_LEVEL_WARN:
		printf(YEL "%s\n" RESET, str);
		return;

	case PSYCHO_DBG_LOG_LEVEL_ERR:
		error_log_output(ctx, str);
		fflush(stdout);
		abort();

	case PSYCHO_DBG_LOG_LEVEL_DBG:
	case PSYCHO_DBG_LOG_LEVEL_TRACE:
		printf(MAG "%s\n" RESET, str);
		return;

	default:
		__builtin_unreachable();
	}
}

static void ctx_config(struct psycho_ctx *const ctx)
{
	ctx->log_bios.enabled = true;
	ctx->log_bios.ptr_deref = false;
	ctx->log_bios.tty_intercept = true;

	ctx->log.level = PSYCHO_DBG_LOG_LEVEL_DBG;
	ctx->log.udata = ctx;
	ctx->log.cb = &ctx_log_msg;

	ctx->cpu.exc_halt = (1 << PSYCHO_CPU_EXC_CODE_RI);
}

static void bios_file_open(struct psycho_ctx *const ctx, const char *const file)
{
	FILE *fd = fopen(file, "rb");

	if (!fd) {
		fprintf(stderr, "Error opening BIOS file %s: %s\n", file,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	const size_t bytes_read =
		fread(ctx->bus.bios, 1, PSYCHO_BUS_BIOS_SIZE, fd);

	if ((ferror(fd)) || bytes_read != PSYCHO_BUS_BIOS_SIZE) {
		fprintf(stderr, "Error reading BIOS file %s: %s\n", file,
			strerror(errno));

		fclose(fd);
		exit(EXIT_FAILURE);
	}
	fclose(fd);
}

static void exe_file_open(struct psycho_ctx *const ctx, const char *const file)
{
	FILE *fd = fopen(file, "rb");

	if (!fd) {
		fprintf(stderr, "Error opening EXE file %s: %s\n", file,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct stat st;
	stat(file, &st);
	const off_t exe_size = st.st_size;

	const size_t bytes_read = fread(exe, 1, (ulong)exe_size, fd);

	if ((ferror(fd)) || bytes_read != (size_t)exe_size) {
		fprintf(stderr, "Error reading EXE file %s: %s\n", file,
			strerror(errno));

		fclose(fd);
		exit(EXIT_FAILURE);
	}
	fclose(fd);

	if (!psycho_ctx_ps_x_exe_run(ctx, exe, (size_t)exe_size)) {
		fprintf(stderr, "The PS-X EXE specified is not valid.\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	static u8 ram[PSYCHO_BUS_RAM_SIZE];
	memset(ram, 0, sizeof(ram));

	memset(exe, 0, sizeof(exe));

	if (argc < 3) {
		fprintf(stderr, "%s: Missing required argument.\n", argv[0]);
		fprintf(stderr, "Syntax: %s [bios_file] [exe_file]\n", argv[0]);

		return EXIT_FAILURE;
	}

	struct psycho_ctx ctx = psycho_ctx_create(ram);

	ctx_config(&ctx);
	bios_file_open(&ctx, argv[1]);
	exe_file_open(&ctx, argv[2]);

	for (;;) {
		psycho_dbg_disasm_instr(&ctx, ctx.cpu.instr, ctx.cpu.pc);
		psycho_ctx_step(&ctx);
		//psycho_dbg_disasm_trace(&ctx);

		//printf("0x%08X\t 0x%08X\t %s\n", ctx.disasm.pc,
		//       ctx.disasm.instr, ctx.disasm.result);
	}
}
