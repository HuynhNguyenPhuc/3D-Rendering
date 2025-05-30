CXX := g++
CXXFLAGS := -std=c++17 -O3 -Wall -fPIC -MMD -MP
LDFLAGS := -Wl,--allow-shlib-undefined

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := libutils.so

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

-include $(DEPS)

clean:
	rm -f *.o *.d $(TARGET)