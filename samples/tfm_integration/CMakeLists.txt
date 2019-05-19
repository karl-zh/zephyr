# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(tfm_integration)

set (TFM_BUILD_PATCH "/home/karzha01/LinaroWork/castor_zephyr/trusted-firmware-m/build")

target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE ${TFM_BUILD_PATCH}/install/export/tfm/inc)
target_link_libraries(${ZEPHYR_CURRENT_LIBRARY} PRIVATE ${TFM_BUILD_PATCH}/install/export/tfm/veneers/s_veneers.o)
