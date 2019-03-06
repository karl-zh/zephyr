cmake_minimum_required(VERSION 3.13.1)
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#
#set(BOARD lpcxpresso54114_m0)
set(BOARD v2m_musca_nonsecure)
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../platform")

set(SERVICE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../service")
set(ERPC_DIR "$ENV{ZEPHYR_BASE}/lib/erpc/erpc_c/")

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(openamp_remote)

if(NOT ("${BOARD}" STREQUAL "lpcxpresso54114_m0"))
#  message(FATAL_ERROR "${BOARD} was specified, but this sample only supports lpcxpresso54114_m0")
endif()

zephyr_library_link_libraries($ENV{ZEPHYR_BASE}/lib/erpc/liberpc.a)

target_sources(app PRIVATE src/main_remote.c
	       ${PLATFORM_DIR}/platform.c
	       ${PLATFORM_DIR}/resource_table.c
	       ${PLATFORM_DIR}/platform_ops.c)

target_include_directories(app PRIVATE ${PLATFORM_DIR})

target_include_directories(app PRIVATE ${SERVICE_DIR})
target_include_directories(app PRIVATE ${ERPC_DIR}/infra)
target_include_directories(app PRIVATE ${ERPC_DIR}/port)
target_include_directories(app PRIVATE ${ERPC_DIR}/config)
