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

# Q = $(if $(filter 1,$V),,@)

# build-image: ; $(info $(M) building docker image) @ ## Run bootstrap for local db
# 	$Q docker build -t media_player .

# run-image: $(build-image)
# 	$Q docker run -it --privileged -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -e PULSE_SERVER=unix:${XDG_RUNTIME_DIR}/pulse/native -v ${XDG_RUNTIME_DIR}/pulse/native:${XDG_RUNTIME_DIR}/pulse/native  --device /dev/dri --device /dev/input -v ~/.config/pulse/cookie:/root/.config/pulse/cookie --device /dev/snd --rm --name my-running-app media_player

all: $(target)

$(target): $(obj)
	$(CXX) -o $@ $^ $(LDLIBS)

-include $(dep)

%.d: %.cpp
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

test: $(target)
	./$(target) test.mp4

.PHONY: clean
clean:
	$(RM) $(obj) $(target) $(dep)

