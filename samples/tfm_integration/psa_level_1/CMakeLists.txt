# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.inc)

project(tfm_psa_level_1)

# Source files in this sample
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/psa_attestation.c)
target_sources(app PRIVATE src/psa_crypto.c)
target_sources(app PRIVATE src/shell.c)
target_sources(app PRIVATE src/util_app_cfg.c)
target_sources(app PRIVATE src/util_app_log.c)
target_sources(app PRIVATE src/util_sformat.c)

# TF-M 'build/install' source files required by this sample
set(TFM_ROOT_PATH $ENV{ZEPHYR_BASE}/ext/tfm/tfm)
set(PSA_API_TEST_BUILD_PATH ${TFM_ROOT_PATH}/../psa-arch-tests/api-tests/BUILD)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/include/cmsis_rtos_v2)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/app/os_wrapper_cmsis_rtos_v2.c)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/interface/src/tfm_psa_ns_api.c)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/interface/src/tfm_ns_interface.c)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/interface/src/tfm_crypto_ipc_api.c)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/interface/src/tfm_sst_ipc_api.c)
target_sources(app PRIVATE ${TFM_ROOT_PATH}/interface/src/tfm_initial_attestation_ipc_api.c)

target_include_directories(app PRIVATE
  ${TFM_ROOT_PATH}/interface/include
  )

# Include the S/NS veneer functions
target_link_libraries(${ZEPHYR_CURRENT_LIBRARY} PRIVATE
  ${TFM_ROOT_PATH}/build/install/export/tfm/veneers/s_veneers.o
  )

target_compile_definitions(app PRIVATE
  -DTFM_PSA_API
  )
