# Pangea-Bet
The aim of this project is to provide the necessary bet API's which are sufficient to play poker from the command line. The initial draft of the game written by jl777 is [here](./docs/BET_Initial_Draft.md).

Bet is the implementation of the Pangea protocol which nees LN and CHIPS. The installation of [LN](https://github.com/sg777/lightning) and [CHIPS](https://github.com/sg777/chips3) must be done before proceeding to play with bet.

## Steps to compile
```
# Dependencies Check 

$ sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip jq libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev cmake

$ sudo apt install make ninja-build libsqlite3-dev libgmp3-dev

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
$ cmake ..
$ make && sudo make install

# Install libwebsockets

$ cd ~
$ git clone https://github.com/sg777/libwebsockets.git
$ mkdir build
$ cmake ..
$ make
$ sudo make install
$ ldconfig

# Installing CHIPS

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
# Running CHIPS Daemon

$ cd ~
$ cd chips/src
$ ./chipsd -addnode=5.9.253.195 &

# Installing Lightning Repo

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

#Installing Bet

$ cd ~
$ git clone https://github.com/sg777/bet.git
$ cd bet
$ make
```

## Usage of this repo

The usage of this repo varies from branch to branch. Each branch has a different gaming logic and their intended gameplays are different.

The branches are listes as follow.
* master
* dev
* highest_card_wins
* poker
* mp_cli
* rest_dev
* poker_test

### highest_card_wins
This branch is used to play the highest card player wins game via CLI, two players can play the game. The player whoever gets the highest card wins the game.

### poker
This branch is used to player poker via CLI

### mp_cli
In this branch implementation is done to support more than two players, and also made a provision to pass the number of players from the command line

### rest_dev
This branch is used to player poker via GUI, the GUI code should be taken from `poker` branch of `[pangea-poker-frontend](https://github.com/sg777/pangea-poker-frontend)` repo.

For the GUI developers the backend message formats are defined [here](./docs/messageFormats.md)

### poker_test
This branch is used to player poker via GUI, the GUI code should be taken from `poker_test` branch of `[pangea-poker-frontend](https://github.com/sg777/pangea-poker-frontend)` repo.


## Communicating over IP Adress
DCV nodes binds to the sockets created over the ports 7797 and 7798. The BVV and Player nodes connect to the binding address via subscribe and push sockets. Here DCV and BVV are the trusted notarized nodes in the network, DCV plays the central role in coordinating the game.

## Command to run DCV
```
$ cd
$ cd bet/privatebet
$ ./bet dcv ipaddress_of_dcv
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## Command to run BVV
```
$ cd
$ cd bet/privatebet
$ ./bet bvv ipaddress_of_dcv
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## Command to run Player
```
$ cd
$ cd bet/privatebet
$ ./bet player ipaddress_of_dcv
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## The scope of this repo

Now using this repo, two players can bet in real time over the deck of cards which are randomly shuffled. In order to play the game any of the playing node can connect to the trusted DCV node in the network by providing it's IP Address.
