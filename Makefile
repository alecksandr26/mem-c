C = cc
C_FLAGS = -pedantic -Wall -ggdb
C_LIBS_FLAGS = -lexcept

BUILD_DIR = build
OBJ_DIR = $(addprefix $(BUILD_DIR)/, obj)
LIB_DIR = $(addprefix $(BUILD_DIR)/, lib)
MAIN = $(addprefix $(BUILD_DIR)/, main.out)
OBJS = $(addprefix $(OBJ_DIR)/, mem_alloc.o heap.o)

SRC_DIR = src
INCLUDE_DIR = include

$(MAIN): main.c $(OBJS) | $(BUILD_DIR)
	$(C) $(C_FLAGS) $^ -o $@ $(C_LIBS_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(C) $(C_FLAGS) -c $< -o $@ $(C_LIBS_FLAGS)

$(LIB_DIR): $(BUILD_DIR)
	mkdir -p $@

$(OBJ_DIR): $(BUILD_DIR)
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -v -rf $(BUILD_DIR)









