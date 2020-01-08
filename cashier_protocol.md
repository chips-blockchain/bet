## Cashier Protocol

This document discusses about how the funds locked and released during the game with the help of notary nodes which are running the cashier protocol.

Notary nodes are the nodes which are elected and trusted by the network. The idea of having notary nodes to lock and release the funds is to remove the control over the funds from the dealer.

Once if the node is trusted and elected as a notary, it runs the cashier protocol server side. The implementation of cashier portocol is present inside the `privatebet` directory.

### Steps to run a notary node with cashier protocol
```
./bet cashier
```

### Notary nodes

The nodes which are trusted in the network are mentioned here. The trusted nodes to demonstrate how the cashier protocol works are mentioned below:
```
1. 159.69.23.28
2. 159.69.23.29
3. 159.69.23.30
4. 159.69.23.31
```
Each trusted node is associated with the public address which is a part of multisig address to which playing nodes sends the funds during the start of the game.

The notary nodes with their associated `P2SH` and `Pubic Key` are as below:
```
| Node IP      | P2SH                               | Public Key                                                         |
|--------------|------------------------------------|--------------------------------------------------------------------|
| 159.69.23.28 | bQepVNtzfjMaBJdaaCq68trQDAPDgKnwrD | 034d2b213240cfb4efcc24cc21a237a2313c0c734a4f0efc30087c095fd010385f |
| 159.69.23.29 | bSa7CrTXykfPZ6yhThjXAoQ8r4H7muiPPC | 02137b5400ace827c225238765d4661a1b4fe589b9b625b10469c69f0867f7bc53 |
| 159.69.23.30 | bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ | 03b020866c9efae106e3c086a640e8b50cce7ae91cb30996ecf0f8816ce5ed8f49 |
| 159.69.23.31 | bR7BXnWT1yVSP9aB57pq22XN2WYNpGgDrD | 0274ae1ce244bd0f9c52edfb6b9e60dc5d22f001dd74af95d1297edbcc8ae39568 |
```

The `m of n` multisig addresses are generated such a way that `m` should always be minimum of `ceil(n/2)` or greater than that. Some of the msig addressess which are generated and preconfigured are mentioned as below:
```
| m of n | msig address(in legacy representation) | associated addresses                                                                                                                                                                                                                                                                    |
|--------|----------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 2 of 3 | bQJTo8knsbSoU7k9oGADa6qfWGWyJtxC3o     | 1. 034d2b213240cfb4efcc24cc21a237a2313c0c734a4f0efc30087c095fd010385f 2. 02137b5400ace827c225238765d4661a1b4fe589b9b625b10469c69f0867f7bc53 3. 0274ae1ce244bd0f9c52edfb6b9e60dc5d22f001dd74af95d1297edbcc8ae39568                                                                       |
| 2 of 4 | bRCUpox55j6sFJBuEn9E1fwNLFKFvRvo9W     | 1. 034d2b213240cfb4efcc24cc21a237a2313c0c734a4f0efc30087c095fd010385f 2. 02137b5400ace827c225238765d4661a1b4fe589b9b625b10469c69f0867f7bc53 3. 03b020866c9efae106e3c086a640e8b50cce7ae91cb30996ecf0f8816ce5ed8f49 4. 0274ae1ce244bd0f9c52edfb6b9e60dc5d22f001dd74af95d1297edbcc8ae39568 |
```

When the game starts since all the trusted notary nodes and the corresponding public keys are preconfigured, each playing nodes identifies the notaries which are active that matches to the criteria set by the dealer. Here assume that if the dealer sets `2 of 4` then each playing node checks if atleast two of the notary nodes are reachable, if so then if the the player has the funds which is greater than or equal to the stack size set by the dealer then the playing node creates an `msig tx` to deposit the funds and shares this `txid` to the dealer.

Dealer then verifies whether the `tx` is legitimate or not as mentioned below:
```
1. Player sends the joining request, along with it's {public_key}.
2. Dealer sends the {stack_size, random_number} to the player.
3. Player checks if it has sufficient funds to join the table, 
  3.1 if yes,
    a. check the status of the notary nodes and deposit stack_size amount to the msig_address
    b. generates the signature for the random_number
    c. send {signature(random_number), tx} to the dealer
  3.2 if no, exit, refill the funds(which is done manually to the address in the chips wallet) and join again if the player wants to play.
4. Dealaer verifies the signature(random_number), if yes it verifies the tx as mentioned in step 5 else discard the message.
5. Steps to verify the tx.
  5.1 check if it's unspent.
  5.2 check if it's sent to the already preconfigured msig_address
  5.3 check if the amount in tx is equals to stack_size
6. if all checks passed in step 5, dealer sends the join_res  
```

Like this dealer keeps track of the joining players along with lock-in transaction as shown below:
```
| player_id | tx   |
|-----------|------|
| 1         | tx_1 |
| 2         | tx_2 |
| ...       | ...  |
| n         | tx_n |
```

Once the game is done dealer sends the spending request of these transactions to the winning player. 
