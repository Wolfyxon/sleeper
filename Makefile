OUT_NAME := sleeper
VERSION := 1.0

CC := gcc
SOURCES := src/*.h src/*.c

LIBS := x11 xi xext playerctl glib-2.0 gobject-2.0
LIB_CFLAGS := $(shell pkg-config --cflags --libs $(LIBS))
CFLAGS := -Wall -Os $(LIB_CFLAGS) -DPROGRAM_VERSION=\"$(VERSION)\" $(CFLAGS)
FLAGS_TXT_FILE := compile_flags.txt

.PHONY: build dev test clean

build: dev
	$(CC) $(SOURCES) $(CFLAGS) -o $(OUT_NAME)

test:
	@CFLAGS="-DRUN_TESTS" make
	@echo
	@./$(OUT_NAME)

clean:
	rm -f $(FLAGS_TXT_FILE)
	rm -f $(OUT_NAME)

run: build
	./$(OUT_NAME) --verbose

dev: $(FLAGS_TXT_FILE)

$(FLAGS_TXT_FILE):
	@echo $(CFLAGS) | xargs -n1 > $(FLAGS_TXT_FILE)
	@echo Created "$(FLAGS_TXT_FILE)" for clangd. Restart your editor to fix include errors
