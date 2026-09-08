#include <stdio.h>
#include <kernel/sample.h>

int main(void)
{
  printf("[EDITOR]: Starting...\n");
  sampleExported();
  //sampleHidden();
  return 0;
}
