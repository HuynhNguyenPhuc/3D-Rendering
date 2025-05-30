CXX := g++
CXXFLAGS := -std=c++17 -Wall -I./utils
LDFLAGS := -L./utils -lutils -Wl,-rpath=./utils


SRCS := whitted_ray_tracing.cpp rendering.cpp path_tracing.cpp forward_ray_marching.cpp backward_ray_marching.cpp
OBJS := $(SRCS:.cpp=.o)
TARGETS := $(SRCS:.cpp=.exe)


all: $(TARGETS)


%.exe: %.o utils
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


utils:
	$(MAKE) -C utils all

utils-clean:
	$(MAKE) -C utils clean


clean:
	rm -f $(OBJS) $(TARGETS)
	$(MAKE) -C utils clean

.PHONY: all clean utils utils-clean