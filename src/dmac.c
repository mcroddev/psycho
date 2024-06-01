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

#include "dmac.h"
#include "dbg_log.h"
#include <stdlib.h>

void dmac_dpcr_set(struct psycho_dmac *const dmac, u32 dpcr)
{
	dmac->dpcr = dpcr;

	uint prio_seen = 0;

	for (uint ch = 0; ch < PSYCHO_DMAC_NUM_CHANNELS; ++ch, dpcr >>= 4) {
		const uint ch_config = dpcr & 0x0F;
		const uint ch_enabled = ch_config >> 3;

		if (!ch_enabled) {
			LOG_DBG(dmac->log, "DMAC: DMA%d channel disabled", ch);
			continue;
		}

		LOG_DBG(dmac->log, "DMAC: DMA%d channel enabled", ch);

		const uint prio = ch_config & 0x7;
		const uint prio_mask = 1 << prio;

		if (prio_seen & prio_mask) {
			// This channel has the same priority as another
			// channel. Uh oh...
			abort();
		} else {
			LOG_DBG(dmac->log,
				"DMAC: DMA%d channel priority set to %d, no "
				"conflict",
				ch, prio);
		}
		prio_seen |= prio_mask;
	}
}
