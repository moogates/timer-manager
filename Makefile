CXX = g++
# CXX = clang++
LDFLAGS = -lpthread
CXXFLAGS = -Wall -Wextra -std=c++11

example : example.cc timer_manager.cc
	$(CXX) example.cc timer_manager.cc $(CXXFLAGS) $(LDFLAGS) -o $@ 

clean:
	rm -fv example
