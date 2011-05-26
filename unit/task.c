#include <stdio.h>

#pragma xkaapi task
static void fu(void)
{
  printf("%s\n", __FUNCTION__);
}

static void bar(void)
{
  printf("%s\n", __FUNCTION__);
}

#pragma xkaapi task
static void baz(void)
{
  printf("%s\n", __FUNCTION__);
}

int main(void)
{
  fu();
  bar();
  baz();
  return 0;
}
