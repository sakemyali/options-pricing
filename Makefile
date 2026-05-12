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
VIZ_SRC   := $(SRC_DIR)/viz_main.cpp $(SRC_DIR)/viz_state.cpp
CLI_SRC   := $(filter-out $(VIZ_SRC), $(SRC_FILES))
LIB_SRC   := $(filter-out $(MAIN_SRC) $(VIZ_SRC), $(SRC_FILES))

# Object files
BUILD_DIR := build
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLI_SRC))
LIB_OBJ   := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(LIB_SRC))

# Test files
TEST_DIR  := tests
TEST_SRC  := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ  := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(TEST_SRC))
CATCH_OBJ := $(BUILD_DIR)/catch_amalgamated.o

# Targets
TARGET    := $(BUILD_DIR)/pricing_engine
TEST_BIN  := $(BUILD_DIR)/test_runner
VIZ_BIN   := $(BUILD_DIR)/pricing_viz

# Dependency files
DEPS := $(OBJ_FILES:.o=.d) $(TEST_OBJ:.o=.d) $(CATCH_OBJ:.o=.d)

.PHONY: all test debug clean compare-opt build-O0 viz

all: $(TARGET)

$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Source object files (engine library + main.cpp; excludes viz_*.cpp)
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

build/O0/pricing_engine_O0: $(patsubst $(SRC_DIR)/%.cpp, build/O0/%.o, $(CLI_SRC))
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

# ----------------------------------------------------------------------------
# Visualizer (ImGui + implot + GLFW + OpenGL)
# Requires: brew install glfw  (macOS) or apt install libglfw3-dev (Linux)
# ----------------------------------------------------------------------------

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    GUI_LDFLAGS := -L/opt/homebrew/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    GUI_INCLUDES := -I/opt/homebrew/include
else
    GUI_LDFLAGS := -lglfw -lGL
    GUI_INCLUDES :=
endif

IMGUI_DIR  := third_party/imgui
IMPLOT_DIR := third_party/implot
IMGUI_INCLUDES := -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(IMPLOT_DIR)

IMGUI_SRC  := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/imgui_demo.cpp \
              $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
              $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
IMPLOT_SRC := $(IMPLOT_DIR)/implot.cpp \
              $(IMPLOT_DIR)/implot_items.cpp

IMGUI_OBJ  := $(patsubst third_party/%.cpp, $(BUILD_DIR)/third_party/%.o, $(IMGUI_SRC))
IMPLOT_OBJ := $(patsubst third_party/%.cpp, $(BUILD_DIR)/third_party/%.o, $(IMPLOT_SRC))
VIZ_OBJ    := $(BUILD_DIR)/viz_state.o $(BUILD_DIR)/viz_main.o

# ImGui + implot translation units: relax warnings (their code is C-ish)
$(BUILD_DIR)/third_party/imgui/%.o: third_party/imgui/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -O2 -Wno-unknown-warning-option -Wno-old-style-cast -Wno-conversion -Wno-unused-but-set-variable -Wno-format-nonliteral $(IMGUI_INCLUDES) $(GUI_INCLUDES) -c $< -o $@

$(BUILD_DIR)/third_party/implot/%.o: third_party/implot/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -O2 -Wno-unknown-warning-option -Wno-old-style-cast -Wno-conversion $(IMGUI_INCLUDES) $(GUI_INCLUDES) -c $< -o $@

# Our viz translation units need imgui/implot/glfw includes too
$(BUILD_DIR)/viz_state.o: $(SRC_DIR)/viz_state.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) $(IMGUI_INCLUDES) $(GUI_INCLUDES) -c $< -o $@

$(BUILD_DIR)/viz_main.o: $(SRC_DIR)/viz_main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) $(IMGUI_INCLUDES) $(GUI_INCLUDES) -c $< -o $@

viz: $(VIZ_BIN)

$(VIZ_BIN): $(LIB_OBJ) $(VIZ_OBJ) $(IMGUI_OBJ) $(IMPLOT_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUI_LDFLAGS)

-include $(DEPS)
