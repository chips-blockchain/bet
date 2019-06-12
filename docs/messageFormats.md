Here we see the game flow along with message formats that are part of it. Below we dive into understanding the message formats we need to understand the entities in the gaming.

As we know, Pangea is a protocol which defines how the deck needs to be presented, shuffled, distrinuted among the players using the supporting entities `DCV (Deck Creating Vendor)` and `BVV (Blinding Value Vendor)`.

`BVV` nodes can be any set of the trsuted nodes, say here notary nodes can act like `BVV` nodes. Which can take part in deck shuffling, distributing and resolving the payment disputes.

`DCV` can act like a dealer node, where the GUI is cnfigured in the node and the players can access it via the dealer's IP like mentioned below. `DCV` also takes part in deck shuffling and distribution.

How the playing nodes need to be set up in the backend is very important in order for the player to start playing. First thing is every player who wants to play using Pangea they need to set up the bet node.

Detailed instructions on how to setup the bet node is mentioned [here](../README.md#Steps-to-compile). Once the node is setup in order to play, the player must need funds, here `CHIPS` is cryptocurrency which is used for betting and playing nodes are connected via `CHIPS-LN`.

In order to have the funds, one needs to create a CHIPS address in LN-Wallet and then needs to fund as shown below.
```
* $cd
* $cd lightning/cli/
* $./lightning-cli newaddr
{ "address" : "bFMfVr28hiQ1SMvM2efHkQL9b82B1LfATb" }
* To this newly generated address transfer the funds using any of the means, once you deposit the funds you can check the balance as follows:
* $./lightning-cli listfunds
{ "outputs" : 
	[ 
		{ "txid" : "803ca551a788da76fc7c5f1e9b6b9c87a76653887a4c316d01b9e676b6fc62b7", "output" : 1, "value" : 19499823, "status" : "confirmed" } ], "channels" : 
	[ 
		{ "peer_id" : "023aba3ac8e237fd4199c996163e6fe4c54c29f349e913d0a97e48d70863b23dc0", "short_channel_id" : "3481632:1:0", "channel_sat" : 494997, "channel_total_sat" : 500000, "funding_txid" : "803ca551a788da76fc7c5f1e9b6b9c87a76653887a4c316d01b9e676b6fc62b7" }, 
		{ "peer_id" : "02c015ab6ea37b95cc6939bb89da6473b0de87250ecd4c3b0d64ddf4c62c5679e8", "short_channel_id" : "3481636:1:0", "channel_sat" : 92300, "channel_total_sat" : 500000, "funding_txid" : "d0db104b8cf6eb9d59e2c16ffd79193fb2f94f31d075b97eab050d33f5eabf69" }, 
		{ "peer_id" : "03658a91403eb125345d7a87f4f91448297c312ccf250314fb9d541d9cf720497d", "short_channel_id" : "3481639:1:0", "channel_sat" : 52700, "channel_total_sat" : 500000, "funding_txid" : "4ecca8551a49155055d03fe535993e40ae2dca6d1c36810e6f9e8e87fbe03322" } ] }

```

In order to play the game the players and delaers should browse to `http://<ip_addr>/pangea-poker-frontend/client/`, where `<ip_addr>` is the IP address of the dealer(`DCV`) node where `poker` branch of `pangea-poker-frontend` running in the backend.

![Home page](./images/poker_home_page.png)


