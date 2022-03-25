## Bet without LN
Here we discuss about various aspects of using bet without LN. In a fully decentralized system its important to record every move in the blockchain for non-repudiation and to maintain trust or to resolve disputes. In this [article](./architecture_doc.md) we discussed why we need to go for bet without ln. 

## Configuring player and dealer nodes
### Dealer configuration
In `dealer_config.ini` under `[dealer]` section dealer can config `bet_ln_config` field. By default it is set to `N`, means that the game moves will be recorded in the chips blockchain and it allow only the players whose configuration settings matches to this to join the table.

```
bet_ln_config        = N           # Set to N(no) if the dealer intends to record the game info on the CHIPS blockchain, else Y is the dealer intends to use lightning network to record game info in ln invoice.
```
### Player configuration
In `player_config.ini` under `[player]` section player can config `bet_ln_config` field. By default it is set to `N`, means that player wants the game moves to be recorded in the chips blockchain and it only looks to join the tables where in dealer config settings matches to this.
```
bet_ln_config              = N      #Set to N if the player willing to use CHIPS blockchain to record game info instead of LN, else Y.
```
## Advantages and Disadvantages of using bet without LN

### Advantages
* **No need of establishing and funding LN channels** Earlier we need to establish LN channel between player and dealer and player has to fund this channel(though very monimal) and it cost one chips blocktime in establishing the LN channels if there isn't any preestablished LN channel b/w player and dealer. Sometimes if chips and ln nodes are not synced fully then we have to wait for ln node to be in full sync with the underlying blockchain node.
* **Portability and easy to setup** <br/>We using c-lightning implementation of LN which is not portable to windows so that limit the players to use either linux or mac environments in order to setup the nodes to play poker. Now without LN, the player just need to have take the binaries of bet and chips and can play.
* **Avoiding local DB to store game info** <br/> Earlier with ln, dealer creates the invoice during the game correspond to the bet amounts and this invoice label info contains the info about the game move and these invoices are stored locally by both dealer and player nodes to provide as a proof in case of disputes to the cashier nodes. Now without ln, player simply makes a dust tx with the game info along with the bet amount in the data part of the tx, since this is available on the chips blockchain there is no need to keep this info locally.
* **Better display on GUI** <br/> All the tx's related to a specific contains the unique table_id, so by filtering with the table_id in the explorer we can showcase the history of the games played beautifully.
* **Benifit to miners and chips ecosystem** This increases tx volume and and also benifits miners with tx_fee.

### Disadvantages
* **chips tx fee** Since players records every move on chips blockchain by making the dust tx, so for every move the it costs player a default chips tx_fee of 0.0005 CHIPS. Lets say during the game if the player makes 10 moves, in which case it incurs a total tx_fee of `0.0005*10`, i,e 0.005 CHIPS.
* **Latency** If we use LN, the invoice creation and payments are instant, without LN for every move the game incurs a latency of one chips blocktime. Since the chips blocktime is less than <5s it's a very reasonal trade off.



