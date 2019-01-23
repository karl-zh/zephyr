/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>

#include <arch/arm/cortex_m/cmsis.h>
#include <soc.h>
#include <core_cm33.h>
#include "Driver_MPC.h"
#if 0
#include "cmsis.h"
#include "target_cfg.h"
#include "platform_retarget_dev.h"
#include "region_defs.h"
#include "tfm_secure_api.h"
#endif

/* Import MPC driver */
extern ARM_DRIVER_MPC Driver_CODE_SRAM_MPC, Driver_QSPI_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM0_MPC, Driver_ISRAM1_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM2_MPC, Driver_ISRAM3_MPC;

#define NS_ROM_BASE    (0x230400)
#define NS_ROM_SIZE    (192 * 1024)
#define NS_RAM_BASE    (0x20010000)
#define NS_RAM_SIZE    (32 * 1024)

/* Define Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START (0x40000000)
#define PERIPHERALS_BASE_NS_END   (0x4FFFFFFF)

/* Allows software, via SAU, to define the code region as a NSC */
#define NSCCFG_CODENSC  1

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
    /* Enables SAU */
    TZ_SAU_Enable();

    /* Configures SAU regions to be non-secure */
    SAU->RNR  = 0;//TFM_NS_REGION_CODE;
    SAU->RBAR = NS_ROM_BASE;//(memory_regions.non_secure_partition_base
                //& SAU_RBAR_BADDR_Msk);
    SAU->RLAR = ((NS_ROM_BASE + NS_ROM_SIZE - 1)//memory_regions.non_secure_partition_limit
                & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk;

    SAU->RNR  = 1;//TFM_NS_REGION_DATA;
    SAU->RBAR = (NS_RAM_BASE/*NS_DATA_START*/ & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = ((NS_RAM_BASE + NS_RAM_SIZE -1)/*NS_DATA_LIMIT*/ & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Configures veneers region to be non-secure callable 
    SAU->RNR  = 2;//TFM_NS_REGION_VENEER;
    SAU->RBAR = (memory_regions.veneer_base  & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk
                | SAU_RLAR_NSC_Msk;*/

    /* Configure the peripherals space */
    SAU->RNR  = 3;//TFM_NS_REGION_PERIPH_1;
    SAU->RBAR = (PERIPHERALS_BASE_NS_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (PERIPHERALS_BASE_NS_END & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk;

    /* FIXME: Secondary image partition info comes from BL2. Configure SAU
     * based on those limits.
     */

    /* Allows SAU to define the code region as a NSC */
    struct spctrl_def* spctrl = CMSDK_SPCTRL;
    spctrl->nsccfg |= NSCCFG_CODENSC;
}

/*------------------- Memory configuration functions -------------------------*/

void mpc_init_cfg(void)
{
    ARM_DRIVER_MPC* mpc_data_region0 = &Driver_ISRAM0_MPC;
    ARM_DRIVER_MPC* mpc_data_region1 = &Driver_ISRAM1_MPC;
    ARM_DRIVER_MPC* mpc_data_region2 = &Driver_ISRAM2_MPC;
    ARM_DRIVER_MPC* mpc_data_region3 = &Driver_ISRAM3_MPC;

   // Driver_CODE_SRAM_MPC.Initialize();
   // Driver_CODE_SRAM_MPC.ConfigRegion(NS_ROM_BASE,//memory_regions.non_secure_partition_base,
   //                              (NS_ROM_BASE + NS_ROM_SIZE - 1),//memory_regions.non_secure_partition_limit,
   //                              ARM_MPC_ATTR_NONSECURE);

    mpc_data_region0->Initialize();
    mpc_data_region0->ConfigRegion(MPC_ISRAM0_RANGE_BASE_S,
                                   MPC_ISRAM0_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);

    mpc_data_region1->Initialize();
    mpc_data_region1->ConfigRegion(MPC_ISRAM1_RANGE_BASE_S,
                                   MPC_ISRAM1_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);

    mpc_data_region2->Initialize();

#if defined(TEST_FRAMEWORK_S) || defined(TEST_FRAMEWORK_NS)
    /* To run the regression tests on Musca A1, it is required to assign more
     * RAM memory in the secure execution environment.
     * So, the secure RAM memory size is 96KB and the non-secure one is 32 KB.
     * When it is not required to run the regression tests, the RAM memory
     * partition is the original one which is 64KB of the RAM memory for each
     * execution environment.
     */
    mpc_data_region2->ConfigRegion(MPC_ISRAM2_RANGE_BASE_S,
                                   MPC_ISRAM2_RANGE_LIMIT_S,
                                   ARM_MPC_ATTR_SECURE);
#else
    mpc_data_region2->ConfigRegion(MPC_ISRAM2_RANGE_BASE_NS,
                                   MPC_ISRAM2_RANGE_LIMIT_NS,
                                   ARM_MPC_ATTR_NONSECURE);
#endif

    mpc_data_region3->Initialize();
    mpc_data_region3->ConfigRegion(MPC_ISRAM3_RANGE_BASE_NS,
                                   MPC_ISRAM3_RANGE_LIMIT_NS,
                                   ARM_MPC_ATTR_NONSECURE);

    /* Lock down the MPC configuration */
    Driver_CODE_SRAM_MPC.LockDown();
    mpc_data_region0->LockDown();
    mpc_data_region1->LockDown();
    mpc_data_region2->LockDown();
    mpc_data_region3->LockDown();

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();
}

#if 1
/*---------------------- PPC configuration functions -------------------------*/

void ppc_init_cfg(void)
{
    struct spctrl_def* spctrl = CMSDK_SPCTRL;
    struct nspctrl_def* nspctrl = CMSDK_NSPCTRL;

    /* Grant non-secure access to peripherals in the PPC0
     * (timer0 and 1, dualtimer, watchdog, mhu 0 and 1)
     */
    spctrl->apbnsppc0 |= (1U << CMSDK_TIMER0_APB_PPC_POS);
    spctrl->apbnsppc0 |= (1U << CMSDK_TIMER1_APB_PPC_POS);
    spctrl->apbnsppc0 |= (1U << CMSDK_DTIMER_APB_PPC_POS);
    spctrl->apbnsppc0 |= (1U << CMSDK_MHU0_APB_PPC_POS);
    spctrl->apbnsppc0 |= (1U << CMSDK_MHU1_APB_PPC_POS);

    /* Grant non-secure access to S32K Timer in PPC1*/
    spctrl->apbnsppc1 |= (1U << CMSDK_S32K_TIMER_PPC_POS);

    /* Grant non-secure access for AHB peripherals on EXP0 */
    spctrl->ahbnsppcexp0 = (1U << MUSCA_PERIPHS_AHB_PPC_POS);

    /* in NS, grant un-privileged for AHB peripherals on EXP0 */
    nspctrl->ahbnspppcexp0 = (1U << MUSCA_PERIPHS_AHB_PPC_POS);

    /* Configure the response to a security violation as a
     * bus error instead of RAZ/WI
     */
    spctrl->secrespcfg |= 1U;
}
#endif
