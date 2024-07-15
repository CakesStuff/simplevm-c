SOURCES = $(shell find src -type f -name "*.c")
HEADERS = $(shell find include -type f -name "*.h")
OBJECTS = $(patsubst src/%.c, build/obj/%.o, $(SOURCES))

default: build/dcc

build:
	mkdir -p $@
build/obj: | build
	mkdir -p $@

build/obj/%.o: src/%.c $(HEADERS) | build/obj
	gcc -Wall -Iinclude -O3 -c $< -o $@
build/dcc: $(OBJECTS) | build
	gcc $^ -o $@

clean:
	rm -rf build

.PHONY: default clean

