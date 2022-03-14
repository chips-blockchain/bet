Solving mental poker using blockchain is one of the toughest problems to solve in web3 decentralized space. To solve problems like these we need to address the blockchain trilemma. 

In our case we developed the layer2 infrastructure(a.k.a bet) which communicates with chips blockchain over a lightning network for fast, secure and scalable transactions.

Even though we addressed the blockchain trilemma, we lack the user experience part. The possible ways to improve the UX is to either to develop a webapp where users can play poker without any setup or to develop native desktop app.

We discuss the tradeoffs in launching poker using web and desktop apps. Before we jump into exploring the possible solutions to improve the UX, let's take a look into the existing flow diagram of Poker and then we discuss the pain points for the end user in playing the poker using the existing approach. 

In the following flow diagram we showcased only one dealer and cashier node for simplicity.

![Existing_flow](https://user-images.githubusercontent.com/8114482/158165776-bec9c813-23ee-4e81-8e64-29d49750e320.png)

**A complete web app**

The Layer2 infrastructure, i.e bet is written in C. Since mental poker is a crypto intensive task and the platform which we developed uses lots of external crypto and p2p libs, it wasn’t a straightforward thing to rewrite everything in js or to make this whole thing run in the browser. 

For understanding, let's take a look at the flow during deck shuffling:

![deck shuffling](https://user-images.githubusercontent.com/8114482/158166277-a0b107f1-90c4-4f74-a38c-7971ce93e46f.png)

We find out that running rewriting the player node logic in the browser (in js) is possible,  since unlike mental poker which is not scalable for more than 2 players, the bet protocol is designed such a way that the player need to perform very minimal crypto operations for deck shuffling and retrieval.  So with this we can develop the web app and deploy in the browser so that the player side computations can be run in the browser. 

It only partly solves the problem and the flow diagram in this scenario looks as follows:

![webapp_flow_diagram](https://user-images.githubusercontent.com/8114482/158166654-86adbeae-e5b3-4d62-a6fc-4d4b7bc8cb81.png)

But here in this scenario from webapp how the communication should happen with the chips and ln nodes is not yet figured out. There are web wallets developed for BTC and web lightning networks for BTC(Chrome extension Joule) exist in place. Since CHIPS is a BTC fork, provided if we come up with the web wallet for CHIPS(like metamask) it solves the problem of interaction with the CHIPS blockchain from the browser, but for player node to make the payments from browser even with the existing approaches on BTC one must need to run the LN node in the backend. So with the possible solutions I’m aware of, it wouldn’t be possible to deploy everything from the browser with the current setup we have.

So here comes the question: **Can we play poker without the Lightning Network?**
In our scenario, we are using a lightning network to record bets during the gameplay, and in any turn based games this game info is very crucial to record it in the blockchain for non-repudiation and to resolve the conflicts. One approach i think of is to run a sidechain by the cashier nodes as validators where in which these nodes record the game info on the blockchain and since the player funds are locked with the cashier nodes when player joins the table and if every bet made by the player is recorded in the blockchain run by the cashier nodes, the players can’t deny their moves in this way we can eliminate the use of lightning network. 

Here is the architecture/flow diagram that depicts how things will work

![bet_without_ln](https://user-images.githubusercontent.com/8114482/158166995-7f5f72a5-6e86-432d-8405-5643a387b53f.png)

For this to achieve here are the following major tasks which we need to accomplish:
1. Develop the player poker logic in a webapp using JS.
2. Develop the side chain run by validator nodes.
3. Develop the web wallet for CHIPS.


