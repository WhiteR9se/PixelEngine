# - - - | Definitions | - - -

MODE          ?= debug

# - - - Directories - - -
INCLUDE_DIR   := include
SRC_DIR       := src
TEST_DIR      := tests
BIN_DIR       := bin
BUILD_DIR     := .build/$(MODE)

# - - - Compiler & Flags - - -
CC            := clang
COMMON_FLAGS  := -std=c11 -Wall -Werror -Wpedantic
DEBUG_FLAGS   := -O0 -g
RELEASE_FLAGS := -O3

CPPFLAGS      := -I$(INCLUDE_DIR) -MMD -MP

ifeq ($(MODE),debug)
  CFLAGS   := $(COMMON_FLAGS) $(DEBUG_FLAGS)
  CPPFLAGS += -DDEBUG
  $(info Build mode: DEBUG)
else ifeq ($(MODE),release)
  CFLAGS   := $(COMMON_FLAGS) $(RELEASE_FLAGS)
  $(info Build mode: RELEASE)
else
  $(error Unknown build mode: '$(MODE)'. Must be 'debug' or 'release')
endif
$(info )


# - - - File Finding - - -

MAIN_SRCS   := $(SRC_DIR)/editor/editor.c
RAW_SRCS    := $(shell find $(SRC_DIR) -name '*.c')
COMMON_SRCS := $(filter-out $(MAIN_SRCS), $(RAW_SRCS))
TEST_SRCS   := $(shell find $(TEST_DIR) -name '*.c')
ALL_SRCS    := $(COMMON_SRCS) $(MAIN_SRCS) $(TEST_SRCS)

COMMON_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(COMMON_SRCS))
MAIN_OBJS   := $(patsubst %.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))
TEST_OBJS   := $(patsubst %.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))
ALL_OBJS    := $(patsubst %.c, $(BUILD_DIR)/%.o, $(ALL_SRCS))

MAIN_BINS   := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(MAIN_SRCS))
TEST_BINS   := $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/tests/%, $(TEST_SRCS))

HEADER_DEPS := $(ALL_OBJS:.o=.d)


# - - - | Building | - - -

# - - - Targets - - -

.PHONY: all tests runTests clean remake

all: $(MAIN_BINS) tests

tests: $(TEST_BINS)

runTests: tests
	@for test_exec in $(TEST_BINS); do \
		echo ""; \
		echo "Running $$test_exec..."; \
		./$$test_exec || exit 1; \
		echo ""; \
	done

clean:
	@rm -rf $(BIN_DIR) $(BUILD_DIR)
	@echo "CLEANED"
	@echo ""

remake: clean all


# - - - Linking rules - - -

$(MAIN_BINS): $(BIN_DIR)/%: $(BUILD_DIR)/$(SRC_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "[BINARY]: $@"
	@echo "  ├── $(BUILD_DIR)/$(SRC_DIR)/$*.o"
	@for obj in $(COMMON_OBJS); do \
		echo "  └── $$obj"; \
	done
	@echo ""

$(TEST_BINS): $(BIN_DIR)/tests/%: $(BUILD_DIR)/$(TEST_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "[TEST]: $@"
	@echo "  ├── $(BUILD_DIR)/$(TEST_DIR)/$*.o"
	@for obj in $(COMMON_OBJS); do \
		echo "  └── $$obj"; \
	done
	@echo ""

# - - - Object compilation rules - - -

$(ALL_OBJS): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  [CC]   $<"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(HEADER_DEPS)
