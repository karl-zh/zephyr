cmake_minimum_required(VERSION 3.13.1)
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#
set(BOARD mps2_an521_nonsecure)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(openamp_remote)

#set(ERPC_DIR "$ENV{ZEPHYR_BASE}/../erpc/")
set(ERPC_DIR "/home/karzha01/LinaroWork/castor_zephyr/erpc/")

zephyr_library_link_libraries(${ERPC_DIR}/build/release/liberpc_client.a)

set(SERVICE_DIR "${ERPC_DIR}/service/")

#if(NOT ("${BOARD}" STREQUAL "lpcxpresso54114_m0"))
#  message(FATAL_ERROR "${BOARD} was specified, but this sample only supports lpcxpresso54114_m0")
#endif()

target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_include_directories(app PRIVATE ${SERVICE_DIR})
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/infra)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/port)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/config)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/setup)
