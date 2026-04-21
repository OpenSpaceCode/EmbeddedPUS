CC ?= cc
# Minimal Makefile: only build and run the unit test binary.
CFLAGS ?= -O2 -Iinclude -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
		  -Wcast-align -Wcast-qual -Wpointer-arith -Wformat=2 \
		  -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wundef \
		  -std=c11
BUILD_DIR = build
CTEST_PATH = $(BUILD_DIR)/tests/ctest

all: ctest

ctest: $(CTEST_PATH)

$(CTEST_PATH): tests/unit_tests.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iinclude tests/unit_tests.c -o $(CTEST_PATH)

run: ctest
	$(CTEST_PATH)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all ctest run clean