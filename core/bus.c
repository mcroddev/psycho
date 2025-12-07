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

#include <string.h>

#include "bus.h"
#include "log.h"

LOG_MODULE(PSYCHO_LOG_MODULE_ID_BUS);

void psycho_bus_bios_data_set(struct psycho_ctx *const ctx, u8 *const data)
{
	ctx->bus.bios = data;
}

u32 psycho_bus_peek_word(struct psycho_ctx *const ctx, const u32 paddr)
{
	return psycho_bus_load_word(ctx, paddr);
}

u32 psycho_bus_load_word(struct psycho_ctx *const ctx, const u32 paddr)
{
	u32 word;

	switch (paddr) {
	case PSYCHO_RAM_ADDR_START ... PSYCHO_RAM_ADDR_END:
		memcpy(&word, &ctx->bus.ram[paddr], sizeof(u32));
		return word;

	case PSYCHO_BIOS_ADDR_START ... PSYCHO_BIOS_ADDR_END:
		memcpy(&word, &ctx->bus.bios[paddr & 0x000FFFFF], sizeof(u32));
		return word;

	default:
		LOG_WARN(ctx,
			 "Unknown word load: 0x%08X; returning 0xFFFF'FFFF",
			 paddr);
		return 0xFFFFFFFF;
	}
}

u8 psycho_bus_load_byte(struct psycho_ctx *const ctx, const u32 paddr)
{
	switch (paddr) {
	case PSYCHO_RAM_ADDR_START ... PSYCHO_RAM_ADDR_END:
		return ctx->bus.ram[paddr];

	case PSYCHO_BIOS_ADDR_START ... PSYCHO_BIOS_ADDR_END:
		return ctx->bus.bios[paddr & 0x000FFFFF];

	default:
		break;
	}
	LOG_WARN(ctx, "Unknown byte load: 0x%08X; returning 0xFF", paddr);
	return 0xFF;
}

void psycho_bus_store_word(struct psycho_ctx *const ctx, const u32 paddr,
			   const u32 word)
{
	switch (paddr) {
	case PSYCHO_RAM_ADDR_START ... PSYCHO_RAM_ADDR_END:
		memcpy(&ctx->bus.ram[paddr], &word, sizeof(u32));
		return;

	default:
		break;
	}

	LOG_WARN(ctx, "Unknown word store: 0x%08X <- 0x%08X; ignoring", paddr,
		 word);
}

void psycho_bus_store_halfword(struct psycho_ctx *const ctx, const u32 paddr,
			       const u16 halfword)
{
	LOG_WARN(ctx, "Unknown halfword store: 0x%08X <- 0x%04X; ignoring",
		 paddr, halfword);
}

void psycho_bus_store_byte(struct psycho_ctx *const ctx, const u32 paddr,
			   const u8 byte)
{
	switch (paddr) {
	case PSYCHO_RAM_ADDR_START ... PSYCHO_RAM_ADDR_END:
		ctx->bus.ram[paddr] = byte;
		return;

	default:
		break;
	}

	LOG_WARN(ctx, "Unknown byte store: 0x%08X <- 0x%02X; ignoring", paddr,
		 byte);
}
