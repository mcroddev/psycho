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

enum {
	BIOS_SIZE = 524288,
};

static struct psycho_ctx ctx;
static u8 bios_data[BIOS_SIZE];

static void on_log_msg(struct psycho_ctx *const ctx,
		       const struct psycho_log_msg *const msg)
{
	printf("%s\n", msg->msg);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s: missing required argument.\n", argv[0]);
		fprintf(stderr, "syntax: %s <bios_file>\n", argv[0]);

		return EXIT_FAILURE;
	}

	struct stat st;
	if (stat(argv[1], &st) < 0) {
		fprintf(stderr, "%s: failed to get file size of %s: %s",
			argv[0], argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	FILE *bios_file = fopen(argv[1], "rb");

	if (!bios_file) {
		fprintf(stderr, "%s: failed to open %s: %s\n", argv[0], argv[1],
			strerror(errno));
		return EXIT_FAILURE;
	}

	const size_t bytes_read =
		fread(bios_data, sizeof(u8), BIOS_SIZE, bios_file);

	if ((bytes_read != BIOS_SIZE) || (ferror(bios_file))) {
		fprintf(stderr, "%s: error reading %s: %s\n", argv[0], argv[1],
			strerror(errno));
		fclose(bios_file);
		return EXIT_FAILURE;
	}

	psycho_ctx_init(&ctx);
	psycho_ctx_bios_data_set(&ctx, bios_data);

	psycho_log_level_set_global(&ctx, PSYCHO_LOG_LEVEL_TRACE);
	ctx.log.msg_cb_func = on_log_msg;

	for (;;)
		if (!psycho_ctx_step(&ctx))
			abort();

	return EXIT_FAILURE;
}
