/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(CONFIG_SOC_MPS2_AN521)
/* SoC level DTS fixup file */

#define DT_NUM_IRQ_PRIO_BITS	DT_ARM_V8M_NVIC_E000E100_ARM_NUM_IRQ_PRIORITY_BITS
#define DT_NUM_MPU_REGIONS	DT_ARM_ARMV8M_MPU_E000ED90_ARM_NUM_MPU_REGIONS

#if defined (CONFIG_ARM_NONSECURE_FIRMWARE)

/* CMSDK APB Timers */
#define DT_CMSDK_APB_TIMER0			DT_ARM_CMSDK_TIMER_40000000_BASE_ADDRESS
#define DT_CMSDK_APB_TIMER_0_IRQ		DT_ARM_CMSDK_TIMER_40000000_IRQ_0

#define DT_CMSDK_APB_TIMER1			DT_ARM_CMSDK_TIMER_40001000_BASE_ADDRESS
#define DT_CMSDK_APB_TIMER_1_IRQ IRQ_TIMER1	DT_ARM_CMSDK_TIMER_40001000_IRQ_0

/* CMSDK APB Dual Timer */
#define DT_CMSDK_APB_DTIMER			DT_ARM_CMSDK_DTIMER_40002000_BASE_ADDRESS
#define DT_CMSDK_APB_DUALTIMER_IRQ		DT_ARM_CMSDK_DTIMER_40002000_IRQ_0

/* CMSDK AHB General Purpose Input/Output (GPIO) */
#define DT_CMSDK_AHB_GPIO0			DT_ARM_CMSDK_GPIO_40100000_BASE_ADDRESS
#define DT_IRQ_PORT0_ALL			DT_ARM_CMSDK_GPIO_40100000_IRQ_0

#define DT_CMSDK_AHB_GPIO1			DT_ARM_CMSDK_GPIO_40101000_BASE_ADDRESS
#define DT_IRQ_PORT1_ALL			DT_ARM_CMSDK_GPIO_40101000_IRQ_0

#define DT_CMSDK_AHB_GPIO2			DT_ARM_CMSDK_GPIO_40102000_BASE_ADDRESS
#define DT_IRQ_PORT2_ALL			DT_ARM_CMSDK_GPIO_40102000_IRQ_0

#define DT_CMSDK_AHB_GPIO3			DT_ARM_CMSDK_GPIO_40103000_BASE_ADDRESS
#define DT_IRQ_PORT3_ALL			DT_ARM_CMSDK_GPIO_40103000_IRQ_0

#define DT_IPM_DEV				DT_ARM_MHU_40003000_LABEL

#else

/* CMSDK APB Timers */
#define DT_CMSDK_APB_TIMER0			DT_ARM_CMSDK_TIMER_50000000_BASE_ADDRESS
#define DT_CMSDK_APB_TIMER_0_IRQ		DT_ARM_CMSDK_TIMER_50000000_IRQ_0

#define DT_CMSDK_APB_TIMER1			DT_ARM_CMSDK_TIMER_50001000_BASE_ADDRESS
#define DT_CMSDK_APB_TIMER_1_IRQ IRQ_TIMER1	DT_ARM_CMSDK_TIMER_50001000_IRQ_0

/* CMSDK APB Dual Timer */
#define DT_CMSDK_APB_DTIMER			DT_ARM_CMSDK_DTIMER_50002000_BASE_ADDRESS
#define DT_CMSDK_APB_DUALTIMER_IRQ		DT_ARM_CMSDK_DTIMER_50002000_IRQ_0

/* CMSDK AHB General Purpose Input/Output (GPIO) */
#define DT_CMSDK_AHB_GPIO0			DT_ARM_CMSDK_GPIO_50100000_BASE_ADDRESS
#define DT_IRQ_PORT0_ALL			DT_ARM_CMSDK_GPIO_50100000_IRQ_0

#define DT_CMSDK_AHB_GPIO1			DT_ARM_CMSDK_GPIO_50101000_BASE_ADDRESS
#define DT_IRQ_PORT1_ALL			DT_ARM_CMSDK_GPIO_50101000_IRQ_0

#define DT_CMSDK_AHB_GPIO2			DT_ARM_CMSDK_GPIO_50102000_BASE_ADDRESS
#define DT_IRQ_PORT2_ALL			DT_ARM_CMSDK_GPIO_50102000_IRQ_0

#define DT_CMSDK_AHB_GPIO3			DT_ARM_CMSDK_GPIO_50103000_BASE_ADDRESS
#define DT_IRQ_PORT3_ALL			DT_ARM_CMSDK_GPIO_50103000_IRQ_0

#define DT_IPM_DEV				DT_ARM_MHU_50003000_LABEL

#endif

#endif /* CONFIG_SOC_MPS2_AN521 */

/* End of SoC Level DTS fixup file */
