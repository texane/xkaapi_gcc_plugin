#include <stdio.h>

#pragma xkaapi task input(bar)
template<typename T> void fu(T bar, T baz)
{
}

int main(void)
{
  fu<int>(42, 42);
  fu<double>(42.f, 42.f);
  return 0;
}
