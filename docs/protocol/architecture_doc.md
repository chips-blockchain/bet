Solving mental poker using blockchain is one of the hardest problems to solve in a decentralized space. To solve problems like these we need to address the blockchain trilemma. In our case we developed the layer2 infrastructure(a.k.a bet) which communicates with chips blockchain over a lightning network for fast, secure and scalable transactions. 

Use of lightning network comes with certain limitations like portability and setup. Here in our case we using c-lightning over CHIPS blockchain but unfortunately the c-lightning is not portable to windows and it communicates to CHIPS daemon over UNIX socket so that means lightning node should run on the same machine where chips nodes is running. With these constraints playing poker over chips is mainly lacking the user experience part. Here in this article we'll discuss the possible ways to improve the UX. 

We first understand the existing flow and setup needed to play the poker and from then we discuss the possibilities and constraints in developing of full pledged web app or desk app. Lets take a look into the existing flow diagram of poker game, in the following flow diagram we shown only one dealer and cashier node for simplicity.

![Existing_flow](https://user-images.githubusercontent.com/8114482/158165776-bec9c813-23ee-4e81-8e64-29d49750e320.png)

As of now, in order to play poker using bet on chips, one has to either setup the nodes either by compiling the source code or by taking the release binaries. Both of these approaches are not very ideal for a naive user. As discussed we see how can we minimise this setup process and we discuss the constraints and possibilities of developing the full pledged web or desktop app.

**A complete web app**

The bet code is written in C which does the deck shuffling and has the necessary functionalities to play the poker. Since mental poker is a very crypto intensive task and for which we using existing efficient implementations of the corresponding libs and that makes rewriting all of it in js is not so straightforward thing. 

For understanding, let's take a look at the flow during deck shuffling to get some fair idea on the complexity of the crypto operations involved:

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


