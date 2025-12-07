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

/**
 * @file log.h Defines the public interface to the logging module.
 *
 * Each module of psycho (e.g., CD-ROM, GPU, SPU) is allowed to emit messages to
 * the host application each with a controllable log granularity.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <stddef.h>

struct psycho_ctx;

enum {
	PSYCHO_LOG_MSG_SIZE_MAX = 512,
};

/**
 * @brief Defines the various log levels of the log module.
 *
 * The log levels are inclusive; for example, if you set a module to report
 * errors
 */
enum psycho_log_level {
	PSYCHO_LOG_LEVEL_OFF,
	PSYCHO_LOG_LEVEL_INFO,
	PSYCHO_LOG_LEVEL_WARN,
	PSYCHO_LOG_LEVEL_ERROR,
	PSYCHO_LOG_LEVEL_DEBUG,
	PSYCHO_LOG_LEVEL_TRACE,
	PSYCHO_LOG_LEVEL_NUM
};

enum psycho_log_module_id {
	PSYCHO_LOG_MODULE_ID_CTX,
	PSYCHO_LOG_MODULE_ID_CPU,
	PSYCHO_LOG_MODULE_ID_DISASM,
	PSYCHO_LOG_MODULE_ID_BUS,
	PSYCHO_LOG_MODULE_ID_NUM
};

struct psycho_log_module {
	enum psycho_log_module_id id;
	enum psycho_log_level level;
};

struct psycho_log_msg {
	enum psycho_log_module_id id;
	enum psycho_log_level level;
	char msg[PSYCHO_LOG_MSG_SIZE_MAX];
	size_t msg_len;
};

typedef void (*on_log_msg_cb)(struct psycho_ctx *,
			      const struct psycho_log_msg *);

struct psycho_log {
	bool enabled;
	struct psycho_log_module modules[PSYCHO_LOG_MODULE_ID_NUM];
};

/**
 * @brief Enables or disables logging.
 *
 * @param ctx The target psycho_ctx emulator context.
 * @param enable `true` to enable logging, `false` to disable it.
 */
void psycho_log_enable(struct psycho_ctx *ctx, bool enable);

/**
 * @brief Applies the desired log level to all modules.
 *
 * @param ctx The target psycho_ctx emulator context.
 * @param level The desired log level.
 */
void psycho_log_level_set_global(struct psycho_ctx *ctx,
				 enum psycho_log_level level);

/**
 * @brief Applies the desired log level to a specific module.
 *
 * @param ctx The target psycho_ctx emulator context.
 * @param module The module to set the log level on.
 * @param level The desired log level.
 */
void psycho_log_module_level_set(struct psycho_ctx *ctx,
				 enum psycho_log_module_id id,
				 enum psycho_log_level level);

#ifdef __cplusplus
}
#endif // __cplusplus
