/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <device.h>
#include <soc.h>
#include <mhu.h>

/* MHU register map structure */
struct mhu_reg_map_t {
	volatile uint32_t cpu0intr_stat; /* (R/ ) CPU 0 Interrupt Status Register */
	volatile uint32_t cpu0intr_set;  /* ( /W) CPU 0 Interrupt Set Register */
	volatile uint32_t cpu0intr_clr;  /* ( /W) CPU 0 Interrupt Clear Register */
	volatile uint32_t reserved0;
	volatile uint32_t cpu1intr_stat; /* (R/ ) CPU 1 Interrupt Status Register */
	volatile uint32_t cpu1intr_set;  /* ( /W) CPU 1 Interrupt Set Register */
	volatile uint32_t cpu1intr_clr;  /* ( /W) CPU 1 Interrupt Clear Register */
	volatile uint32_t reserved1[1004];
	volatile uint32_t pidr4;         /* ( /W) Peripheral ID 4 */
	volatile uint32_t reserved2[3];
	volatile uint32_t pidr0;         /* ( /W) Peripheral ID 0 */
	volatile uint32_t pidr1;         /* ( /W) Peripheral ID 1 */
	volatile uint32_t pidr2;         /* ( /W) Peripheral ID 2 */
	volatile uint32_t pidr3;         /* ( /W) Peripheral ID 3 */
	volatile uint32_t cidr0;         /* ( /W) Component ID 0 */
	volatile uint32_t cidr1;         /* ( /W) Component ID 1 */
	volatile uint32_t cidr2;         /* ( /W) Component ID 2 */
	volatile uint32_t cidr3;         /* ( /W) Component ID 3 */
};

#define DEV_CFG(dev) \
	((const struct mhu_device_config * const)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct mhu_data *)(dev)->driver_data)
#define MHU_REGS(dev) \
	((volatile struct mhu_reg_map_t  *)(DEV_CFG(dev))->base)

static uint32_t mhu_get_status(const struct device* mhudev,
						enum mhu_cpu_id_t cpu_id,
						uint32_t* status)
{
	struct mhu_reg_map_t* p_mhu_dev;

	if(status == NULL) {
		return MHU_ERR_INVALID_ARG;
	}

	p_mhu_dev = (struct mhu_reg_map_t*)MHU_REGS(mhudev);

	switch(cpu_id) {
	case MHU_CPU1:
		*status = p_mhu_dev->cpu1intr_stat;
		break;
	case MHU_CPU0:
	default:
		*status = p_mhu_dev->cpu0intr_stat;
		break;
	}

	return MHU_ERR_NONE;
}

static void mhu_set_val(const struct device* mhudev,
						enum mhu_cpu_id_t cpu_id,
						uint32_t set_val)
{
	struct mhu_reg_map_t* p_mhu_dev;

	p_mhu_dev = (struct mhu_reg_map_t*)MHU_REGS(mhudev);

	switch(cpu_id) {
	case MHU_CPU1:
		p_mhu_dev->cpu1intr_set = set_val;
		break;
	case MHU_CPU0:
	default:
		p_mhu_dev->cpu0intr_set = set_val;
		break;
	}
}

static void mhu_clear_val(const struct device* mhudev,
						enum mhu_cpu_id_t cpu_id,
						uint32_t clear_val)
{
	struct mhu_reg_map_t* p_mhu_dev;

	p_mhu_dev = (struct mhu_reg_map_t*)MHU_REGS(mhudev);

	switch(cpu_id) {
	case MHU_CPU1:
		p_mhu_dev->cpu1intr_clr = clear_val;
		break;
	case MHU_CPU0:
	default:
		p_mhu_dev->cpu0intr_clr = clear_val;
		break;
	}
}

static int mhu_init(struct device *mhudev)
{
	const struct mhu_device_config *config = DEV_CFG(mhudev);

	config->irq_config_func(mhudev);

	return 0;
}

void mhu_isr(void *arg)
{
	struct device *mhudev = arg;
	struct mhu_data *driver_data = DEV_DATA(mhudev);
	enum mhu_cpu_id_t cpu_id;
	uint32_t mhu_status;

	if (driver_data->get_cpu_id) {
		cpu_id = driver_data->get_cpu_id();
	} else {
		return;
	}

	mhu_get_status(mhudev, cpu_id, &mhu_status);
	mhu_clear_val(mhudev, cpu_id, mhu_status);

	if (driver_data->callback) {
		driver_data->callback(driver_data->callback_ctx, cpu_id,
						        &mhu_status);
	}
}

static void mhu_register_cb(const struct device *mhudev,
						mhu_callback_t cb,
						void *context,
						mhu_get_cpu_id_t get_cpu_id)
{
	struct mhu_data *driver_data = DEV_DATA(mhudev);

	driver_data->callback = cb;
	driver_data->callback_ctx = context;
	driver_data->get_cpu_id = get_cpu_id;
}

static const struct mhu_driver_api mhu_driver_api = {
	.mhu_register_cb = mhu_register_cb,
	.mhu_get_status = mhu_get_status,
	.mhu_set_val = mhu_set_val,
	.mhu_clear = mhu_clear_val,
};

#ifdef CONFIG_MHU0

static void mhu_irq_config_func_0(struct device *dev);

static struct mhu_device_config mhu_cfg_0 = {
	.base = (u8_t *)DT_ARM_MHU_0_BASE_ADDRESS,
	.irq_config_func = mhu_irq_config_func_0,
};

static struct mhu_data mhu_data_0 = {
	.callback = NULL,
	.callback_ctx = NULL,
	.get_cpu_id = NULL,
};

DEVICE_AND_API_INIT(mhu_0,
			"MHU_0",
			&mhu_init,
			&mhu_data_0,
			&mhu_cfg_0, PRE_KERNEL_1,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			&mhu_driver_api);

static void mhu_irq_config_func_0(struct device *dev)
{
	IRQ_CONNECT(DT_ARM_MHU_0_IRQ_0,
			DT_ARM_MHU_0_IRQ_0,
			mhu_isr,
			DEVICE_GET(mhu_0),
			0);
	irq_enable(DT_ARM_MHU_0_IRQ_0);
}

#endif /* CONFIG_MHU0 */

#ifdef CONFIG_MHU1

static void mhu_irq_config_func_1(struct device *dev);

static struct mhu_device_config mhu_cfg_1 = {
	.base = (u8_t *)DT_ARM_MHU_1_BASE_ADDRESS,
	.irq_config_func = mhu_irq_config_func_1,
};

static struct mhu_data mhu_data_1 = {
	.callback = NULL,
	.callback_ctx = NULL,
	.get_cpu_id = NULL,
};

DEVICE_AND_API_INIT(mhu_1,
			"MHU_1",
			&mhu_init,
			&mhu_data_1,
			&mhu_cfg_1, PRE_KERNEL_1,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			&mhu_driver_api);

static void mhu_irq_config_func_1(struct device *dev)
{
	IRQ_CONNECT(DT_ARM_MHU_1_IRQ_0,
			DT_ARM_MHU_1_IRQ_0_PRIORITY,
			mhu_isr,
			DEVICE_GET(mhu_1),
			0);
	irq_enable(DT_ARM_MHU_1_IRQ_0);
}

#endif /* CONFIG_MHU1 */
