CC ?= cc
# Minimal Makefile: only build and run the unit test binary.
CFLAGS ?= -O2 -Iinclude -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
		  -Wcast-align -Wcast-qual -Wpointer-arith -Wformat=2 \
		  -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wundef \
		  -std=c11
BUILD_DIR = build
CTEST_PATH = $(BUILD_DIR)/tests/ctest
SRC_FILES = src/pus.c src/pus_codec.c src/pus_context.c src/pus_handler.c
TEST_FILES = tests/unit_tests.c tests/test_pus_types.c tests/test_pus_codec.c tests/test_pus_handler.c

all: ctest

ctest: $(CTEST_PATH)

$(CTEST_PATH): $(TEST_FILES) $(SRC_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_FILES) $(SRC_FILES) -o $(CTEST_PATH)

run: ctest
	$(CTEST_PATH)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all ctest run clean
