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

#include "compiler.h"
#include "gpu.h"
#include "dbg_log.h"

// clang-format off

#define GP01_CMD	(24)
#define GP01_PARAM	(0x00FFFFFF)

#define GP1_CMD_RESET		(0x00)
#define GP1_CMD_DMA_DIR		(0x04)

#define GPUSTAT_DISP_EN	(1U << 23)
#define GPUSTAT_IRQ	(1U << 24)

#define GPUSTAT_DMA_DIR_MASK	((1U << 29) | (1U << 30))

#define GPUSTAT_RESET_VAL	(0x14802000)

// clang-format on

static void fifo_clear(struct psycho_gpu *const gpu)
{
	(void)gpu;
}

void gpu_gp1(struct psycho_gpu *const gpu, const u32 packet)
{
	switch (packet >> GP01_CMD) {
	case GP1_CMD_RESET:
		fifo_clear(gpu);
		gpu->gpustat = GPUSTAT_RESET_VAL;

		break;

	case GP1_CMD_DMA_DIR:
		gpu->gpustat = (gpu->gpustat & ~GPUSTAT_DMA_DIR_MASK) |
			       ((packet & GP01_PARAM) & GPUSTAT_DMA_DIR_MASK);

		LOG_TRACE(gpu->log, "DMA direction changed");
		break;

	default:
		LOG_WARN(gpu->log,
			 "Unknown GPU GP1 packet (cmd=0x%02X, param=0x%05X)",
			 packet >> GP01_CMD, packet & GP01_PARAM);
		break;
	}
}
