#include <stdio.h>

#pragma xkaapi task
static void fu(void) {}

static void bar(void) {}

#pragma xkaapi task
static void baz(void) {}

int main(void)
{
  fu();
  bar();
  baz();
  return 0;
}
