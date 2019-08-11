CXX = g++
# CXX = clang++
LDFLAGS = -lpthread
CXXFLAGS = -Wall -Wextra -std=c++11

example : example.cc timer_manager.h
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -fv example
