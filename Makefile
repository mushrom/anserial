SRC = anserial.cpp
OBJ = $(SRC:.cpp=.o)

CXXFLAGS += -Wall -std=c++17 -O2

anserial: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	-rm -f anserial $(OBJ)
