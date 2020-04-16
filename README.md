# Pangea-Bet

The aim of this project is to provide the necessary bet API's which are sufficient to play poker from the command line. The initial draft of the game written by jl777 is [here](./docs/BET_Initial_Draft.md).

Bet is the implementation of the Pangea protocol which nees LN and CHIPS. The installation of [LN](https://github.com/sg777/lightning) and [CHIPS](https://github.com/sg777/chips3) must be done before proceeding to play with bet.

## Compilation Guidelines

Steps to compile this repo is mentioned in [compile.md](./compile.md). 

## Docker Setup
The node set up can also be done using the docker, steps to setup the docker has been mentioned [here](./docker_setup.md).

## Usage of this repo

The usage of this repo varies from branch to branch. Each branch has a different gaming logic and their intended gameplays are different.

The branches are listes as follow.
* master
* highest_card_wins
* poker
* mp_cli
* rest_dev
* poker_test

### master
The implementation of poker using the GUI has been implemented here.

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
```
### Configuring the Table

The dealer can configure the table parameters, the steps to configure the table parameters are mentioned [here](./configure_dealer.md).

## Command to run BVV
```
$ cd
$ cd bet/privatebet
$ ./bet bvv ipaddress_of_dcv
```
## Command to run Player
```
$ cd
$ cd bet/privatebet
$ ./bet player ipaddress_of_dcv
```
## Command to run cashier
```
$ cd
$ cd bet/privatebet
$ ./bet cashier
```

The detailed description of the cashier protocol is mentioned [here](./cashier_protocol.md)

