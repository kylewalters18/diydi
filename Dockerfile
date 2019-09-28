FROM ubuntu:latest

RUN apt-get -y update && apt-get install -y
RUN apt-get -y install clang clang-format cmake gdb

WORKDIR /diydi

LABEL Name=diydi
