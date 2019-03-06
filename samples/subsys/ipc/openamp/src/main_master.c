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
#include "erpc_matrix_multiply.h"

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

static struct rcv_buff rpmsg_recv = {0};

static void rpmsg_recv_callback(struct rpmsg_channel *channel, void *data,
				int data_length, void *private,
				unsigned long src)
{
	if (data_length < RX_BUFF_SIZE) {
		memset(rpmsg_recv.rcv, 0x00, RX_BUFF_SIZE);
		memcpy(rpmsg_recv.rcv, (unsigned char *)data, data_length);
		printk("Mrcv %d.\n", data_length);
	} else {
		printk("M %s received %d bytes.\n", __func__, data_length);
	}

//	received_data = *((unsigned int *) data);
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
//	return rpmsg_send(rp_channel, &message, sizeof(message));
}

/*!
 * @brief erpcMatrixMultiply function implementation.
 *
 * This is the implementation of the erpcMatrixMultiply function called by the primary core.
 *
 * @param matrix1 First matrix
 * @param matrix2 Second matrix
 * @param result_matrix Result matrix
 */
void erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    int32_t i, j, k;
    const int32_t matrix_size = 5;
//	matrix1 = (char*)matrix1 - 0x390 + 0x10000;
//	matrix2 = (char*)matrix2 - 0x390 + 0x10000;
//	result_matrix = (char*)result_matrix - 0x390 + 0x10000;

    printk("Calculating the matrix multiplication... %x %x %x\r\n", matrix1, matrix2, result_matrix);

    /* Clear the result matrix */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            result_matrix[i][j] = 0;
        }
    }

    /* Multiply two matrices */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            for (k = 0; k < matrix_size; ++k)
            {
                result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    printk("Done! %d\r\n", result_matrix[1][1]);
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

void __assert_func (const char *file, int line, const char *func, const char *e)
{
//	printk("%s,%s,%s\n")
}

int serial_write(int fd, char *buf, int size)
{
	printk("Mwrite %x, %d\n", buf, size);
//	return 0;
	return rpmsg_send(rp_channel, buf, size);
}

int serial_read(int fd, char *buf, int size)
{
	while (k_sem_take(&message_received, K_NO_WAIT) != 0)
		hil_poll(proc, 0);

	printk("Mread %x, %d\n",buf, size);
	memcpy(buf, rpmsg_recv.rcv, size);
	return size;
}

int serial_open(const char *port)
{
	printk("serial open %s\n", port);
	return 0;
}

int serial_close(int fd)
{
	return 0;
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

	printk("OpenAMP created.\n");
	while (1) {
		k_sleep(500);
	}
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
	printk("Starting application thread!\n");
	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
//	erpc_transport_t

	 void * transport = erpc_transport_serial_init("zass", 115200);
//	erpc_mbf_t
	 void * message_buffer_factory = erpc_mbf_static_init();

	erpc_server_init(transport ,message_buffer_factory);

	/* adding the service to the server */
	erpc_add_service_to_server(create_MatrixMultiplyService_service());

	printk("MatrixMultiply service added\r\n");

	while (1) {
		/* process message */
		erpc_server_poll();
	}
}
