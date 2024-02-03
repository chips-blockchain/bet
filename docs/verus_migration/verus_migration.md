Integrating chips into verus and use of vdxf ID's for bet
----------------------------------------------------------

In the existing setup the communication between the nodes/entities happen over sockets. Game info is stored in process memory, local DB and On chain. Process memory contains the game state and cards info which are necessary to play, local DB contains tx info and game state which is used for bookkeeping and to raise the disputes, On chain info is used by the cashiers to allow the players to join the table, settle the funds during the game and to resolve disputes if any. 

The main issue in using sockets for communication is its dependancy on the IP addresses. Not many can afford to have static public IP address and due to NATing we often see issues in discovering the nodes. Some of the crucial features like player disconnections became very hard as we haven't define data sets well enough for player to rejoin, and any diconnections on publisher side(i.e dealer) made it very complex to recover the lost game state. With vdxf ID's since we storing the data on blockchain, it's easy to maintain and retrieve the state of the game and that helps to handle player disconnections and disputes in a very reliable manner. Unlike earlier where we discover nodes using IP, using vdxf IDs we discover nodes with a registered identity on verus chipschain.  

Completely relying on vdxf ID's comes with a cost as every update we make to the data using vdxf ID's requires a tx on to blockchain and that costs a tx_fee and adds a block latency. For poker game on average a player node makes upto 20-25 data updates, that costs upto `25*tx_fee` per poker game, since the tx_fee of CHIPS is significantly lower the cost incurred for data updated per game is not very sinigificant and on the other hand to handle block latency for every data update, verus provides the API's to fetch the info from the TX's from mempool and also provides the functionality to make updates to an ID by taking into consideration of any relevant TX's relate to that ID in the mempool. 

### What verus vdxf ID's brings:

I see vdxf IDs are like a well organized restricted public database where in which we can store and retrive the data. We can organize this data under IDs, sub IDs with multiple levels of nesting. Under each ID the data is stored in key value pairs. 
Every ID is associated with an utxo, updateidentity is nothing but spending the utxo associated with that ID, when multiple authorized parties tries to update the ID, its like a race, whoever submits the spending tx first that one gets accepted and others get rejected, that means IDs gets updated with the values whose spending tx got accepted and other parties has to submit their changes in next block by making a new spending tx again. 

So with that in mind its important to notedown the following characteristics of ID updates:
1. When multiple partities trying to update either the same key or multiple keys of the ID at the same time(or you can say in the same block), then only the update tx that network sees first gets a chance and others simply gets rejected. 
2. For concurrent updates, one possible approach is always first check if there is any spent tx in mempool with regard to that ID, and make a spend tx on top of that mempool tx then in which case the multiple updates to the ID happens in the same block.`[Note: How this to be achieved is yet to test].`
3. As we know that we can store multiple key value sets in contentmultimap, but while designing itself to the possible extent always make sure that if any data that needs concurrent updates handle with multiple ID's, subID's. Many applications can't afford the waittime of one blocktime for every update to ID to happen.
4. `getidentity`only returns the data that is updated with latest UTXO.  In order to get the data that is updated on an ID over a period(end block height - start blockheight) of time we need to use `getidentitycontent`.

In bet we are representing the data in JSON format and exchanging it over sockets, unlike socket communications with vdxf IDs we need to be cautious about space like how we storing the data on chain and what data we are storing on the chain. To minimize the storage space on blockchain we encode the data as compact strucutres. At the moment we using structures to encode the data for few APIs and eventually once we get clear idea on contents of each update and what exactly an ID can hold then we can map the ID data to a structure and encode in hex and store.

How we going to create the ID's and managing them is an interesting thing to discuss on. The objective of bet is not just for poker but bet as a platform it has capability to host any card games, betting etc..., so with that in mind we create game specific ID's and any data relate to that game will be grouped under it. Lets say we have chips chain launched on verus, for poker game we register ID as `poker.chips@` and for blackjack we register ID as `blackjack.chips10@` and for betting we register ID as `bet.chips@` and so on.  Under chips the ID `poker.chips` groups all the information under it, and we again create subID's under `poker.chips` to organize the data for example a subID like `dealer.poker.chips` contains the information about the dealers and likewise `cashier.poker.chips` contains the information about cashiers. 

In the context of poker the more details about ID creation is mentioned [here](./id_creation_process.md) and about how we create and use keys without any conflicts is mentioned [here](./ids_keys_data.md).

### How we updating the ID's

Going forward to have API's from verus that can list all the information attached to that ID along with its history(i.e `getidentitycontent`, this API is available now). But right now, we have `getidentity` API using which we can only fetch the latest information that is updated to the `contentmultimap`.

In the scenaiors where for a given key where the values needs to be updated in an incremental way, one way of doing it using `updateidentity` is first to read the latest value of the corresponding key using `getidentity` and then append the incremental value to it and update that on chain using `updateidentity`, but since `getidentitycontent` has been available now we can update the data in an incremental way without appending it to the previously updated data.

For example, the key `chips.vrsc::bet.cashiers` is used for to store the cashiers info. So whenever a new cashier gets added that info is appended to the existing cahsiers info. Lets say we have cashiers array with two IP's as below:
```
chips.vrsc::bet.cashiers
{
			ips: [{1.2.3.4}, {a.b.c.d}]
}
```
and if a new IP {w.x.y.z} is to be added to this list, then the updated cashiers list looks as follows:
```
chips.vrsc::bet.cashiers
{
			ips: [{1.2.3.4}, {a.b.c.d}, {w.x.y.z}]
}
```

### Conversion between CHIPS and other PBaaS currencies in Mike words
```
    If you use liquidity baskets on-chain, you can convert between CHIPS and any currencies that was defined on any chain 
    in the Verus PBaaS network and Ethereum +any ERC20 as well. You can do all of this on the CHIPS chain and all conversion 
    fees will be split 1/2 to LPs and 1/2 to miners/stakers. You can use CHIPS or any other currency you want to support in 
    your game, even a liquidity basket currency. Any currency definition can be exported from one chain to another, converted 
    at the fairest possible rate on-chain with arbitrage hooks for miners and stakers, and used anywhere on the network. New 
    currencies or liquidity basket currencies defined on CHIPS, just like those defined on Verus, can be sent over to Ethereum 
    as well and will automatically be ERC20s on Ethereum. All the conversions will leave 1/2 the fees in the liquidity basket 
    currencies, raising their values relative to reserves, and the other 1/2 will automatically buy CHIPS from the liquidity 
    basket to pay miners and stakers.
```
### Some challenges

Couple of challenges we are exploring at the moment is to handle the heartbeat protocol, using sockets the connecting nodes send or response to the `live` message as a part of heartbeat protocol to get to know the stauts of the nodes. Since we completely going away from using sockets so with vdxf ID's we are exploring the options to implementing the heartbeat protocol, since the timer functionality is already been implemented in GUI, we incorporate same with vdxf ID's, i.e by enforcing the time to response (either absolute time or interms of block latency) other nodes can get to know the status of the node that needs to take an action.
