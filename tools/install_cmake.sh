#!/bin/bash

INSTALL_CMAKE() {
    echo "Install Cmake"
    if [ ! -f external/cmake/bin/cmake ];
    then
        echo 'cmake does not exists... compiling...'
        cd external/cmake && \
        ./bootstrap && \
        make
        # sudo make install
    else
        echo 'cmake exists'
        ls -lh external/cmake/bin/cmake
        external/cmake/bin/cmake --version
    fi
}

ARCH=$(uname -m)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ $ARCH == "aarch64"* ]] || [[ $ARCH == "armv7l"* ]]; then
        echo '$ARCH equals'

        if ! command -v cmake &> /dev/null
        then
            echo "cmake could not be found"
            INSTALL_CMAKE
        else
            echo "cmake is found"
            CMAKE_VERSION=$(cmake --version | grep "cmake version" | cut -d " " -f 3)
            # echo $CMAKE_VERSION

            CMAKE_MAJOR_VERSION=$(echo $CMAKE_VERSION | cut -d "." -f 1)
            CMAKE_MINOR_VERSION=$(echo $CMAKE_VERSION | cut -d "." -f 2)
            CMAKE_VERSION_TO_COMPARE="$CMAKE_MAJOR_VERSION.$CMAKE_MINOR_VERSION"
            CMAKE_MIN_VERSION_REQUIRED=3.13
            if (( $(echo "$CMAKE_MIN_VERSION_REQUIRED > $CMAKE_VERSION_TO_COMPARE" |bc -l) )); then
                echo "$CMAKE_VERSION_TO_COMPARE = You have lower version of cmake than required."
                INSTALL_CMAKE
            else
                echo "$CMAKE_VERSION_TO_COMPARE = You have higher version of cmake than required."
            fi
        fi
    else
        if ! command -v cmake &> /dev/null
        then
            sudo mkdir -p /opt/cmake
            wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
            sudo mv cmake-3.16.1-Linux-x86_64.sh /opt/cmake/
            sudo sh /opt/cmake/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
            sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
        fi
    fi
fi