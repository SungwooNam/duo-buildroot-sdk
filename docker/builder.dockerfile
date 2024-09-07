FROM  ubuntu:22.04
LABEL maintainer_name="Sungwoo Nam"
LABEL maintainer_email="the.sungwoo@gmail.com"

ENV DEBIAN_FRONTEND="noninteractive"

RUN echo "deb http://fi.archive.ubuntu.com/ubuntu/ jammy main restricted universe multiverse" > /etc/apt/sources.list && \
    echo "deb http://fi.archive.ubuntu.com/ubuntu/ jammy-updates main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb http://fi.archive.ubuntu.com/ubuntu/ jammy-backports main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb http://security.ubuntu.com/ubuntu jammy-security main restricted universe multiverse" >> /etc/apt/sources.list

RUN apt-get -y update && \
    apt-get install -y locales && \ 
    locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8  
ENV LANGUAGE=en_US:en  
ENV LC_ALL=en_US.UTF-8  

ENV USERNAME=duo
ARG USER_ID=1000
ARG GROUP_ID=1000

RUN apt-get install -y sudo && \
    groupadd -g ${GROUP_ID} ${USERNAME} && \
    useradd -s /bin/bash -u ${USER_ID} -g ${USERNAME} ${USERNAME} && \
    install -d -m 0755 -o ${USERNAME} -g ${USERNAME} /home/${USERNAME} && \
    chsh -s /bin/bash ${USERNAME} && \
    echo "${USERNAME}:${USERNAME}!" | chpasswd && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" | tee -a /etc/sudoers

RUN apt-get install -y \
    pkg-config build-essential ninja-build automake autoconf libtool wget curl \
    git gcc libssl-dev bc slib squashfs-tools android-sdk-libsparse-utils jq \
    python3-distutils scons parallel tree python3-dev python3-pip \
    device-tree-compiler ssh cpio fakeroot libncurses5 flex bison \
    libncurses5-dev genext2fs rsync unzip dosfstools mtools tcl \
    openssh-client cmake expect

