#include <stdio.h>
#include <kernel/defines.h>

ENGINE_API void sampleExported(void)
{
  printf("[KERNEL] : sampleExported() called successfully!\n");
}

void sampleHidden(void)
{
  printf("[KERNEL] : sampleHidden() called!\n");
}
