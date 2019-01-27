/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <ipm.h>

enum cpu_id_t {
	MHU_CPU0 = 0,
	MHU_CPU1,
	MHU_CPU_MAX,
};

/* (Secure System Control) Base Address */
#define SSE_200_SYSTEM_CTRL_S_BASE	(0x50021000UL)
#define SSE_200_SYSTEM_CTRL_INITSVTOR1	(SSE_200_SYSTEM_CTRL_S_BASE + 0x114)
#define SSE_200_SYSTEM_CTRL_CPU_WAIT	(SSE_200_SYSTEM_CTRL_S_BASE + 0x118)
#define SSE_200_CPU_ID_UNIT_BASE	(0x5001F000UL)

#define NON_SECURE_FLASH_ADDRESS	(192 * 1024)
#define NON_SECURE_IMAGE_HEADER		(0x400)
#define NON_SECURE_FLASH_OFFSET		(0x10000000)

struct device *mhu0;

static void wakeup_cpu1(void)
{
	/* Set the Initial Secure Reset Vector Register for CPU 1 */
	*(u32_t *)(SSE_200_SYSTEM_CTRL_INITSVTOR1) =
					CONFIG_FLASH_BASE_ADDRESS +
					NON_SECURE_FLASH_ADDRESS +
					NON_SECURE_IMAGE_HEADER -
					NON_SECURE_FLASH_OFFSET;

	/* Set the CPU Boot wait control after reset */
	*(u32_t *)(SSE_200_SYSTEM_CTRL_CPU_WAIT) = 0;
}

static u32_t sse_200_platform_get_cpu_id(void)
{
	volatile u32_t *p_cpu_id = (volatile u32_t *)SSE_200_CPU_ID_UNIT_BASE;

	return (u32_t)*p_cpu_id;
}

static int main_cpu_0(void)
{
	while (1) {
	}
}

static int main_cpu_1(void)
{
	const u32_t set_mhu = 1;

	ipm_send(mhu0, 0, MHU_CPU0, &set_mhu, 1);

	while (1) {
	}
}

static void mhu_isr_callback(void *context, u32_t cpu_id,
					volatile void *data)
{
	const u32_t set_mhu = 1;

	printk("MHU ISR on CPU %d\n", cpu_id);
	if (cpu_id == MHU_CPU0) {
		ipm_send(mhu0, 0, MHU_CPU1, &set_mhu, 1);
	} else if (cpu_id == MHU_CPU1) {
		printk("MHU Test Done.\n");
	}
}

void main(void)
{
	printk("IPM MHU sample on %s\n", CONFIG_BOARD);

	mhu0 = device_get_binding(DT_ARM_MHU_0_LABEL);
	if (!mhu0) {
		printk("CPU %d, get MHU0 fail!\n",
				sse_200_platform_get_cpu_id());
		while (1) {
		}
	} else {
		printk("CPU %d, get MHU0 success!\n",
				sse_200_platform_get_cpu_id());
		ipm_register_callback(mhu0, mhu_isr_callback, mhu0);
	}

	if (sse_200_platform_get_cpu_id() == MHU_CPU0) {
		wakeup_cpu1();
		main_cpu_0();
	}

	main_cpu_1();
}
