CXX = g++
CFLAGS = -std=c++17 -Wall -Os

symbol_finder: symbol_finder.cpp
	$(CXX) $< -o $@ $(CFLAGS)

clean:
	rm symbol_finder
