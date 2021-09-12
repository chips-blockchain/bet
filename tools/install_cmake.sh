#!/bin/bash
ARCH=$(uname -m)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ $ARCH == "aarch64"* ]]; then
        echo '$ARCH equals'
        if [ ! -f external/cmake/bin/cmake ];
        then
            echo 'cmake does not exists... compileing...'
            cd external/cmake && \
            ./bootstrap && \
            make
            # sudo make install
        else
            echo 'cmake exists'
            ls -lh external/cmake/bin/cmake
            external/cmake/bin/cmake --version
        fi
    else
        if ! command -v cmake &> /dev/null
        then
            sudo mkdir -p /opt/cmake
            wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
            mv cmake-3.16.1-Linux-x86_64.sh /opt/cmake/
            sh /opt/cmake/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
            sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
        fi
    fi
fi