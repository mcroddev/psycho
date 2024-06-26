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

#include <string.h>
#include "bus.h"
#include "dbg_log.h"
#include "dmac.h"
#include "gpu.h"

// clang-format off

#define RAM_BEG		(PSYCHO_BUS_RAM_BEG)
#define RAM_END		(PSYCHO_BUS_RAM_END)

#define BIOS_BEG	(PSYCHO_BUS_BIOS_BEG)
#define BIOS_END	(PSYCHO_BUS_BIOS_END)
#define BIOS_MASK	(0x000FFFFF)

#define SPAD_BEG	(PSYCHO_BUS_SPAD_BEG)
#define SPAD_END	(PSYCHO_BUS_SPAD_END)
#define SPAD_MASK	(0x00000FFF)

#define I_STAT_ADDR	(0x1F801070)
#define I_MASK_ADDR	(0x1F801074)

// clang-format on

u32 bus_lw(const struct psycho_bus *const bus, const u32 paddr)
{
	u32 word = 0xFFFFFFFF;

	switch (paddr) {
	case RAM_BEG ... RAM_END:
		memcpy(&word, &bus->ram[paddr], sizeof(u32));
		break;

	case SPAD_BEG ... SPAD_END:
		memcpy(&word, &bus->spad[paddr & SPAD_MASK], sizeof(u32));
		break;

	case DMAC_DPCR_ADDR:
		word = bus->dmac.dpcr;
		break;

	case DMAC_DICR_ADDR:
		word = bus->dmac.dicr;
		break;

	case I_STAT_ADDR:
		word = bus->i_stat;
		break;

	case I_MASK_ADDR:
		word = bus->i_mask;
		break;

	case GPU_GPUSTAT_READ_ADDR:
		word = bus->gpu.gpustat;
		break;

	case BIOS_BEG ... BIOS_END:
		memcpy(&word, &bus->bios[paddr & BIOS_MASK], sizeof(u32));
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "load word; returning 0xFFFF'FFFF",
			 paddr);
		return word;
	}

	LOG_TRACE(bus->log, "Loaded word 0x%08X from 0x%08X", word, paddr);
	return word;
}

u16 bus_lh(const struct psycho_bus *bus, const u32 paddr)
{
	u16 hword = 0xFFFF;

	switch (paddr) {
	case RAM_BEG ... RAM_END:
		memcpy(&hword, &bus->ram[paddr], sizeof(u16));
		break;

	case SPAD_BEG ... SPAD_END:
		memcpy(&hword, &bus->spad[paddr & SPAD_MASK], sizeof(u16));
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "load half-word; returning 0xFFFF",
			 paddr);
		return hword;
	}
	LOG_TRACE(bus->log, "Loaded half-word 0x%04X from 0x%08X", hword,
		  paddr);
	return hword;
}

u8 bus_lb(const struct psycho_bus *const bus, const u32 paddr)
{
	u8 byte = 0xFF;

	switch (paddr) {
	case RAM_BEG ... RAM_END:
		byte = bus->ram[paddr];
		break;

	case SPAD_BEG ... SPAD_END:
		byte = bus->spad[paddr & SPAD_MASK];
		break;

	case BIOS_BEG ... BIOS_END:
		byte = bus->bios[paddr & BIOS_MASK];
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "load byte; returning 0xFF",
			 paddr);
		return byte;
	}

	LOG_TRACE(bus->log, "Loaded byte 0x%02X from 0x%08X", byte, paddr);
	return byte;
}

void bus_sw(struct psycho_bus *const bus, const u32 paddr, const u32 word)
{
	switch (paddr) {
	case RAM_BEG ... RAM_END:
		memcpy(&bus->ram[paddr], &word, sizeof(u32));
		break;

	case SPAD_BEG ... SPAD_END:
		memcpy(&bus->spad[paddr & SPAD_MASK], &word, sizeof(u32));
		break;

	case DMAC_MDECin_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECin].madr = word;
		break;

	case DMAC_MDECin_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECin].bcr = word;
		break;

	case DMAC_MDECin_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECin].chcr = word;
		break;

	case DMAC_MDECout_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECout].madr = word;
		break;

	case DMAC_MDECout_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECout].bcr = word;
		break;

	case DMAC_MDECout_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_MDECout].chcr = word;
		break;

	case DMAC_GPU_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_GPU].madr = word;
		break;

	case DMAC_GPU_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_GPU].bcr = word;
		break;

	case DMAC_GPU_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_GPU].chcr = word;
		break;

	case DMAC_CDROM_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_CDROM].madr = word;
		break;

	case DMAC_CDROM_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_CDROM].bcr = word;
		break;

	case DMAC_CDROM_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_CDROM].chcr = word;
		break;

	case DMAC_SPU_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_SPU].madr = word;
		break;

	case DMAC_SPU_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_SPU].bcr = word;
		break;

	case DMAC_SPU_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_SPU].chcr = word;
		break;

	case DMAC_PIO_MADR_ADDR:
		bus->dmac.channels[DMAC_CH_PIO].madr = word;
		break;

	case DMAC_PIO_BCR_ADDR:
		bus->dmac.channels[DMAC_CH_PIO].bcr = word;
		break;

	case DMAC_PIO_CHCR_ADDR:
		bus->dmac.channels[DMAC_CH_PIO].chcr = word;
		break;

	case DMAC_DPCR_ADDR:
		dmac_dpcr_set(&bus->dmac, word);
		break;

	case DMAC_DICR_ADDR:
		bus->dmac.dicr = word;
		break;

	case I_STAT_ADDR:
		bus->i_stat &= word;
		break;

	case I_MASK_ADDR:
		bus->i_mask = word;
		break;

	case GPU_GP0_WRITE_ADDR:
		gpu_gp0(&bus->gpu, word);
		break;

	case GPU_GP1_WRITE_ADDR:
		gpu_gp1(&bus->gpu, word);
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "store word 0x%08X; ignoring",
			 paddr, word);
		return;
	}
	LOG_TRACE(bus->log, "Stored word 0x%08X at 0x%08X", word, paddr);
}

void bus_sh(struct psycho_bus *const bus, const u32 paddr, const u16 hword)
{
	switch (paddr) {
	case RAM_BEG ... RAM_END:
		memcpy(&bus->ram[paddr], &hword, sizeof(u16));
		break;

	case SPAD_BEG ... SPAD_END:
		memcpy(&bus->spad[paddr & SPAD_MASK], &hword, sizeof(u16));
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "store half-word 0x%04X; ignoring",
			 paddr, hword);
		return;
	}
	LOG_TRACE(bus->log, "Stored half-word 0x%04X at 0x%08X", hword, paddr);
}

void bus_sb(struct psycho_bus *const bus, const u32 paddr, const u8 byte)
{
	switch (paddr) {
	case RAM_BEG ... RAM_END:
		bus->ram[paddr] = byte;
		break;

	case SPAD_BEG ... SPAD_END:
		bus->spad[paddr & SPAD_MASK] = byte;
		break;

	default:
		LOG_WARN(bus->log,
			 "Unknown physical address 0x%08X when attempting to "
			 "store byte 0x%02X; ignoring",
			 paddr, byte);
		break;
	}
	LOG_TRACE(bus->log, "Stored byte 0x%02X at 0x%08X", byte, paddr);
}
