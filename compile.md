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
$ cd ~ && wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
$ mkdir /opt/cmake && sh ~/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
$ ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
# check the cmake version
$ cmake --version

# Installing Jsmn
$ cd ~ && git clone https://github.com/zserge/jsmn.git && cd jsmn && make

# Installing Libwally
$ cd ~ && git clone https://github.com/ElementsProject/libwally-core.git
$ cd libwally-core && ./tools/autogen.sh && ./configure && make && make check

# Install nanomsg-next-generation 
$ cd ~ && git clone https://github.com/nanomsg/nng.git
$ cd nng && mkdir build && cd build && cmake -G Ninja .. && ninja && ninja install

# Installing libwebsockets
$ cd ~ && git clone https://github.com/sg777/libwebsockets.git
$ cd libwebsockets && mkdir build && cd build && cmake -DLWS_WITH_HTTP2=1 .. && make && make install
$ ldconfig /usr/local/lib

# Install Berkeley 4.8 db libs
# Source: https://cryptoandcoffee.com/mining-gems/install-berkeley-4-8-db-libs-on-ubuntu-16-04/
$ mkdir ~/db-4.8.30 && cd ~/db-4.8.30 && wget http://download.oracle.com/berkeley-db/db-4.8.30.zip && unzip db-4.8.30.zip
$ cd db-4.8.30 && cd build_unix/ && ../dist/configure --prefix=/usr/local --enable-cxx && make && make install

```

### Installing CHIPS
```
$ cd ~ && git clone https://github.com/chips-blockchain/chips && cd chips && ./autogen.sh
# ./configure LDFLAGS="-L/chips/db4/lib/" CPPFLAGS="-I/chips/db4/include/" -without-gui -without-miniupnpc --disable-tests --disable-bench --with-gui=no
# cd src && make -> will build everything, including QT wallet
$ sudo cp chips-cli /usr/bin # just need to get chips-cli to work from command line
$ sudo ldconfig /usr/local/lib # thanks smaragda!

# Bootstraping CHIPS
$ mkdir ~/.chips && cd ~/.chips && wget http://bootstrap3rd.dexstats.info/CHIPS-bootstrap.tar.gz
$ tar xvzf CHIPS-bootstrap.tar.gz
$ rm CHIPS-bootstrap.tar.gz

# Running CHIPS Daemon 
# see https://github.com/chips-blockchain/chips#step-2-create-chips-data-dir-chipsconf-file-and-restrict-access-to-it
```

### Installing Lightning Repo

```
$ cd ~ && git clone https://github.com/sg777/lightning.git
$ cd lightning && make
$ cd /usr/bin/ &&  nano lightning-cli
# Insert the following into the newly created file

#/bin/bash
~/lightning/cli/lightning-cli $1 $2 $3 $4 $5 $6 | jq .

# ctrl + O and ctrl + X to exit nano editor

$ chmod +x /usr/bin/lightning-cli

# Running the Lightning Daemon
# Create a tmux session
$ tmux new -s lightning
# Then inside the tmux session you've just created
$ ~/lightning/lightningd/lightningd --log-level=debug &
# CTRL + B, then D to detach from the tmux session
# to attach to the session again `tmux a -t lightning`
# get chain info
$ lightning-cli getinfo
```

### Installing Bet
```
$ cd ~ && git clone https://github.com/chips-blockchain/bet && cd bet && make

# Running Bet Dealer
# e.g. Dealer node is at 45.77.139.155 (you will know this IP from someone who will be running a dealer node OR you can run the dealer node yourself)
$ cd ~/bet/privatebet && ./bet dcv 45.77.139.155

# Running Bet Player
# e.g. You need to specify the dealer node IP
$ cd ~/bet/privatebet && ./bet player 45.77.139.155
```
