class fu
{
public:
#pragma xkaapi task input(baz)
  void bar(int baz) {}
};


int main(int ac, char** av)
{
  fu f;
  f.bar(42);
  return 0;
}
