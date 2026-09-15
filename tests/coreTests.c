#include <kernel/testing/testManager.h>
#include <kernel/testing/expect.h>
#include <kernel/dataStructures/bitset.h>
#include <kernel/ecs/stupidSimple.h>
#include <kernel/memory/tracker.h>

// ==========================================
//              BITSET TESTS
// ==========================================

uint8_t testBitsetInitializationAndAccess(void)
{
  Bitset set;
  // Initialize with 64 capacity, letting it allocate its own memory
  bool created = bitSetCreate(&set, 64, NULL);
  EXPECT_TO_BE_TRUE(created);

  // Bits should default to 0
  EXPECT_TO_BE_FALSE(bitsetGet(&set, 10));
  
  // Test Setting
  bitsetSet(&set, 10);
  EXPECT_TO_BE_TRUE(bitsetGet(&set, 10));

  // Test Clearing
  bitsetClear(&set, 10);
  EXPECT_TO_BE_FALSE(bitsetGet(&set, 10));

  // Test Toggling
  bitsetToggle(&set, 42);
  EXPECT_TO_BE_TRUE(bitsetGet(&set, 42));

  bitsetDestroy(&set);
  return ENGINE_TEST_PASS;
}

uint8_t testBitsetOperations(void)
{
  Bitset setA, setB;
  bitSetCreate(&setA, 64, NULL);
  bitSetCreate(&setB, 64, NULL);

  bitsetSet(&setA, 1);
  bitsetSet(&setA, 2);
  
  bitsetSet(&setB, 2);
  bitsetSet(&setB, 3);

  // Test Intersection (A & B). Only bit 2 should survive.
  bitsetIntersection(&setA, &setB);
  EXPECT_TO_BE_FALSE(bitsetGet(&setA, 1));
  EXPECT_TO_BE_TRUE(bitsetGet(&setA, 2));
  EXPECT_TO_BE_FALSE(bitsetGet(&setA, 3));

  bitsetDestroy(&setA);
  bitsetDestroy(&setB);
  return ENGINE_TEST_PASS;
}

// ==========================================
//                ECS TESTS
// ==========================================

typedef struct {
  float current;
  float max;
} Health;

typedef struct {
  float damage;
  int   magazine;
} Weapon;

uint8_t testECSCoreMechanics(void)
{
  memorySetLimit(1024 * 1024 * 2, MEMORY_TAG_ECS_COMPONENT);
  ecsInit();

  // Test Entity Creation
  Entity player = ecsCreateEntity();
  EXPECT_TO_BE_TRUE(ecsIsEntityValid(player));

  // Test Component Registration
  ComponentType compHealth = ecsRegisterComponent(sizeof(Health));
  ComponentType compWeapon = ecsRegisterComponent(sizeof(Weapon));

  // Test Component Addition
  Health playerHealth = { 100.0f, 100.0f };
  ecsAddComponent(player, compHealth, &playerHealth);
  EXPECT_TO_BE_TRUE(ecsHasComponent(player, compHealth));

  Weapon vandal = { 40.0f, 25 }; 
  ecsAddComponent(player, compWeapon, &vandal);
  EXPECT_TO_BE_TRUE(ecsHasComponent(player, compWeapon));

  // Test Component Retrieval and Data Integrity
  Weapon* equipped = (Weapon*)ecsGetComponent(player, compWeapon);
  EXPECT_TO_BE_NOT_NULL(equipped);
  EXPECT_FLOAT_TO_BE(40.0f, equipped->damage, 0.001f);
  EXPECT_TO_BE(25, equipped->magazine);

  // Test Component Removal
  ecsRemoveComponent(player, compWeapon);
  EXPECT_TO_BE_FALSE(ecsHasComponent(player, compWeapon));

  ecsDestroy();
  return ENGINE_TEST_PASS;
}

int main(void)
{
  // Group 1: Data Structures
  testRegister(testBitsetInitializationAndAccess, "Bitset: Set, Get, and Clear", 1);
  testRegister(testBitsetOperations, "Bitset: Intersections", 1);

  // Group 2: Entity Component System
  testRegister(testECSCoreMechanics, "ECS: Entity and Component Lifecycle", 2);

  return testRunAll();
}