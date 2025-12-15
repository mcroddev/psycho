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

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "core/ctx.h"
#include "log.h"

static const char *const log_level_name[PSYCHO_LOG_LEVEL_NUM] = {
	// clang-format off

	[PSYCHO_LOG_LEVEL_INFO]		= "info",
	[PSYCHO_LOG_LEVEL_WARN]		= "warn",
	[PSYCHO_LOG_LEVEL_ERROR]	= "error",
	[PSYCHO_LOG_LEVEL_DEBUG]	= "debug",
	[PSYCHO_LOG_LEVEL_TRACE]	= "trace"

	// clang-format on
};

static const char *const module_name[PSYCHO_LOG_MODULE_ID_NUM] = {
	// clang-format off

	[PSYCHO_LOG_MODULE_ID_CTX]	= "ctx",
	[PSYCHO_LOG_MODULE_ID_CPU]	= "cpu",
	[PSYCHO_LOG_MODULE_ID_DISASM]	= "disasm",
	[PSYCHO_LOG_MODULE_ID_BUS]	= "bus"

	// clang-format on
};

void psycho_log_level_set_global(struct psycho_ctx *const ctx,
				 const enum psycho_log_level level)
{
	for (size_t i = 0; i < PSYCHO_LOG_MODULE_ID_NUM; ++i)
		ctx->log.modules[i] = level;
}

void psycho_log_module_level_set(struct psycho_ctx *const ctx,
				 const enum psycho_log_module_id id,
				 const enum psycho_log_level level)
{
	ctx->log.modules[id] = level;
}

void psycho_log_message_dispatch(struct psycho_ctx *const ctx,
				 const enum psycho_log_module_id id,
				 const enum psycho_log_level level,
				 const char *const str, ...)
{
	struct psycho_log_msg msg;

	msg.msg_len = sprintf(msg.msg, "[%s/%s] ", log_level_name[level],
			      module_name[id]);

	va_list args;
	va_start(args, str);

	const int num_chars =
		vsnprintf(&msg.msg[msg.msg_len], sizeof(msg.msg), str, args);

	va_end(args);

	assert(num_chars < PSYCHO_LOG_MSG_SIZE_MAX);

	msg.msg_len += num_chars;

	msg.id = id;
	msg.level = level;

	ctx->event_cb(ctx, PSYCHO_EVENT_LOG_MESSAGE, &msg);
}
