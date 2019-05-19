/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>

#include "tfm_api.h"

#define IPC_TEST_SERVICE1_SID        (0x1000)
#define IPC_TEST_SERVICE1_MIN_VER    (0x0001)

/**
 * \brief Retrieve the version of the PSA Framework API.
 *
 * \note This is a functional test only and doesn't
 *       mean to test all possible combinations of
 *       input parameters and return values.
 */
static void tfm_ipc_test_1001()
{
	uint32_t version;

	version = tfm_psa_framework_version_veneer();
	if (version == PSA_FRAMEWORK_VERSION) {
		printk("The version of the PSA Framework API is %d.\r\n", version);
	} else {
		printk("The version of the PSA Framework API is not valid!\r\n");
		return;
	}
}

/**
 * \brief Retrieve the minor version of a RoT Service.
 */
static void tfm_ipc_test_1002()
{
	uint32_t version;

	version = tfm_psa_version_veneer(IPC_TEST_SERVICE1_SID);
	if (version == PSA_VERSION_NONE) {
		printk("RoT Service is not implemented or caller is not authorized" \
		      "to access it!\r\n");
		return;
	} else {
		/* Valid version number */
		printk("The minor version is %d.\r\n", version);
	}
}

/**
 * \brief Connect to a RoT Service by its SID.
 */
static void tfm_ipc_test_1003()
{
	psa_handle_t handle;

	handle = tfm_psa_connect_veneer(IPC_TEST_SERVICE1_SID, IPC_TEST_SERVICE1_MIN_VER);
	if (handle > 0) {
		printk("Connect success!\r\n");
	} else {
		printk("The RoT Service has refused the connection!\r\n");
		return;
	}
	tfm_psa_close_veneer(handle);
}

psa_status_t psa_call(psa_handle_t handle,
                      const psa_invec *in_vec,
                      size_t in_len,
                      psa_outvec *out_vec,
                      size_t out_len)
{
	/* FixMe: sanity check can be added to offload some NS thread checks from
	 * TFM secure API
	 */

	/* Due to v8M restrictions, TF-M NS API needs to add another layer of
	 * serialization in order for NS to pass arguments to S
	 */
	psa_invec in_vecs, out_vecs;

	in_vecs.base = in_vec;
	in_vecs.len = in_len;
	out_vecs.base = out_vec;
	out_vecs.len = out_len;
	return tfm_psa_call_veneer((uint32_t)handle,
                                (uint32_t)&in_vecs,
                                (uint32_t)&out_vecs);
}

/**
 * \brief Call a RoT Service.
 */
static void tfm_ipc_test_1004()
{
	char str1[] = "str1";
	char str2[] = "str2";
	char str3[128], str4[128];
	struct psa_invec invecs[2] = {{str1, sizeof(str1)/sizeof(char)},
	                          {str2, sizeof(str2)/sizeof(char)}};
	struct psa_outvec outvecs[2] = {{str3, sizeof(str3)/sizeof(char)},
	                            {str4, sizeof(str4)/sizeof(char)}};
	psa_handle_t handle;
	psa_status_t status;
	uint32_t min_version;

	min_version = tfm_psa_version_veneer(IPC_TEST_SERVICE1_SID);
	printk("TFM service support minor version is %d.\r\n", min_version);
	handle = tfm_psa_connect_veneer(IPC_TEST_SERVICE1_SID, IPC_TEST_SERVICE1_MIN_VER);
	status = psa_call(handle, invecs, 2, outvecs, 2);
	if (status >= 0) {
		printk("psa_call is successful!\r\n");
	} else if (status == PSA_DROP_CONNECTION) {
		printk("The connection has been dropped by the RoT Service!\r\n");
		return;
	} else {
		printk("psa_call is failed!\r\n");
		return;
	}
	printk("outvec1 is: %s\r\n", outvecs[0].base);
	printk("outvec2 is: %s\r\n", outvecs[1].base);
	tfm_psa_close_veneer(handle);
}

void main(void)
{
	tfm_ipc_test_1001();
	tfm_ipc_test_1002();
	tfm_ipc_test_1003();
	tfm_ipc_test_1004();

	printk("Hello World! %s\n", CONFIG_BOARD);
}
