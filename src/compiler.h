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

/// @file compiler.h Provides wrappers around some compiler features for easier
/// readability and usage.

#pragma once

/// @brief This function will always be inlined regardless of compiler
/// heuristics or optimization level.
#define ALWAYS_INLINE inline __attribute__((always_inline))

/// @brief The return value of this function should not be discarded.
#define NODISCARD __attribute__((warn_unused_result))

/// @brief This statement will not be reached.
#define UNREACHABLE (__builtin_unreachable())

/// @brief This function will not return a `NULL` pointer.
#define RETURNS_NONNULL __attribute__((returns_nonnull))

/// @brief Performs the same compile-time `printf` format checks on a
/// `printf`-like function.
/// @param index The argument which is the format string.
/// @param first The number of the first argument to check against the format
/// string.
#define FORMAT_CHK(index, first) __attribute__((format(printf, index, first)))

/// @brief This branch is unlikely to be executed.
#define unlikely(x) (__builtin_expect(!!(x), 0))

/// @brief This branch is likely to be executed.
#define likely(x) (__builtin_expect(!!(x), 1))

/// @brief This case label is intended to fall through.
#define FALLTHROUGH __attribute__((fallthrough))
