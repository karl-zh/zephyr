/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <mhu.h>

#if 0
/* System control memory mapped register access structure */
struct sysctrl_t {
    volatile uint32_t secdbgstat;             /* (R/ ) Secure Debug Configuration
                                               *       Status Register*/
    volatile uint32_t secdbgset;              /* ( /W) Secure Debug Configuration
                                               *       Set Register */
    volatile uint32_t secdbgclr;              /* ( /W) Secure Debug Configuration
                                               *       Clear Register */
    volatile uint32_t scsecctrl;              /* (R/W) System Control Security
                                               *       Control Register */
    volatile uint32_t fclk_div;               /* (R/W) Fast Clock Divider
                                               *       Configuration Register */
    volatile uint32_t sysclk_div;             /* (R/W) System Clock Divider
                                               *       Configuration Register */
    volatile uint32_t clockforce;             /* (R/W) Clock Forces */
    volatile uint32_t reserved0[57];
    volatile uint32_t resetsyndrome;          /* (R/W) Reset syndrome */
    volatile uint32_t resetmask;              /* (R/W) Reset MASK */
    volatile uint32_t swreset;                /* ( /W) Software Reset */
    volatile uint32_t gretreg;                /* (R/W) General Purpose Retention
                                               *       Register */
    volatile uint32_t initsvtor0;             /* (R/W) Initial Secure Reset Vector
                                               *       Register For CPU 0 */
    volatile uint32_t initsvtor1;             /* (R/W) Initial Secure Reset
                                               *       Vector Register For CPU 1*/
    volatile uint32_t cpuwait;                /* (R/W) CPU Boot wait control
                                               *       after reset */
    volatile uint32_t reserved1;
    volatile uint32_t wicctrl;                /* (R/W) CPU WIC Request and
                                               *       Acknowledgement */
    volatile uint32_t ewctrl;                 /* (R/W) External Wakeup Control */
    volatile uint32_t reserved2[54];
    volatile uint32_t pdcm_pd_sys_sense;      /* (R/W) Power Control Dependency
                                               *       Matrix PD_SYS
                                               *       Power Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_cpu0core_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_CPU0CORE
                                               *       Power Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_cpu1core_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_CPU1CORE
                                               *       Power Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_sram0_sense;    /* (R/W) Power Control Dependency
                                               *       Matrix PD_SRAM0 Power
                                               *       Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_sram1_sense;    /* (R/W) Power Control Dependency
                                               *       Matrix PD_SRAM1 Power
                                               *       Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_sram2_sense;    /* (R/W) Power Control Dependency
                                               *       Matrix PD_SRAM2 Power
                                               *       Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_sram3_sense;    /* (R/W) Power Control Dependency
                                               *       Matrix PD_SRAM3 Power
                                               *       Domain Sensitivity.*/
    volatile uint32_t reserved3[5];
    volatile uint32_t pdcm_pd_cc_sense;       /* (R/W) Power Control Dependency
                                               *       Matrix PD_CC
                                               *       Power Domain Sensitivity.*/
    volatile uint32_t pdcm_pd_exp0_out_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_EXP0 Sensitivity. */
    volatile uint32_t pdcm_pd_exp1_out_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_EXP1 Sensitivity. */
    volatile uint32_t pdcm_pd_exp2_out_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_EXP2 Sensitivity. */
    volatile uint32_t pdcm_pd_exp3_out_sense; /* (R/W) Power Control Dependency
                                               *       Matrix PD_EXP3 Sensitivity. */
    volatile uint32_t reserved4[864];
    volatile uint32_t pidr4;                  /* (R/ ) Peripheral ID 4 */
    volatile uint32_t reserved5[3];
    volatile uint32_t pidr0;                  /* (R/ ) Peripheral ID 0 */
    volatile uint32_t pidr1;                  /* (R/ ) Peripheral ID 1 */
    volatile uint32_t pidr2;                  /* (R/ ) Peripheral ID 2 */
    volatile uint32_t pidr3;                  /* (R/ ) Peripheral ID 3 */
    volatile uint32_t cidr0;                  /* (R/ ) Component ID 0 */
    volatile uint32_t cidr1;                  /* (R/ ) Component ID 1 */
    volatile uint32_t cidr2;                  /* (R/ ) Component ID 2 */
    volatile uint32_t cidr3;                  /* (R/ ) Component ID 3 */
};
#endif

/* (Secure System Control) Base Address */
#define MUSCA_SYSTEM_CTRL_S_BASE	(0x50021000UL)
#define MUSCA_CPU_ID_UNIT_BASE		(0x5001F000UL)

/* Secure System Control (SYSCTRL) Alias */
#define CMSDK_SYSCTRL_BASE_S		MUSCA_SYSTEM_CTRL_S_BASE

#define SHARED_MEM_BASE				(CONFIG_SRAM_BASE_ADDRESS + \
										(CONFIG_SRAM_SIZE * 1024) - 8)
#define NON_SECURE_FLASH_ADDRESS	(192 * 1024)
#define NON_SECURE_IMAGE_HEADER		(0x400)

struct device *mhu0;
volatile uint32_t *cnt = (volatile uint32_t*)SHARED_MEM_BASE;
uint32_t test_mhu = 0;

static void wakeup_cpu1(void)
{
	struct sysctrl_t *ptr = (struct sysctrl_t *)CMSDK_SYSCTRL_BASE_S;
	ptr->initsvtor1 = 0x10230400;//CONFIG_FLASH_BASE_ADDRESS + NON_SECURE_FLASH_ADDRESS +
						//NON_SECURE_IMAGE_HEADER;
	ptr->cpuwait = 0;
}

static enum mhu_cpu_id_t musca_platform_get_cpu_id(void)
{
	volatile uint32_t* p_cpu_id = (volatile uint32_t*)MUSCA_CPU_ID_UNIT_BASE;

	return (enum mhu_cpu_id_t)*p_cpu_id;
}

static void init_isolation_hw(void)
{
    /* Configures non-secure memory spaces in the target */
    sau_and_idau_cfg();
    mpc_init_cfg();
    ppc_init_cfg();
}

/* Prioritise secure exceptions to avoid NS being able to pre-empt secure
 * SVC or SecureFault
 */
static void set_secure_exception_priorities(void)
{
    uint32_t VECTKEY;
    SCB_Type *scb = SCB;
    uint32_t AIRCR;

    /* Set PRIS flag is AIRCR */
    AIRCR = scb->AIRCR;
    VECTKEY = (~AIRCR & SCB_AIRCR_VECTKEYSTAT_Msk);
    scb->AIRCR = SCB_AIRCR_PRIS_Msk |
                 VECTKEY |
                 (AIRCR & ~SCB_AIRCR_VECTKEY_Msk);
}

static int main_cpu_0(void)
{
    (*cnt) = 0;
    init_isolation_hw();

    while(1)
    {
        *cnt = (*cnt) + 1;
        if((*cnt & 0xFFFF) == 0) {
            printk("MHU set cpu 1.\n");
            mhu_set_value(mhu0, MHU_CPU1, 1);
        }
        if (test_mhu > 20) {
            printk("MHU Test Done.\n");
            while(1){}
        }
        if(*cnt == 15) {
            wakeup_cpu1();
        }
    }
}

static void mhu_isr_callback(void *context, enum mhu_cpu_id_t cpu_id,
					volatile void *data)
{
	printk("MHU ISR on CPU %d\n", cpu_id);
    test_mhu++;
}

void main(void)
{
    printk("Hello From %s CPU %d, cnt add 0x%x,%x\n", CONFIG_BOARD,
        musca_platform_get_cpu_id(), cnt, *cnt);

    mhu0 = device_get_binding("MHU_0");
    if (!mhu0) {
        printk("CPU %d, get MHU0 fail!\n", musca_platform_get_cpu_id());
    } else {
        printk("CPU %d, get MHU0 success!\n", musca_platform_get_cpu_id());
        mhu_register_callback(mhu0, mhu_isr_callback,
                mhu0, musca_platform_get_cpu_id);
    }

    if (musca_platform_get_cpu_id() == MHU_CPU0) {
        main_cpu_0();
    }

    while(1)
    {
        if((*cnt & 0xFFFF) != 0 && (*cnt & 0x1FFF) == 0) {
            mhu_set_value(mhu0, MHU_CPU0, 1);
            }
    }

}

