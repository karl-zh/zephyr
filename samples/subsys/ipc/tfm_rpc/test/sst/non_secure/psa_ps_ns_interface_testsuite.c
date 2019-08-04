/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <misc/printk.h>

#include "sst_ns_tests.h"

#include <stdio.h>
#include <string.h>

#include "psa/protected_storage.h"

/* Test UIDs */
#define WRITE_ONCE_UID  1U /* Cannot be modified or deleted once created */
#define TEST_UID_1      2U
#define TEST_UID_2      3U
#define TEST_UID_3      4U

/* Invalid values */
#define INVALID_UID              0U
#define INVALID_DATA_LEN         UINT32_MAX
#define INVALID_OFFSET           UINT32_MAX
#define INVALID_FLAG             (1U << 31)
#define INVALID_THREAD_NAME      "Thread_INVALID"

/* Write once data */
#define WRITE_ONCE_DATA          "THE_FIVE_BOXING_WIZARDS_JUMP_QUICKLY"
#define WRITE_ONCE_DATA_SIZE     (sizeof(WRITE_ONCE_DATA) - 1)
#define WRITE_ONCE_READ_DATA     "############################################"
#define WRITE_ONCE_RESULT_DATA   ("####" WRITE_ONCE_DATA "####")

#define WRITE_DATA               "THEQUICKBROWNFOXJUMPSOVERALAZYDOG"
#define WRITE_DATA_SIZE          (sizeof(WRITE_DATA) - 1)
#define READ_DATA                "_________________________________________"
#define RESULT_DATA              ("____" WRITE_DATA "____")

/**
 * Several tests use a buffer to read back data from an asset. This buffer is
 * larger than the size of the asset data by PADDING_SIZE bytes. This allows
 * us to ensure that the only the expected data is read back and that it is read
 * back correctly.
 *
 * For example if the buffer and asset are as follows:
 * Buffer - "XXXXXXXXXXXX", Asset data - "AAAA"
 *
 * Then a correct and successful read would give this result: "XXXXAAAAXXXX"
 * (Assuming a PADDING_SIZE of 8)
 */
#define BUFFER_SIZE 24
#define PADDING_SIZE 8
#define HALF_PADDING_SIZE 4

#define BUFFER_PLUS_PADDING_SIZE (BUFFER_SIZE + PADDING_SIZE)
#define BUFFER_PLUS_HALF_PADDING_SIZE (BUFFER_SIZE + HALF_PADDING_SIZE)

/**
 * \brief Tests set function with:
 * - Valid UID, no data, no flags
 * - Invalid UID, no data, no flags
 */
void tfm_sst_rpc_test_1001(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_1;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = 0;
    const uint8_t write_data[] = {0};

    /* Set with no data and no flags and a valid UID */
    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail with valid UID");
        return;
    }

    /* Attempt to set a second time */
    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail the second time with valid UID");
        return;
    }

    /* Set with an invalid UID */
    status = psa_ps_set(INVALID_UID, data_len, write_data, flags);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Set should not succeed with an invalid UID");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests set function with:
 * - Zero create flags
 * - Valid create flags (with previously created UID)
 * - Invalid create flags
 */
void tfm_sst_rpc_test_1002(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_2;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    const uint8_t write_data[] = WRITE_DATA;

    /* Set with no flags */
    status = psa_ps_set(WRITE_ONCE_UID, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail with no flags");
        return;
    }

    /* Set with valid flag: PSA_PS_FLAG_WRITE_ONCE (with previously created UID)
     * Note: Once created, WRITE_ONCE_UID cannot be deleted. It is reused across
     * multiple tests.
     */
    status = psa_ps_set(WRITE_ONCE_UID, WRITE_ONCE_DATA_SIZE, WRITE_ONCE_DATA,
                        PSA_PS_FLAG_WRITE_ONCE);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail with valid flags (and existing UID)");
        return;
    }

    /* Set with invalid flags */
    status = psa_ps_set(uid, data_len, write_data, INVALID_FLAG);
    if (status != PSA_PS_ERROR_FLAGS_NOT_SUPPORTED) {
        printk("Set should not succeed with invalid flags");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests set function with:
 * - NULL data pointer and zero data length
 *
 * \note A request with a null data pointer and data length not equal to zero is
 *       treated as a secure violation. TF-M framework will reject such requests
 *       and not return to the NSPE so this case is not tested here.
 *
 */
void tfm_sst_rpc_test_1003(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_3;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = 0;

    /* Set with NULL data pointer */
    status = psa_ps_set(uid, data_len, NULL, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should succeed with NULL data pointer and zero length");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests set function with:
 * - Data length longer than maximum permitted
 */
void tfm_sst_rpc_test_1004(void)
{
    /* A parameter with a buffer pointer where its data length is longer than
     * maximum permitted, it is treated as a secure violation.
     * TF-M framework will stop this transaction and not return from this
     * request to NSPE.
     */
    printk("This test is DEPRECATED and the test execution was SKIPPED\r\n");

#if 0
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_1;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = INVALID_DATA_LEN;
    const uint8_t write_data[] = WRITE_DATA;

    /* Set with data length longer than the maximum supported */
    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Set should not succeed with invalid data length");
        return;
    }
#endif

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests set function with:
 * - Write once UID that has already been created
 */
void tfm_sst_rpc_test_1005(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = WRITE_ONCE_UID;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t write_len = WRITE_DATA_SIZE;
    const uint32_t read_len = WRITE_ONCE_DATA_SIZE;
    const uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = WRITE_ONCE_READ_DATA;

    /* Set a write once UID a second time */
    status = psa_ps_set(uid, write_len, write_data, flags);
    if (status != PSA_PS_ERROR_WRITE_ONCE) {
        printk("Set should not rewrite a write once UID");
        return;
    }

    /* Get write once data */
    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail");
        return;
    }

    /* Check that write once data has not changed */
    if (memcmp(read_data, WRITE_ONCE_RESULT_DATA, sizeof(read_data)) != 0) {
        printk("Write once data should not have changed");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get function with:
 * - Valid data, zero offset
 * - Valid data, non-zero offset
 */
void tfm_sst_rpc_test_1006(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_2;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    uint32_t data_len = WRITE_DATA_SIZE;
    uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;
    const uint8_t *p_read_data = read_data;

    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get the entire data */
    status = psa_ps_get(uid, offset, data_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail");
        return;
    }

    /* Check that the data is correct, including no illegal pre- or post-data */
    if (memcmp(read_data, RESULT_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to result data");
        return;
    }

    /* Reset read data */
    memcpy(read_data, READ_DATA, sizeof(read_data));

    /* Read from offset 2 to 2 bytes before end of the data */
    offset = 2;
    data_len -= offset + 2;

    status = psa_ps_get(uid, offset, data_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail");
        return;
    }

    /* Check that the correct data was read */
    if (memcmp(p_read_data, "____", HALF_PADDING_SIZE) != 0) {
        printk("Read data contains illegal pre-data");
        return;
    }

    p_read_data += HALF_PADDING_SIZE;

    if (memcmp(p_read_data, write_data + offset, data_len) != 0) {
        printk("Read data incorrect");
        return;
    }

    p_read_data += data_len;

    if (memcmp(p_read_data, "____", HALF_PADDING_SIZE) != 0) {
        printk("Read data contains illegal post-data");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get function with:
 * - Zero data length, zero offset
 * - Zero data length, non-zero offset
 */
void tfm_sst_rpc_test_1007(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_3;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t write_len = WRITE_DATA_SIZE;
    const uint32_t read_len = 0;
    uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;

    status = psa_ps_set(uid, write_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get zero data from zero offset */
    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail with zero data len");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to original read data");
        return;
    }

    offset = 5;

    /* Get zero data from non-zero offset */
    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to original read data");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get function with:
 * - Unset UID
 * - Invalid UID
 */
void tfm_sst_rpc_test_1008(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_1;
    const uint32_t data_len = 1;
    const uint32_t offset = 0;
    uint8_t read_data[] = READ_DATA;

    /* Get with UID that has not yet been set */
    status = psa_ps_get(uid, offset, data_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_ERROR_UID_NOT_FOUND) {
        printk("Get succeeded with non-existant UID");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data not equal to original read data");
        return;
    }

    /* Get with invalid UID */
    status = psa_ps_get(INVALID_UID, offset, data_len,
                        read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Get succeeded with invalid UID");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data not equal to original read data");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get function with:
 * - Offset greater than UID length
 * - Data length greater than UID length
 * - Data length + offset greater than UID length
 */
void tfm_sst_rpc_test_1009(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_2;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t write_len = WRITE_DATA_SIZE;
    uint32_t read_len;
    uint32_t offset;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;

    status = psa_ps_set(uid, write_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get with offset greater than UID's length */
    read_len = 1;
    offset = write_len + 1;

    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_ERROR_OFFSET_INVALID) {
        printk("Get should not succeed with offset too large");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to original read data");
        return;
    }

    /* Get with data length greater than UID's length */
    read_len = write_len + 1;
    offset = 0;

    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_ERROR_INCORRECT_SIZE) {
        printk("Get should not succeed with data length too large");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to original read data");
        return;
    }

    /* Get with offset + data length greater than UID's length, but individually
     * valid
     */
    read_len = write_len;
    offset = 1;

    status = psa_ps_get(uid, offset, read_len, read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_ERROR_INCORRECT_SIZE) {
        printk("Get should not succeed with offset + data length too large");
        return;
    }

    /* Check that the read data is unchanged */
    if (memcmp(read_data, READ_DATA, sizeof(read_data)) != 0) {
        printk("Read data should be equal to original read data");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get function with:
 * - NULL data pointer and zero data length
 *
 * \note A request with a null data pointer and data length not equal to zero is
 *       treated as a secure violation. TF-M framework will reject such requests
 *       and not return to the NSPE so this case is not tested here.
 *
 */
void tfm_sst_rpc_test_1010(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_3;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    const uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;

    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get with NULL data pointer */
    status = psa_ps_get(uid, offset, 0, NULL);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should succeed with NULL data pointer and zero length");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get info function with:
 * - Write once UID
 */
void tfm_sst_rpc_test_1011(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = WRITE_ONCE_UID;
    struct psa_ps_info_t info = {0};

    /* Get info for write once UID */
    status = psa_ps_get_info(uid, &info);
    if (status != PSA_PS_SUCCESS) {
        printk("Get info should not fail for write once UID");
        return;
    }

    /* Check that the info struct contains the correct values */
    if (info.size != WRITE_ONCE_DATA_SIZE) {
        printk("Size incorrect for write once UID");
        return;
    }

    if (info.flags != PSA_PS_FLAG_WRITE_ONCE) {
        printk("Flags incorrect for write once UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get info function with:
 * - Valid UID
 */
void tfm_sst_rpc_test_1012(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_1;
    struct psa_ps_info_t info = {0};
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    const uint8_t write_data[] = WRITE_DATA;

    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get info for valid UID */
    status = psa_ps_get_info(uid, &info);
    if (status != PSA_PS_SUCCESS) {
        printk("Get info should not fail with valid UID");
        return;
    }

    /* Check that the info struct contains the correct values */
    if (info.size != data_len) {
        printk("Size incorrect for valid UID");
        return;
    }

    if (info.flags != flags) {
        printk("Flags incorrect for valid UID");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get info function with:
 * - Unset UID
 * - Invalid UID
 */
void tfm_sst_rpc_test_1013(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_2;
    struct psa_ps_info_t info = {0};

    /* Get info with UID that has not yet been set */
    status = psa_ps_get_info(uid, &info);
    if (status != PSA_PS_ERROR_UID_NOT_FOUND) {
        printk("Get info should not succeed with unset UID");
        return;
    }

    /* Check that the info struct has not been modified */
    if (info.size != 0) {
        printk("Size should not have changed");
        return;
    }

    if (info.flags != 0) {
        printk("Flags should not have changed");
        return;
    }

    /* Get info with invalid UID */
    status = psa_ps_get_info(INVALID_UID, &info);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Get info should not succeed with invalid UID");
        return;
    }

    /* Check that the info struct has not been modified */
    if (info.size != 0) {
        printk("Size should not have changed");
        return;
    }

    if (info.flags != 0) {
        printk("Flags should not have changed");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get info function with:
 * - NULL info pointer
 */
void tfm_sst_rpc_test_1014(void)
{
    /* A parameter with a null pointer is treated as a secure violation.
     * TF-M framework will stop this transaction and not return from this
     * request to NSPE.
     */
    printk("This test is DEPRECATED and the test execution was SKIPPED\r\n");

#if 0
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_3;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    const uint8_t write_data[] = WRITE_DATA;

    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get info with NULL info pointer */
    status = psa_ps_get_info(uid, NULL);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Get info should not succeed with NULL info pointer");
        return;
    }

    /* Call remove to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }
#endif

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests remove function with:
 * - Valid UID
 */
void tfm_sst_rpc_test_1015(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_1;
    struct psa_ps_info_t info = {0};
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    const uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;

    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Call remove with valid ID */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail with valid UID");
        return;
    }

    /* Check that get info fails for removed UID */
    status = psa_ps_get_info(uid, &info);
    if (status != PSA_PS_ERROR_UID_NOT_FOUND) {
        printk("Get info should not succeed with removed UID");
        return;
    }

    /* Check that get fails for removed UID */
    status = psa_ps_get(uid, offset, data_len, read_data);
    if (status != PSA_PS_ERROR_UID_NOT_FOUND) {
        printk("Get should not succeed with removed UID");
        return;
    }

    /* Check that remove fails for removed UID */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_ERROR_UID_NOT_FOUND) {
        printk("Remove should not succeed with removed UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests remove function with:
 * - Write once UID
 */
void tfm_sst_rpc_test_1016(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = WRITE_ONCE_UID;

    /* Call remove with write once UID */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_ERROR_WRITE_ONCE) {
        printk("Remove should not succeed with write once UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests remove function with:
 * - Invalid UID
 */
void tfm_sst_rpc_test_1017(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = INVALID_UID;

    /* Call remove with an invalid UID */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_ERROR_INVALID_ARGUMENT) {
        printk("Remove should not succeed with invalid UID");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests data block compact feature.
 *        Set UID 1 to locate it at the beginning of the block. Then set UID 2
 *        to be located after UID 1 and remove UID 1. UID 2 will be compacted to
 *        the beginning of the block. This test verifies that the compaction
 *        works correctly by reading back UID 2.
 */
void tfm_sst_rpc_test_1023(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid_1 = TEST_UID_2;
    const psa_ps_uid_t uid_2 = TEST_UID_3;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len_2 = WRITE_DATA_SIZE;
    const uint32_t offset = 0;
    const uint8_t write_data_1[] = "UID 1 DATA";
    const uint8_t write_data_2[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;

    /* Set UID 1 */
    status = psa_ps_set(uid_1, sizeof(write_data_1), write_data_1, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail for UID 1");
        return;
    }

    /* Set UID 2 */
    status = psa_ps_set(uid_2, data_len_2, write_data_2, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail for UID 2");
        return;
    }

    /* Remove UID 1. This should cause UID 2 to be compacted to the beginning of
     * the block.
     */
    status = psa_ps_remove(uid_1);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail for UID 1");
        return;
    }

    /* If the compact worked as expected, the test should be able to read back
     * the data from UID 2 correctly.
     */
    status = psa_ps_get(uid_2, offset, data_len_2,
                        read_data + HALF_PADDING_SIZE);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail for UID 2");
        return;
    }

    if (memcmp(read_data, RESULT_DATA, sizeof(read_data)) != 0) {
        printk("Read buffer has incorrect data");
        return;
    }

    /* Remove UID 2 to clean up storage for the next test */
    status = psa_ps_remove(uid_2);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail for UID 2");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests set and multiple partial gets.
 */
void tfm_sst_rpc_test_1024(void)
{
    psa_ps_status_t status;
    uint32_t i;
    const psa_ps_uid_t uid = TEST_UID_1;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t data_len = WRITE_DATA_SIZE;
    uint32_t offset = 0;
    const uint8_t write_data[] = WRITE_DATA;
    uint8_t read_data[] = READ_DATA;

    /* Set the entire data into UID */
    status = psa_ps_set(uid, data_len, write_data, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Set should not fail");
        return;
    }

    /* Get the data from UID one byte at a time */
    for (i = 0; i < data_len; ++i) {
        status = psa_ps_get(uid, offset, 1,
                            (read_data + HALF_PADDING_SIZE + i));
        if (status != PSA_PS_SUCCESS) {
            printk("Get should not fail for partial read");
            return;
        }

        ++offset;
    }

    if (memcmp(read_data, RESULT_DATA, sizeof(read_data)) != 0) {
        printk("Read buffer has incorrect data");
        return;
    }

    /* Remove UID to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests multiple sets to the same UID.
 */
void tfm_sst_rpc_test_1025(void)
{
    psa_ps_status_t status;
    const psa_ps_uid_t uid = TEST_UID_2;
    const psa_ps_create_flags_t flags = PSA_PS_FLAG_NONE;
    const uint32_t offset = 0;
    const uint8_t write_data_1[] = "ONE";
    const uint8_t write_data_2[] = "TWO";
    const uint8_t write_data_3[] = "THREE";
    uint8_t read_data[] = READ_DATA;

    /* Set write data 1 into UID */
    status = psa_ps_set(uid, sizeof(write_data_1), write_data_1, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("First set should not fail");
        return;
    }

    /* Set write data 2 into UID */
    status = psa_ps_set(uid, sizeof(write_data_2), write_data_2, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Second set should not fail");
        return;
    }

    /* Set write data 3 into UID */
    status = psa_ps_set(uid, sizeof(write_data_3), write_data_3, flags);
    if (status != PSA_PS_SUCCESS) {
        printk("Third set should not fail");
        return;
    }

    status = psa_ps_get(uid, offset, sizeof(write_data_3), read_data);
    if (status != PSA_PS_SUCCESS) {
        printk("Get should not fail");
        return;
    }

    /* Check that get returns the last data to be set */
    if (memcmp(read_data, write_data_3, sizeof(write_data_3)) != 0) {
        printk("Read buffer has incorrect data");
        return;
    }

    /* Remove UID to clean up storage for the next test */
    status = psa_ps_remove(uid);
    if (status != PSA_PS_SUCCESS) {
        printk("Remove should not fail");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

/**
 * \brief Tests get support function.
 */
void tfm_sst_rpc_test_1026(void)
{
    uint32_t support_flags;

    support_flags = psa_ps_get_support();
    if (support_flags != 0) {
        printk("Support flags should be 0");
        return;
    }

    printk("Test %s pass!\n", __func__);
}

