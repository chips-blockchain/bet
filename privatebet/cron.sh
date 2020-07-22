#!/bin/bash
SERVICE="cashierd"
date
if pgrep -x "$SERVICE" >/dev/null
then
    echo "$SERVICE is running"
else
    echo "$SERVICE stopped"
    cd /root/bet/privatebet
    ./cashierd cashier 159.69.23.30
    # uncomment to start nginx if stopped
    # systemctl start nginx
    # mail  
fi
