## Configuring the cashier nodes
These are the nodes elected by the community and these nodes play very crucial role in establishing the trust in the ecosystem. The configuration details of these nodes are found in `cashier_nodes.ini` which is located at the path `~/bet/privatebet/config`. Since these nodes are elected by the CHIPS community so changes to this cashier configuration file is not allowed. 

Since there isn't any election for the cashier nodes is happened so no checks on the cashier nodes enforced either on the player or dealer side yet, so for now anyone can configure their own cashier nodes. 

To add more context, here is the overview of who read this file for what purpose:
1. Dealer picks up the ips from this config file and finds which are all reachable to it and compute msig from the pubkey of the reachable cashier nodes and this address is given to players to deposit their funds during the game.
2. Players use this config file, to check whether if the dealer provides the msig address from the nodes which are in the config file.
3. Dealer chooses the first available cashier node from this config file as the BVV for the deck shuffling process.
 

the changes to this configuration fileThese configuration settings can't be modified and any changes to it either by the dealer or the player nodes may result by the and changes made to it are ignored. 

```
[cashier]
node-1 = {                                                                                   \
		"pubkey":	"0377653051fe9dd919ad2d70422f692a43c46ac72fcfe2dfd3302ec30dd16aaf40",    \
		"ip":	"159.69.23.29"                                                               \
}                                                                                            #
node-2 = {                                                                                   \
		"pubkey":	"039ee4a07033df0add41e23ddc3865061e8eb97d719d843d5db2f3371b61a4eb34",    \
		"ip":	"116.203.25.27"                                                              \
}                                                                                            #
node-3 = {                                                                                   \
		"pubkey":	"0342e186006c560fe9bdc7bc0440b6fbfa46a4a852f5d4b28d67ab712ebf2e4744",    \
		"ip":	"159.69.23.30"                                                               \
}                                                                                            #
node-4 = {                                                                                   \
		"pubkey":	"03d0a6326bcf918aed07557462963953f2125a11cfcb9a7b05630f930e8e554956",    \
		"ip":	"159.69.23.31"                                                               \
}                                                                                            #
node-5 = {                                                                                   \
		"pubkey":	"03ec3475afbb7b0a241101ecbfba55f39862f7b9b5d77e492ba8354016e3379456",    \
		"ip":	"141.94.227.65"                                                              \
}                                                                                            #
node-6 = {                                                                                   \
		"pubkey":	"03ec892625ccd75710b68cf9f67f5c7a88936cd15215ba5a6071a5ed98717eca52",    \
		"ip":	"141.94.227.66"                                                              \
}                                                                                            #
node-7 = {                                                                                   \
		"pubkey":	"03086147692189929685b17b950f34bfef8158426f8475de9b0b20b8dcea17e7c4",    \
		"ip":	"141.94.227.67"                                                              \
}                                                                                            #
node-8 = {                                                                                   \
		"pubkey":	"025485b01a7bc485894ac6f31f4e9b6f9c6a8cf97a4927c6bad1a4128d5ae80800",    \
		"ip":	"141.94.227.68"                                                              \
}  
```

### To become a cashier node
The rules are not set yet, but there is a good discussion going on [here](https://github.com/chips-blockchain/bet/issues/193). Please feel free to share your views on it. Atm, I'm(sg777) hosting few nodes as cashier nodes and using them, going forward its going to be changed. 
