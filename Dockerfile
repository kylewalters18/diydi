FROM ubuntu:18.04

RUN apt-get -y update && \
    apt-get -y --no-install-recommends install clang clang-format cmake make gdb && \
    apt-get clean

WORKDIR /diydi

LABEL Name=diydi
