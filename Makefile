C = cc
GP = gprof

C_BASE_FLAGS = -march=native -mtune=native -mavx2 -msse2 -fPIC
C_DEBUG_FLAGS = -Wall -Wextra -pedantic -ggdb
C_OPTIMIZE_FLAGS = -O3 -flto -DNDEBUG -funroll-loops -fprefetch-loop-arrays \
                   -finline-functions -fno-plt -ftree-vectorize -ffast-math
C_OMIT_FP = -fomit-frame-pointer
C_PG_FLAGS = 
C_EXTRA_FLAGS =

C_FLAGS = $(C_BASE_FLAGS) $(C_EXTRA_FLAGS) $(C_PG_FLAGS)
C_LIBS_FLAGS = -lexcept
C_FLAGS_WHOLE_ARCHIVE = -Wl,--whole-archive
C_FLAGS_NO_WHOLE_ARCHIVE = -Wl,--no-whole-archive

AR = ar rc

M = makepkg
M_FLAGS = -f --config .makepkg.conf --skipinteg --noextract

BUILD_DIR = build
OBJ_DIR = $(addprefix $(BUILD_DIR)/, obj)
LIB_DIR = $(addprefix $(BUILD_DIR)/, lib)
TEST_DIR = $(addprefix $(BUILD_DIR)/, test)
EXAMPLE_DIR = $(addprefix $(BUILD_DIR)/, example)
UPLOAD_DIR = $(addprefix $(BUILD_DIR)/, upload)
MAIN = $(addprefix $(BUILD_DIR)/, main.out)
OBJS = $(addprefix $(OBJ_DIR)/, heap.o chk.o page.o mem.o mem_dbg.o)
LIBS = $(addprefix $(LIB_DIR)/, libmem.a libmem.so)
TESTS = $(addprefix $(TEST_DIR)/, test.out)
EXAMPLES = $(addprefix $(EXAMPLE_DIR)/, example.out example_ralloc.out example_mem_copy.out)

SRC_DIR = src
INCLUDE_DIR = include

PKGNAME = mem-c
GCU = ssh://aur@aur.archlinux.org/$(PKGNAME).git # git clone

.PHONY: all, run, clean, compile, test, profile, res, pkg
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

example: $(EXAMPLES)
	$(foreach example, $(EXAMPLES), ./$(example))

profile: C_PG_FLAGS = -pg
profile: C_EXTRA_FLAGS = $(C_DEBUG_FLAGS)
profile: $(MAIN) | run
	$(GP) $(MAIN) gmon.out
	rm gmon.out

# TODO: Broken until we create another test and fix and adapt to another new unit test library
$(TEST_DIR)/%.out: $(SRC_DIR)/%.c  $(LIBS) | $(TEST_DIR)
	$(C) $(C_FLAGS) $< $(LIB_DIR)/libmem.a -o $@ -lunittest $(C_LIBS_FLAGS)

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

$(UPLOAD_DIR): $(BUILD_DIR)
	mkdir -p $@


$(BUILD_DIR):
	mkdir -p $@


compile: C_EXTRA_FLAGS = $(C_OPTIMIZE_FLAGS) $(C_OMIT_FP)
compile: clean $(MAIN)


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

pkg:
	$(M) $(M_FLAGS)


$(UPLOAD_DIR)/$(PKGNAME): $(UPLOAD_DIR)
	cd $< && git clone $(GCU)

upload-aur: $(UPLOAD_DIR)/$(PKGNAME)
	cp PKGBUILD $</
	cd $</ && $(M) --printsrcinfo > .SRCINFO
	cd $</ && git add PKGBUILD .SRCINFO
	echo -n "Commit-msg: "
	@read commitmsg
	cd $</ && git commit -m commitmsg
	cd $</ && git push


