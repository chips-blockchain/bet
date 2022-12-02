Integrating chips into verus and use of vdxf ID's for bet
----------------------------------------------------------

In the existing setup we communicate using sockets and any game related info we storing it by making dust transactions and payment related info using the normal transactions. In all the cases we are attaching data to these transactions.

The main issue we identified using this apprach is in establishing the socket connections where in which all the entities should be having the public static IP addresses. Player disconnections also didn't handled well, when player disconnects game aborts and everyone gets their funds, so I can say I haven't handled the player disconnections at all.

What verus vdxf ID's brings:

I see vdxf ID is like a public database where in which we can store and retrive the data. In order to store the data we need to create ID's, and to the ID's we can attach the data using as key value pairs.

For communications in bet we encoding the data in JSON, storing the entire JSON object on the blockchain using vdxfID's is very expensive in terms of size. So one approach is like we encode the as a binary onject with out key names, like the way we encode network packet headers where in which the fields are of constant size we just stack up the values to form a binary blob of fixed size. For any JSON key which is of variable length we add length followed by the data. Only extra work we need to do for this is to write decoder and encoders for each of the different JSON objects we been using.

When it comes to grouping the ID's, we go by grouping them based on the game, lets say we have chips chain launched on verus, for poker game we register the name poker under chips as poker.chips and all the info relate to this game we store under poker.chips by creating the subID's under it. 

chips
	poker
		cashiers
		dealers		
		players
		tables
and also the idea is to limit the nesting to 2 levels under chips and only token we define is for poker, that will be used across to create ID's under poker.

To avoid conflicts with other namespaces, as mentioned by mike every ID we create should be prefixed with chips.vrsc::
For example lets say we need to publish all cashier nodes info for which we define a key something like chips.vrsc::bet.cashiers and to which we store the data.
In the scenaiors where in which if a key value needs to be updated in the incremental way, for which first the latest key info need to be read and then the new data which we need to add should be append on top of existing data and publish it all.

Lets say here in our case the key chips.vrsc::bet.cashiers is used for to store the cashiers info which is an array of cashier identities as shown below:

chips.vrsc::bet.cashiers
{
			ips: [{1.2.3.4}, {a.b.c.d}]
}
and if a new IP {w.x.y.z} is added to the list of existing IP's then first the existing info should be read and the new IP {w.x.y.z} should be append to the existing list and the whole new list has to be published and it should be like as follows:
chips.vrsc::bet.cashiers
{
			ips: [{1.2.3.4}, {a.b.c.d}, {w.x.y.z}]
}

Couple of challenges we are exploring at the moment is to handle the heartbeat protocol using the different nodes in the bet setup will know if the node is active or not and this heartbeat protocol communicate over the sockets. So here either by completely using vdxf ID's or by some means using nSPV we need to figure it out.



