CC ?= cc
CFLAGS ?= -O2 -std=c99 \
          -Iinclude -Itests \
          -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
          -Wcast-align -Wcast-qual -Wpointer-arith -Wformat=2 \
          -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wundef

BUILD_DIR    = build
CTEST_PATH   = $(BUILD_DIR)/tests/ctest
EXAMPLE_PATH = $(BUILD_DIR)/example/obc_example

SRC_FILES  = $(wildcard src/*.c)
TEST_FILES = $(wildcard tests/*.c)

SRC_OBJS   = $(patsubst src/%.c,   $(BUILD_DIR)/src/%.o,   $(SRC_FILES))
TEST_OBJS  = $(patsubst tests/%.c, $(BUILD_DIR)/tests/%.o, $(TEST_FILES))

.PHONY: all ctest example clean

all: ctest

ctest: $(CTEST_PATH)
	$(CTEST_PATH)

$(CTEST_PATH): $(SRC_OBJS) $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%.o: tests/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

example: $(EXAMPLE_PATH)
	$(EXAMPLE_PATH)

$(EXAMPLE_PATH): $(SRC_OBJS) $(BUILD_DIR)/example/obc_example.o
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/example/%.o: example/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

coverage-html:
	bash tools/coverage_html.sh