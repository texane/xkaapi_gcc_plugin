#include <stdio.h>

#if 0 /* generated code */

struct __xkaapi_fu_args
{
  int bar;
  int baz;
};

static void __xkaapi_fu_entry(void* sp)
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
    kaapi_push_task(kaapi_self_thread(), __args);
    kaapi_barrier();
  }
#else /* original code */
  fu(42, 42);
#endif
  return 0;
}
