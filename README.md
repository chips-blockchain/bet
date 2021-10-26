![example workflow](https://github.com/sg777/bet/actions/workflows/bet-cd.yml/badge.svg)
# Pangea-Bet

## Abstract
Bet is a decentralized adopatable gaming platform that meet the needs of any card game and predictable betting scenarios. The initial draft is written by jl777 is [here](./docs/BET_Initial_Draft.md). The underlying cryptocurrency that bet uses is [CHIPS](https://github.com/chips-blockchain/chips) and for real time betting and bet uses [CHIPS over LN](https://github.com/chips-blockchain/lightning). 

The detailed technical whitepaper written by sg777 is [here.](https://cdn.discordapp.com/attachments/455737840668770315/456036359870611457/Unsolicited_PANGEA_WP.pdf)

# Guide to use & understand Bet is as follows

## Compilation Guidelines
1. [Compiling On Ubuntu](./docs/protocol/ubuntu_compile.md)
2. [Detailed Compilation Guidelines](./docs/protocol/compile.md)

### Setting up Bet Nodes
1. [Setting Up Player Node](./docs/protocol/player_setup.md)
2. Setting up Dealer Node.
3. Setting up Cashier Node.

### [Coomunication b/w Bet Nodes](./docs/protocol/communication.md) 

### [Glossary](./docs/protocol/glossary.md) 

## Installation

> Note: You will need an exposed IP to play the game

### Ports to be used and open

Below are the list of the ports used and should be remained open incase if you have any firewall configurations.
```
* 7797 - This port is used to have pub-sub communication between Dealer and player nodes.
* 7798 - This port is used to have pull-push communication between Dealer and player nodes.
* 7901 - This port is used to have pub-sub communication between Cashier and any other{player,dealer} nodes.
* 7902 - This port is used to have push-pull communication between Cashier and any other{player,dealer} nodes.
* 9000 - This port is used to have websocket communication between GUI and {player,dealer} nodes.
```

### Compilation Guidelines

Install Dependencies:

```bash
sudo apt-get update
sudo apt-get install -y \
  autoconf automake build-essential git libtool libgmp-dev \
  libsqlite3-dev python3 net-tools zlib1g-dev libsodium-dev \
  gettext wget libcurl3-gnutls ninja-build libssl-dev \
  libcurl4-gnutls-dev libevent-dev
```

Then clone this repo and build the binaries

```bash
git clone https://github.com/chips-blockchain/bet
cd bet
./configure
make
```

Steps to compile this repo and all the other required dependencies to play Pangea Poker are mentioned in [compile.md](./docs/protocol/compile.md).

### Docker Setup

The node set up can also be done [using Docker](https://github.com/chips-blockchain/docker).

## Running the game

### Command to run DCV
```
$ cd ~/bet/privatebet
$ ./bet dcv <dealer_ip>
```
The `dealer_ip` should be a `static public ip` of a machine on which a dealer node runs.

#### Configuring the Table

The dealer can configure the table parameters, the steps to configure the table parameters are mentioned [here](./configure_dealer.md).

### Command to run Player

```
$ cd
$ cd bet/privatebet
$ ./bet player
```

### Command to run Cashier

```
$ cd
$ cd bet/privatebet
$ ./cashierd cashier cashier_ip
```
The `cashier_ip` should be a `static public ip` of a machine on which a cashier node runs. The cashier nodes are the trusted nodes in the network and are elcted and chosen by the community. The set of trusted nodes at the moment are [here](./privatebet/config/cashier_nodes.json).

To avoid the unavailability of the nodes due to any crahing and disconnections, the cashier daemons are scheduled using crontab. The contents of the shell script which is scheduled using the crontab is as follows:
```
#!/bin/bash
SERVICE="cashierd"
date
if pgrep -x "$SERVICE" >/dev/null
then
    echo "$SERVICE is running"
else
    echo "$SERVICE stopped"
    cd /root/bet/privatebet
    ./cashierd cashier <ip_addr>
fi
```
Using `crontab -e` one can edit the crontab file and schedule it run all the time by adding the following line at the end of cron file <br/>`* * * * * /root/cron.sh`.<br/> Here `cron.sh` contains the above shell commands and replace `<ip_addr>` with the static public ipv4 address of the node where this daemon is running.

The detailed description of the cashier protocol is mentioned [here](./cashier_protocol.md).

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

### LN Upgrade
Since bet uses the lightning network for the real time payments, so its necessary for the CHIPS LN node to be sync with upstream lightning network. As we seen the changes in the input and output of LN API's in the upstream we should be cautious about porting those changes into the CHIPS LN node. 
The LN commands that bet uses at the moment during the process of the game are listed below:
```
getinfo
fundchannel
pay
connect
listfunds
invoice
dev-blockheight
peer-channel-state
listpeers
newaddr
```
So we should test the functiolities of these API's to see any changes are made everytime when we port something to the downstream CHIPS LN node.

### CHIPS Upgrade
The underlying crypto on which the pangea protocol built on is CHIPS. As we know CHIPS initially is a BTC fork with the modified chain and mining parameters to suit to the real time gaming applications. So CHIPS will be periodically updated from the BTC upstream changes to fetch the changes that are needed. 

With that little background, the CHIPS API's used by bet are mentioned below. So whenever the CHIPS upstream changes are made its important to test these API's for bet compatability.

```
importaddress
getnewaddress
listaddressgroupings
getblock
getrawtransaction
decoderawtransaction
gettransaction
sendrawtransaction
signrawtransactionwithwallet 
listunspent
createrawtransaction
getblockcount
getbalance
addmultisigaddress 
```
