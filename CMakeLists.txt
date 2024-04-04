# SPDX-License-Identifier: MIT
#
# Copyright 2024 Michael Rodriguez
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

# The minimum version of CMake required is tied to the CMake version present in
# the repositories of the oldest Ubuntu LTS version which is still under
# standard support. At this time, that version is 20.04 "Focal Fossa", which
# contains CMake version 3.16.3 in its repositories.
#
# Although an exact end of life date has not yet been determined for 20.04, it
# is slated for sometime in April 2025. When this happens, the minimum CMake
# version required should be bumped to whatever the minimum CMake version is in
# 22.04 LTS "Jammy Jellyfish". This process should repeat continuously.

# The FATAL_ERROR option is accepted but ignored by CMake 2.6 and higher. It
# should be specified so CMake versions 2.4 and lower fail with an error instead
# of just a warning.
cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

project(psycho
	VERSION 1.0.0.0
	DESCRIPTION "Sony PlayStation 1 emulator"
	HOMEPAGE_URL "https://github.com/mcroddev/psycho"
	LANGUAGES C)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
	message(FATAL_ERROR
		"In-tree builds are forbidden; please create another directory "
		"and rerun CMake from there.")
endif()

# Generate a "compile_commands.json" file containing the exact compiler calls
# for all translation units of the project. This is required for certain IDEs
# and text editors to use code completion and real-time diagnostics.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Controls whether or not to use the FOLDER target property to organize targets
# into folders.
#
# Not all CMake generators support recording folder details for targets. The
# Xcode and Visual Studio generators are examples of generators that do.
# Similarly, not all IDEs support presenting targets using folder hierarchies,
# even if the CMake generator used provides the necessary information.
if (CMAKE_VERSION LESS 3.26)
	# CMake versions >=3.26 treat this property as ON by default.
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# The minimum compiler versions required are tied the same way as the minimum
# required CMake version is; refer to the top of the file for the criteria.
set(CLANG_VER_MIN "10.0.0")
set(GCC_VER_MIN "9.4.0")

option(PSYCHO_WARNINGS_ARE_ERRORS "Treat all warnings as errors" OFF)
option(PSYCHO_ENABLE_SANITIZERS "Enable ASan and UBSan if possible" OFF)

option(
	PSYCHO_ENABLE_LTO
	"Enable link-time optimization (not allowed for Debug builds)"
	OFF
)

# Note that an INTERFACE library is not a "real" library; it does not produce
# artifacts on disk nor does it require source files to be specified; in this
# case it is a way for us to set properties that get inherited by targets when
# they "link" to this library.
add_library(psycho_build_config_base INTERFACE)

# These flags are supported by both clang and gcc and are used regardless of the
# build type and language.
#
# Be sure to verify that this assertion holds when updating the minimum compiler
# versions, as we have no way to automate that process.
set(COMPILER_FLAGS_BASE
	-Wall
	-Walloca
	-Wbad-function-cast
	-Wcast-align
	-Wcast-qual
	-Wconversion
	-Wdisabled-optimization
	-Wdouble-promotion
	-Wextra
	-Wfloat-equal
	-Wformat=2
	-Winit-self
	-Winline
	-Winvalid-pch
	-Wmissing-declarations
	-Wmissing-format-attribute
	-Wmissing-include-dirs
	-Wmissing-noreturn
	-Wmissing-prototypes
	-Wnested-externs
	-Wnull-dereference
	-Wold-style-definition
	-Wpacked
	-Wpadded
	-Wredundant-decls
	-Wshadow
	-Wstrict-prototypes
	-Wswitch-default
	-Wswitch-enum
	-Wundef
	-Wvla
	-Wwrite-strings
	-fno-stack-protector
	-fcf-protection=none
	-fno-asynchronous-unwind-tables
	-fstack-clash-protection
	-fno-pie
	-fno-stack-clash-protection)

# These flags will apply if and only if gcc is the compiler.
set(COMPILER_FLAGS_BASE_GCC
	-Waggressive-loop-optimizations
	-Walloc-zero
	-Wduplicated-branches
	-Wduplicated-cond
	-Wformat-signedness
	-Wjump-misses-init
	-Wlogical-op
	-Wstringop-truncation
	-Wsuggest-attribute=cold
	-Wsuggest-attribute=const
	-Wsuggest-attribute=malloc
	-Wsuggest-attribute=pure
	-Wunsafe-loop-optimizations
	-Wunsuffixed-float-constants)

if (PSYCHO_ENABLE_SANITIZERS)
	list(APPEND COMPILER_FLAGS_BASE -fsanitize=address,undefined)

	target_link_options(
		psycho_build_config_base INTERFACE
		-fsanitize=address,undefined
	)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	list(APPEND COMPILER_FLAGS_BASE "-ggdb3")
	list(APPEND COMPILER_FLAGS_BASE ${COMPILER_FLAGS_BASE_GCC})

	target_link_options(psycho_build_config_base INTERFACE -no-pie)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
	list(APPEND COMPILER_FLAGS_BASE "-glldb")
	target_link_options(psycho_build_config_base INTERFACE -no-pie)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND COMPILER_FLAGS_BASE "-Og")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release|RelWithDebInfo")
	list(APPEND COMPILER_FLAGS_BASE "-Ofast")
endif()

target_compile_definitions(psycho_build_config_base INTERFACE -D_FORTIFY_SOURCE=0)

target_compile_options(
	psycho_build_config_base INTERFACE
	${COMPILER_FLAGS_BASE}
)

add_library(psycho_build_config_c INTERFACE)

if (CMAKE_VERSION LESS "3.21.7")
	target_compile_features(psycho_build_config_c INTERFACE c_std_11)
else()
	target_compile_features(psycho_build_config_c INTERFACE c_std_17)
endif()

target_link_libraries(psycho_build_config_c INTERFACE psycho_build_config_base)

add_subdirectory(src)
add_subdirectory(debugger)
