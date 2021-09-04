sudo apt-get update
sudo apt-get install -y software-properties-common autoconf git build-essential libtool libprotobuf-c-dev libgmp-dev libsqlite3-dev python python3 zip libevent-dev pkg-config libssl-dev libcurl4-gnutls-dev make libboost-all-dev automake jq wget ninja-build libsqlite3-dev libgmp3-dev valgrind libcli-dev libsecp256k1-dev libsodium-dev libbase58-dev nano tmux

# Install Berkeley 4.8 db libs (chips dependecy)
# Source: https://cryptoandcoffee.com/mining-gems/install-berkeley-4-8-db-libs-on-ubuntu-16-04/
mkdir ~/db-4.8.30 && cd ~/db-4.8.30 && wget http://download.oracle.com/berkeley-db/db-4.8.30.zip && unzip db-4.8.30.zip
cd db-4.8.30 && cd build_unix/ && ../dist/configure --prefix=/usr/local --enable-cxx && make && sudo make install

# Installing Jsmn 
# cd ~ && git clone https://github.com/zserge/jsmn.git && cd jsmn && make

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
sudo ldconfig /usr/local/lib

# Install chips blockchain
cd ~ && git clone https://github.com/chips-blockchain/chips && cd chips && ./build.sh
cd ~/chips/src
sudo ln -s $HOME/chips/src/chipsd /usr/local/bin/
sudo ln -s $HOME/chips/src/chips-cli /usr/local/bin/
sudo ldconfig /usr/local/lib

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
rpcallowip=127.0.0.1" > $HOME/.chips/chips.conf

# Bootstraping CHIPS
cd ~/.chips && wget http://bootstrap3rd.dexstats.info/CHIPS-bootstrap.tar.gz && tar xvzf CHIPS-bootstrap.tar.gz && rm CHIPS-bootstrap.tar.gz

# Install all chips blockchain repos
cd ~ && git clone https://github.com/chips-blockchain/lightning.git
cd lightning && make
sudo ln -s $HOME/lightning/lightningd/lightningd /usr/local/bin/
sudo ln -s $HOME/lightning/cli/lightning-cli /usr/local/bin/

cd ~ && git clone https://github.com/chips-blockchain/bet && cd bet && git checkout n_player && make

cd ~ && curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.37.2/install.sh | bash && source ~/.bashrc && nvm install node

cd ~ && git clone https://github.com/chips-blockchain/pangea-poker.git && cd ~/pangea-poker && npm install && git checkout dev