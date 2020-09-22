# Compilation Guidelines

You can either compile the project manually or run it using Docker.

## Docker

All the dependencies have already been cloned/compiled in the docker container. Please refer to 
https://github.com/chips-blockchain/docs#docker for information on Docker.

> Note: When you run this docker container, since we are sharing the host network with the docker container, make sure to stop the chips and ln nodes in the host node.

## Compilation steps

Tested with Ubuntu 16.04 and Ubuntu 18.04

### Dependencies Check 

```
$ sudo apt-get update
$ sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano tmux

# nanomsg-next-generation requires cmake 3.13 or higher
wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh /cmake-3.16.1-Linux-x86_64.sh
mkdir /opt/cmake
sh /cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Installing Jsmn
$ cd ~
$ git clone https://github.com/zserge/jsmn.git
$ cd jsmn && make

# Installing Libwally
$ cd ~
$ git clone https://github.com/ElementsProject/libwally-core.git
$ cd libwally-core
$ ./tools/autogen.sh
$ ./configure
$ make
$ make check

# Install nanomsg-next-generation 
$ cd ~
$ git clone https://github.com/nanomsg/nng.git
$ cd nng
$ mkdir build
$ cd build
$ cmake -G Ninja ..
$ ninja
$ ninja test
$ sudo ninja install

# Installing libwebsockets
$ cd ~
$ git clone https://github.com/sg777/libwebsockets.git
$ cd libwebsockets
$ mkdir build
$ cd build
$ cmake -DLWS_WITH_HTTP2=1 ..
$ make && sudo make install
$ ldconfig /usr/local/lib

# Install Berkeley 4.8 db libs
$ mkdir ~/db-4.8.30 && cd ~/db-4.8.30
$ wget http://download.oracle.com/berkeley-db/db-4.8.30.zip
$ unzip db-4.8.30.zip
$ cd db-4.8.30
$ cd build_unix/
$ ../dist/configure --prefix=/usr/local --enable-cxx
$ make
$ make install

```

### Installing CHIPS
```
$ cd ~
$ git clone https://github.com/sg777/chips3.git
$ cd chips3
$ ./autogen.sh
$ ./configure --with-boost=/usr/local/ 
$ cd src
$ make -j8 chipsd
$ make chips-cli
$ sudo cp chips-cli /usr/bin # just need to get chips-cli to work from command line
# make -> will build everything, including QT wallet
$ sudo ldconfig /usr/local/lib # thanks smaragda!

# Bootstraping CHIPS
$ cd ~/.chips && \
$ wget http://bootstrap3rd.dexstats.info/CHIPS-bootstrap.tar.gz && \
$ tar xvzf CHIPS-bootstrap.tar.gz && \
$ rm CHIPS-bootstrap.tar.gz

# Running CHIPS Daemon
$ cd ~/chips
$ ./autogen.sh
$ ./configure LDFLAGS="-L/chips/db4/lib/" CPPFLAGS="-I/chips/db4/include/" -without-gui -without-miniupnpc --disable-tests --disable-bench --with-gui=no
$ make -j2
```

### Installing Lightning Repo

```
$ cd ~
$ git clone https://github.com/sg777/lightning.git
$ cd lightning
$ make
$ cd src
$ cp lightning-cli /usr/bin
$ ldconfig

# Running Lightning Daemon
$ cd ~
$ cd lightning
$ ./lightningd/lightningd --log-level=debug &
```

### Installing Bet
```
$ cd ~
$ git clone https://github.com/sg777/bet.git
$ cd bet
$ make

# Running Bet Dealer
# e.g. Dealer node is at 45.77.139.155
$ cd ~/bet/privatebet && ./bet dcv 45.77.139.155

# Running Bet Player
# e.g. You need to specify the dealer node IP
$ cd ~/bet/privatebet && ./bet player 45.77.139.155
```
