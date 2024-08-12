C = cc
GP = gprof
C_GP_FLAGS = -pg


C_DEBUG_FLAGS = -Wall -Wextra -pedantic -ggdb
C_COMPILE_FLAGS = -O2 -DNDEBUG -fno-stack-protector -z execstack -no-pie -fPIC
C_FLAGS = $(C_GP_FLAGS) $(C_DEBUG_FLAGS)
C_LIBS_FLAGS = -lexcept

AR = ar rc



BUILD_DIR = build
OBJ_DIR = $(addprefix $(BUILD_DIR)/, obj)
LIB_DIR = $(addprefix $(BUILD_DIR)/, lib)
TEST_DIR = $(addprefix $(BUILD_DIR)/, test)
MAIN = $(addprefix $(BUILD_DIR)/, main.out)
OBJS = $(addprefix $(OBJ_DIR)/, heap.o chk.o page.o mem.o)
LIBS = $(addprefix $(LIB_DIR)/, libmem.a)
TESTS = $(addprefix $(TEST_DIR)/, test.out)

SRC_DIR = src
INCLUDE_DIR = include

.PHONY: all, run, clean, compile, test, profile
all: $(MAIN) $(TESTS)

run: $(MAIN)
	./$<

$(MAIN): main.c $(LIBS) | $(BUILD_DIR)
	$(C) $(C_GP_FLAGS) $(C_FLAGS) $^ -o $@ $(C_LIBS_FLAGS)

test: $(TESTS)
	$(foreach test, $(TESTS), ./$(test))

profile: $(MAIN) | run
	$(GP) $< gmon.out
	rm gmon.out

$(TEST_DIR)/%.out: $(SRC_DIR)/%.c  $(LIBS) | $(TEST_DIR)
	$(C) $(C_FLAGS) $^ -o $@ $(C_LIBS_FLAGS) -lunittest

$(TEST_DIR): $(BUILD_DIR)
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(C) $(C_FLAGS) -c $< -o $@ $(C_LIBS_FLAGS)

$(LIB_DIR)/%.a: $(OBJS) | $(LIB_DIR)
	$(AR) $@ $^

$(LIB_DIR): $(BUILD_DIR)
	mkdir -p $@

$(OBJ_DIR): $(BUILD_DIR)
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@


compile: C_FLAGS = $(C_COMPILE_FLAGS)
compile: clean $(MAIN) $(TESTS)

clean:
	rm -v -rf $(BUILD_DIR)









