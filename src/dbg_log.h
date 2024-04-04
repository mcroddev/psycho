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

#pragma once

#include "psycho/dbg_log.h"

void dbg_log_msg(const struct psycho_dbg_log *log, uint level, const char *msg,
		 ...);

#define LOG_HANDLE(lvl, args...)                              \
	({                                                    \
		if ((ctx->log.level >= lvl) && ctx->log.cb) { \
			dbg_log_msg(&ctx->log, lvl, args);    \
		}                                             \
	})

// clang-format off

#define LOG_INFO(args...)	(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_INFO, args))
#define LOG_WARN(args...)	//(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_WARN, args))
#define LOG_ERR(args...)	(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_ERR, args))
#define LOG_FATAL(args...)	(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_FATAL, args))
#define LOG_DBG(args...)	(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_DBG, args))
#define LOG_TRACE(args...)	(LOG_HANDLE(PSYCHO_DBG_LOG_LEVEL_TRACE, args))

// clang-format on
