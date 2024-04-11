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

/// @file bus.h Defines the private interface for the system bus interconnect.

#pragma once

#include "psycho/bus.h"

/// @brief Loads a word from the system bus.
/// @param bus The psycho_bus instance.
/// @param paddr The physical address to load the word from.
/// @returns The word from the system bus.
u32 bus_lw(const struct psycho_bus *bus, u32 paddr);

/// @brief Loads a half-word from the system bus.
/// @param bus The psycho_bus instance.
/// @param paddr The physical address to load the half-word from.
/// @returns The half-word from the system bus.
u16 bus_lh(const struct psycho_bus *bus, u32 paddr);

/// @brief Loads a byte from the system bus.
/// @param bus The psycho_bus instance.
/// @param paddr The physical address to load the byte from.
/// @returns The byte from the system bus.
u8 bus_lb(const struct psycho_bus *bus, u32 paddr);

/// @brief Stores a word to the system bus.
/// @param ctx The psycho_ctx instance.
/// @param paddr The physical address to store the word into.
/// @param word The word to store.
void bus_sw(struct psycho_bus *bus, u32 paddr, u32 word);

/// @brief Stores a half-word to the system bus.
/// @param ctx The psycho_ctx instance.
/// @param paddr The physical address to store the half-word into.
/// @param hword The half-word to store.
void bus_sh(struct psycho_bus *bus, u32 paddr, u16 hword);

/// @brief Stores a byte to the system bus.
/// @param ctx The psycho_ctx instance.
/// @param paddr The physical address to store the byte into.
/// @param byte The byte to store.
void bus_sb(struct psycho_bus *bus, u32 paddr, u8 byte);
