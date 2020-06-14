FROM ubuntu:18.04
MAINTAINER kaityo256

ENV USER user
ENV HOME /home/${USER}
ENV SHELL /bin/bash

RUN useradd -m ${USER}
RUN gpasswd -a ${USER} sudo
RUN echo 'user:userpass' | chpasswd

RUN apt-get update && apt-get install -y \
    g++ \
    g++-8-aarch64-linux-gnu \
    git \
    m4 \
    python-dev \
    scons \
    sudo \
    vim \
    zlib1g-dev

USER ${USER}

RUN cd ${HOME} \
 && mkdir build \
 && cd build \
 && git clone --depth 1 https://github.com/RIKEN-RCCS/riken_simulator.git

RUN cd ${HOME} \
 && cd build/riken_simulator \
 && sed -i "369,372s:^:#:" SConstruct \
 && scons build/ARM/gem5.opt -j 20

RUN cd ${HOME} \
 && git clone https://github.com/kaityo256/aarch64env.git
