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

/// @file cpu.h Provides public information about the CPU interpreter.

#pragma once

#include "bus.h"
#include "dbg_log.h"
#include "cpu_defs.h"
#include "types.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct psycho_cpu {
	u32 gpr[PSYCHO_CPU_GPR_REGS_NUM];
	u32 cp0_cpr[PSYCHO_CPU_CP0_CPR_REGS_NUM];

	/// @brief Geometry Transformation Engine (GTE, COP2) coprocessor
	/// registers.
	///
	/// @note The implementation makes extensive use of type-punning through
	/// unions to avoid not only explicit casting but for overall code
	/// clarity. While this is officially undefined behavior, it is
	/// permitted as a gcc/clang extension. Issues that are raised regarding
	/// this behavior will be closed as not a bug. Please refer to the gcc
	/// documentation excerpt at https://preview.tinyurl.com/typepun.

	/// @brief Geometry Transformation Engine (GTE, COP2) coprocessor
	/// registers.
	///
	/// @note The implementation makes extensive use of type-punning through
	/// unions to avoid not only explicit casting but for overall code
	/// clarity. While this is officially undefined behavior, it is
	/// permitted as a gcc/clang extension. Issues that are raised regarding
	/// this behavior will be closed as not a bug. Please refer to the gcc
	/// documentation excerpt at https://preview.tinyurl.com/typepun.
	struct {
		/// @brief Data registers.
		union {
			struct {
				s16 V[3][4];
				u8 RGBC[4];
				u16 OTZ; // 7
				u16 pad0;
				s16 IR0; // 8
				u16 pad1;
				s16 IR1; // 9
				u16 pad2;
				s16 IR2; // 10
				u16 pad3;
				s16 IR3; // 11
				s32 SXY0; // 12
				s32 SXY1; // 13
				s32 SXY2; // 14
				s32 SXYP; // 15
				u16 SZ0; // 16
				u16 pad4;
				u16 SZ1; // 17
				u16 pad5;
				u16 SZ2; // 18
				u16 pad6;
				u16 SZ3; // 19
				u32 RGB0; // 20
				u32 RGB1; // 21
				u32 RGB2; // 22
				u32 RES1; // 23
				s32 MAC0; // 24
				s32 MAC1; // 25
				s32 MAC2; // 26
				s32 MAC3; // 27
				u16 IRGB; // 28
				u16 pad7;
				u16 ORGB; // 29
				s32 LZCS; // 30
				s32 LZCR; // 31
			};
			u32 regs[PSYCHO_CPU_CP2_CPR_REGS_NUM];
		} cpr;

		/// @brief Control registers.
		union {
			struct {
				s16 RT[3][3];
				s32 TR[3];
				s32 L11L12; // 8
				s32 L13L21; // 9
				s32 L22L23; // 10
				s32 L31L32; // 11
				s16 L33; // 12
				s32 RBK; // 13
				s32 GBK; // 14
				s32 BBK; // 15
				s32 LR1LR2; // 16
				s32 LR3LG1; // 17
				s32 LG2LG3; // 18
				s32 LB1LB2; // 19
				s16 LB3; // 20
				s32 RFC; // 21
				s32 GFC; // 22
				s32 BFC; // 23
				s32 OFX; // 24
				s32 OFY; // 25
				u16 H; // 26
				u16 pad0;
				s16 DQA; // 27
				s32 DQB; // 28
				s16 ZSF3; // 29
				u16 pad1;
				s16 ZSF4; // 30
				u32 FLAG; // 31
			};
			u32 regs[PSYCHO_CPU_CP2_CCR_REGS_NUM];
		} ccr;

		s64 MAC0;
		s64 MAC1;
		s64 MAC2;
		s64 MAC3;
	} cp2;

	struct {
		uint dst;
		u32 val;
	} lds_pend, lds_next;

	u32 instr;
	u32 pc;
	u32 npc;

	u32 hi;
	u32 lo;

	u16 exc_halt;

	struct psycho_bus *bus;
	struct psycho_dbg_log *log;
};
#pragma GCC diagnostic pop
