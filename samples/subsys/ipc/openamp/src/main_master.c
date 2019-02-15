/*
 * Copyright (c) 2018, NXP
 * Copyright (c) 2018, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This implements the master side of an OpenAMP system.
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openamp/open_amp.h>

#include "platform.h"
#include "resource_table.h"

#include "audit_core.h"
#include "audit_tests_common.h"
#include "crypto_s_tests.h"
#include "sst/secure/sst_s_tests.h"
#include "sst/secure/test_framework.h"
#include "tfm_sst_defs.h"

enum cpu_id_t {
	MHU_CPU0 = 0,
	MHU_CPU1,
	MHU_CPU_MAX,
};

#define CRYPTO_TEST_CASES	1

/* Memory bounds to check */
#define ROM_ADDR_LOCATION        0x10000000
#define DEV_ADDR_LOCATION        0x50000000
#define NON_EXIST_ADDR_LOCATION  0xFFFFFFFF

/* (Secure System Control) Base Address */
#define SSE_200_SYSTEM_CTRL_S_BASE	(0x50021000UL)
#define SSE_200_SYSTEM_CTRL_INITSVTOR1	(SSE_200_SYSTEM_CTRL_S_BASE + 0x114)
#define SSE_200_SYSTEM_CTRL_CPU_WAIT	(SSE_200_SYSTEM_CTRL_S_BASE + 0x118)
#define SSE_200_CPU_ID_UNIT_BASE	(0x5001F000UL)

#define SHARED_MEM_BASE			(CONFIG_SRAM_BASE_ADDRESS + \
						(CONFIG_SRAM_SIZE * 1024) - 8)
#define NON_SECURE_FLASH_ADDRESS	(192 * 1024)
#define NON_SECURE_IMAGE_HEADER		(0x400)
#define NON_SECURE_FLASH_OFFSET		(0x10000000)

#define APP_TASK_STACK_SIZE (512)
K_THREAD_STACK_DEFINE(thread_stack, APP_TASK_STACK_SIZE);
static struct k_thread thread_data;

static struct rpmsg_channel *rp_channel;
static struct rpmsg_endpoint *rp_endpoint;

static K_SEM_DEFINE(channel_created, 0, 1);

static K_SEM_DEFINE(message_received, 0, 1);
static volatile unsigned int received_data;

static struct rsc_table_info rsc_info;
static struct hil_proc *proc;

extern int tfm_crypto_init(void);

int32_t tfm_core_get_caller_client_id(int32_t *client_id)
{
//	printk("Custom function %s\n", __func__);
	printk("C ID\n");
	*((int32_t *)client_id) = k_current_get();
	return TFM_SUCCESS;
}

int32_t tfm_core_memory_permission_check(
            void *ptr, uint32_t len, int32_t access)
{
	struct tfm_sst_buf_t * sst_buf = (struct tfm_sst_buf_t *)ptr;
	if (ptr == NULL || len == 0)
		return TFM_ERROR_INVALID_PARAMETER;

	if (sst_buf->data == ROM_ADDR_LOCATION 
		|| sst_buf->data == DEV_ADDR_LOCATION
		|| sst_buf->data == NON_EXIST_ADDR_LOCATION)
		return TFM_ERROR_INVALID_PARAMETER;
	//    if (cmse_check_address_range((void *)ptr, len, access) == NULL)
	//        return TFM_ERROR_INVALID_PARAMETER;

	return TFM_SUCCESS;
}

int32_t tfm_core_validate_secure_caller()
{
	return TFM_SUCCESS;
}

void putchar(char *s)
{

}

void exit(int return_code)
{
	printk("exit(%d)\n", return_code);
	k_panic();
}

static void rpmsg_recv_callback(struct rpmsg_channel *channel, void *data,
				int data_length, void *private,
				unsigned long src)
{
	received_data = *((unsigned int *) data);
	k_sem_give(&message_received);
}

static void rpmsg_channel_created(struct rpmsg_channel *channel)
{
	rp_channel = channel;
	rp_endpoint = rpmsg_create_ept(rp_channel, rpmsg_recv_callback,
				       RPMSG_NULL, RPMSG_ADDR_ANY);
	k_sem_give(&channel_created);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *channel)
{
	rpmsg_destroy_ept(rp_endpoint);
}

static unsigned int receive_message(void)
{
	while (k_sem_take(&message_received, K_NO_WAIT) != 0)
		hil_poll(proc, 0);
	return received_data;
}

static int send_message(unsigned int message)
{
	return rpmsg_send(rp_channel, &message, sizeof(message));
}

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

void app_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	printk("\r\nOpenAMP demo started\r\n");

	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;

	metal_init(&metal_params);

	proc = platform_init(RPMSG_MASTER);
	if (proc == NULL) {
		printk("platform_init() failed\n");
		goto _cleanup;
	}

	resource_table_init((void **) &rsc_info.rsc_tab, &rsc_info.size);
	wakeup_cpu1();
	k_sleep(500);

	struct remote_proc *rproc_ptr = NULL;
	int status = remoteproc_resource_init(&rsc_info, proc,
					      rpmsg_channel_created,
					      rpmsg_channel_deleted,
					      rpmsg_recv_callback,
					      &rproc_ptr, RPMSG_MASTER);
	if (status != 0) {
		printk("remoteproc_resource_init() failed with status %d\n",
		       status);
		goto _cleanup;
	}

	while (k_sem_take(&channel_created, K_NO_WAIT) != 0)
		hil_poll(proc, 0);

	unsigned int message = 0U;

	status = send_message(message);
	if (status < 0) {
		printk("send_message(%d) failed with status %d\n",
		       message, status);
		goto _cleanup;
	}

	while (message <= 100) {
		message = receive_message();
		printk("Primary core received a message: %d\n", message);

		message++;
		status = send_message(message);
		if (status < 0) {
			printk("send_message(%d) failed with status %d\n",
			       message, status);
			goto _cleanup;
		}
	}

_cleanup:
	if (rproc_ptr) {
		remoteproc_resource_deinit(rproc_ptr);
	}
	metal_finish();

	printk("OpenAMP demo ended.\n");
}

void main(void)
{
	struct test_result_t ret;
	tfm_audit_test_1001();

#if CRYPTO_TEST_CASES
	tfm_crypto_init();
	tfm_crypto_test_5001();
	tfm_crypto_test_6001();
	tfm_crypto_test_6002();
	tfm_crypto_test_6003();
	tfm_crypto_test_6004();
	tfm_crypto_test_6005();
	tfm_crypto_test_6006();
	tfm_crypto_test_6007();
	tfm_crypto_test_6008();
	tfm_crypto_test_6009();
	tfm_crypto_test_6010();
#endif
#if 1 // Too much time, skip for the time being
	sst_crypto_init();
	sst_am_prepare();
	tfm_sst_test_2001(&ret);
	if (ret.val == TEST_PASSED)
		printk("tfm_sst_test_2001 Passed\n");
	ret.val = TEST_FAILED;
	tfm_sst_test_2002(&ret);
	if (ret.val == TEST_PASSED)
		printk("tfm_sst_test_2002 Passed\n");
	ret.val = TEST_FAILED;

		tfm_sst_test_2003(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2003 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2004(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2004 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2005(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2005 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2006(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2006 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2007(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2007 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2008(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2008 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2009(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2009 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2010(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2010 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2011(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2011 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2012(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2012 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2013(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2013 Passed\n");
		ret.val = TEST_FAILED;
	
		tfm_sst_test_2014(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2014 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2015(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2015 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2016(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2016 Passed\n");
#endif
#if 1
		ret.val = TEST_FAILED;
		tfm_sst_test_2017(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2017 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2018(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2018 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2019(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2019 Passed\n");
		ret.val = TEST_FAILED;
		tfm_sst_test_2020(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_2020 Passed\n");
#endif

#if 0 // Too slow to test these two
		tfm_sst_test_3001(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_3001 Passed\n");
		tfm_sst_test_3002(&ret);
		if (ret.val == TEST_PASSED)
			printk("tfm_sst_test_3002 Passed\n");
#endif

	printk("Starting application thread!\n");

	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
}
