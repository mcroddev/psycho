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

/// @file dbg_disasm.h Provides the public interface for the disassembler.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

struct psycho_ctx;

/// @brief The maximum size of a disassembly result.
#define PSYCHO_DBG_DISASM_LEN_MAX (512)

#define PSYCHO_DBG_DISASM_COMMENTS_NUM_MAX (8)

struct psycho_dbg_disasm {
	/// @brief The current disassembly result.
	char result[PSYCHO_DBG_DISASM_LEN_MAX];

	/// @brief The comments specified during disassembly which can be
	/// appended to the result pre or post instruction execution.
	uint comments[PSYCHO_DBG_DISASM_COMMENTS_NUM_MAX];

	/// @brief The length of the current disassembly result.
	int len;

	/// @brief The number of comments that were specified.
	uint num_comments;

	/// @brief The instruction being disassembled.
	u32 instr;

	/// @brief The program counter to take into account when disassembling
	/// branch or jump instructions.
	u32 pc;
};

/// @brief Disassembles an instruction.
///
/// @param ctx The psycho_ctx instance.
/// @param instr The instruction to disassemble.
/// @param pc The program counter to take into account if the instruction is a
/// branch or jump.
///
/// @note Presumably, you would be calling this function with the current
/// instruction and program counter.
void psycho_dbg_disasm_instr(struct psycho_ctx *ctx, u32 instr, u32 pc);

/// @brief Executes a pre or post instruction execution trace of the last
/// disassembled instruction.
///
/// @param ctx The psycho_ctx instance.
void psycho_dbg_disasm_trace(struct psycho_ctx *ctx);

#ifdef __cplusplus
}
#endif // __cplusplus
