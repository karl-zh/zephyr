/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CRYPTO_S_TESTS_H__
#define __CRYPTO_S_TESTS_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "test/framework/test_framework.h"

void register_testsuite_s_crypto_interface(struct test_suite_t *p_test_suite);
void tfm_crypto_test_5001(void);
void tfm_crypto_test_6001(void);
void tfm_crypto_test_6002(void);
void tfm_crypto_test_6003(void);
void tfm_crypto_test_6004(void);
void tfm_crypto_test_6005(void);
void tfm_crypto_test_6006(void);
void tfm_crypto_test_6007(void);
void tfm_crypto_test_6008(void);
void tfm_crypto_test_6009(void);
void tfm_crypto_test_6010(void);

#endif /* __CRYPTO_S_TESTS_H__ */
