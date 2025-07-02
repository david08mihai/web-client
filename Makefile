EXEC = client

SRC = client.cpp buffer.cpp helpers.cpp requests.cpp

HDR = buffer.h helpers.h requests.hpp

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

all: $(EXEC)

$(EXEC): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

clean:
	rm -f $(EXEC)

rebuild: clean all
