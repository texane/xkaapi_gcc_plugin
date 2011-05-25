#include <stdio.h>

// class
// {
// #pragma xkaapi task input(bar)
//   void baz(int bar)
//   {
//   }
// };

#pragma xkaapi task input(bar)
template<typename T> void fu(T bar, T baz)
{
}

#pragma xkaapi task input(bar)
template<typename T> void fu(T* bar, T* baz)
{
}

template<> void fu(double bar, double baz)
{
}

int main(void)
{
  fu(452, 908);
  {
    fu(42, 42);

#pragma xkaapi data alloca(bar)
    int bar = 42;

    {
      int bar;
    }

#pragma xkaapi data alloca(baz)
    int baz = 24;

    fu(bar, baz);

    fu((double)bar, (double)baz);
  }

  return 0;
}
