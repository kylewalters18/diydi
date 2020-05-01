FROM ubuntu:18.04

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install \
        ssh \
        ca-certificates \
        zsh \
        curl \
        vim \
        entr \
        git \
        clang \
        clang-format \
        clang-tidy \
        cppcheck \
        cmake \
        make \
        gdb \
    && apt-get clean

RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/robbyrussell/oh-my-zsh/master/tools/install.sh)"
RUN sed -i 's/robbyrussell/bira/g' ~/.zshrc
