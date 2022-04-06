In order to make any node setup, first we need to compile all the repos listed below which are needed for this project.
1. chips
2. lightning
3. bet

In this document we only discuss how to compile manually from the command line step by step on ubuntu 18.04 & above. For more detailed compilation guidelines follow this [guide.](./compile.md)

# Building chips
## Build dependencies for ubuntu 18.04 and lower
```
sudo apt-get update
sudo apt-get install software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 python3-mako zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano bsdmainutils
add-apt-repository ppa:bitcoin/bitcoin
sudo apt-get update
sudo apt-get install -y libdb4.8-dev libdb4.8++-dev
```
## Build dependencies for ubuntu 20.04 

When you run the following commands
```
sudo add-apt-repository ppa:bitcoin/bitcoin
sudo apt-get update
sudo apt-get install libdb4.8-dev libdb4.8++-dev
```
You most likely get the following error:
```
Err:15 http://ppa.launchpad.net/bitcoin/bitcoin/ubuntu disco Release
404 Not Found [IP: 91.189.95.83 80]
```
In that case you try the following to install libdb4.8:
```
sudo add-apt-repository --remove ppa:bitcoin/bitcoin
sudo apt-get update
wget http://download.oracle.com/berkeley-db/db-4.8.30.zip
unzip db-4.8.30.zip
cd db-4.8.30
cd build_unix/
../dist/configure --prefix=/usr/local --enable-cxx
make
make install
```
In compiling libdb4.8 if you get the following error
```
error: definition of ‘int __atomic_compare_exchange(db_atomic_t*, atomic_value_t, atomic_value_t)’ ambiguates built-in declaration ‘bool __atomic_compare_exchange(long unsigned int, volatile void*, void*, void*, int, int)’
```
Then fix is it as [follows](https://gist.github.com/danieldk/5700533)
```
Navigate to the file atomic.h that is in the dbinc directory. Somewhere around line 147, you will see _atomic_compare_exchange((p), (o), (n)) (as in line 9 above). Change that line to be __atomic_compare_exchange_db((p), (o), (n)). You are just adding a _db at the end of the function name.

You will also do something simolar around line 179. After that, re-run make to build Berkeley DB 4.8.
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
