#ifndef KAAPI_H_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(__cplusplus)
extern "C"
#endif
void* kaapi_pushdata_aligned(size_t size)
{
  printf("%s(%lu)\n", __FUNCTION__, size);
  return malloc(size);
}

#if defined(__cplusplus)
extern "C"
#endif
void kaapi_pushtask(void* thread, void* sp)
{
  printf("%s(%lx, %lx)\n", __FUNCTION__, (uintptr_t)thread, (uintptr_t)sp);
}

#if defined(__cplusplus)
extern "C"
#endif
void kaapi_barrier(void)
{
  printf("%s\n", __FUNCTION__);
}

#endif /* KAAPI_H_INCLUDED */
