/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "audit_core.h"

#include "audit_wrappers.h"
#include "audit_s_tests.h"
#include "audit_tests_common.h"

/*!
 * \def BASE_RETRIEVAL_LOG_INDEX
 *
 * \brief Base index from where to start elements retrieval
 */
#define BASE_RETRIEVAL_LOG_INDEX (6)

/*!
 * \def FIRST_RETRIEVAL_LOG_INDEX
 *
 * \brief Index of the first element in the log
 */
#define FIRST_RETRIEVAL_LOG_INDEX (0)

#if 0
/* List of tests */
static void tfm_audit_test_1001(struct test_result_t *ret);

static struct test_t audit_veneers_tests[] = {
    {&tfm_audit_test_1001, "TFM_AUDIT_TEST_1001",
     "Secure functional", {0} },
};

void register_testsuite_s_audit_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

    list_size = (sizeof(audit_veneers_tests) /
                 sizeof(audit_veneers_tests[0]));

    set_testsuite("Audit Logging secure interface test (TFM_AUDIT_TEST_1XXX)",
                  audit_veneers_tests, list_size, p_test_suite);
}

#endif

/**
 * \brief Functional test of the Secure interface
 *
 * \note This is a functional test only and doesn't
 *       mean to test all possible combinations of
 *       input parameters and return values.
 *       This tests will leave the log in a certain
 *       status which, in turn, will be evaluated by
 *       the Non Secure functional tests. If any tests
 *       are added here that will leave the log in a
 *       different state, Non Secure functional tests
 *       need to be amended accordingly.
 */
void tfm_audit_test_1001()
{
    enum psa_audit_err err;
    uint8_t local_buffer[LOCAL_BUFFER_SIZE], idx;
    struct psa_audit_record *record = (struct psa_audit_record *)
                                                  &local_buffer[0];
    uint32_t num_records, stored_size, record_size;
    struct psa_audit_record *retrieved_buffer;

    struct audit_core_retrieve_output retrieve_output_s;

    /* Fill the log with 36 records, each record is 28 bytes
     * we end up filling the log without wrapping
     */
    for (idx=0; idx<INITIAL_LOGGING_REQUESTS; idx++) {
        record->size = sizeof(struct psa_audit_record) - 4;
        record->id = DUMMY_TEST_RECORD_ID_BASE + idx;

        /* The record doesn't contain any payload */
        err = audit_core_add_record(record);
        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Record addition has returned an error\n");
            return;
        }
    }

    /* Get the log size */
    err = audit_core_get_info(&num_records, &stored_size);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Getting log info has returned error\n");
        return;
    }

    if (stored_size != INITIAL_LOGGING_SIZE) {
        printk("Expected log size is " STR(INITIAL_LOGGING_SIZE));
        return;
    }

    if (num_records != INITIAL_LOGGING_REQUESTS) {
        printk("Expected log records are " STR(INITIAL_LOGGING_REQUESTS));
        return;
    }

    /* Retrieve two log records starting from a given index */
    for (idx=BASE_RETRIEVAL_LOG_INDEX; idx<BASE_RETRIEVAL_LOG_INDEX+2; idx++) {

        struct audit_core_retrieve_input retrieve_input_s =
                                             {.record_index = idx,
                                              .buffer_size = LOCAL_BUFFER_SIZE,
                                              .token = NULL,
                                              .token_size = 0};
        retrieve_output_s.buffer =
          &local_buffer[(idx-BASE_RETRIEVAL_LOG_INDEX)*STANDARD_LOG_ENTRY_SIZE];
        retrieve_output_s.record_size = &record_size;

        err = audit_core_retrieve_record_wrapper(&retrieve_input_s,
                                               &retrieve_output_s);

        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Retrieve indexes 6 or 7 has returned an error\n");
            return;
        }

        if (record_size != STANDARD_LOG_ENTRY_SIZE) {
            printk("Expected log size is " STR(STANDARD_LOG_ENTRY_SIZE));
            return;
        }
    }

    /* Inspect the content of the second log record retrieved */
    retrieved_buffer = (struct psa_audit_record *)
        &local_buffer[offsetof(struct log_hdr,size)+STANDARD_LOG_ENTRY_SIZE];

    if (retrieved_buffer->id != ( DUMMY_TEST_RECORD_ID_BASE +
                                  (BASE_RETRIEVAL_LOG_INDEX+1) )) {
        printk("Unexpected argument in the index 7 entry\n");
        return;
    }

    /* Retrieve the last two log records */
    for (idx=num_records-2; idx<num_records; idx++) {

        struct audit_core_retrieve_input retrieve_input_s =
                                             {.record_index = idx,
                                              .buffer_size = LOCAL_BUFFER_SIZE,
                                              .token = NULL,
                                              .token_size = 0};
        retrieve_output_s.buffer =
            &local_buffer[(idx-(num_records-2))*STANDARD_LOG_ENTRY_SIZE];
        retrieve_output_s.record_size = &record_size;

        err = audit_core_retrieve_record_wrapper(&retrieve_input_s,
                                               &retrieve_output_s);

        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Retrieve of last two log records has returned error\n");
            return;
        }

        if (record_size != STANDARD_LOG_ENTRY_SIZE) {
            printk("Expected log size is " STR(STANDARD_LOG_ENTRY_SIZE));
            return;
        }
    }

    /* Inspect the first record retrieved in the local buffer */
    retrieved_buffer = (struct psa_audit_record *)
                           &local_buffer[offsetof(struct log_hdr,size)];

    if (retrieved_buffer->id != ( DUMMY_TEST_RECORD_ID_BASE +
                                  (INITIAL_LOGGING_REQUESTS-2) )) {
        printk("Unexpected argument in the second last entry\n");
        return;
    }

    /* Retrieve the first log item */
    struct audit_core_retrieve_input retrieve_input_s_first =
                                         {.record_index = 0,
                                          .buffer_size = LOCAL_BUFFER_SIZE,
                                          .token = NULL,
                                          .token_size = 0};

    retrieve_output_s.buffer = &local_buffer[0];
    retrieve_output_s.record_size = &record_size;

    err = audit_core_retrieve_record_wrapper(&retrieve_input_s_first,
                                           &retrieve_output_s);

    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Retrieve of the first log entry has returned error\n");
        return;
    }

    if (record_size != STANDARD_LOG_ENTRY_SIZE) {
        printk("Expected log size is " STR(STANDARD_LOG_ENTRY_SIZE));
        return;
    }

    if (retrieved_buffer->id != DUMMY_TEST_RECORD_ID_BASE) {
        printk("Unexpected argument in the first entry\n");
        return;
    }

    /* Retrieve the last log item */
    struct audit_core_retrieve_input retrieve_input_s_last =
                                         {.record_index = num_records-1,
                                          .buffer_size = LOCAL_BUFFER_SIZE,
                                          .token = NULL,
                                          .token_size = 0};

    retrieve_output_s.buffer = &local_buffer[0];
    retrieve_output_s.record_size = &record_size;

    err = audit_core_retrieve_record_wrapper(&retrieve_input_s_last,
                                           &retrieve_output_s);

    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Retrieve of last two log entries has returned error\n");
        return;
    }

    if (record_size != STANDARD_LOG_ENTRY_SIZE) {
        printk("Expected log size is " STR(STANDARD_LOG_ENTRY_SIZE));
        return;
    }

    /* Inspect the item just retrieved */
    if (retrieved_buffer->id != ( DUMMY_TEST_RECORD_ID_BASE +
                                  (INITIAL_LOGGING_REQUESTS-1) )) {
        printk("Unexpected argument in the second last entry\n");
        return;
    }

    /* Fill one more log record, this will wrap */
    record->size = sizeof(struct psa_audit_record) - 4;
    record->id = DUMMY_TEST_RECORD_ID_BASE + INITIAL_LOGGING_REQUESTS;

    /* The addition of this new log item will wrap the log ending */
    err = audit_core_add_record(record);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Record addition has returned an error\n");
        return;
    }

    /* Get the log size */
    err = audit_core_get_info(&num_records, &stored_size);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Getting log info has returned error\n");
        return;
    }

    /* Check that the log state is the same, the item addition just performed
     * is resulted into the removal of the oldest entry, so log size and number
     * of log records is still the same as before
     */
    if (stored_size != INITIAL_LOGGING_SIZE) {
        printk("Expected log size is " STR(INITIAL_LOGGING_SIZE));
        return;
    }

    if (num_records != INITIAL_LOGGING_REQUESTS) {
        printk("Expected log records are " STR(INITIAL_LOGGING_REQUESTS));
        return;
    }

    /* Retrieve the last two log records */
    for (idx=num_records-2; idx<num_records; idx++) {

        struct audit_core_retrieve_input retrieve_input_s =
                                             {.record_index = idx,
                                              .buffer_size = LOCAL_BUFFER_SIZE,
                                              .token = NULL,
                                              .token_size = 0};

        retrieve_output_s.buffer =
            &local_buffer[(idx-(num_records-2))*STANDARD_LOG_ENTRY_SIZE];
        retrieve_output_s.record_size = &record_size;

        /* Retrieve the last two items */
        err = audit_core_retrieve_record_wrapper(&retrieve_input_s,
                                               &retrieve_output_s);

        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Retrieve of last two log records has returned error\n");
            return;
        }

        if (record_size != STANDARD_LOG_ENTRY_SIZE) {
            printk("Expected record size is " STR(STANDARD_LOG_ENTRY_SIZE));
            return;
        }
    }

    /* Inspect the first record retrieved */
    if (retrieved_buffer->id != ( DUMMY_TEST_RECORD_ID_BASE +
                                  (INITIAL_LOGGING_REQUESTS-1) )) {
        printk("Unexpected argument in the second last entry\n");
        return;
    }

    /* Inspect the second record retrieved in the local buffer */
    retrieved_buffer = (struct psa_audit_record *)
        &local_buffer[offsetof(struct log_hdr,size)+STANDARD_LOG_ENTRY_SIZE];

    if (retrieved_buffer->id != ( DUMMY_TEST_RECORD_ID_BASE +
                                  (INITIAL_LOGGING_REQUESTS) )) {
        printk("Unexpected argument in the last entry\n");
        return;
    }

    /* Fill now one big record that will invalidate all existing records */
    record->size = MAX_LOG_RECORD_SIZE;
    record->id = DUMMY_TEST_RECORD_ID_BASE + INITIAL_LOGGING_REQUESTS + 1;

    /* The record has maximum possible payload for log size of 1024 */
    err = audit_core_add_record(record);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Record addition has returned an error\n");
        return;
    }

    /* Get the log size */
    err = audit_core_get_info(&num_records, &stored_size);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Getting log info has returned error\n");
        return;
    }

    /* Check that the log state has one element with maximum size */
    if (stored_size != MAX_LOG_SIZE) {
        printk("Expected log size is " STR(MAX_LOG_SIZE));
        return;
    }

    if (num_records != 1) {
        printk("Expected log records are 1\n");
        return;
    }

    /* Try to retrieve the maximum possible size that fits our buffer.
     * As there is just one big record filling the whole space, nothing
     * will be returned and the API will fail
     */
    struct audit_core_retrieve_input retrieve_input_s_max =
                                         {.record_index = 0,
                                          .buffer_size = LOCAL_BUFFER_SIZE,
                                          .token = NULL,
                                          .token_size = 0};

    retrieve_output_s.buffer = &local_buffer[0];
    retrieve_output_s.record_size = &record_size;

    err = audit_core_retrieve_record_wrapper(&retrieve_input_s_max,
                                           &retrieve_output_s);

    if (err != PSA_AUDIT_ERR_FAILURE) {
        printk("Retrieve of index 0 should fail as it's too big\n");
        return;
    }

    if (record_size != 0) {
        printk("Retrieved log size has unexpected size instead of 0\n");
        return;
    }

    /* Add two standard length records again */
    for (idx=0; idx<2; idx++) {
        record->size = sizeof(struct psa_audit_record) - 4;
        record->id = DUMMY_TEST_RECORD_ID_BASE +
                     INITIAL_LOGGING_REQUESTS + 2 + idx;

        /* The record doesn't contain any payload */
        err = audit_core_add_record(record);
        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Record addition has returned an error\n");
            return;
        }
    }

    /* Get the log size */
    err = audit_core_get_info(&num_records, &stored_size);
    if (err != PSA_AUDIT_ERR_SUCCESS) {
        printk("Getting log info has returned error\n");
        return;
    }

    /* As the log was full, the addition of the last two log records results
     * in the resetting of the log completely. The log will contain only
     * the last two items we have just added.
     */
    if (stored_size != FINAL_LOGGING_SIZE) {
        printk("Expected log size is " STR(FINAL_LOGGING_SIZE));
        return;
    }

    if (num_records != FINAL_LOGGING_REQUESTS) {
        printk("Expected log records are " STR(FINAL_LOGGING_REQUESTS));
        return;
    }

    /* Check the length of each record individually */
    for (idx=0; idx<num_records; idx++) {
        err = audit_core_get_record_info(idx, &stored_size);
        if (err != PSA_AUDIT_ERR_SUCCESS) {
            printk("Getting record size individually has returned error\n");
            return;
        }

        if (stored_size != STANDARD_LOG_ENTRY_SIZE) {
            printk("Unexpected log record size for a single standard item\n");
            return;
        }
    }

    /* Check that if requesting length of a record which is not there fails */
    err = audit_core_get_record_info(num_records, &stored_size);
    if (err != PSA_AUDIT_ERR_FAILURE) {
        printk("Getting record size for non-existent record has not failed\n");
        return;
    }

    printk("tfm_audit_test_1001 Pass!\n");

//    ret->val = TEST_PASSED;
}
