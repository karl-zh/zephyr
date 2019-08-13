/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

//#include "test/framework/test_framework_helpers.h"
#include "tfm_api.h"
#include "../crypto_tests_common.h"
#if 0
static struct test_t crypto_tests[] = {
    {&tfm_crypto_rpc_test_6001, "tfm_crypto_rpc_test_6001",
     "Non Secure Key management interface", {0} },
    {&tfm_crypto_rpc_test_6002, "tfm_crypto_rpc_test_6002",
     "Non Secure Symmetric encryption (AES-128-CBC) interface", {0} },
    {&tfm_crypto_rpc_test_6003, "tfm_crypto_rpc_test_6003",
     "Non Secure Symmetric encryption (AES-128-CFB) interface", {0} },
    {&tfm_crypto_rpc_test_6005, "tfm_crypto_rpc_test_6005",
     "Non Secure Symmetric encryption (AES-128-CTR) interface", {0} },
    {&tfm_crypto_rpc_test_6007, "tfm_crypto_rpc_test_6007",
     "Non Secure Symmetric encryption invalid cipher (AES-128-GCM)", {0} },
    {&tfm_crypto_rpc_test_6008, "tfm_crypto_rpc_test_6008",
     "Non Secure Symmetric encryption invalid cipher (AES-152-CBC)", {0} },
    {&tfm_crypto_rpc_test_6009, "tfm_crypto_rpc_test_6009",
     "Non Secure Symmetric encryption invalid cipher (HMAC-128-CFB)", {0} },
    {&tfm_crypto_rpc_test_6010, "tfm_crypto_rpc_test_6010",
     "Non Secure Hash (SHA-1) interface", {0} },
    {&tfm_crypto_rpc_test_6011, "tfm_crypto_rpc_test_6011",
     "Non Secure Hash (SHA-224) interface", {0} },
    {&tfm_crypto_rpc_test_6012, "tfm_crypto_rpc_test_6012",
     "Non Secure Hash (SHA-256) interface", {0} },
    {&tfm_crypto_rpc_test_6013, "tfm_crypto_rpc_test_6013",
     "Non Secure Hash (SHA-384) interface", {0} },
    {&tfm_crypto_rpc_test_6014, "tfm_crypto_rpc_test_6014",
     "Non Secure Hash (SHA-512) interface", {0} },
    {&tfm_crypto_rpc_test_6019, "tfm_crypto_rpc_test_6019",
     "Non Secure HMAC (SHA-1) interface", {0} },
    {&tfm_crypto_rpc_test_6020, "tfm_crypto_rpc_test_6020",
     "Non Secure HMAC (SHA-256) interface", {0} },
    {&tfm_crypto_rpc_test_6021, "tfm_crypto_rpc_test_6021",
     "Non Secure HMAC (SHA-384) interface", {0} },
    {&tfm_crypto_rpc_test_6022, "tfm_crypto_rpc_test_6022",
     "Non Secure HMAC (SHA-512) interface", {0} },
    {&tfm_crypto_rpc_test_6024, "tfm_crypto_rpc_test_6024",
     "Non Secure HMAC with long key (SHA-1) interface", {0} },
    {&tfm_crypto_rpc_test_6030, "tfm_crypto_rpc_test_6030",
     "Non Secure AEAD (AES-128-CCM) interface", {0} },
    {&tfm_crypto_rpc_test_6031, "tfm_crypto_rpc_test_6031",
     "Non Secure AEAD (AES-128-GCM) interface", {0} },
    {&tfm_crypto_rpc_test_6032, "tfm_crypto_rpc_test_6032",
     "Non Secure key policy interface", {0} },
    {&tfm_crypto_rpc_test_6033, "tfm_crypto_rpc_test_6033",
     "Non Secure key policy check permissions", {0} },
};

void register_testsuite_ns_crypto_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(crypto_tests) / sizeof(crypto_tests[0]));

    set_testsuite("Crypto non-secure interface test (tfm_crypto_rpc_test_6XXX)",
                  crypto_tests, list_size, p_test_suite);
}
#endif

struct test_result_t test_result;
struct test_result_t *ret = &test_result;

/**
 * \brief Non-Secure interface test for Crypto
 *
 * \details The scope of this set of tests is to functionally verify
 *          the interfaces specified by psa/crypto.h are working
 *          as expected. This is not meant to cover all possible
 *          scenarios and corner cases.
 *
 */
void tfm_crypto_rpc_test_6001(void)
{
    ret->val = TEST_FAILED;
    psa_key_interface_test(PSA_KEY_TYPE_AES, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6002(void)
{
    ret->val = TEST_FAILED;
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CBC_NO_PADDING, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6003(void)
{
    ret->val = TEST_FAILED;
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CFB, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6005(void)
{
    ret->val = TEST_FAILED;
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CTR, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6007(void)
{
    ret->val = TEST_FAILED;
    /* GCM is an AEAD mode */
    psa_invalid_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_GCM, 16, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6008(void)
{
    ret->val = TEST_FAILED;
    psa_invalid_key_length_test(ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6009(void)
{
    ret->val = TEST_FAILED;
    /* HMAC is not a block cipher */
    psa_invalid_cipher_test(PSA_KEY_TYPE_HMAC, PSA_ALG_CFB, 16, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6010(void)
{
    ret->val = TEST_FAILED;
    psa_hash_test(PSA_ALG_SHA_1, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6011(void)
{
    ret->val = TEST_FAILED;
    psa_hash_test(PSA_ALG_SHA_224, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6012(void)
{
    ret->val = TEST_FAILED;
    psa_hash_test(PSA_ALG_SHA_256, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6013(void)
{
    psa_hash_test(PSA_ALG_SHA_384, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6014(void)
{
    ret->val = TEST_FAILED;
    psa_hash_test(PSA_ALG_SHA_512, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6019(void)
{
    ret->val = TEST_FAILED;
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_1), 0, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6020(void)
{
    ret->val = TEST_FAILED;
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_256), 0, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6021(void)
{
    ret->val = TEST_FAILED;
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_384), 0, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6022(void)
{
    ret->val = TEST_FAILED;
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_512), 0, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}
void tfm_crypto_rpc_test_6024(void)
{
    ret->val = TEST_FAILED;
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_1), 1, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6030(void)
{
    ret->val = TEST_FAILED;
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6031(void)
{
    ret->val = TEST_FAILED;
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_GCM, ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6032(void)
{
    ret->val = TEST_FAILED;
    psa_policy_key_interface_test(ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}

void tfm_crypto_rpc_test_6033(void)
{
    ret->val = TEST_FAILED;
    psa_policy_invalid_policy_usage_test(ret);

    if (ret->val == TEST_PASSED)
        printk("Test %s pass!\n", __func__);
    else
        printk("Test %s Fail!\n", __func__);
}
