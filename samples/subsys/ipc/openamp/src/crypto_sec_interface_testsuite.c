/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>

//#include "test/framework/test_framework_helpers.h"
#include "tfm_crypto_veneers.h"

#define BIT_SIZE_TEST_KEY (128)
#define BYTE_SIZE_TEST_KEY (BIT_SIZE_TEST_KEY/8)

#if 0
/* List of tests */
static void tfm_crypto_test_5001(struct test_result_t *ret);

static struct test_t crypto_tests[] = {
    {&tfm_crypto_test_5001, "TFM_CRYPTO_TEST_5001",
     "Secure Key management interface", {0}},
};

void register_testsuite_s_crypto_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(crypto_tests) / sizeof(crypto_tests[0]));

    set_testsuite("Crypto secure interface tests (TFM_CRYPTO_TEST_5XXX)",
                  crypto_tests, list_size, p_test_suite);
}

#endif
/**
 * \brief Secure interface test for Crypto
 *
 * \details The scope of this test is to perform a basic validation of
 *          the secure interface for the Key management module.
 */
void tfm_crypto_test_5001(struct test_result_t *ret)
{
    enum tfm_crypto_err_t err;
    uint32_t i = 0;
    const psa_key_slot_t slot = 0;
    const uint8_t data[] = "THIS IS MY KEY1";
    psa_key_type_t type = PSA_KEY_TYPE_NONE;
    size_t bits = 0;
    uint8_t exported_data[sizeof(data)] = {0};
    size_t exported_data_size = 0;

    err = tfm_crypto_import_key(slot,
                                       PSA_KEY_TYPE_AES,
                                       data,
                                       sizeof(data));

    if (err != TFM_CRYPTO_ERR_PSA_SUCCESS) {
        printk("Error importing a key\n");
        return;
    }

    err = tfm_crypto_get_key_information(slot, &type, &bits);
    if (err != TFM_CRYPTO_ERR_PSA_SUCCESS) {
        printk("Error getting key metadata\n");
        return;
    }

    if (bits != BIT_SIZE_TEST_KEY) {
        printk("The number of key bits is different from expected\n");
        return;
    }

    if (type != PSA_KEY_TYPE_AES) {
        printk("The type of the key is different from expected\n");
        return;
    }

    err = tfm_crypto_export_key(slot,
                                       exported_data,
                                       sizeof(data),
                                       &exported_data_size);

    if (err != TFM_CRYPTO_ERR_PSA_SUCCESS) {
        printk("Error exporting a key\n");
        return;
    }

    if (exported_data_size != BYTE_SIZE_TEST_KEY) {
        printk("Number of bytes of exported key different from expected\n");
        return;
    }

    /* Check that the exported key is the same as the imported one */
    for (i=0; i<exported_data_size; i++) {
        if (exported_data[i] != data[i]) {
            printk("Exported key doesn't match the imported key\n");
            return;
        }
    }

    err = tfm_crypto_destroy_key(slot);
    if (err != TFM_CRYPTO_ERR_PSA_SUCCESS) {
        printk("Error destroying the key\n");
        return;
    }

    err = tfm_crypto_get_key_information(slot, &type, &bits);
    if (err != TFM_CRYPTO_ERR_PSA_ERROR_EMPTY_SLOT) {
        printk("Key slot should be empty now\n");
        return;
    }

     printk("tfm_crypto_test_5001 Pass!\n");
//    ret->val = TEST_PASSED;
}
