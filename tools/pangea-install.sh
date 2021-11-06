#!/bin/bash


# Install if sudo command isn't available
if ! command -v sudo &> /dev/null
then
    echo "sudo could not be found"
	apt update
	apt install -y sudo
fi

# Install curl command if isn't available
if ! command -v curl &> /dev/null
then
    echo "curl could not be found"
	if [[ $EUID -eq 0 ]]; then
		sudo apt update
		sudo apt install -y curl
	else
		apt update
		apt install -y sudo curl
	fi
fi

# Install wget command if isn't available
if ! command -v wget &> /dev/null
then
    echo "wget could not be found"
	if [[ $EUID -eq 0 ]]; then
		sudo apt update
		sudo apt install -y wget
	else
		apt update
		apt install -y sudo wget
	fi
fi

# Install systemctl command if isn't available
if ! command -v systemctl &> /dev/null
then
    echo "systemctl could not be found"
	if [[ $EUID -eq 0 ]]; then
		sudo apt update
		sudo apt install -y systemctl
	else
		apt update
		apt install -y sudo systemctl
	fi
fi

# Check if Chips is already installed on system
# If it's not installed then get the copy of pre-built binaries
# and install them in /usr/loca/bin/
# Then setup chips.conf 
# and download blockchain bootstrap too for quick setup
# Once ready, install Chips as system service and enables/starts it
if ! command -v chipsd &> /dev/null
then
    echo "chipsd could not be found"
    cd $HOME
    wget https://github.com/chips-blockchain/chips/releases/download/16.99.0/chips-bins-linux-amd64.tgz
	tar xvf chips-bins-linux-amd64.tgz
	sudo mv chips-cli chips-tx chipsd /usr/local/bin/
	rm -rf qt/ chips-bins-linux-amd64.tgz

	mkdir $HOME/.chips/
	cd $HOME/.chips/
	rm -rf chainstate blocks
	wget https://eu.bootstrap.dexstats.info/CHIPS-bootstrap.tar.gz
	tar xvf CHIPS-bootstrap.tar.gz
	rm CHIPS-bootstrap.tar.gz

	echo "server=1
#daemon=1
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

	echo "[Unit]
Description=Chips
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/chipsd
Restart=always
RestartSec=5s
SyslogIdentifier=CHIPS
User=$(whoami)
Group=$(whoami)

[Install]
WantedBy=multi-user.target" > chipsd.service

	sudo mv chipsd.service /etc/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable chipsd.service
	sudo systemctl start chipsd.service
	sudo systemctl status chipsd.service
fi


# Check if c-lightning is installed
# and if it's not found on system install it in /opt/lightning
if ! command -v lightningd &> /dev/null
then
    echo "lightningd could not be found"
    cd $HOME
    sudo apt update
    sudo apt install -y libsodium-dev
    wget https://github.com/chips-blockchain/lightning/releases/download/v0.5.2-2016-11-21-10051-geafd19526/c-lightning-linux-x86_64-v0.5.2-2016-11-21-10051-geafd19526.tar.gz
    tar xvf c-lightning-linux-x86_64-v0.5.2-2016-11-21-10051-geafd19526.tar.gz
    rm c-lightning-linux-x86_64-v0.5.2-2016-11-21-10051-geafd19526.tar.gz
    sudo mv dist /opt/lightning
    sudo ln -s /opt/lightning/bin/lightningd /usr/local/bin/
    sudo ln -s /opt/lightning/bin/lightning-cli /usr/local/bin/

    echo "[Unit]
Description=Chips c-Lightning daemon
Requires=chipsd.service
After=chipsd.service

[Service]
ExecStart=/usr/local/bin/lightningd --pid-file=$HOME/.lightning/lightning.pid --daemon
PIDFile=$HOME/.lightning/lightning.pid
User=$(whoami)
Type=forking
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target" > lightning.service
	sudo mv lightning.service /etc/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable lightning.service
	# sudo systemctl start lightning.service
	sudo systemctl status lightning.service
fi


# Install bet on system if not found
if ! command -v $HOME/privatebet/bet &> /dev/null
then
    echo "$HOME/privatebet/bet could not be found"
    cd $HOME
    sudo apt update
    sudo apt install -y libcurl4-gnutls-dev
    wget https://github.com/chips-blockchain/bet/releases/download/v0.1.3-529-g70182f2/bet-linux-x86_64-v0.1.3-529-g70182f2.tar.gz
    tar xvf bet-linux-x86_64-v0.1.3-529-g70182f2.tar.gz
    rm bet-linux-x86_64-v0.1.3-529-g70182f2.tar.gz
    echo "bet is located at $PWD/privatebet/"
    # sudo mv privatebet/bet /usr/local/bin
    # sudo mv privatebet/cashierd /usr/local/bin
    # mkdir -p $HOME/.pangea/
    # mv privatebet/config/* $HOME/.pangea/
fi

