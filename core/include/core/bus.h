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

#include "types.h"

struct psycho_ctx;

enum {
	RAM_ADDR_START = 0x00000000,
	RAM_ADDR_END = 0x00200000,
	RAM_SIZE = RAM_ADDR_END - RAM_ADDR_START,

	BIOS_ADDR_START = 0x1FC00000,
	BIOS_ADDR_END = 0x1FC7FFFF,
	BIOS_SIZE = BIOS_ADDR_END - BIOS_ADDR_START
};

struct psycho_bus {
	u8 *bios;
	u8 *ram;
};

u32 psycho_bus_peek_word(struct psycho_ctx *ctx, u32 paddr);

#ifdef __cplusplus
}
#endif // __cplusplus
