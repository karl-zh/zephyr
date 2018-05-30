/*
 * Copyright (c) 2018 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <kernel.h>
#include <pinmux.h>
#include <soc.h>
#include <sys_io.h>
#include <gpio/gpio_cmsdk_ahb.h>

#include "pinmux/pinmux.h"

static void arm_musca_pinmux_defaults(void)
{

}

static int arm_musca_pinmux_init(struct device *port)
{
	ARG_UNUSED(port);

	arm_musca_pinmux_defaults();

	return 0;
}

SYS_INIT(arm_musca_pinmux_init, PRE_KERNEL_1,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
