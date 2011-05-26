#include <stdio.h>

#pragma xkaapi task
static void fu(void) {}

int main(void)
{
  fu();
  return 0;
}
