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

/// @file dbg_bios_call_log.h Defines the public interface for the BIOS call
/// tracer.

#pragma once

#include <stdbool.h>

#define PSYCHO_DBG_LOG_BIOS_TTY_BUF_SIZE (256)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct psycho_dbg_log_bios {
	/// @brief Holds the current BIOS call string. This is solely meant for
	/// internal processing and should not be modified by public parties.
	/// The result of the BIOS call trace is output as a debug log message.
	char str[1024];

	/// @brief Holds the current TTY string. This is only relevant if
	/// `tty_intercept` is `true`. This is solely meant for internal
	/// processing and should not be modified by public parties.
	char tty_buf[PSYCHO_DBG_LOG_BIOS_TTY_BUF_SIZE];

	bool enabled;

	/// @brief Should we explicitly intercept TTY output?
	///
	/// Explicit TTY output interception is a special case of the BIOS call
	/// tracer. If enabled, individual "putchar" BIOS calls will not be
	/// logged, but each character will be stored into a buffer. Once a
	/// newline character is seen, the buffer contents are emitted as a full
	/// string.
	///
	/// You probably want to enable this; it is most of the time a verbose
	/// nuisance otherwise.
	bool tty_intercept;

	/// @brief Are we waiting for the current BIOS call to complete?
	///
	/// If a BIOS function returns non-void, we need to wait until it
	/// completes so we can grab the return value.
	bool waiting;

	/// @brief Will pointers be dereferenced?
	bool ptr_deref;

	size_t tty_buf_len;
};
#pragma GCC diagnostic pop
