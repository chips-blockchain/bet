Here we see the game flow along with message formats that are part of it. Below we dive into understanding the message formats we need to understand the entities in the gaming.

As we know, Pangea is a protocol which defines how the deck needs to be presented, shuffled, distrinuted among the players using the supporting entities `DCV (Deck Creating Vendor)` and `BVV (Blinding Value Vendor)`.

`BVV` nodes can be any set of the trsuted nodes, say here notary nodes can act like `BVV` nodes. Which can take part in deck shuffling, distributing and resolving the payment disputes.

`DCV` can act like a dealer node, where the GUI is cnfigured in the node and the players can access it via the dealer's IP like mentioned below. `DCV` also takes part in deck shuffling and distribution.

How the playing nodes need to be set up in the backend is very important in order for the player to start playing. First thing is every player who wants to play using Pangea they need to set up the bet node.

Detailed instructions on how to setup the bet node is mentioned [here](../README.md#Steps-to-compile). Once the node is setup in order to play, the player must need funds, here `CHIPS` is cryptocurrency which is used for betting and playing nodes are connected via `CHIPS-LN`.

In order to have the funds 

Once the user browses to the `http://<ip_addr>/pangea-poker-frontend/client/`, below is the page that they get to see where `<ip_addr>` is the IP address of the node where `poker` branch of `pangea-poker-frontend` configured.

![Home page](./images/poker_home_page.png)


