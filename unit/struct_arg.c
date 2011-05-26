#include <stdio.h>

/* user defined types */
struct triple { int x, y, z; };

struct bar
{
  struct triple tri;
  int alpha;
  int beta;
};


#if 0 /* generated code */
/* TODO */
#endif /* generated code */

#pragma xkaapi task input(b)
static void fu(struct bar* b)
{
  printf("%s(%d, %d)\n", __FUNCTION__, b->alpha, b->tri.y);
}

int main(void)
{
  struct bar b;
  b.alpha = 42;
  b.tri.y = 42;

#if 0 /* generated code */
  /* TODO */
#else /* original code */
  fu(&b);
#endif

  return 0;
}
