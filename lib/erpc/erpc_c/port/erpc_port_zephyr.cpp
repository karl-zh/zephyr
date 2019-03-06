/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"
#include <new>

extern "C" {
//#include "kernel.h"
/**
 * @brief Allocate memory from heap.
 *
 * This routine provides traditional malloc() semantics. Memory is
 * allocated from the heap memory pool.
 *
 * @param size Amount of memory requested (in bytes).
 *
 * @return Address of the allocated memory if successful; otherwise NULL.
 * @req K-HEAP-001
 */
extern void *k_malloc(size_t size);

/**
 * @brief Free memory allocated from heap.
 *
 * This routine provides traditional free() semantics. The memory being
 * returned must have been allocated from the heap memory pool or
 * k_mem_pool_malloc().
 *
 * If @a ptr is NULL, no operation is performed.
 *
 * @param ptr Pointer to previously allocated memory.
 *
 * @return N/A
 * @req K-HEAP-001
 */
extern void k_free(void *ptr);

/**
 * @brief Allocate memory from heap, array style
 *
 * This routine provides traditional calloc() semantics. Memory is
 * allocated from the heap memory pool and zeroed.
 *
 * @param nmemb Number of elements in the requested array
 * @param size Size of each array element (in bytes).
 *
 * @return Address of the allocated memory if successful; otherwise NULL.
 * @req K-HEAP-001
 */
extern void *k_calloc(size_t nmemb, size_t size);

};

using namespace std;
void *operator new(std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(std::size_t count, const std::nothrow_t &tag) THROW
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count, const std::nothrow_t &tag) THROW

{
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr) THROW
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) THROW
{
    erpc_free(ptr);
}

void *erpc_malloc(size_t size)
{
    void *p = k_malloc(size);
    return p;
}

void erpc_free(void *ptr)
{
    k_free(ptr);
}

/* Provide function for pure virtual call to avoid huge demangling code being linked in ARM GCC */
#if ((defined(__GNUC__)) && (defined(__arm__)))
extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}
#endif
