#!/bin/bash

printf "\n\nThis will take about 3-5 minutes. Please be patient.\n\n"

lightgreen="\e[1;32m"
lightblue="\e[1;34m"
reset="\033[0m"

prog() {
    local w=80 p=$1;  shift
    # create a string of spaces, then change them to dots
    printf -v dots "%*s" "$(( $p*$w/100 ))" ""; dots=${dots// /.};
    # print those dots on a fixed-width space plus the percentage etc.
    printf "\r\e[K|%-*s| %3d %% %s" "$w" "$dots" "$p" "$*";
}

progressBar() {
    for x in {1..100} ; do
        if [[ "$x" -eq 100 ]]
        then
            prog "$x" finished
        else
            prog "$x" still working...
        fi
        sleep $1
    done ; echo
}

# setting up CHIPS

chipsuser=$(openssl rand -hex 12)
chipspass=$(openssl rand -hex 12)

echo "Creating CHIPS config\n"

cd ~/.chips && 
cat >> chips.conf <<EOL
server=1
daemon=1
txindex=1
rpcuser=chipsuser$chipsuser
rpcpassword=$chipspass
addnode=159.69.23.29
addnode=95.179.192.102
addnode=149.56.29.163
addnode=145.239.149.173
addnode=178.63.53.110
addnode=151.80.108.76
addnode=185.137.233.199
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
EOL

cd ~/chips/src && ./chipsd &
sleep 2
progressBar "0.1"

echo "Starting Lightning node"
~/lightning/lightningd/lightningd --log-level=debug > ~/lightning.log 2>&1 &

progressBar "1.5"

# Get a new address to fund your Lightning Node
# This returns an address, which needs to be funded first in order to open a channel with another node.
newaddr=$(lightning-cli newaddr)

printf "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n"
printf "CHIPS and Lightning Node are running\n"
printf "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n"

printf "${lightblue}NEXT STEPS${reset}\n\n"

printf "1. FUND YOUR CHIPS ADDRESS with CHIPS to be able to join the game.\n"
printf "$lightgreen{ \"address\": \"bUBbFW6yYTH3qXmynTyBtQ1FEcVSDT7ZNw\" }${reset}"
printf "\n\n"

printf "2. Run bet (player OR dealer).\n\n"
printf "DEALER \n"
printf "cd ~/bet/privatebet && ./bet dcv 45.77.139.155"
printf "\n\n"
printf "PLAYER \n"
printf "cd ~/bet/privatebet && ./bet player\n\n"



