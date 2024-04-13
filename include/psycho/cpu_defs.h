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

#define PSYCHO_CPU_GPR_REGS_NUM		(32)
#define PSYCHO_CPU_CP0_CPR_REGS_NUM	(32)
#define PSYCHO_CPU_CP2_CPR_REGS_NUM	(32)
#define PSYCHO_CPU_CP2_CCR_REGS_NUM	(32)

#define PSYCHO_CPU_EXC_CODE_AdEL	(0x4)
#define PSYCHO_CPU_EXC_CODE_AdES	(0x5)
#define PSYCHO_CPU_EXC_CODE_Sys		(0x8)
#define PSYCHO_CPU_EXC_CODE_Bp		(0x9)
#define PSYCHO_CPU_EXC_CODE_RI		(0xA)
#define PSYCHO_CPU_EXC_CODE_Ovf		(0xC)

#define PSYCHO_CPU_GTE_NUM_VECS (3)
#define PSYCHO_CPU_GTE_NUM_VEC_COMPONENTS (3)

extern const char *const psycho_cpu_gpr_names[PSYCHO_CPU_GPR_REGS_NUM];
extern const char *const psycho_cpu_cp0_cpr_names[PSYCHO_CPU_CP0_CPR_REGS_NUM];
extern const char *const psycho_cpu_cp2_cpr_names[PSYCHO_CPU_CP2_CPR_REGS_NUM];
extern const char *const psycho_cpu_cp2_ccr_names[PSYCHO_CPU_CP2_CCR_REGS_NUM];
