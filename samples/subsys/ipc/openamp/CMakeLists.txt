cmake_minimum_required(VERSION 3.13.1)
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#
#set(BOARD lpcxpresso54114_m4)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(openamp)

if(NOT ("${BOARD}" STREQUAL "lpcxpresso54114_m4"))
#  message(FATAL_ERROR "${BOARD} was specified, but this sample only supports lpcxpresso54114_m4")
endif()

enable_language(C ASM)

zephyr_include_directories(inc)
zephyr_include_directories(inc/interface)

zephyr_library_link_libraries($ENV{ZEPHYR_BASE}/lib/tfm/tfm_audit.a)


target_sources(app PRIVATE src/audit_s_interface_testsuite.c)

zephyr_library_link_libraries($ENV{ZEPHYR_BASE}/lib/tfm/tfm_crypto.a)

target_sources(app PRIVATE src/crypto_sec_interface_testsuite.c)
target_sources(app PRIVATE src/crypto_ns_interface_testsuite.c)

target_sources(app PRIVATE src/plat/dummy_crypto_keys.c)
if(ADD_TFM_LIB)
zephyr_library_link_libraries($ENV{ZEPHYR_BASE}/lib/tfm/tfm_storage.a)


target_sources(app PRIVATE src/plat/dummy_nv_counters.c)

#target_sources(app PRIVATE src/sst/secure/s_test_helpers.c)
target_sources(app PRIVATE src/sst/secure/sst_sec_interface_testsuite.c)
target_sources(app PRIVATE src/sst/secure/sst_reliability_testsuite.c)
endif()

# Location of external dependencies:
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform")

target_sources(app PRIVATE src/main_master.c
	       ${PLATFORM_DIR}/platform.c
	       ${PLATFORM_DIR}/platform_ops.c
	       ${PLATFORM_DIR}/resource_table.c)

include(ExternalProject)

ExternalProject_Add(
  openamp_remote
  SOURCE_DIR ${APPLICATION_SOURCE_DIR}/remote
  INSTALL_COMMAND ""      # This particular build system has no install command
  BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/openamp_remote-prefix/src/openamp_remote-build/zephyr/zephyr.bin"
  # NB: Do we need to pass on more CMake variables?
)
#add_dependencies(core_m0_inc_target openamp_remote)

target_include_directories(app PRIVATE ${PLATFORM_DIR})
