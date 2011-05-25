#pragma xkaapi task input(bar)
static void fu(int bar)
{
}

int main(void)
{
  {
#pragma xkaapi data alloca(bar)
    int bar;
    bar = 42;
    fu(bar);
  }

  return 0;
}
