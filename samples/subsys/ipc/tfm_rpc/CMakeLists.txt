cmake_minimum_required(VERSION 3.13.1)
# Copyright (c) 2019, Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0
#

if(("${BOARD}" STREQUAL "mps2_an521_nonsecure"))
	set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/overlay/${BOARD}.overlay)
else()
	message(FATAL_ERROR "${BOARD} was not supported")
endif()

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(openamp_remote)

set(ERPC_DIR "$ENV{ZEPHYR_BASE}/../../../erpc/")
set(TFM_EXPORT_DIR "$ENV{ZEPHYR_BASE}/ext/tfm/tfm/build/install/export/tfm")
set(SST_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test/sst/non_secure")

zephyr_library_link_libraries(${ERPC_DIR}/build/release/liberpc_client.a)

set(SERVICE_DIR "${ERPC_DIR}/service/")

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/tfm_ns_lock_zephyr_rtos.c)

#target_sources(app PRIVATE ${SST_TEST_DIR}/ns_test_helpers.c)
target_sources(app PRIVATE ${SST_TEST_DIR}/psa_ps_ns_interface_testsuite.c)

target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_sst_api.c)
# target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_audit_api.c)
#target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_crypto_api.c)
#target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_initial_attestation_api.c)
# target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_nspm_api.c)
#target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_nspm_svc_handler.c)
# target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_platform_api.c)
#target_sources(app PRIVATE ${TFM_EXPORT_DIR}/src/tfm_psa_ns_api.c)

target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/samples/subsys/ipc/openamp/)
target_include_directories(app PRIVATE ${TFM_EXPORT_DIR}/inc)
target_compile_definitions(app PRIVATE -DTFM_PARTITION_TEST_CORE_IPC)
target_compile_definitions(app PRIVATE -DTFM_PSA_API)

target_include_directories(app PRIVATE ${SERVICE_DIR})
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/infra)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/port)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/config)
target_include_directories(app PRIVATE ${ERPC_DIR}/erpc_c/setup)

target_include_directories(app PRIVATE ${SST_TEST_DIR})
