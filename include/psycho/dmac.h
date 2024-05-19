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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

// clang-format off

#define PSYCHO_DMAC_NUM_CHANNELS	(7)

// clang-format on

struct psycho_dmac_channel {
	/// @brief DMA base address (R/W)
	u32 madr;

	/// @brief DMA block control (R/W)
	u32 bcr;

	/// @brief DMA channel channel (R/W)
	u32 chcr;
};

struct psycho_dmac {
	struct psycho_dmac_channel channels[PSYCHO_DMAC_NUM_CHANNELS];

	/// @brief DMA control register (R/W)
	u32 dpcr;

	/// @brief DMA interrupt register (R/W)
	u32 dicr;
};

#ifdef __cplusplus
}
#endif // __cplusplus
