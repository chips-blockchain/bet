# Compilation Guidelines

You can either compile the project manually or run it using Docker. Compilation is pretty lengthy :) 

## Docker

All the dependencies have already been cloned/compiled in the docker container. 

Please refer to https://github.com/chips-blockchain/docker for information on Docker.

## Compilation steps

Tested with Ubuntu 16.04 and Ubuntu 18.04

### Dependencies Check 

```bash
sudo apt-get update
sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano tmux

# Install Berkeley 4.8 db libs (chips dependecy)
# Source: https://cryptoandcoffee.com/mining-gems/install-berkeley-4-8-db-libs-on-ubuntu-16-04/
mkdir ~/db-4.8.30 && cd ~/db-4.8.30 && wget http://download.oracle.com/berkeley-db/db-4.8.30.zip && unzip db-4.8.30.zip
cd db-4.8.30 && cd build_unix/ && ../dist/configure --prefix=/usr/local --enable-cxx && make && sudo make install
```

### Installing BET dependencies

```bash
# Installing Jsmn 
cd ~ && git clone https://github.com/zserge/jsmn.git && cd jsmn && make

# Installing Libwally 
cd ~ && git clone https://github.com/ElementsProject/libwally-core.git
cd libwally-core/ && git submodule update --init --recursive
./tools/autogen.sh && ./configure && make && make check && sudo make install

# nanomsg-next-generation requires cmake 3.13 or higher
cd ~ && wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh
sudo mkdir /opt/cmake && sudo sh ~/cmake-3.16.1-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# check the cmake version
cmake --version

# Install nanomsg-next-generation  
cd ~ && git clone https://github.com/nanomsg/nng.git
cd nng && mkdir build && cd build && cmake -G Ninja .. && ninja && sudo ninja install

# Installing libwebsockets
cd ~ && git clone https://github.com/sg777/libwebsockets.git
cd libwebsockets && mkdir build && cd build && cmake -DLWS_WITH_HTTP2=1 .. && make && sudo make install
ldconfig /usr/local/lib
```

### Installing CHIPS

```bash
cd ~ && git clone https://github.com/chips-blockchain/chips && cd chips && ./build.sh
cd ~/chips/src
sudo ln -s $HOME/chips/src/chipsd /usr/local/bin/
sudo ln -s $HOME/chips/src/chips-cli /usr/local/bin/
sudo ldconfig /usr/local/lib

--------------------
# Bootstraping CHIPS
--------------------
mkdir $HOME/.chips/ && cd $HOME/.chips/ && wget http://bootstrap3rd.dexstats.info/CHIPS-bootstrap.tar.gz
tar xvzf CHIPS-bootstrap.tar.gz
rm CHIPS-bootstrap.tar.gz
```

### Running CHIPS Daemon

**Create `chips.conf` file:**

Create `chips.conf` file with random username, password, txindex and daemon turned on.

You can copy the following lines in your bash shell and press enter. It will create the CHIPS configuration file with random RPC username and password with all needed settings.
    
```bash
mkdir -p $HOME/.chips/
echo "server=1
daemon=1
txindex=1
rpcuser=$(openssl rand -base64 12)
rpcpassword=$(openssl rand -base64 12)
addnode=159.69.23.29
addnode=95.179.192.102
addnode=149.56.29.163
addnode=145.239.149.173
addnode=178.63.53.110
addnode=151.80.108.76
addnode=185.137.233.199
rpcbind=127.0.0.1
rpcallowip=127.0.0.1" > $HOME/chips/chips.conf
```

#### Run CHIPS daemon
```shell
cd ~
chipsd
```

#### Check if CHIPS is running
```shell
chips-cli getinfo
```

The output might be an error followed by `Loading blocks...`. CHIPS is loading our bootstrapped blocks. Just give it some time and try again later. Successful result is an object with info.

#### Preview block download status
```
cd ~
tail -f $HOME/.chips/debug.log
```

### Installing Lightning Network Node

```
cd ~ && git clone https://github.com/chips-blockchain/lightning.git
cd lightning && make
sudo ln -s $HOME/lightning/lightningd/lightningd /usr/local/bin/
sudo ln -s $HOME/lightning/cli/lightning-cli /usr/local/bin/
```

### Running the Lightning Daemon

NOTE: You do not need to run Lightning Network Daemon until CHIPS daemon is fully synced. You can tell if the CHIPS daemon is fully synced by looking at the commad output of `chips-cli getinfo`, where the output of `blocks` and `headers` will be matching. Example:

```bash
chips-cli getinfo
{
  ...
  ...
  "chain": "main",
  "blocks": 7709392,
  "headers": 7709392,
  ...
  ...
} 
```

Once the CHIPS daemon is fully synced with the network, run Lightning Network Daemon. It shouldn't take much time to sync with CHIPS blockchain if CHIPS is fully synced. Its still a good idea to run it in a tmux session.

```
# Create a tmux session
tmux new -s lightning

# Then inside the tmux session you've just created
lightningd --log-level=debug &
# CTRL + B, then D to detach from the tmux session, to attach to the session again `tmux a -t lightning`

# Get chain info - If it returns your node’s id, you’re all set.
lightning-cli getinfo

# If you don't see output `lightning-cli` of this command formatted, you can add `jq` with pipe command like following:
lightning-cli getinfo | jq .

# Get a new address to fund your Lightning Node
# This returns an address, which needs to be funded first in order to open a channel with another node.
lightning-cli newaddr

# Run the following command to check if your node has funds
lightning-cli listfunds

# Optionally, using these two parameters, you can connect to a node visible on the LN explorer
lightning-cli connect
lightning-cli fundchannel
```

Join the [CHIPS discord](https://discord.gg/bcSpzWb) to get a small amount of CHIPS

[Tmux cheatsheet](https://tmuxcheatsheet.com/)

### Installing Bet
```
cd ~ && git clone https://github.com/chips-blockchain/bet && cd bet && make
```

### Running Bet Dealer

First determin what is the public IP of your server/VPS/VM/node. You probably know that already if you connected to this server over the internet through ssh terminal connection. In any case, you can also use the following command to get the public IP of your node:

```bash
curl ifconfig.co
# Example output would be like 45.77.139.155, which you can use in next commands.
```

```bash
# e.g. Dealer node is at 45.77.139.155 (you will know this IP from someone who will be running a dealer node OR you can run the dealer node yourself)
$ cd ~/bet/privatebet && ./bet dcv 45.77.139.155
```

### Running Bet Player
```bash
cd ~/bet/privatebet && ./bet player
```

You might see something like this when running bet for the first time. The number of blocks will keep on rising. This means LN is not synced. You just need to let it be for a while and let it sync. Feel free to exit the currently running command. The sync is happening in the background in the tmux session that you have started earlier.
```
root@959aa68123b4:~/bet# cd ~/bet/privatebet && ./bet dcv 45.77.139.155
ln is 48513 blocks behind chips network
```
