Here we see the info about downloading precombiled binaries & downloading the CHIPS bootstrap node.

## Downloading precombiled binaries for Linux
As we know in order to play game as we need chips, ln and bet repos. The binaries for these repos can be downloaded from below and atm the binaries work on ubuntu[fully tested] and macos[not fully tested].
### Downloading and Running CHIPS Binaries
```
Release Page: https://github.com/chips-blockchain/chips/releases/tag/16.99.0
For Linux[Fully tested]: https://github.com/chips-blockchain/chips/releases/download/16.99.0/chips-bins-linux-amd64.tgz
wget https://github.com/chips-blockchain/chips/releases/download/16.99.0/chips-bins-linux-amd64.tgz
tar -xvf chips-bins-linux-amd64.tgz
./chipsd &  #If any issues start the daemon with -reindex -addnode=5.9.253.195
```
### Downloading and Running LN Binaries
```
Release Page: https://github.com/chips-blockchain/lightning/releases
For Linux: https://github.com/chips-blockchain/lightning/releases/download/v0.5.2-2016-11-21-10032-gf09a68768/c-lightning-linux-x86_64-v0.5.2-2016-11-21-10032-gf09a68768.tar.gz
wget https://github.com/chips-blockchain/lightning/releases/download/v0.5.2-2016-11-21-10032-gf09a68768/c-lightning-linux-x86_64-v0.5.2-2016-11-21-10032-gf09a68768.tar.gz
tar -xvf c-lightning-linux-x86_64-v0.5.2-2016-11-21-10032-gf09a68768.tar.gz 
./lightningd/lightningd &
```
### Downloading and Running BET Binaries
```
Release Page: https://github.com/chips-blockchain/bet/releases
For Linux[Fully tested]: https://github.com/chips-blockchain/bet/releases/download/v0.1.3-500-g2f3ba78/bet-linux-x86_64-v0.1.3-500-g2f3ba78.tar.gz
wget https://github.com/chips-blockchain/bet/releases/download/v0.1.3-500-g2f3ba78/bet-linux-x86_64-v0.1.3-500-g2f3ba78.tar.gz
tar -xvf bet-linux-x86_64-v0.1.3-500-g2f3ba78.tar.gz
cd privatebet
./bet player #To start player node
```
> Note: If any issues are faced while running the downloaded binaries, please login the issue or post in the discord or try directly [compiling](./ubuntu_compile.md) from the source code.

## Downloading CHIPS bootstrap node
```
Bootstrap nodes info: https://dexstats.info/bootstrap.php
CHIPS Boorstrap Link: https://eu.bootstrap.dexstats.info/CHIPS-bootstrap.tar.gz
wget https://eu.bootstrap.dexstats.info/CHIPS-bootstrap.tar.gz
copy the blocks downloaded to .chips folder from home directory and start the chips daemon with -reindex
```
