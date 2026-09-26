CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17
LDLIBS   := -lpthread

SRC := $(wildcard *.cpp)
BIN := planificador

$(BIN): $(SRC) $(wildcard *.hpp)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) $(LDLIBS)

clean:
	rm -f $(BIN)

.PHONY: clean