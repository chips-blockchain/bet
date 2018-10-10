# bet
The aim of this project is to provide the necessary bet API's which are sufficient to play poker from the command line. The initial draft of the game written by jl777 is [here](./docs/BET_Initial_Draft.md).
 
Basically, there exists three entities in the game DCV, BVV and Player. The role of DCV and BVV is to assist in deck shuffling, dealing and fair play of the game. To test the API's written from the player perspective we run the DCV and BVV nodes, just to start with below or the steps to run DCV and BVV.
## Steps to compile
```
# Dependencies Check 

$ sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip jq libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev cmake

$ apt install make ninja-build libsqlite3-dev libgmp3-dev

#Install nanomsg-next-generation 

$ git clone https://github.com/nanomsg/nng.git
$ cd nng
$ mkdir build
$ cd build
$ cmake -G Ninja ..
$ ninja
$ ninja test
$ ninja install


$ cd ~
$ git clone https://github.com/sg777/bet.git
$ cd bet
$ make
```
## Communicating over IPC
Nodes communicating through IPC, `ipc:///tmp/bet.ipc` is an uplink channel{PUSH-PULL} between players to DCV, where as `ipc:///tmp/bet1.ipc` is the downlink channel{PUB-SUB} between DCV and Players.

## Command to run DCV
```
$ cd
$ cd bet/privatebet
$ ./bet dcv
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## Command to run BVV
```
$ cd
$ cd bet/privatebet
$ ./bet bvv
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## Command to run Player
```
$ cd
$ cd bet/privatebet
$ ./bet player
(ipc:///tmp/bet.ipc) bound
(ipc:///tmp/bet1.ipc) bound
```
## Communicating over IP Adress
DCV nodes binds to the sockets created over the ports 7797 and 7798. The BVV and Player nodes connect to the binding address via subscribe and push sockets.

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


## Scope of these API's
Using `create-player` one can create the player then create deck using `create-deck`. Once if the player and deck creation is done, player can join the table using `join-req`.

Once the player joins the table and table is filled, DCV asks for player initialization(i.e deck shuffling), for that first player blinds the deck using `blind-deck` and sends the blinded-deck to DCV using 'player-init'. 

Once after the DCV receives the initialization messages from all the players it provides DCV blinded deck and passed to BVV, then BVV blinds it further and provides the BVV blinded deck to the players. Player by using `dcv-init`, `bvv-init` and  `dcv-bvv-init` API's stores the blinded deck Informationation values received from DCV and BVV, which further used to reveal the deck of cards.

## Note
The bet-api documentation is [here](./docs/bet-api.md)

## Help from the community
Any feedback, suggestions on the API's or new API's which should be present inorder to play the poker from command line, please raise an issue/suggestion or message me(myid: sg777) directly or post in the #CHIPS channel on [discord](https://discord.gg/jF9uxJM).
