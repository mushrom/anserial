BUILD = ./build
LIBSRC = $(wildcard src/*.cpp)
LIBOBJ = $(LIBSRC:.cpp=.o)

MAINSRC = $(wildcard src/anserial/*.cpp)
MAINOBJ = $(MAINSRC:.cpp=.o)

EXAMPLE_SRC = $(wildcard examples/*.cpp)
EXAMPLE_BIN = $(EXAMPLE_SRC:.cpp=)

OBJ = $(LIBOBJ) $(MAINOBJ)
CXXFLAGS += -Wall -std=c++17 -O2 -I./include

all: dirtree libs bin tests examples

.PHONY: test

.PHONY: dirtree
dirtree: $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)/{bin,lib,obj,test}

.PHONY: tests
tests: $(BUILD)/bin/anserial
	echo "Testing."

.PHONY: bin
bin: $(BUILD)/bin/anserial

.PHONY: libs
libs: $(BUILD)/lib/anserial.a

$(EXAMPLE_BIN) : $(BUILD)/lib/anserial.a $(EXAMPLE_SRC)
	@# XXX: easier than messing around with make patterns for now...
	@for thing in $(EXAMPLE_BIN); do \
		echo CXX $$thing; \
		$(CXX) $(CXXFLAGS) $$thing.cpp -o $$thing $(BUILD)/lib/anserial.a; \
	done;

.PHONY: examples
examples: $(EXAMPLE_BIN)

$(LIBOBJ) $(MAINOBJ): $(BUILD)

$(BUILD)/bin/anserial: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(BUILD)/lib/anserial.a: $(LIBOBJ)
	ar rvs $@ $(LIBOBJ)

.PHONY: clean
clean:
	-rm -r $(OBJ) $(BUILD) $(EXAMPLE_BIN)
