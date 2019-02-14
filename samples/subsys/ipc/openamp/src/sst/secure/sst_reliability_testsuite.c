/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sst_tests.h"

#include <stdio.h>
#include <string.h>

//#include "test/framework/test_framework_helpers.h"
#include "tfm_sst_defs.h"
//#include "secure_fw/services/secure_storage/assets/sst_asset_defs.h"
#include "psa_sst_api.h"
#include "s_test_helpers.h"
#include "sst_asset_defs.h"
#include "sst_asset_management.h"

/* Test suite defines */
#define LOOP_ITERATIONS_001 5U

/* Perform one more create/delete than the number of assets to check that
 * object metadata is being freed correctly.
 */
#define LOOP_ITERATIONS_002 (SST_NUM_ASSETS + 1U)

#define WRITE_BUF_SIZE      33U
#define READ_BUF_SIZE       (WRITE_BUF_SIZE + 6U)

#define WRITE_DATA          "PACKMYBOXWITHFIVEDOZENLIQUORJUGS"
#define READ_DATA           "######################################"
#define RESULT_DATA         ("###" WRITE_DATA "###")

/* Define default asset's token */
#define ASSET_TOKEN      NULL
#define ASSET_TOKEN_SIZE 0

/* Define test suite for SST reliability tests */
/* List of tests */
/*
static void tfm_sst_test_3001(struct test_result_t *ret);
static void tfm_sst_test_3002(struct test_result_t *ret);

static struct test_t reliability_tests[] = {
    {&tfm_sst_test_3001, "TFM_SST_TEST_3001",
     "repetitive writes in an asset", {0} },
    {&tfm_sst_test_3002, "TFM_SST_TEST_3002",
     "repetitive create, write, read and delete", {0} },
};
void register_testsuite_s_sst_reliability(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(reliability_tests) /
                          sizeof(reliability_tests[0]));

    set_testsuite("SST reliability tests (TFM_SST_TEST_3XXX)",
                  reliability_tests, list_size, p_test_suite);
}
*/

/**
 * \brief Tests repetitive writes in an asset with the follow permissions:
 *        CLIENT_ID   | Permissions
 *        ------------|------------------------
 *        S_CLIENT_ID | REFERENCE, READ, WRITE
 */
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
void tfm_sst_test_3001(struct test_result_t *ret)
{
    uint32_t asset_offset = 0;
    const uint32_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint32_t itr;
    uint8_t wrt_data[WRITE_BUF_SIZE] = WRITE_DATA;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;
    struct tfm_sst_token_t s_token;
    struct tfm_sst_buf_t   s_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        return;
    }

    /* Pack the token information in the token structure */
    s_token.token = ASSET_TOKEN;
    s_token.token_size = ASSET_TOKEN_SIZE;

    /* Checks write permissions in create function */
    err = sst_am_create(asset_uuid, (uint32_t)&s_token);
    if (err != PSA_SST_ERR_SUCCESS) {
        printk("Create should not fail for client S_CLIENT_ID\n");
        return;
    }

    /* Sets write and read sizes */
    io_data.size = WRITE_BUF_SIZE-1;

    for (itr = 0; itr < LOOP_ITERATIONS_001; itr++) {
        TEST_LOG("  > Iteration %d of %d\r", itr + 1, LOOP_ITERATIONS_001);

        do {
            /* Sets data structure */
            io_data.data = wrt_data;
            io_data.offset = asset_offset;

            /* Pack buffer information in the buffer structure */
            s_data.size = io_data.size;
            s_data.offset = io_data.offset;
            s_data.data = (uint8_t *)io_data.data;
            
            /* Checks write permissions in the write function */
            err = sst_am_write(asset_uuid, (uint32_t)&s_token, (uint32_t)&s_data);            
            /* err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                                io_data.size, io_data.offset, io_data.data); */
            if (err != PSA_SST_ERR_SUCCESS) {
                printk("Write should not fail for client S_CLIENT_ID\n");
                return;
            }

            /* Updates data structure to point to the read buffer */
            io_data.data = &read_data[3];

            s_data.data = (uint8_t *)io_data.data;
            /* Checks read permissions in the read function */
            err = sst_am_read(SST_DIRECT_CLIENT_READ, asset_uuid, (uint32_t)&s_token,
                              (uint32_t)&s_data);
            /* err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                               io_data.size, io_data.offset, io_data.data);*/
            if (err != PSA_SST_ERR_SUCCESS) {
                printk("Read should not fail for client S_CLIENT_ID\n");
                return;
            }

            /* Checks read data buffer content */
            if (memcmp(read_data, RESULT_DATA, READ_BUF_SIZE) != 0) {
                printk("Read buffer contains incorrect data\n");
                return;
            }

            /* Reset read buffer data */
            memset(io_data.data, 'X', io_data.size);

            /* Moves asset offsets to next position */
            asset_offset += io_data.size;

        } while (asset_offset < SST_ASSET_MAX_SIZE_X509_CERT_LARGE);

        /* Resets asset_offset */
        asset_offset = 0;
    }

    printk("\n");

    /* Checks write permissions in delete function */
    err = sst_am_delete(asset_uuid, (uint32_t)&s_token);
    /* err = psa_sst_delete(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE); */
    if (err != PSA_SST_ERR_SUCCESS) {
        printk("Delete should not fail for client S_CLIENT_ID\n");
        return;
    }

    ret->val = TEST_PASSED;
}
#else
void tfm_sst_test_3001(struct test_result_t *ret)
{
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint32_t itr;
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    uint8_t data[BUFFER_PLUS_PADDING_SIZE] = {0};
    uint32_t i;
    struct tfm_sst_token_t s_token;
    struct tfm_sst_buf_t   s_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        return;
    }

    /* Pack the token information in the token structure */
    s_token.token = ASSET_TOKEN;
    s_token.token_size = ASSET_TOKEN_SIZE;

    /* Checks write permissions in create function */
    err = sst_am_create(asset_uuid, (uint32_t)&s_token);
    if (err != PSA_SST_ERR_SUCCESS) {
        printk("Create should not fail for client S_CLIENT_ID\n");
        return;
    }

    /* Sets write and read sizes */
    io_data.size = SST_ASSET_MAX_SIZE_AES_KEY_192;

    for (itr = 0; itr < LOOP_ITERATIONS_001; itr++) {
        TEST_LOG("  > Iteration %d of %d\r", itr + 1, LOOP_ITERATIONS_001);

        memset(data, 0, BUFFER_PLUS_PADDING_SIZE);
        /* Sets data structure */
        io_data.data = data;
        io_data.offset = 0;

        /* Pack buffer information in the buffer structure */
        s_data.size = io_data.size;
        s_data.offset = io_data.offset;
        s_data.data = (uint8_t *)io_data.data;
        
        /* Checks write permissions in the write function */
        err = sst_am_write(asset_uuid, (uint32_t)&s_token, (uint32_t)&s_data);

        /* err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                            io_data.size, io_data.offset, io_data.data); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Write should not fail for client S_CLIENT_ID\n");
            return;
        }

        memset(data, 'X', BUFFER_PLUS_PADDING_SIZE);
        io_data.data = data + HALF_PADDING_SIZE;

        s_data.data = (uint8_t *)io_data.data;
        /* Checks read permissions in the read function */
        err = sst_am_read(SST_DIRECT_CLIENT_READ, asset_uuid, (uint32_t)&s_token,
                          (uint32_t)&s_data);
        /* err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           io_data.size, io_data.offset, io_data.data); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Read should not fail for client S_CLIENT_ID\n");
            return;
        }

        /* Checks read data buffer content */
        if (memcmp(data, "XXXX", HALF_PADDING_SIZE) != 0) {
            printk("Read buffer contains illegal pre-data\n");
            return;
        }

        for (i=HALF_PADDING_SIZE; i<(BUFFER_PLUS_HALF_PADDING_SIZE); i++) {
            if (data[i] != 0) {
                printk("Read buffer has read incorrect data\n");
                return;
            }
        }

        if (memcmp((data+BUFFER_PLUS_HALF_PADDING_SIZE), "XXXX",
                    HALF_PADDING_SIZE) != 0) {
            printk("Read buffer contains illegal post-data\n");
            return;
        }
    }

    printk("\n");

    /* Checks write permissions in delete function */
    err = sst_am_delete(asset_uuid, (uint32_t)&s_token);
    /* err = psa_sst_delete(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE); */
    if (err != PSA_SST_ERR_SUCCESS) {
        printk("Delete should not fail for client S_CLIENT_ID\n");
        return;
    }

    ret->val = TEST_PASSED;
}
#endif /* SST_ENABLE_PARTIAL_ASSET_RW */

/**
 * \brief Tests repetitive creates, reads, writes and deletes, with the follow
 *        permissions:
 *        CLIENT_ID   | Permissions
 *        ------------|------------------------
 *        S_CLIENT_ID | REFERENCE, READ, WRITE
 */
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
void tfm_sst_test_3002(struct test_result_t *ret)
{
    uint32_t asset_offset = 0;
    const uint32_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint32_t itr;
    uint8_t wrt_data[WRITE_BUF_SIZE] = WRITE_DATA;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;
    struct tfm_sst_token_t s_token;
    struct tfm_sst_buf_t   s_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        return;
    }

    /* Sets write and read sizes */
    io_data.size = WRITE_BUF_SIZE-1;

    for (itr = 0; itr < LOOP_ITERATIONS_002; itr++) {
        TEST_LOG("  > Iteration %d of %d\r", itr + 1, LOOP_ITERATIONS_002);

        /* Pack the token information in the token structure */
        s_token.token = ASSET_TOKEN;
        s_token.token_size = ASSET_TOKEN_SIZE;
        
        /* Checks write permissions in create function */
        err = sst_am_create(asset_uuid, (uint32_t)&s_token);
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Create should not fail for client S_CLIENT_ID\n");
            return;
        }

        do {
            /* Sets data structure */
            io_data.data = wrt_data;
            io_data.offset = asset_offset;

            /* Pack buffer information in the buffer structure */
            s_data.size = io_data.size;
            s_data.offset = io_data.offset;
            s_data.data = (uint8_t *)io_data.data;
            
            /* Checks write permissions in the write function */
            err = sst_am_write(asset_uuid, (uint32_t)&s_token, (uint32_t)&s_data);            
            /* err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                                io_data.size, io_data.offset, io_data.data); */
            if (err != PSA_SST_ERR_SUCCESS) {
                printk("Write should not fail for client S_CLIENT_ID\n");
                return;
            }

            /* Updates data structure to point to the read buffer */
            io_data.data = &read_data[3];

            s_data.data = (uint8_t *)io_data.data;
            /* Checks read permissions in the read function */
            err = sst_am_read(SST_DIRECT_CLIENT_READ, asset_uuid, (uint32_t)&s_token,
                              (uint32_t)&s_data);
            /* err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                               io_data.size, io_data.offset, io_data.data); */
            if (err != PSA_SST_ERR_SUCCESS) {
                printk("Read should not fail for client S_CLIENT_ID\n");
                return;
            }

            /* Checks read data buffer content */
            if (memcmp(read_data, RESULT_DATA, READ_BUF_SIZE) != 0) {
                read_data[READ_BUF_SIZE-1] = 0;
                printk("Read buffer contains incorrect data\n");
                return;
            }

            /* Reset read buffer data */
            memset(io_data.data, 'X', io_data.size);

            /* Moves asset offsets to next position */
            asset_offset += io_data.size;

        } while (asset_offset < SST_ASSET_MAX_SIZE_X509_CERT_LARGE);

        /* Resets asset_offset */
        asset_offset = 0;

        /* Checks write permissions in delete function */
        err = sst_am_delete(asset_uuid, (uint32_t)&s_token);
        /* err = psa_sst_delete(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Delete should not fail for client S_CLIENT_ID\n");
            return;
        }
    }

    printk("\n");

    ret->val = TEST_PASSED;
}

#else
void tfm_sst_test_3002(struct test_result_t *ret)
{
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint32_t itr;
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    uint8_t data[BUFFER_PLUS_PADDING_SIZE] = {0};
    uint32_t i;
    struct tfm_sst_token_t s_token;
    struct tfm_sst_buf_t   s_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        return;
    }

    /* Sets write and read sizes */
    io_data.size = SST_ASSET_MAX_SIZE_AES_KEY_192;

    for (itr = 0; itr < LOOP_ITERATIONS_002; itr++) {
        TEST_LOG("  > Iteration %d of %d\r", itr + 1, LOOP_ITERATIONS_002);
        
        /* Pack the token information in the token structure */
        s_token.token = ASSET_TOKEN;
        s_token.token_size = ASSET_TOKEN_SIZE;
        
        /* Checks write permissions in create function */
        err = sst_am_create(asset_uuid, (uint32_t)&s_token);
        /* err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Create should not fail for client S_CLIENT_ID\n");
            return;
        }

        memset(data, 0, BUFFER_PLUS_PADDING_SIZE);
        /* Sets data structure */
        io_data.data = data;
        io_data.offset = 0;

        /* Pack buffer information in the buffer structure */
        s_data.size = io_data.size;
        s_data.offset = io_data.offset;
        s_data.data = (uint8_t *)io_data.data;
        
        /* Checks write permissions in the write function */
        err = sst_am_write(asset_uuid, (uint32_t)&s_token, (uint32_t)&s_data);
        /* err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                            io_data.size, io_data.offset, io_data.data); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Write should not fail for client S_CLIENT_ID\n");
            return;
        }

        memset(data, 'X', BUFFER_PLUS_PADDING_SIZE);
        io_data.data = data + HALF_PADDING_SIZE;

        s_data.data = (uint8_t *)io_data.data;
        /* Checks read permissions in the read function */
        err = sst_am_read(SST_DIRECT_CLIENT_READ, asset_uuid, (uint32_t)&s_token,
                          (uint32_t)&s_data);
        /* err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           io_data.size, io_data.offset, io_data.data); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Read should not fail for client S_CLIENT_ID\n");
            return;
        }

        /* Checks read data buffer content */
        if (memcmp(data, "XXXX", HALF_PADDING_SIZE) != 0) {
            printk("Read buffer contains illegal pre-data\n");
            return;
        }

        for (i=HALF_PADDING_SIZE; i<(BUFFER_PLUS_HALF_PADDING_SIZE); i++) {
            if (data[i] != 0) {
                printk("Read buffer has read incorrect data\n");
                return;
            }
        }

        if (memcmp((data+BUFFER_PLUS_HALF_PADDING_SIZE), "XXXX",
                    HALF_PADDING_SIZE) != 0) {
            printk("Read buffer contains illegal post-data\n");
            return;
        }

        /* Checks write permissions in delete function */
        err = sst_am_delete(asset_uuid, (uint32_t)&s_token);
        /* err = psa_sst_delete(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE); */
        if (err != PSA_SST_ERR_SUCCESS) {
            printk("Delete should not fail for client S_CLIENT_ID\n");
            return;
        }
    }

    printk("\n");

    ret->val = TEST_PASSED;
}
#endif /* SST_ENABLE_PARTIAL_ASSET_RW */
