FROM gcc:11.1

USER root

ENV DISPLAY=:0.0
ENV XDG_RUNTIME_DIR=/run/user/1000

RUN apt-get update && apt-get install -y \
    pulseaudio \
    libavformat-dev \
    libavcodec-dev \
    libswscale-dev \
    libavdevice-dev \
    libswresample-dev \
    libavutil-dev \
    libsdl2-2.0-0 \
    libsdl2-image-dev \
    libz-dev \
    libudev-dev

COPY . /usr/src/media_player
WORKDIR /usr/src/media_player
RUN make build

USER 1000
