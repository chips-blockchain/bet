Integrating chips into verus and use of vdxf ID's for bet
----------------------------------------------------------

In the existing setup we communicate over sockets and any game related info we partly storing it in local DB(that helps player to raise disputes), partly on blockchain by making use of data part of tx(for cashiers to verify to make game settlements and to resolve game disputes) and partly in processs(that helps to maintain gamestate during the game). 

The main issue we have in using sockets for communication are IP addresses. Not many can afford public IP addresses and due to NATing we observed its not reliable. Some of the crucial features like player disconnections became very hard as we haven't define data sets well enough for player to rejoin, and any diconnections on publisher side made it this very complex. We believe using vdxf ID's we can organize the data more efficiently with availability and that in turn helps in solving many issues we have with the existing setup.

Is everything good with vdxf ID's, not absolutely. The update of data takes blocktime and tx_fee, during the game our initial estimate that we might be needing 20-25 updates to the ID's that means it's cost `20*tx_fee` more, we overcome this by reducing the mining fee.

### What verus vdxf ID's brings:

I see vdxf ID is like a well organized restricted public database(ah..did I mean blockchain?) where in which we can store and retrive the data. We can organize this data under ID's, sub ID's with multiple levels of nesting. Under each ID one can store the data by attaching it to a specific key values. It's like basket having paritition to store apples and oranges separately and even more..

For communications in bet we encoding it in JSON format and storing it on data part of blockchain, since we are not using any sockets when we migrate to use vdxf ID's its obvious that we need to be storing more info on to the vdxf ID's. To minimize the storage space on blockchain, we encode them as compact strucutres and store and retrive using with binary format.

How we going to create the ID's and managing them is an interesting thing to discuss on. The objective of bet is not just for poker but that its a platform that can be extended to any number of games, so with that in mind we create game specific ID's and any data relate to that game will be grouped under it. Lets say we have chips chain launched on verus, for poker game we register ID as `poker.chips@` and for blackjack we register ID as `blackjack.chips10@` and for betting we register ID as bet.chips@ and so on.  under chips as poker.chips and all the info relate to this game we store under poker.chips by creating the subID's under it. 

In the context of poker the more details about ID creation is [here](./id_creation_process.md) and about how we create and use keys without any conflicts is mentioned [here](./ids_keys_data.md).

### How we updating the ID's

Going forward to have API's from verus that can list all the information attached to that ID along with its history. But right now, we have `getidentity` API using which we can only the latest information that is updated to the `contentmultimap`.

In the scenaiors where in which if a new key value needs to be updated in an incremental way, for which first the latest key info will be read and to it new data been added then it will be updated using `updateidentity`.

For example, the key `chips.vrsc::bet.cashiers` is used for to store the cashiers info. So whenever a new cashier gets added that info is appended to the existing cahsiers info. Lets say we have cashiers array with two identifiers as below:
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

### Some challenges

Couple of challenges we are exploring at the moment is to handle the heartbeat protocol using the different nodes in the bet setup will know if the node is active or not and this heartbeat protocol communicate over the sockets. So here either by completely using vdxf ID's or by some means using nSPV we need to figure it out.



