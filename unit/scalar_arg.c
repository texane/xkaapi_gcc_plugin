#include <stdio.h>
#include <stdlib.h>
#include "kaapi.h"


/* internal builtin function */

static void fu(int, int);

/* struct __xkaapi_fu_args */
/* { */
/*   int bar; */
/*   int baz; */
/* }; */

#if defined(__cplusplus)
extern "C"
#endif
void __xkaapi_dummy_adapter(void* sp, void* thread)
{
  void** const args = (void**)sp;
  printf("%s(%lx, %lx)\n", __FUNCTION__, (uintptr_t)sp, (uintptr_t)thread);

#if 0 /* dont call it yet */
  fu((int)(uintptr_t)args[0], (int)(uintptr_t)args[1]);
#else
  printf("wouldbe_fu: %lx, %lx\n",
	 (int)(uintptr_t)args[0],
	 (int)(uintptr_t)args[1]);
#endif
}


#if 0 /* generated code */

struct __xkaapi_fu_args
{
  int bar;
  int baz;
};

static void __xkaapi_fu_entry(void* sp, void* thread)
{
  struct __xkaapi_args* const args = sp;
  fu(args->bar, args->baz);
}

#endif /* generated code */


#pragma xkaapi task input(bar, baz)
static void fu(int bar, int baz)
{
  printf("%s(%d, %d)\n", __FUNCTION__, bar, baz);
}

int main(void)
{
#if 0 /* generated code */
  {
    struct __xkaapi_fu_args* const __args =
      kaapi_pushdata_aligned(sizeof(struct __xkaapi_fu_args));
    __args->bar = bar;
    __args->baz = baz;
    __kaapi_push_task(kaapi_self_thread(), __args);
    __kaapi_barrier();
  }
#else /* original code */
  printf("before_fu\n");
  fu(24, 42);
  printf("after__fu\n");
#endif
  return 0;
}
