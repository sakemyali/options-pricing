CXX       := c++
CXXFLAGS  := -std=c++17 -Wall -Wextra -Wpedantic -O2
CXXFLAGS_O0 := -std=c++17 -Wall -Wextra -Wpedantic -O0
DBGFLAGS  := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined
DEPFLAGS   = -MMD -MP

INCLUDES  := -Iinclude -Ithird_party

# Source files
SRC_DIR   := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
MAIN_SRC  := $(SRC_DIR)/main.cpp
LIB_SRC   := $(filter-out $(MAIN_SRC), $(SRC_FILES))

# Object files
BUILD_DIR := build
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
LIB_OBJ   := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(LIB_SRC))

# Test files
TEST_DIR  := tests
TEST_SRC  := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ  := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(TEST_SRC))
CATCH_OBJ := $(BUILD_DIR)/catch_amalgamated.o

# Targets
TARGET    := $(BUILD_DIR)/pricing_engine
TEST_BIN  := $(BUILD_DIR)/test_runner

# Dependency files
DEPS := $(OBJ_FILES:.o=.d) $(TEST_OBJ:.o=.d) $(CATCH_OBJ:.o=.d)

.PHONY: all test debug clean compare-opt build-O0

all: $(TARGET)

$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Source object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) -c $< -o $@

# Catch2 amalgamated (compile once, reuse)
$(CATCH_OBJ): third_party/catch2/catch_amalgamated.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -Ithird_party/catch2 -c $< -o $@

# Test object files
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) -Ithird_party/catch2 -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ) $(CATCH_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

debug: CXXFLAGS := $(DBGFLAGS)
debug: clean all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

build-O0: build/O0/pricing_engine_O0

build/O0/pricing_engine_O0: $(patsubst $(SRC_DIR)/%.cpp, build/O0/%.o, $(SRC_FILES))
	$(CXX) $(CXXFLAGS_O0) $^ -o $@

build/O0/%.o: $(SRC_DIR)/%.cpp | build/O0
	$(CXX) $(CXXFLAGS_O0) $(DEPFLAGS) $(INCLUDES) -c $< -o $@

build/O0:
	mkdir -p build/O0

compare-opt: all build-O0
	@echo "=== O0 build (no optimization) ==="
	@./build/O0/pricing_engine_O0 compare-opt-internal --label O0 --paths 1000000
	@echo "=== O2 build (default, optimized) ==="
	@./build/pricing_engine compare-opt-internal --label O2 --paths 1000000
	@echo "=== see throughput numbers above; -O2 should be measurably faster ==="

-include $(DEPS)
