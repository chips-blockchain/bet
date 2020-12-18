# Compilation Guidelines

You can either compile the project manually or run it using Docker. Compilation is pretty lengthy :) 

## Docker

All the dependencies have already been cloned/compiled in the docker container. 

Please refer to https://github.com/chips-blockchain/docker for information on Docker.

## Compilation steps

Tested with Ubuntu 16.04 and Ubuntu 18.04

### Dependencies Check 

```
$ sudo apt-get update
$ sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano tmux

# Install Berkeley 4.8 db libs (chips dependecy)
# Source: https://cryptoandcoffee.com/mining-gems/install-berkeley-4-8-db-libs-on-ubuntu-16-04/
$ mkdir ~/db-4.8.30 && cd ~/db-4.8.30 && wget http://download.oracle.com/berkeley-db/db-4.8.30.zip && unzip db-4.8.30.zip
$ cd db-4.8.30 && cd build_unix/ && ../dist/configure --prefix=/usr/local --enable-cxx && make && make install
```

BET dependencies
```
# Installing Jsmn 
$ cd ~ && git clone https://github.com/zserge/jsmn.git && cd jsmn && make

# Installing Libwally 
$ cd ~ && git clone https://github.com/ElementsProject/libwally-core.git
$ cd libwally-core/ && git submodule init
$ git submodule sync --recursive
$ git submodule update --init --recursive
$ ./tools/autogen.sh && ./configure && make && make check

# nanomsg-next-generation requires cmake 3.13 or higher
$ cd ~ && wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
$ mkdir /opt/cmake && sh ~/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
$ ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# check the cmake version
$ cmake --version

# Install nanomsg-next-generation  
$ cd ~ && git clone https://github.com/nanomsg/nng.git
$ cd nng && mkdir build && cd build && cmake -G Ninja .. && ninja && ninja install

# Installing libwebsockets
$ cd ~ && git clone https://github.com/sg777/libwebsockets.git
$ cd libwebsockets && mkdir build && cd build && cmake -DLWS_WITH_HTTP2=1 .. && make && make install
$ ldconfig /usr/local/lib

```

### Installing CHIPS
```
$ cd ~ && git clone https://github.com/chips-blockchain/chips && cd chips && ./build.sh
$ cd ~/chips/src && sudo cp chips-cli /usr/bin
$ sudo ldconfig /usr/local/lib # thanks smaragda!

--------------------
# Bootstraping CHIPS
--------------------
$ mkdir ~/.chips && cd ~/.chips && wget http://bootstrap3rd.dexstats.info/CHIPS-bootstrap.tar.gz
$ tar xvzf CHIPS-bootstrap.tar.gz
$ rm CHIPS-bootstrap.tar.gz
```

### Running CHIPS Daemon

  #### Create `chips.conf` file

  Create chips.conf file with random username, password, txindex and daemon turned on:
    
  ```shell
  cd ~
  mkdir .chips
  nano .chips/chips.conf
  ```

  Add the following lines into your `chips.conf` file

  ```JSON
  server=1
  daemon=1
  txindex=1
  rpcuser=chipsuser
  rpcpassword=passworddrowssap
  addnode=159.69.23.29
  addnode=95.179.192.102
  addnode=149.56.29.163
  addnode=145.239.149.173
  addnode=178.63.53.110
  addnode=151.80.108.76
  addnode=185.137.233.199
  rpcbind=127.0.0.1
  rpcallowip=127.0.0.1
  ```

  #### Symlinking the binaries (already done in Docker container)
  ```shell
  sudo ln -sf /root/chips/src/chips-cli /usr/local/bin/chips-cli
  sudo ln -sf /root/chips/src/chipsd /usr/local/bin/chipsd
  sudo chmod +x /usr/local/bin/chips-cli
  sudo chmod +x /usr/local/bin/chipsd
  ```
  #### Run
  ```shell
  cd ~
  cd chips/src
  ./chipsd &
  ```

  #### Check
  ```shell
  chips-cli getinfo
  ```

  #### Preview block download status
  ```
  cd ~
  cd .chips
  tail -f debug.log
  ```

### Installing Lightning Network Node

```
$ cd ~ && git clone https://github.com/chips-blockchain/lightning.git
$ cd lightning && make
$ cd /usr/bin/ && sudo nano lightning-cli
# Insert the following into the newly created file

#/bin/bash
~/lightning/cli/lightning-cli $1 $2 $3 $4 $5 $6 | jq .

# ctrl + O to save the output and ctrl + X to exit nano editor

$ chmod +x /usr/bin/lightning-cli
```

### Running the Lightning Daemon

LN will need a while to sync. It could take some time so its a good idea to run it in a tmux session.

```
# Create a tmux session
$ tmux new -s lightning

# Then inside the tmux session you've just created
$ ~/lightning/lightningd/lightningd --log-level=debug &
# CTRL + B, then D to detach from the tmux session, to attach to the session again `tmux a -t lightning`

# Get chain info - If it returns your node’s id, you’re all set.
$ lightning-cli getinfo

# Get a new address to fund your Lightning Node
# This returns an address, which needs to be funded first in order to open a channel with another node.
$ lightning-cli newaddr

# Run the following command to check if your node has funds
$ lightning-cli listfunds

# Optionally, using these two parameters, you can connect to a node visible on the LN explorer
$ lightning-cli connect
$ lightning-cli fundchannel
```

Join the [CHIPS discord](https://discord.gg/bcSpzWb) to get a small amount of CHIPS

[Tmux cheatsheet](https://tmuxcheatsheet.com/)

### Installing Bet
```
$ cd ~ && git clone https://github.com/chips-blockchain/bet && cd bet && make
```

### Running Bet Dealer
```
# e.g. Dealer node is at 45.77.139.155 (you will know this IP from someone who will be running a dealer node OR you can run the dealer node yourself)
$ cd ~/bet/privatebet && ./bet dcv 45.77.139.155
```
### Running Bet Player
```
$ cd ~/bet/privatebet && ./bet player
```
