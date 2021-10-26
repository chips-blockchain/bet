## CHIPS Upgrade
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

## LN Upgrade
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

