BUILD = ./build
SRC = anserial.cpp
OBJ = $(SRC:.cpp=.o)

CXXFLAGS += -Wall -std=c++17 -O2 -I./include

all: dirtree libs bin tests

.PHONY: test

.PHONY: dirtree
dirtree:
	mkdir -p $(BUILD)/{bin,lib,obj,test}

.PHONY: tests
tests: $(BUILD)/bin/anserial
	echo "Testing."

.PHONY: bin
bin: $(BUILD)/bin/anserial

.PHONY: libs
libs: $(BUILD)/lib/anserial.a

$(BUILD)/bin/anserial: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(BUILD)/lib/anserial.a: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	-rm -r $(OBJ) $(BUILD)
