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

# The minimum version of CMake required is tied to the CMake version shipped by
# the oldest currently supported Ubuntu LTS release.
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(
	psycho
	VERSION 0.1.0
	DESCRIPTION "Sony PlayStation 1 emulator"
	HOMEPAGE_URL "https://github.com/mcroddev/psycho"
	LANGUAGES C
)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(
		FATAL_ERROR
		"In-tree builds are forbidden; please create a build directory "
		"and rerun CMake from there."
	)
endif()

if (CMAKE_BUILD_TYPE MATCHES "MinSizeRel|RelWithDebInfo")
	message(
		FATAL_ERROR
		"Invalid build type specified: only Release and Debug are "
		"permitted."
	)
endif()

if (CMAKE_VERSION LESS 3.26)
	# CMake >=3.26 sets this property to ON by default.
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(CLANG_VER_MIN 14.0.0)
set(GCC_VER_MIN 11.4.0)

function(error_unsupported_compiler)
	message(
		FATAL_ERROR
		"Your compiler is not supported by this project; please use "
		"either clang >=${CLANG_VER_MIN} or gcc >=${GCC_VER_MIN}.")
endfunction()

function(psycho_configure_base_settings_c)
	# These flags are set regardless of the build type and compiler. Note
	# that some flags are not supported by both clang and gcc; keep that in
	# mind when updating this list.
	set(COMPILE_FLAGS
		-Wall
		-Walloc-zero
		-Walloca
		-Wbad-function-cast
		-Wcast-qual
		-Wdate-time
		-Wdisabled-optimization
		-Wdouble-promotion
		-Wduplicated-branches
		-Wduplicated-cond
		-Wextra
		-Wfloat-equal
		-Wformat-overflow=2
		-Wformat-signedness
		-Wformat-truncation=2
		-Wformat=2
		-Winit-self
		-Winline
		-Winvalid-pch
		-Wjump-misses-init
		-Wlogical-op
		-Wmain
		-Wmissing-declarations
		-Wmissing-include-dirs
		-Wmissing-noreturn
		-Wmissing-prototypes
		-Wnested-externs
		-Wno-aggressive-loop-optimizations
		-Wnull-dereference
		-Wold-style-definition
		-Wpacked
		-Wredundant-decls
		-Wshadow
		-Wshift-overflow=2
		-Wstrict-prototypes
		-Wsuggest-attribute=cold
		-Wsuggest-attribute=const
		-Wsuggest-attribute=format
		-Wsuggest-attribute=malloc
		-Wsuggest-attribute=noreturn
		-Wsuggest-attribute=pure
		-Wswitch-default
		-Wswitch-enum
		-Wsync-nand
		-Wtrampolines
		-Wundef
		-Wuninitialized
		-Wunknown-pragmas
		-Wunsafe-loop-optimizations
		-Wunsuffixed-float-constants
		-Wunused-macros
		-Wvla
		-Wwrite-strings
	)

	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		if (CMAKE_C_COMPILER_VERSION LESS ${GCC_VER_MIN})
			error_unsupported_compiler()
		endif()

		# These flags are only supported by gcc and are set regardless
		# of the build type.
		set(GCC_BUILD_FLAGS)
	elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
		if (CMAKE_C_COMPILER_VERSION LESS ${CLANG_VER_MIN})
			error_unsupported_compiler()
		endif()
	else()
		error_unsupported_compiler()
	endif()

	# Some people say you shouldn't touch CMAKE_*_FLAGS_*, and then there
	# are some people who say the Earth is flat. Both positions are equally
	# wrong, no matter how many people might say otherwise.
	#
	# The whole point of this function is to provide a project-wide
	# interface library to propagate our desired configuration for code
	# under *our* control without affecting third-party libraries.
	#
	# For some very strange reason, it seems as though targets do not get an
	# isolated copy of the default compile flags specified by the build
	# type, which means targets will *always* compile with options prepended
	# from CMAKE_*_FLAGS_*, forcing you to modify the global flags anyway if
	# you must, which breaks the principle of isolated, individual targets.
	#
	# Not sure if there's another solution to this, but for now we're going
	# to just modify the default flags anyway.
	string(
		REGEX REPLACE "^-g$" "-ggdb3"
		CMAKE_C_FLAGS_DEBUG
		"${CMAKE_C_FLAGS_DEBUG}"
	)

	string(APPEND CMAKE_C_FLAGS_DEBUG " -Og")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "" FORCE)

	string(
		REPLACE "-O3" "-Ofast"
		CMAKE_C_FLAGS_RELEASE
		"${CMAKE_C_FLAGS_RELEASE}"
	)

	set(
		CMAKE_C_FLAGS_RELEASE
		"${CMAKE_C_FLAGS_RELEASE}"
		CACHE STRING "" FORCE
	)

	add_library(psycho_cfg_base_c INTERFACE)
	target_compile_options(psycho_cfg_base_c INTERFACE ${COMPILE_FLAGS})
endfunction()

psycho_configure_base_settings_c()
add_subdirectory(core)
