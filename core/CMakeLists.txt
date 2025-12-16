# SPDX-License-Identifier: MIT
#
# Copyright 2025 Michael Rodriguez
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(SRCS bios-trace.c bus.c cpu.c ctx.c disasm.c log.c)

set(HDRS_PUBLIC
	include/core/bios-trace.h
	include/core/bus.h
	include/core/cpu.h
	include/core/ctx.h
	include/core/log.h
	include/core/types.h
)

add_library(core STATIC ${SRCS} ${HDRS_PUBLIC})
target_include_directories(core PUBLIC include)
target_link_libraries(core PRIVATE psycho_cfg_base_c)

# Unfortunately, interface targets do not propagate a desired language standard.
# We have no choice but to leave it to individual targets to set the project
# wide standard correctly.
#
# See: https://discourse.cmake.org/t/setting-c-standard-on-interface-libraries/10933/3
set_target_properties(
	core PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS ON
)
