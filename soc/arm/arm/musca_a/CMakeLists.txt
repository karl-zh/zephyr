#
# Copyright (c) 2018 Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0
#

zephyr_include_directories(
CMSIS_Driver/Config
Native_Driver
Device/Include
Device/Config
)

zephyr_sources(
  soc.c
  arm_mpu_regions.c
  Device/Source/platform_retarget_dev.c
  Native_Driver/mpc_sie200_drv.c
#  Native_Driver/mpu_armv8m_drv.c
  CMSIS_Driver/Driver_MPC.c
  )

zephyr_compile_options(
  -D__ARM_FEATURE_CMSE=3
)
