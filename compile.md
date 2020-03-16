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
$ cmake -DLWS_WITH_HTTP2=1 ..
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
## Running uisng Docker

All the above repos are already been cloned in the docker container, use the following command to to start the Docker container:
```
docker run --net=host -t -i -v /root/.chips:/root/.chips:rw  -v /root/.chipsln:/root/.chipsln:rw  norbertdragan/pangea-poker
```
Note: When you run this docker container, since we are sharing the host network with the docker container, so make sure to stop the chips and ln nodes in the host node.
