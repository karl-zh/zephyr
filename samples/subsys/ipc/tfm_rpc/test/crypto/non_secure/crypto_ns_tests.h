/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CRYPTO_NS_TESTS_H__
#define __CRYPTO_NS_TESTS_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "test/framework/test_framework.h"

/**
 * \brief Register testsuite for Crypto non-secure interface.
 *
 * \param[in] p_test_suite The test suite to be executed.
 */
void register_testsuite_ns_crypto_interface(struct test_suite_t *p_test_suite);


/* List of tests */
void tfm_crypto_rpc_test_6001(void);
void tfm_crypto_rpc_test_6002(void);
void tfm_crypto_rpc_test_6003(void);
void tfm_crypto_rpc_test_6005(void);
void tfm_crypto_rpc_test_6007(void);
void tfm_crypto_rpc_test_6008(void);
void tfm_crypto_rpc_test_6009(void);
void tfm_crypto_rpc_test_6010(void);
void tfm_crypto_rpc_test_6011(void);
void tfm_crypto_rpc_test_6012(void);
void tfm_crypto_rpc_test_6013(void);
void tfm_crypto_rpc_test_6014(void);
void tfm_crypto_rpc_test_6019(void);
void tfm_crypto_rpc_test_6020(void);
void tfm_crypto_rpc_test_6021(void);
void tfm_crypto_rpc_test_6022(void);
void tfm_crypto_rpc_test_6024(void);
void tfm_crypto_rpc_test_6030(void);
void tfm_crypto_rpc_test_6031(void);
void tfm_crypto_rpc_test_6032(void);
void tfm_crypto_rpc_test_6033(void);
#ifdef __cplusplus
}
#endif

#endif /* __CRYPTO_NS_TESTS_H__ */
