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

#include "compiler.h"
#include "types.h"

enum {
	PSYCHO_CPU_CLOCK_SPEED_HZ = 33868800,
};

enum psycho_cpu_gpr {
	// clang-format off

	CPU_GPR_ZERO	= 0,
	CPU_GPR_AT	= 1,
	CPU_GPR_V0	= 2,
	CPU_GPR_V1	= 3,
	CPU_GPR_A0	= 4,
	CPU_GPR_A1	= 5,
	CPU_GPR_A2	= 6,
	CPU_GPR_A3	= 7,
	CPU_GPR_T0	= 8,
	CPU_GPR_T1	= 9,
	CPU_GPR_T2	= 10,
	CPU_GPR_T3	= 11,
	CPU_GPR_T4	= 12,
	CPU_GPR_T5	= 13,
	CPU_GPR_T6	= 14,
	CPU_GPR_T7	= 15,
	CPU_GPR_S0	= 16,
	CPU_GPR_S1	= 17,
	CPU_GPR_S2	= 18,
	CPU_GPR_S3	= 19,
	CPU_GPR_S4	= 20,
	CPU_GPR_S5	= 21,
	CPU_GPR_S6	= 22,
	CPU_GPR_S7	= 23,
	CPU_GPR_T8	= 24,
	CPU_GPR_T9	= 25,
	CPU_GPR_K0	= 26,
	CPU_GPR_K1	= 27,
	CPU_GPR_GP	= 28,
	CPU_GPR_SP	= 29,
	CPU_GPR_FP	= 30,
	CPU_GPR_RA	= 31,
	CPU_GPR_NUM	= 32

	// clang-format on
};

enum psycho_cpu_cop0_regs {
	// clang-format off

	CPU_COP0_BPC	= 3,
	CPU_COP0_BDA	= 5,
	CPU_COP0_TAR	= 6,
	CPU_COP0_DCIC	= 7,
	CPU_COP0_BADA	= 8,
	CPU_COP0_BDAM	= 9,
	CPU_COP0_BPCM	= 11,
	CPU_COP0_SR	= 12,
	CPU_COP0_CAUSE	= 13,
	CPU_COP0_EPC	= 14,
	CPU_COP0_PRID	= 15,
	CPU_COP0_NUM	= 32

	// clang-format on
};

enum { RESET_PC = 0xBFC00000 };
