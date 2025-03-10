sudo apt --assume-yes update

sudo apt --assume-yes install -y \
    git \
    wget \
    silversearcher-ag \
    python2.7 \
    python-pip \
    pkg-config \
    build-essential \
    clang \
    cgroup-tools \
    libapr1-dev libaprutil1-dev \
    libboost-all-dev \
    libyaml-cpp-dev \
    libjemalloc-dev \
    python3-dev \
    python3-pip \
    python3-wheel \
    python3-setuptools \
    libjpeg-dev \
    zlib1g-dev \
    libgoogle-perftools-dev

pip2 install pyyaml
pip2 install tabulate
pip3 install simplerpc
