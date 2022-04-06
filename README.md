[![bet CD](https://github.com/chips-blockchain/bet/actions/workflows/bet-cd.yml/badge.svg?branch=master)](https://github.com/chips-blockchain/bet/actions/workflows/bet-cd.yml)
# Pangea-Bet

## Abstract
Bet is a decentralized adopatable gaming platform that meet the needs of any card game and predictable betting scenarios. The initial draft is written by jl777 is [here](./docs/BET_Initial_Draft.md). The underlying cryptocurrency that bet uses is [CHIPS](https://github.com/chips-blockchain/chips) and for real time betting and bet uses [CHIPS over LN](https://github.com/chips-blockchain/lightning). 

The detailed technical whitepaper is [here.](https://cdn.discordapp.com/attachments/455737840668770315/456036359870611457/Unsolicited_PANGEA_WP.pdf)

## Configuration
Bet can be played with or without LN. please go through the [existing tradeoffs](./docs/protocol/architecture_doc.md) we have at the moment to understand the ecosystem in a better way also please go through the [bet without ln](./docs/protocol/bet_without_ln.md) on how to play poker without using ln and it contains some nice info about various methods and info about [various tx's](./docs/protocol/tx_flow.md) involved during the gameplay.

## Setup
There are many different ways in which you can setup the nodes to play the poker using bet. Here are some of the ways in which you can setup the nodes:
* The easy approach - Using the docker 
* The hardway way -  By compiling the chips and bet repos all the way. 
* Using the compilation script

Irrespective of whichever approach you follow, the prerequisites and configuration of nodes remain same
### Prerequisites
##### For Player and Dealer nodes
* Ports used by the respective entities must be open. Below are the list of the ports used and should be remained open incase if you have any firewall configurations.
```
* 7797 - This port is used to have pub-sub communication between Dealer and player nodes.
* 7798 - This port is used to have pull-push communication between Dealer and player nodes.
* 7901 - This port is used to have pub-sub communication between Cashier and any other{player,dealer} nodes.
* 7902 - This port is used to have push-pull communication between Cashier and any other{player,dealer} nodes.
* 9000 - This port is used to have websocket communication between GUI and {player,dealer} nodes.
```
#### For Dealer and Cashier nodes
* The dealer and cashier nodes should need the static public IP for the nodes on which they setup.



Lets take a look into one by one approach and you can follow either of these approaches to setup the chips and bet nodes.

## Approach1 :- Playing using Docker - It all takes few commands with the setup time <5 mins 
The docker image is built on top of the ubuntu 20.04 base image for bet. The simplest way to play poker is using the docker images. All the docker images for the bet are maintained here
```
https://hub.docker.com/r/sg777/bet/tags
```
Pull the latest tag (at the time of writing this **v1.2** is the latest tag) and do the following to setup the nodes and play
#### step1 :- Pulling the docker image 
```
docker pull sg777/bet:v1.2
```
#### step2 :- Running the docker image 
```
docker run -it --name poker --net=host sg777/bet:v1.2
```
#### step3 :- Running chips node
Once you have access to the docker shell, you can load the chips from the [chips bootstrap node](./docs/protocol/release.md#downloading-chips-bootstrap-node). Running the below script pulls the bootstrap node and configures the chips node.
```
cd && load_bootstrap_node.sh
```
Then run the chips daemon
```
./chips/src/chipsd &
```
#### step4 :- If you are a player run the player as follows
```
cd
./bet/privatebet/bet player
```
#### step4 :-  If you are the dealer run the dealer as follows
```
cd
./bet/privatebet/bet dcv <host_ip>

```

## Approach2 :- Compiling the repos from scratch

### Compilation Guidelines
1. [Compiling only bet repo](./docs/protocol/ubuntu_compile.md#building-bet)
2. [Compiling all the repos needed on Ubuntu](./docs/protocol/ubuntu_compile.md)
3. [Detailed compilation of all the repos needed on various platforms](./docs/protocol/compile.md)

### Downloading precomipled binaries & CHIPS bootstrap node
1. [Precompiled binaries](./docs/protocol/release.md#downloading-precombiled-binaries-for-linux)
2. [CHIPS bootstrap node](./docs/protocol/release.md#downloading-chips-bootstrap-node)

## Approach3 :- Using the installation script

If you wish to install everything required for setting up Pangea Poker, you can use the following command to auto-install Pangea Poker:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/chips-blockchain/bet/master/tools/install-pangea.sh)"
```


## Setting up Bet Nodes
1. [Setting Up Player Node](./docs/protocol/player_setup.md)
2. [Setting Up Dealer Node](./docs/protocol/dealer_setup.md)
   <br/>In this we discuss more details about how to setup the dealer node and also steps to host a private table. 
3. [Setting Up Cashier Node](./docs/protocol/cashier_setup.md)

## Coomunication b/w Bet Nodes
The communication b/w the nodes at very high level is described [here](./docs/protocol/node_communication.md). 

## CHIPS & LN Upgrade
[Here](./docs/protocol/upgrade.md) contains the list of API's that bet uses from chips and ln. So whenever any upstream changes made to either chips and ln repos one must be careful to check if there is any change in these API functionalities, inputs and outputs.

## Glossary
Many of the times we use short hand abbreviations, so glossary for it is [here](./docs/protocol/glossary.md). 
