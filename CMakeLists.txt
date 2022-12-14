# This file is part of Addcont ID Dumper
# Copyright © 2020 浅倉麗子

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)

set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "DolceSDK toolchain file")
set(DOLCESDK_CMAKE_FILE "$ENV{DOLCESDK}/share/dolce.cmake" CACHE PATH "DolceSDK CMake functions file")
include("${DOLCESDK_CMAKE_FILE}" REQUIRED)

option(CMAKE_TLS_VERIFY "Verify TLS" ON)
include(ExternalProject REQUIRED)

project(aciddumper LANGUAGES C)

add_compile_options(
  -Os -std=c99
  -Wall -Wextra -Wshadow -Wundef
  -fno-common -flto
)

add_link_options(
  -nostdlib
  --entry=module_start
)

set(LOGGING "1" CACHE STRING "Enable logging")
set(FILE_LOGGING "0" CACHE STRING "Enable file logging")
set(LOG_LEVEL "INFO" CACHE STRING "Log verbosity level")

add_compile_definitions(
  SCE_DBG_LOGGING_ENABLED=${LOGGING}
  SCE_DBG_FILE_LOGGING_ENABLED=${FILE_LOGGING}
  SCE_DBG_MINIMUM_LOG_LEVEL=SCE_DBG_LOG_LEVEL_${LOG_LEVEL}
)

if(FILE_LOGGING)
  add_compile_definitions(SCE_DBG_LOG_PREFIX="[ACIDDumper] ")
else()
  add_compile_definitions(SCE_DBG_LOG_PREFIX="\\033[0;35m[ACIDDumper]\\033[0m ")
endif()

# External dependencies

set(DEP_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dep")
include_directories("${DEP_INSTALL_PREFIX}/include")
link_directories("${DEP_INSTALL_PREFIX}/lib")

ExternalProject_Add(dep-psp2dbg
  URL https://git.shotatoshounenwachigau.moe/vita/psp2dbg/snapshot/psp2dbg-master.tar.gz
  PREFIX dep/psp2dbg
  CMAKE_ARGS -D CMAKE_INSTALL_PREFIX=${DEP_INSTALL_PREFIX}
  EXCLUDE_FROM_ALL ON
)

add_custom_target(dep-all
  DEPENDS dep-psp2dbg
)

# Build self

set(ELF "aciddumper.elf")
set(SELF "aciddumper.skprx")
set(MODULE_INFO "aciddumper.yml")

add_executable("${ELF}"
  aciddumper.c
)

target_link_libraries("${ELF}"
  psp2dbgForKernel

  taihenForKernel_stub

  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceThreadmgrForDriver_stub
)

dolce_create_self("${SELF}"
  "${ELF}"
  CONFIG "${MODULE_INFO}"
)
