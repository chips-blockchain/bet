# Introduction

Since the pangea is fully decentralized protocol, the funds needed during the game will be managed from the player's backend wallet. The simple thing if one wants to play using any gaming application designed over pangea protocol, the player needs to deposit the funds to the address which the player owns and can withdraw to the addresses which the player owns in the backend wallet.


# Depositing the Funds
When the player connects to the backend node via GUI, the below information is pushed to the GUI from the backend. The backend node gives the details about the existing balance, and the addr which the backend node can spent any tx associated to it.
```
{
    method: "walletInfo", 
    addr: "bQepVNtzfjMaBJdaaCq68trQDAPDgKnwrD", 
    balance: 0.13269034
}
```
Depending on the stack size and the amount of CHIPS available, the player node deposit any amount greter than `(stack_size-balance-tx_fee)` in order to join the corresponding table and play.

## Conditions on depositing the funds
1. Funds can be transferred from any address to the address provided by the backend node.

# Withdrawing the Funds
At any point of time, if the player wants to withdraw the funds then player can only withdraw the funds to any of the addresses owned by the backend wallet. The reson for this restriction is, players are connecting to the backend just by entering the IP of the back end node in the GUI.
So if the betting node is running in the backend, then anyone with the knowledge of the IP can actually connect to the backend and there is no authorization required for this to happen. So player can connect to the backend node without any authorization in order to play the game, during such scenarios if we allow withdraw the funds to any address then there is a possibility that attacker/intruder can withdraw funds to his own wallet.
By restricting the withdrawl of funds only to the addresses which belong to the backend node, we can prevent the unauthorized withdrawl of the funds. 

The sequence of steps that occur during the withdrawl of the funds is depicted below:
## step1: GUI sends withdrawRequest to the backend
```
{
   method: withdrawRequest
}
```
## step2: Backend sends withdrawResponse to the GUI
```
{
    method: withdrawResponse,
    balace: xxxx,
    addr: [addr1,addr2,...,addrn]
}
```
Where `addr` contains the list of addresses which are owned by the backend wallet.
## step3: GUI sends withdraw to the backend
```
{
    method: withdraw,
    amount: yyyy,
    addr: some_addr_from_the_above_list
}
```
When the player receives the withdraw, it checks whether the `addr` is actually owned by the backend wallet or not, the reason for this check is since the player is the only one who can access the backend wallet either remotely or directly in an authorized approach so he can only spend the funds belongs to the backend wallet.

## step4: Backend sends withdrawInfo to the GUI
 ```
 {
   method: withdrawInfo,
   txid: tx_id,
   balance: zzzz
}
```
## Conditions on withdrawing the funds
1. Funds can be transferred only to the address which are owned by the backend wallet. 

# Improving the usability

Say for example, if the player wants to maintain the funds via GUI wallet, the following steps needs to be followed:
1. Players generates the new address in the backend wallet.
2. Export the backend wallet to the GUI wallet which the player wishes to use to manage the funds.
That's it, so now the wallet in the GUI and backend node knows the privkey of the address, then the player can spend the funds from anywhere.

# Security Concerns

From the above discussion it's clear that funds can't be moved out of the players wallet. 

Any concerns related to the security will be discussed here.
