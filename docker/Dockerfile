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
    qemu-user-binfmt \
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

RUN cd ${HOME} \
 && echo alias gem5=\'~/build/riken_simulator/build/ARM/gem5.opt ~/build/riken_simulator/configs/example/se.py -c\' >> .bashrc \
 && echo alias ag++=\'aarch64-linux-gnu-g++-8 -static -march=armv8-a+sve\' >> .bashrc
