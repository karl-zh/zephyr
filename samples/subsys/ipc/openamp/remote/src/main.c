/*
 * Copyright (c) 2018, NXP
 * Copyright (c) 2018, Nordic Semiconductor ASA
 * Copyright (c) 2018, Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <ipm.h>
#include <misc/printk.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openamp/open_amp.h>
#include <metal/device.h>

#include "common.h"
#include "erpc_matrix_multiply.h"
#include "erpc_client_setup.h"

#define ERPC_CLIENT_SUPPORT 0

#define APP_TASK_STACK_SIZE (1024)
K_THREAD_STACK_DEFINE(thread_stack, APP_TASK_STACK_SIZE);
static struct k_thread thread_data;

static struct device *ipm_handle;

static metal_phys_addr_t shm_physmap[] = { SHM_START_ADDR };
static struct metal_device shm_device = {
	.name = SHM_DEVICE_NAME,
	.bus = NULL,
	.num_regions = 1,
	{
		{
			.virt       = (void *) SHM_START_ADDR,
			.physmap    = shm_physmap,
			.size       = SHM_SIZE,
			.page_shift = 0xffffffff,
			.page_mask  = 0xffffffff,
			.mem_flags  = 0,
			.ops        = { NULL },
		},
	},
	.node = { NULL },
	.irq_num = 0,
	.irq_info = NULL
};

static volatile unsigned int received_data;

static struct virtio_vring_info rvrings[2] = {
	[0] = {
		.info.align = VRING_ALIGNMENT,
	},
	[1] = {
		.info.align = VRING_ALIGNMENT,
	},
};
static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
static struct virtqueue *vq[2];

static unsigned char virtio_get_status(struct virtio_device *vdev)
{
	return sys_read8(VDEV_STATUS_ADDR);
}

static u32_t virtio_get_features(struct virtio_device *vdev)
{
	return 1 << VIRTIO_RPMSG_F_NS;
}

static void virtio_notify(struct virtqueue *vq)
{
	u32_t dummy_data = 0x00110011; /* Some data must be provided */
	u32_t current_core = *(volatile u32_t *)0x5001F000;

//	ipm_send(ipm_handle, 0, 0, &dummy_data, sizeof(dummy_data));
	ipm_send(ipm_handle, 0, current_core ? 0 : 1, &dummy_data, 1);
}

struct virtio_dispatch dispatch = {
	.get_status = virtio_get_status,
	.get_features = virtio_get_features,
	.notify = virtio_notify,
};

static K_SEM_DEFINE(data_sem, 0, 1);
static K_SEM_DEFINE(data_rx_sem, 0, 1);

#if 1
#define RX_BUFF_SIZE            256

#define MSG_SIZE                4
#define MSGQ_LEN                512

struct rcv_buff {
    unsigned char rcv[RX_BUFF_SIZE];
    unsigned int len;
    unsigned int start;
};

struct k_msgq rcv_msgq;
static char __aligned(4) tbuffer[MSG_SIZE * MSGQ_LEN];
#endif

static void platform_ipm_callback(void *context, u32_t id, volatile void *data)
{
	printk("Rmhu\n");
	k_sem_give(&data_sem);
}

int endpoint_cb(struct rpmsg_endpoint *ept, void *data,
		size_t len, u32_t src, void *priv)
{

	int ret, rx_data;
	printk("RedLen %d.\n", len);
	received_data = *((unsigned int *) data);

#if ERPC_CLIENT_SUPPORT
	for (int i  = 0; i < len; i++) {
		rx_data = *((char *)data + i);
		             printk("Red %x.", rx_data);
		ret = k_msgq_put(&rcv_msgq, (void *)&rx_data, K_NO_WAIT);
		if (ret != 0)
			printk(" %s error %d.\n", __func__, ret);
		//             zassert_equal(ret, 0, NULL);
	}
#endif
	k_sem_give(&data_rx_sem);

	return RPMSG_SUCCESS;
}

static K_SEM_DEFINE(ept_sem, 0, 1);

struct rpmsg_endpoint my_ept;
struct rpmsg_endpoint *ep = &my_ept;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(ep);
}

static unsigned int receive_message(void)
{
	int ret, rx_data;

	while (k_sem_take(&data_rx_sem, K_NO_WAIT) != 0) {
		int status = k_sem_take(&data_sem, K_FOREVER);

		if (status == 0) {
			virtqueue_notification(vq[1]);
		}
	}

#if ERPC_CLIENT_SUPPORT
	for (int i = 0; i < 4; i++) {
		ret = k_msgq_get(&rcv_msgq, (void *)&rx_data, K_NO_WAIT);
	}
#endif
	return received_data;
}

#if ERPC_CLIENT_SUPPORT
void __assert_func (const char *file, int line, const char *func, const char *e)
{
       printk("%s,%d,%s,%s\n", file, line, func, e);
}

void print_log(const char *fmt)
{
       printk("%s", fmt);
}
#endif

#if ERPC_CLIENT_SUPPORT
int rpmsg_openamp_send(struct rpmsg_endpoint *ept, const void *data,
			     int len)
{
	if (ept->dest_addr == RPMSG_ADDR_ANY)
		return RPMSG_ERR_ADDR;
	return rpmsg_send_offchannel_raw(ept, ept->addr, ept->dest_addr, data,
					 len, true);
}

int rpmsg_openamp_read(struct rpmsg_endpoint *ept, char *data,
			     int len)
{
	int ret, rx_data, status;
	char *buf = data;

	while (k_sem_take(&data_rx_sem, K_NO_WAIT) != 0) {
		printk("RSR S2 %d!\r\n", len);
		if (len > k_msgq_num_used_get(&rcv_msgq))
			status = k_sem_take(&data_sem, K_FOREVER);
		else
			status = 0;

		if (status == 0) {
			virtqueue_notification(vq[1]);
			break;
		}
	}

	printk("RSR size %d!\r\n", len);
	while (len > k_msgq_num_used_get(&rcv_msgq)) {
		k_sleep(5);
	}

	for (int i = 0; i < len; i++) {
	       ret = k_msgq_get(&rcv_msgq, (void *)&rx_data, K_NO_WAIT);
	       buf[i] = (rx_data & 0xFF);
	             printk("RSR %d.", buf[i]);
	//             zassert_equal(ret, 0, NULL);
	}
	printk("RSR out!\r\n");
	return len;
}
#endif

static int send_message(unsigned int message)
{
#if ERPC_CLIENT_SUPPORT
	return rpmsg_openamp_send(ep, &message, sizeof(message));
#else
	return rpmsg_send(ep, &message, sizeof(message));
#endif
}

#if ERPC_CLIENT_SUPPORT
typedef struct ErpcMessageBufferFactory *erpc_mbf_t;
typedef struct ErpcTransport *erpc_transport_t;
#endif

void app_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	int status = 0;
	unsigned int message = 0U;
	struct metal_device *device;
	struct rpmsg_device *rdev;
	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;

	printk("\r\nOpenAMP[remote] demo started\r\n");

	status = metal_init(&metal_params);
	if (status != 0) {
		printk("metal_init: failed - error code %d\n", status);
		return;
	}

	status = metal_register_generic_device(&shm_device);
	if (status != 0) {
		printk("Couldn't register shared memory device: %d\n", status);
		return;
	}

	status = metal_device_open("generic", SHM_DEVICE_NAME, &device);
	if (status != 0) {
		printk("metal_device_open failed: %d\n", status);
		return;
	}

	io = metal_device_io_region(device, 0);
	if (io == NULL) {
		printk("metal_device_io_region failed to get region\n");
		return;
	}

	/* setup IPM */
	ipm_handle = device_get_binding(DT_ARM_MHU_0_LABEL);
	if (ipm_handle == NULL) {
		printk("device_get_binding failed to find device\n");
		return;
	}

	ipm_register_callback(ipm_handle, platform_ipm_callback, NULL);

	status = ipm_set_enabled(ipm_handle, 1);
	if (status != 0) {
		printk("ipm_set_enabled failed\n");
		return;
	}
	printk("ipm_set_enabled Done\n");

#if ERPC_CLIENT_SUPPORT
	k_msgq_init(&rcv_msgq, tbuffer, MSG_SIZE, MSGQ_LEN);
#endif
	/* setup vdev */
	vq[0] = virtqueue_allocate(VRING_SIZE);
	if (vq[0] == NULL) {
		printk("virtqueue_allocate failed to alloc vq[0]\n");
		return;
	}
	vq[1] = virtqueue_allocate(VRING_SIZE);
	if (vq[1] == NULL) {
		printk("virtqueue_allocate failed to alloc vq[1]\n");
		return;
	}

	vdev.role = RPMSG_REMOTE;
	vdev.vrings_num = VRING_COUNT;
	vdev.func = &dispatch;
	rvrings[0].io = io;
	rvrings[0].info.vaddr = (void *)VRING_TX_ADDRESS;
	rvrings[0].info.num_descs = VRING_SIZE;
	rvrings[0].info.align = VRING_ALIGNMENT;
	rvrings[0].vq = vq[0];

	rvrings[1].io = io;
	rvrings[1].info.vaddr = (void *)VRING_RX_ADDRESS;
	rvrings[1].info.num_descs = VRING_SIZE;
	rvrings[1].info.align = VRING_ALIGNMENT;
	rvrings[1].vq = vq[1];

	vdev.vrings_info = &rvrings[0];

	/* setup rvdev */
	status = rpmsg_init_vdev(&rvdev, &vdev, NULL, io, NULL);
	if (status != 0) {
		printk("rpmsg_init_vdev failed %d\n", status);
		return;
	}

	rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);

	status = rpmsg_create_ept(ep, rdev, "k", RPMSG_ADDR_ANY,
			RPMSG_ADDR_ANY, endpoint_cb, rpmsg_service_unbind);
	if (status != 0) {
		printk("rpmsg_create_ept failed %d\n", status);
		return;
	}

#if 1
	while (message < 99) {
		message = receive_message();
		printk("Remote core received a message: %d\n", message);

		message++;
		status = send_message(message);
		if (status < 0) {
			printk("send_message(%d) failed with status %d\n",
			       message, status);
			goto _cleanup;
		}
	}
#endif

#if ERPC_CLIENT_SUPPORT

//  erpc_transport_t
	message = receive_message();
	
	printk("Remote core received a message: %d\n", message);

	erpc_transport_t transport = erpc_transport_rpmsg_openamp_init(ep);
	//     erpc_mbf_t
	erpc_mbf_t message_buffer_factory = erpc_mbf_static_init();

	erpc_client_init(transport ,message_buffer_factory);

	Matrix matrix1;
	Matrix matrix2;
	Matrix matrixR = {{0}, {0}};
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
	printk("MulRet %d.\n", matrixR[2][2]);
//	erpcMatrixMultiply(matrix1, matrix2, matrixR);
//	printk("MulRetX %d.\n", matrixR[2][2]);

	while (1) {
		k_sleep(500);
	}
#endif


_cleanup:
	rpmsg_deinit_vdev(&rvdev);
	metal_finish();

	printk("OpenAMP demo ended.\n");
}

void main(void)
{
	printk("Starting application thread!\n");
	k_sleep(500);
	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
}
