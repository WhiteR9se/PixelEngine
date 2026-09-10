#include <kernel/testing/testManager.h>
#include <kernel/testing/expect.h>
#include <stdint.h>

uint8_t func(void)
{
  char c = 'A';
  EXPECT_TO_BE('A', c);

  return ENGINE_TEST_PASS;
}

int main(int argc, char *argv[])
{
  testRegister(func, "Sample", 1);
  return testRunAll();
}
