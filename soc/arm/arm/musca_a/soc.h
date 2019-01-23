/*
 * Copyright (c) 2018 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SOC_H_
#define _SOC_H_

#ifndef _ASMLANGUAGE
#include "system_cmsdk_musca.h"
#include <generated_dts_board.h>
#include <misc/util.h>
#endif

#include "platform_regs.h"           /* Platform registers */

/* SRAM MPC ranges and limits */
/* Internal memory */
#define MPC_ISRAM0_RANGE_BASE_NS   0x20000000
#define MPC_ISRAM0_RANGE_LIMIT_NS  0x20007FFF
#define MPC_ISRAM0_RANGE_BASE_S    0x30000000
#define MPC_ISRAM0_RANGE_LIMIT_S   0x30007FFF

#define MPC_ISRAM1_RANGE_BASE_NS   0x20008000
#define MPC_ISRAM1_RANGE_LIMIT_NS  0x2000FFFF
#define MPC_ISRAM1_RANGE_BASE_S    0x30008000
#define MPC_ISRAM1_RANGE_LIMIT_S   0x3000FFFF

#define MPC_ISRAM2_RANGE_BASE_NS   0x20010000
#define MPC_ISRAM2_RANGE_LIMIT_NS  0x20017FFF
#define MPC_ISRAM2_RANGE_BASE_S    0x30010000
#define MPC_ISRAM2_RANGE_LIMIT_S   0x30017FFF

#define MPC_ISRAM3_RANGE_BASE_NS   0x20018000
#define MPC_ISRAM3_RANGE_LIMIT_NS  0x2001FFFF
#define MPC_ISRAM3_RANGE_BASE_S    0x30018000
#define MPC_ISRAM3_RANGE_LIMIT_S   0x3001FFFF

/* Code SRAM memory */
#define MPC_CODE_SRAM_RANGE_BASE_NS  (0x00000000)
#define MPC_CODE_SRAM_RANGE_LIMIT_NS (0x00200000)
#define MPC_CODE_SRAM_RANGE_BASE_S   (0x10000000)
#define MPC_CODE_SRAM_RANGE_LIMIT_S  (0x10200000)

/* QSPI Flash memory */
#define MPC_QSPI_RANGE_BASE_NS        (0x00200000)
#define MPC_QSPI_RANGE_LIMIT_NS       (0x00240000)
#define MPC_QSPI_RANGE_BASE_S         (0x10200000)
#define MPC_QSPI_RANGE_LIMIT_S        (0x10240000)

#endif /* _SOC_H_ */
