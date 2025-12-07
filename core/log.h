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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "core/log.h"

#define LOG_MODULE(x) static const enum psycho_log_module_id m_log_module = (x)

#define LOG_HANDLE(ctx, lvl, args...)                                    \
	({                                                               \
		struct psycho_ctx *m_ctx = (ctx);                        \
                                                                         \
		if ((m_ctx->log.enabled) &&                              \
		    (m_ctx->log.modules[m_log_module].level >= (lvl)))   \
			psycho_log_message_dispatch(m_ctx, m_log_module, \
						    (lvl), args);        \
	})

#define LOG_INFO(ctx, args...) LOG_HANDLE((ctx), PSYCHO_LOG_LEVEL_INFO, args)
#define LOG_WARN(ctx, args...) LOG_HANDLE((ctx), PSYCHO_LOG_LEVEL_WARN, args)
#define LOG_ERROR(ctx, args...) LOG_HANDLE((ctx), PSYCHO_LOG_LEVEL_ERROR, args)
#define LOG_DEBUG(ctx, args...) LOG_HANDLE((ctx), PSYCHO_LOG_LEVEL_DEBUG, args)
#define LOG_TRACE(ctx, args...) LOG_HANDLE((ctx), PSYCHO_LOG_LEVEL_TRACE, args)

void psycho_log_message_dispatch(struct psycho_ctx *ctx,
				 enum psycho_log_module_id id,
				 enum psycho_log_level level, const char *str,
				 ...);

#ifdef __cplusplus
}
#endif // __cplusplus
