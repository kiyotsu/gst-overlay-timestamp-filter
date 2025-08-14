FROM restreamio/gstreamer:latest-prod AS builder

RUN apt-get update && \
    apt-get install -y \
        libgstreamer1.0-dev \
        libcairo2-dev \
        build-essential \
        cmake

COPY src /usr/src/gst-plugin/src
COPY CMakeLists.txt /usr/src/gst-plugin

WORKDIR /usr/src/gst-plugin
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make


FROM python:3.12-slim

RUN apt-get update && \
    apt-get install -y \
        python3-gi \
        gstreamer1.0-plugins-base \
        gstreamer1.0-plugins-good \
        gstreamer1.0-libav \
        gstreamer1.0-tools \
        libgirepository1.0-dev \
        libgstreamer1.0-dev \
        libcairo2-dev \
        x11-apps \
        && apt-get clean

COPY --from=builder /usr/src/gst-plugin/build/*.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/
COPY example/player.py /app/player.py
COPY example/requirements.txt /app/requirements.txt

RUN pip install -r /app/requirements.txt
WORKDIR /app

ENTRYPOINT ["python3", "player.py"]
CMD []