#include <stdio.h>

#pragma xkaapi task input(bar)
static void fu(int bar, int baz)
{
}

int main(void)
{
  {
#pragma xkaapi data alloca(bar)
    int bar = 42;

#pragma xkaapi data alloca(baz)
    int baz = 24;

    fu(bar, baz);
  }

  return 0;
}
