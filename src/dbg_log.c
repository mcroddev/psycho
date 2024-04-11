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

#define LOG_MSG_MAX (512)

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "dbg_log.h"

FORMAT_CHK(3, 4)
void dbg_log_msg(struct psycho_dbg_log *const log, const uint lvl,
		 const char *const msg, ...)
{
	// clang-format off
	static const struct {
		const char *const str;
		size_t len;
	} lvl_data[] = {
		[PSYCHO_DBG_LOG_LEVEL_INFO]  = {"[info] ", sizeof("[info] ")},
		[PSYCHO_DBG_LOG_LEVEL_WARN]  = {"[warn] ", sizeof("[warn] ")},
		[PSYCHO_DBG_LOG_LEVEL_ERR]   = {"[error] ", sizeof("[error] ")},
		[PSYCHO_DBG_LOG_LEVEL_DBG]   = {"[debug] ", sizeof("[debug] ")},
		[PSYCHO_DBG_LOG_LEVEL_TRACE] = {"[trace] ", sizeof("[trace] ")}
	};
	// clang-format on

	char str[LOG_MSG_MAX];
	memcpy(str, lvl_data[lvl].str, lvl_data[lvl].len);

	va_list args;
	va_start(args, msg);
	vsprintf(&str[lvl_data[lvl].len - 1], msg, args);
	va_end(args);

	log->cb(log->udata, lvl, str);
}
