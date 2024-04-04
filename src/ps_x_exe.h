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

#include <stdbool.h>
#include <string.h>

#include "compiler.h"

#include "psycho/ps_x_exe.h"
#include "psycho/types.h"

// clang-format off

#define PS_X_EXE_OFFSET_ID		(0x00)
#define PS_X_EXE_OFFSET_PC		(0x10)
#define PS_X_EXE_OFFSET_GP		(0x14)
#define PS_X_EXE_OFFSET_DEST		(0x18)
#define PS_X_EXE_OFFSET_SIZE		(0x1C)
#define PS_X_EXE_OFFSET_SP_FP_BASE	(0x30)
#define PS_X_EXE_OFFSET_SP_FP_OFFS	(0x34)
#define PS_X_EXE_OFFSET_DATA		(0x800)

// clang-format on

NODISCARD ALWAYS_INLINE u32 ps_x_exe_word_read(const u8 *const data,
					       const uint index)
{
	u32 word;
	memcpy(&word, &data[index], sizeof(u32));

	return word;
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_size_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_SIZE);
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_dest_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_DEST);
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_pc_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_PC);
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_gp_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_GP);
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_sp_fp_base_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_SP_FP_BASE);
}

NODISCARD ALWAYS_INLINE u32 ps_x_exe_sp_fp_offs_get(const u8 *const data)
{
	return ps_x_exe_word_read(data, PS_X_EXE_OFFSET_SP_FP_OFFS);
}

NODISCARD ALWAYS_INLINE bool ps_x_exe_valid(const u8 *const data,
					    const size_t len)
{
	if (len < PSYCHO_PS_X_EXE_HEADER_SIZE) {
		return false;
	}

	if (memcmp(&data[PS_X_EXE_OFFSET_ID], "PS-X EXE", sizeof("PS-X EXE")) !=
	    0) {
		return false;
	}

	const u32 size = ps_x_exe_size_get(data);

	if (size != (len - PSYCHO_PS_X_EXE_HEADER_SIZE)) {
		return false;
	}
	return true;
}
