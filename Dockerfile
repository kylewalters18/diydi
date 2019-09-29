FROM ubuntu:latest

RUN apt-get -y update && \
    apt-get -y --no-install-recommends install clang clang-format cmake make gdb && \
    apt-get clean

WORKDIR /diydi

LABEL Name=diydi
