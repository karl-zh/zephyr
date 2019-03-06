/*
 * Copyright (c) 2018, NXP
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This implements the remote side of an OpenAMP system.
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

#define APP_TASK_STACK_SIZE (512)
K_THREAD_STACK_DEFINE(thread_stack, APP_TASK_STACK_SIZE);
static struct k_thread thread_data;

static K_SEM_DEFINE(channel_created, 0, 1);

static K_SEM_DEFINE(message_received, 0, 1);
static volatile unsigned int received_data;

static struct rsc_table_info rsc_info;
static struct hil_proc *proc;

static struct rpmsg_channel *rp_channel;
static struct rpmsg_endpoint *rp_endpoint;

#define NO_RCV_MSGQ 0

#if RCV_MSGQ
struct k_msgq rcv_msgq;
static char __aligned(4) tbuffer[MSG_SIZE * MSGQ_LEN];

static void rpmsg_recv_callback(struct rpmsg_channel *channel, void *data,
				int data_length, void *private,
				unsigned long src)
{
	int ret, rx_data;
	printk("Rrcv %d.\n", data_length);
	for (int i  = 0; i < data_length; i++) {
		rx_data = *((char *)data + i);
//		printk("%x ", rx_data);
		ret = k_msgq_put(&rcv_msgq, &rx_data, K_NO_WAIT);
		if (ret != 0)
			printk(" %s error.\n", __func__);
//		zassert_equal(ret, 0, NULL);
	}

//	received_data = *((unsigned int *) data);
	k_sem_give(&message_received);
	printk("RrcvD %d.\n", data_length);
}
#else
static struct rcv_buff rpmsg_recv = {{0}, 0};

static void rpmsg_recv_callback(struct rpmsg_channel *channel, void *data,
				int data_length, void *private,
				unsigned long src)
{
	if (data_length < RX_BUFF_SIZE) {
		memset(rpmsg_recv.rcv, 0x00, RX_BUFF_SIZE);
		memcpy(rpmsg_recv.rcv, (unsigned char *)data, data_length);
		printk("Rrcv %d.\n", data_length);
	} else {
		printk("R %s received %d bytes.\n", __func__, data_length);
	}

//	received_data = *((unsigned int *) data);
	k_sem_give(&message_received);
}

#endif
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

void __assert_func (const char *file, int line, const char *func, const char *e)
{
//	printk("%s,%s,%s\n")
}

int serial_write(int fd, char *buf, int size)
{
	printk("Rwrite %x, %d\n", buf, size);
//	return 0;
	return rpmsg_send(rp_channel, buf, size);
}

#if RCV_MSGQ
int serial_read(int fd, char *buf, int size)
{
//	printk("serial read %d", size);
//	return 0;
	int ret, rx_data;
	while (k_sem_take(&message_received, K_NO_WAIT) != 0)
		hil_poll(proc, 0);


	for (int i = 0; i < size; i++) {
		ret = k_msgq_get(&rcv_msgq, &rx_data, K_NO_WAIT);
		buf[i] = (rx_data & 0xFF);
		printk("%d ", buf[i]);
//		zassert_equal(ret, 0, NULL);
	}

	printk("RreadD %d\n", size);
	return size;
}
#else
int serial_read(int fd, char *buf, int size)
{
	while (k_sem_take(&message_received, K_NO_WAIT) != 0)
		hil_poll(proc, 0);

	printk("Rread %x, %d\n",buf, size);
	memcpy(buf, rpmsg_recv.rcv, size);
	return size;
}
#endif
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

	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;

	metal_init(&metal_params);

	proc = platform_init(RPMSG_REMOTE);
	if (proc == NULL) {
		goto _cleanup;
	}

	resource_table_init((void **) &rsc_info.rsc_tab, &rsc_info.size);

	struct remote_proc *rproc_ptr = NULL;
	int status = remoteproc_resource_init(&rsc_info, proc,
					      rpmsg_channel_created,
					      rpmsg_channel_deleted,
					      rpmsg_recv_callback,
					      &rproc_ptr, RPMSG_REMOTE);
	if (status != 0) {
		goto _cleanup;
	}

	while (k_sem_take(&channel_created, K_NO_WAIT) != 0)
		hil_poll(proc, 0);

	while (1) {
		k_sleep(500);
	}

	unsigned int message = 0U;

	while (message <= 100) {
		message = receive_message();
		message++;
		status = send_message(message);
		if (status <= 0) {
			goto _cleanup;
		}
	}

_cleanup:
	if (rproc_ptr) {
		remoteproc_resource_deinit(rproc_ptr);
	}
}
typedef struct ErpcMessageBufferFactory *erpc_mbf_t;
typedef struct ErpcTransport *erpc_transport_t;

void main(void)
{
	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);

#if RCV_MSGQ
	k_msgq_init(&rcv_msgq, tbuffer, MSG_SIZE, MSGQ_LEN);
#endif

	 //  erpc_transport_t
	 erpc_transport_t transport = erpc_transport_serial_init("zssR", 115200);
//	erpc_mbf_t
	 erpc_mbf_t message_buffer_factory = erpc_mbf_static_init();

	erpc_client_init(transport ,message_buffer_factory);

	Matrix matrix1 = {3};
	Matrix matrix2 = {5};
	Matrix matrixR = {0};
	int matrix_size = 5, i, j;

    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            matrix1[i][j] = 3;
            matrix2[i][j] = 5;
        }
    }

	erpcMatrixMultiply(matrix1, matrix2, matrixR);
	printk("R %d\n", matrixR[2][2]);
	erpcMatrixMultiply(matrix1, matrix2, matrixR);
	printk("RB %d\n", matrixR[2][2]);
}
