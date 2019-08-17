BUILD = ./build
LIBSRC = $(wildcard src/*.cpp)
LIBOBJ = $(LIBSRC:.cpp=.o)

MAINSRC = $(wildcard src/anserial/*.cpp)
MAINOBJ = $(MAINSRC:.cpp=.o)

OBJ = $(LIBOBJ) $(MAINOBJ)
CXXFLAGS += -Wall -std=c++17 -O2 -I./include

all: dirtree libs bin tests

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

$(LIBOBJ) $(MAINOBJ): $(BUILD)

$(BUILD)/bin/anserial: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(BUILD)/lib/anserial.a: $(LIBOBJ)
	ar rvs $@ $(LIBOBJ)

.PHONY: clean
clean:
	-rm -r $(OBJ) $(BUILD)
