#!/bin/bash
sudo mkdir /opt/cmake
wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
mv cmake-3.16.1-Linux-x86_64.sh /opt/cmake/
sh /opt/cmake/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
