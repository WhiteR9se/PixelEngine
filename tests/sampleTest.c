#define JUST_LIB_IMPL_ALL
#include <kernel/justLibrary.h>

JustTestResult func(void)
{
  char c = 'A';
  JUST_EXPECT_TO_BE('A', c);

  return JUST_TEST_PASS;
}

int main(int argc, char *argv[])
{
  justTestRegister(func, "Sample", 1);
  return justTestRunAll();
}
