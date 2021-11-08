In order to make any node setup, first we need to compile all the repos listed below which are needed for this project.
1. chips
2. lightning
3. bet

In this document we only discuss how to compile manually from the command line step by step on ubuntu 18.04 & above. For more detailed compilation guidelines follow this [guide.](./compile.md)

# Building chips
## Build dependencies
```
sudo apt-get update
sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 python3-mako zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano
add-apt-repository ppa:bitcoin/bitcoin
sudo apt-get update
sudo apt-get install -y libdb4.8-dev libdb4.8++-dev
```
If boost is not there install boost library.
```
cd
wget wget "https://sourceforge.net/projects/boost/files/boost/1.72.0/boost_1_72_0.tar.gz"
tar -xvf boost_1_72_0.tar.gz
cd boost_1_72_0
./bootstrap.sh
./b2
./b2 install
```

## Build Chips
```
git clone https://github.com/chips-blockchain/chips.git
cd chips
./autogen.sh
./configure --with-boost=/usr/local/
cd src
make -j8 chipsd
make chips-cli
cp chips-cli /usr/bin # just need to get chips-cli to work from command line
sudo ldconfig /usr/local/lib
```
## Run Chips Daemon
```
./chipsd -addnode=5.9.253.195 &
```

# Building Lightning

## Build Lightning
```
git clone https://github.com/chips-blockchain/lightning.git
cd lightning && ./configure && make
sudo make install
```
## Run Lightning Daemon
```
cd
cd lightning/lightningd/
./lightningd --log-level=debug &
```

# Building Bet

## Build dependancies 
```
sudo apt-get update
sudo apt-get install -y \
  autoconf automake build-essential git libtool libgmp-dev \
  libsqlite3-dev python3 net-tools zlib1g-dev libsodium-dev \
  gettext wget libcurl3-gnutls ninja-build libssl-dev \
  libcurl4-gnutls-dev libevent-dev
```
## Build Bet

```
git clone https://github.com/chips-blockchain/bet
cd bet
./configure
make
```
