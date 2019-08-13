BUILD = ./build
SRC = anserial.cpp
OBJ = $(SRC:.cpp=.o)

CXXFLAGS += -Wall -std=c++17 -O2 -I./include

all: dirtree $(BUILD)/lib/anserial.a tests

.PHONY: test

.PHONY: dirtree
dirtree:
	mkdir -p $(BUILD)/{bin,lib,obj}

.PHONY: tests
tests: $(BUILD)/bin/anserial
	echo "Testing."

$(BUILD)/bin/anserial: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(BUILD)/lib/anserial.a: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	-rm -r $(OBJ) $(BUILD)
