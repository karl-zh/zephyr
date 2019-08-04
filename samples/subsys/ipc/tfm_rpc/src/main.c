/*
 * Copyright (c) 2019, Linaro Limited
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
#include "psa/client.h"
#include "psa_manifest/sid.h"

#include "sst_ns_tests.h"

#define ERPC_CLIENT_SUPPORT 1

#define APP_TASK_STACK_SIZE (2048)
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
	u32_t current_core = sse_200_platform_get_cpu_id();

	ipm_send(ipm_handle, 0, current_core ? 0 : 1, 0, 1);
}

struct virtio_dispatch dispatch = {
	.get_status = virtio_get_status,
	.get_features = virtio_get_features,
	.notify = virtio_notify,
};

static K_SEM_DEFINE(data_sem, 0, 1);
static K_SEM_DEFINE(data_rx_sem, 0, 1);

#if ERPC_CLIENT_SUPPORT
#define RX_BUFF_SIZE            256

#define MSG_SIZE                4
#define MSGQ_LEN                512

struct k_msgq rcv_msgq;
static char __aligned(4) tbuffer[MSG_SIZE * MSGQ_LEN];
#endif

static void platform_ipm_callback(void *context, u32_t id, volatile void *data)
{
	k_sem_give(&data_sem);
}

int endpoint_cb(struct rpmsg_endpoint *ept, void *data,
		size_t len, u32_t src, void *priv)
{
	int ret, rx_data;

	received_data = *((unsigned int *) data);

#if ERPC_CLIENT_SUPPORT
	for (int i  = 0; i < len; i++) {
		rx_data = *((char *)data + i);
		ret = k_msgq_put(&rcv_msgq, (void *)&rx_data, K_NO_WAIT);
		if (ret != 0) {
			printk("No enough space in the queue %d.", i);
			break;
		}
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
void __assert_func(const char *file, int line, const char *func, const char *e)
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

	status = k_sem_take(&data_sem, K_FOREVER);

	if (status == 0) {
		virtqueue_notification(vq[1]);
	}

	while (len > k_msgq_num_used_get(&rcv_msgq)) {
		k_sleep(5);
	}

	for (int i = 0; i < len; i++) {
		ret = k_msgq_get(&rcv_msgq, (void *)&rx_data, K_NO_WAIT);
		buf[i] = (rx_data & 0xFF);
	}
	return len;
}
#endif

#if ERPC_CLIENT_SUPPORT
typedef struct ErpcMessageBufferFactory *erpc_mbf_t;
typedef struct ErpcTransport *erpc_transport_t;
#endif

#define IPC_TEST_SERVICE1_SID        (0x1000)
#define IPC_TEST_SERVICE1_MIN_VER    (0x0001)

/**
 * \brief Retrieve the version of the PSA Framework API.
 *
 * \note This is a functional test only and doesn't
 *       mean to test all possible combinations of
 *       input parameters and return values.
 */
static void tfm_ipc_rpc_1001(void)
{
	uint32_t version;

	version = psa_framework_version();
	if (version == PSA_FRAMEWORK_VERSION) {
		printk("The version of the PSA Framework API is %d.\n", version);
	} else {
		printk("The version of the PSA Framework API is not valid!\n");
		return;
	}
    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Retrieve the minor version of a RoT Service.
 */
static void tfm_ipc_rpc_1002(void)
{
	uint32_t version;

	version = psa_version(IPC_SERVICE_TEST_BASIC_SID);
	if (version == PSA_VERSION_NONE) {
		printk("RoT Service is not implemented or caller is not authorized" \
				  "to access it!\n");
		return;
	} else {
		/* Valid version number */
		printk("The minor version is %d.\n", version);
	}
    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Connect to a RoT Service by its SID.
 */
static void tfm_ipc_rpc_1003(void)
{
	psa_handle_t handle;

	handle = psa_connect(IPC_SERVICE_TEST_BASIC_SID,
							IPC_SERVICE_TEST_BASIC_VERSION);
	if (handle > 0) {
		printk("Connect success!\n");
	} else {
		printk("The RoT Service has refused the connection!\n");
		return;
	}
	psa_close(handle);
    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Call a RoT Service.
 */
static void tfm_ipc_rpc_1004(void)
{
	char str1[] = "str1";
	char str2[] = "str2";
	char str3[128], str4[128];
	struct psa_invec invecs[2] = {{str1, sizeof(str1)/sizeof(char)},
									{str2, sizeof(str2)/sizeof(char)} };
	struct psa_outvec outvecs[2] = {{str3, sizeof(str3)/sizeof(char)},
									{str4, sizeof(str4)/sizeof(char)} };
	psa_handle_t handle;
	psa_status_t status;
	uint32_t min_version;

	min_version = psa_version(IPC_SERVICE_TEST_BASIC_SID);
	printk("TFM service support minor version is %d.\n", min_version);
	handle = psa_connect(IPC_SERVICE_TEST_BASIC_SID,
							IPC_SERVICE_TEST_BASIC_VERSION);
	status = psa_call(handle, PSA_IPC_CALL, invecs, 2, outvecs, 2);
	if (status >= 0) {
		printk("psa_call is successful!\n");
	} else {
		printk("psa_call is failed!\n");
		return;
	}

	printk("outvec1 is: %s\n", outvecs[0].base);
	printk("outvec2 is: %s\n", outvecs[1].base);
	psa_close(handle);
    printk("Test %s pass!\n", __func__);
}

static void tfm_ipc_rpc_1005(void)
{
	psa_handle_t handle;
	psa_status_t status;
	int test_result;
	struct psa_outvec outvecs[1] = {{&test_result, sizeof(test_result)} };

	handle = psa_connect(IPC_CLIENT_TEST_BASIC_SID,
							IPC_CLIENT_TEST_BASIC_VERSION);
	if (handle > 0) {
		printk("Connect success!\n");
	} else {
		printk("The RoT Service has refused the connection!\n");
		printk("Test %s fail!\n", __func__);
		return;
	}

	status = psa_call(handle, PSA_IPC_CALL, NULL, 0, outvecs, 1);
	if (status >= 0) {
		printk("Call success!\n");
		if (test_result > 0) {
			printk("Test %s pass!\n", __func__);
		} else {
			printk("Test %s fail!\n", __func__);
		}
	} else {
		printk("Call failed!\n");
		printk("Test %s fail!\n", __func__);
	}

	psa_close(handle);
}

static void tfm_ipc_rpc_1006(void)
{
	psa_handle_t handle;
	psa_status_t status;
	int test_result;
	struct psa_outvec outvecs[1] = {{&test_result, sizeof(test_result)} };

	handle = psa_connect(IPC_CLIENT_TEST_PSA_ACCESS_APP_MEM_SID,
							IPC_CLIENT_TEST_PSA_ACCESS_APP_MEM_VERSION);
	if (handle > 0) {
		printk("Connect success!\n");
	} else {
		printk("The RoT Service has refused the connection!\n");
		return;
	}

	status = psa_call(handle, PSA_IPC_CALL, NULL, 0, outvecs, 1);
	if (status >= 0) {
		printk("Call success!\n");
		if (test_result > 0) {
			printk("Test %s pass!\n", __func__);
		} else {
			printk("Test %s fail!\n", __func__);
		}
	} else {
		printk("Test %s fail!\n", __func__);
	}

	psa_close(handle);
}

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

	printk("\nOpenAMP[remote] demo started\r\n");

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
	ipm_handle = device_get_binding(DT_IPM_DEV);
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

#if ERPC_CLIENT_SUPPORT

	message = receive_message();

	printk("Remote received test message: %d\n", message);

	erpc_transport_t transport = erpc_transport_rpmsg_openamp_init(ep);
	erpc_mbf_t message_buffer_factory = erpc_mbf_static_init();

	erpc_client_init(transport, message_buffer_factory);

	Matrix matrix1;
	Matrix matrix2;
	Matrix matrixR = {{0}, {0} };
	int matrix_size = 5, i, j;

	for (i = 0; i < matrix_size; ++i) {
		for (j = 0; j < matrix_size; ++j) {
			matrix1[i][j] = 3;
			matrix2[i][j] = 5;
		}
	}

	erpcMatrixMultiply(matrix1, matrix2, matrixR);
	printk("MulRet %d.\n", matrixR[2][2]);

	for (i = 0; i < matrix_size; ++i) {
		for (j = 0; j < matrix_size; ++j) {
			matrix1[i][j] = 7;
			matrix2[i][j] = 2;
		}
	}
	erpcMatrixMultiply(matrix1, matrix2, matrixR);
	printk("MulRetX %d.\n", matrixR[2][2]);


	tfm_ipc_rpc_1001();
	tfm_ipc_rpc_1002();
	tfm_ipc_rpc_1003();
	tfm_ipc_rpc_1004();
	tfm_ipc_rpc_1005();
	tfm_ipc_rpc_1006();

	tfm_sst_rpc_test_1001();
	tfm_sst_rpc_test_1002();
	tfm_sst_rpc_test_1003();
	tfm_sst_rpc_test_1004();
	tfm_sst_rpc_test_1005();
	tfm_sst_rpc_test_1006();
	tfm_sst_rpc_test_1007();
	tfm_sst_rpc_test_1008();
	tfm_sst_rpc_test_1009();
	tfm_sst_rpc_test_1010();
	tfm_sst_rpc_test_1011();
	tfm_sst_rpc_test_1012();
	tfm_sst_rpc_test_1013();
	tfm_sst_rpc_test_1014();
	tfm_sst_rpc_test_1015();
	tfm_sst_rpc_test_1016();
	tfm_sst_rpc_test_1017();

	tfm_sst_rpc_test_1023();
	tfm_sst_rpc_test_1024();
	tfm_sst_rpc_test_1025();
	tfm_sst_rpc_test_1026();

	while (1) {
		k_sleep(500);
	}
#endif

	rpmsg_deinit_vdev(&rvdev);
	metal_finish();

	printk("OpenAMP demo ended.\n");
}

void main(void)
{
	printk("Starting application thread!\n");
#if ERPC_CLIENT_SUPPORT
	k_msgq_init(&rcv_msgq, tbuffer, MSG_SIZE, MSGQ_LEN);
#endif
	k_sleep(500);
	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
}
