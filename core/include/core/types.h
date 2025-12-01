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

/**
 * @file types.h Defines fixed-width integer types.
 *
 * These fixed-width integer types are exactly the same as the fixed-width
 * integer types provided by `stdint.h`, but the names are shortened for
 * brevity.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

/** Unsigned 64-bit integer */
typedef uint64_t u64;

/** Signed 64-bit integer */
typedef int64_t s64;

/** Unsigned 32-bit integer */
typedef uint32_t u32;

/** Signed 32-bit integer */
typedef int32_t s32;

/** Unsigned 16-bit integer */
typedef uint16_t u16;

/** Signed 16-bit integer */
typedef int16_t s16;

/** Unsigned 8-bit integer */
typedef uint8_t u8;

/** Signed 8-bit integer */
typedef int8_t s8;

#ifdef __cplusplus
}
#endif // __cplusplus
