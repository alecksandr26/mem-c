C = cc
GP = gprof

C_DEBUG_FLAGS = -Wall -Wextra -pedantic -ggdb -fPIC
C_COMPILE_FLAGS = -O2 -DNDEBUG -fno-stack-protector -z execstack -no-pie -fPIC
C_FLAGS = $(C_GP_FLAGS) $(C_DEBUG_FLAGS)
C_LIBS_FLAGS = -lexcept
C_FLAGS_WHOLE_ARCHIVE = -Wl,--whole-archive
C_FLAGS_NO_WHOLE_ARCHIVE = -Wl,--no-whole-archive
C_PG_FLAGS = -pg

AR = ar rc

BUILD_DIR = build
OBJ_DIR = $(addprefix $(BUILD_DIR)/, obj)
LIB_DIR = $(addprefix $(BUILD_DIR)/, lib)
TEST_DIR = $(addprefix $(BUILD_DIR)/, test)
EXAMPLE_DIR = $(addprefix $(BUILD_DIR)/, example)
MAIN = $(addprefix $(BUILD_DIR)/, main.out)
OBJS = $(addprefix $(OBJ_DIR)/, heap.o chk.o page.o mem.o mem_dbg.o)
LIBS = $(addprefix $(LIB_DIR)/, libmem.a libmem.so)
TESTS = $(addprefix $(TEST_DIR)/, test.out)
EXAMPLES = $(addprefix $(EXAMPLE_DIR)/, example.out)

SRC_DIR = src
INCLUDE_DIR = include

.PHONY: all, run, clean, compile, test, profile, res
all: $(LIBS) $(MAIN) $(EXAMPLES) $(TESTS)

run: $(MAIN)
	@echo "Running the DEMO of the project"
	./$< > data.txt

res:
	@echo "Printing the already calculated results"
	python3 $(SRC_DIR)/script.py

$(MAIN): main.c $(LIBS) | $(BUILD_DIR)
	$(C) $(C_PG_FLAGS) $(C_FLAGS) $< $(LIB_DIR)/libmem.a -o $@ $(C_LIBS_FLAGS)

test: $(TESTS)
	$(foreach test, $(TESTS), ./$(test))

profile: $(MAIN) | run
	$(GP) $(MAIN) gmon.out
	rm gmon.out

$(TEST_DIR)/%.out: $(SRC_DIR)/%.c  $(LIBS) | $(TEST_DIR)
	$(C) $(C_FLAGS) $< -L./$(LIB_DIR) -lmem -o $@ -lunittest

$(TEST_DIR): $(BUILD_DIR)
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(C) $(C_FLAGS) -c $< -o $@ $(C_LIBS_FLAGS)

$(LIB_DIR)/%.a: $(OBJS) | $(LIB_DIR)
	$(AR) $@ $^
	ranlib $@

$(LIB_DIR)/%.so: $(OBJS) | $(LIB_DIR)
	$(C) -shared -o $@ $(C_FLAGS_WHOLE_ARCHIVE) $^ $(C_LIBS_FLAGS) $(C_FLAGS_NO_WHOLE_ARCHIVE)

$(EXAMPLE_DIR)/%.out: $(SRC_DIR)/%.c $(LIBS) | $(EXAMPLE_DIR)
	$(C) $(C_FLAGS) -o $@ $< -L./$(LIB_DIR) -lmem -Wl,-rpath,./build/lib

$(EXAMPLE_DIR): $(BUILD_DIR)
	mkdir -p $@

$(LIB_DIR): $(BUILD_DIR)
	mkdir -p $@

$(OBJ_DIR): $(BUILD_DIR)
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@


compile: C_FLAGS = $(C_COMPILE_FLAGS)
compile: clean $(MAIN) $(TESTS)


install: compile
	sudo cp $(LIB_DIR)/libmem.so /usr/lib
	sudo cp $(LIB_DIR)/libmem.a /usr/lib
	sudo cp $(INCLUDE_DIR)/mem.h /usr/include

uinstall:
	sudo rm /usr/lib/libmem.so 
	sudo rm /usr/lib/libmem.a
	sudo rm /usr/include/mem.h

clean:
	rm -v -rf $(BUILD_DIR)
	make








