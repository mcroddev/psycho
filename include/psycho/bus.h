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

#include "dbg_log.h"

// clang-format off

#define PSYCHO_BUS_RAM_BEG	(0x00000000)
#define PSYCHO_BUS_RAM_END	(0x00200000)
#define PSYCHO_BUS_RAM_SIZE	((PSYCHO_BUS_RAM_END - PSYCHO_BUS_RAM_BEG) - 1)

#define PSYCHO_BUS_SPAD_BEG	(0x1F800000)
#define PSYCHO_BUS_SPAD_END	(0x1F8003FF)
#define PSYCHO_BUS_SPAD_SIZE	((PSYCHO_BUS_SPAD_END - PSYCHO_BUS_SPAD_BEG) - 1)

#define PSYCHO_BUS_BIOS_BEG	(0x1FC00000)
#define PSYCHO_BUS_BIOS_END	(0x1FC7FFFF)
#define PSYCHO_BUS_BIOS_SIZE	((PSYCHO_BUS_BIOS_END - PSYCHO_BUS_BIOS_BEG) - 1)

// clang-format on

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct psycho_bus {
	u8 bios[PSYCHO_BUS_BIOS_SIZE];
	u8 spad[PSYCHO_BUS_SPAD_SIZE];
	struct psycho_dbg_log *log;
	u8 *ram;
};
#pragma GCC diagnostic pop
