CXX = g++
CXXFLAGS = -O3 -std=c++14 -D__STDC_CONSTANT_MACROS \
		   -Wall -Wextra -Wextra -pedantic \
		   -Wdisabled-optimization -Wctor-dtor-privacy -Wmissing-declarations \
		   -Woverloaded-virtual -Wshadow -Wno-unused -Winline
LDLIBS = -I'/usr/local/include' -lavformat -lavcodec -lswscale -lavdevice -lSDL2 -lSDL2_image -lz -lm -lpthread -lswresample -lavutil 

# https://stackoverflow.com/questions/45491333/how-to-evaluate-once-a-makefile-conditional-variable
DATE    ?= $(shell date '+%Y%m%d%H%M%S')
DATE    := ${DATE}

VERSION ?= $(shell git describe --tags --match=v* 2> /dev/null || \
			cat $(CURDIR)/.version 2> /dev/null || echo v0)

BIN      = $(CURDIR)/bin
BINARY 	:= player
SRC 	= $(wildcard *.cpp)
OBJ 	= $(SRC:.cpp=.o)
TAG     ?= latest

Q = $(if $(filter 1,$V),,@)
V = 0
M = $(shell printf "\033[34;1m▶\033[0m")

# build-image: ; $(info $(M) building docker image) @ ## Run bootstrap for local db
# 	$Q docker build -t media_player .

# run-image: $(build-image)
# 	$Q docker run -it --privileged -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -e PULSE_SERVER=unix:${XDG_RUNTIME_DIR}/pulse/native -v ${XDG_RUNTIME_DIR}/pulse/native:${XDG_RUNTIME_DIR}/pulse/native  --device /dev/dri --device /dev/input -v ~/.config/pulse/cookie:/root/.config/pulse/cookie --device /dev/snd --rm --name my-running-app media_player

build: clean $(BINARY)-$(VERSION)-$(DATE)

# Tools

$(BIN):
	@mkdir -p $@

$(BINARY)-$(VERSION)-$(DATE): ${OBJ} ; $(info $(M) building executable…) @ ## Build program binary
	$(CXX) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(obj) $(target) $(dep)

