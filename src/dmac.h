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

// clang-format off

#define DMAC_MDECin_MADR_ADDR	(0x1F801080)
#define DMAC_MDECin_BCR_ADDR	(0x1F801084)
#define DMAC_MDECin_CHCR_ADDR	(0x1F801088)
#define DMAC_MDECout_MADR_ADDR	(0x1F801090)
#define DMAC_MDECout_BCR_ADDR	(0x1F801094)
#define DMAC_MDECout_CHCR_ADDR	(0x1F801098)
#define DMAC_GPU_MADR_ADDR	(0x1F8010A0)
#define DMAC_GPU_BCR_ADDR	(0x1F8010A4)
#define DMAC_GPU_CHCR_ADDR	(0x1F8010A8)
#define DMAC_CDROM_MADR_ADDR	(0x1F8010B0)
#define DMAC_CDROM_BCR_ADDR	(0x1F8010B4)
#define DMAC_CDROM_CHCR_ADDR	(0x1F8010B8)
#define DMAC_SPU_MADR_ADDR	(0x1F8010C0)
#define DMAC_SPU_BCR_ADDR	(0x1F8010C4)
#define DMAC_SPU_CHCR_ADDR	(0x1F8010C8)
#define DMAC_PIO_MADR_ADDR	(0x1F8010D0)
#define DMAC_PIO_BCR_ADDR	(0x1F8010D4)
#define DMAC_PIO_CHCR_ADDR	(0x1F8010D8)
#define DMAC_OTC_MADR_ADDR	(0x1F8010E0)
#define DMAC_OTC_BCR_ADDR	(0x1F8010E4)
#define DMAC_OTC_CHCR_ADDR	(0x1F8010E8)
#define DMAC_DPCR_ADDR		(0x1F8010F0)
#define DMAC_DICR_ADDR		(0x1F8010F4)

#define DMAC_CH_MDECin	(0)
#define DMAC_CH_MDECout	(1)
#define DMAC_CH_GPU	(2)
#define DMAC_CH_CDROM	(3)
#define DMAC_CH_SPU	(4)
#define DMAC_CH_PIO	(5)

// clang-format on
