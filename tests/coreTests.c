#include <string.h>
#define JUST_LIB_IMPL_ALL
#include <kernel/justLibrary.h>

// ==========================================
//              BITSET TESTS
// ==========================================

JustTestResult testBitsetInitializationAndAccess(void)
{
  JustBitset set;
  // Initialize with 64 capacity, letting it allocate its own memory
  bool created = justBitsetCreate(&set, 64, NULL, "BITSET_INITIALIZE");
  JUST_EXPECT_TO_BE_TRUE(created);

  // Bits should default to 0
  JUST_EXPECT_TO_BE_FALSE(justBitsetGet(&set, 10));
  
  // Test Setting
  justBitsetSet(&set, 10);
  JUST_EXPECT_TO_BE_TRUE(justBitsetGet(&set, 10));

  // Test Clearing
  justBitsetClear(&set, 10);
  JUST_EXPECT_TO_BE_FALSE(justBitsetGet(&set, 10));

  // Test Toggling
  justBitsetToggle(&set, 42);
  JUST_EXPECT_TO_BE_TRUE(justBitsetGet(&set, 42));

  justBitsetDestroy(&set);
  return JUST_TEST_PASS;
}

JustTestResult testBitsetOperations(void)
{
  JustBitset setA, setB;
  justBitsetCreate(&setA, 64, NULL, "BITSET_OPS");
  justBitsetCreate(&setB, 64, NULL, "BITSET_OPS");

  justBitsetSet(&setA, 1);
  justBitsetSet(&setA, 2);
  
  justBitsetSet(&setB, 2);
  justBitsetSet(&setB, 3);

  // Test Intersection (A & B). Only bit 2 should survive.
  justBitsetIntersection(&setA, &setB);
  JUST_EXPECT_TO_BE_FALSE(justBitsetGet(&setA, 1));
  JUST_EXPECT_TO_BE_TRUE(justBitsetGet(&setA, 2));
  JUST_EXPECT_TO_BE_FALSE(justBitsetGet(&setA, 3));

  justBitsetDestroy(&setA);
  justBitsetDestroy(&setB);
  return JUST_TEST_PASS;
}



int main(void)
{
  // Group 1: Data Structures
  justTestRegister(testBitsetInitializationAndAccess, "Bitset: Set, Get, and Clear", 1);
  justTestRegister(testBitsetOperations, "Bitset: Intersections", 1);

  // Group 2: Entity Component System
  //justTestRegister(testECSCoreMechanics, "ECS: Entity and Component Lifecycle", 2);

  return justTestRunAll();
}
