CXX = g++
CXXFLAGS = -O3 -std=c++14 -D__STDC_CONSTANT_MACROS \
		   -Wall -Wextra -Wextra -pedantic \
		   -Wdisabled-optimization -Wctor-dtor-privacy -Wmissing-declarations \
		   -Woverloaded-virtual -Wshadow -Wno-unused -Winline
LDLIBS = -I'/usr/local/include' -lavformat -lavcodec -lswscale -lavdevice -lSDL2 -lSDL2_image -lz -lm -lpthread -lswresample -lavutil 

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)
target = player

all: $(target)

$(target): $(obj)
	$(CXX) -o $@ $^ $(LDLIBS)

-include $(dep)

%.d: %.cpp
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

test: $(target)
	./$(target) test1.mp4

.PHONY: clean
clean:
	$(RM) $(obj) $(target) $(dep)

