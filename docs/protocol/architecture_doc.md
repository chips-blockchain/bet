### Overview

Solving mental poker using blockchain is one of the hardest problems to solve in a decentralized space. To solve problems like these we need to address the blockchain trilemma. In our case we developed the layer2 infrastructure(a.k.a bet) which communicates with chips blockchain over a lightning network for fast, secure and scalable transactions. 

## System in place

Use of lightning network comes with certain limitations like portability and setup. Here in our case we using c-lightning implementation of ln over CHIPS blockchain. Unfortunately the c-lightning is not portable to windows,  since it communicates to CHIPS daemon over UNIX socket which means lightning node should run on the same machine where chips nodes is running. With these constraints playing poker over chips is mainly lacking the user experience part. Here in this article we'll discuss the possible ways to improve the UX. 

We first understand the existing flow and setup needed to play the poker and from then we discuss the possibilities and constraints in developing of full pledged web app or desk app. Lets take a look into the existing flow diagram of poker game, in the following flow diagram we shown only one dealer and cashier node for simplicity.

![Existing_flow](https://user-images.githubusercontent.com/8114482/158165776-bec9c813-23ee-4e81-8e64-29d49750e320.png)

As of today, to play poker one has to setup the nodes either by compiling the source code or by taking the release binaries. Both of these approaches are not very ideal for naive users. To minimise the complications involved in this setup process we further discuss the possibilities of developing a full scale web or desktop app. 

## A complete web app

The bet code is written in C which does the deck shuffling and other necessary functionalities to play the poker. Since mental poker is a very crypto intensive task and for which bet is using the existing implementations of the corresponding crypto libs and that tight dependancy on the external libs makes the job of porting the bet into web based prorgamming languages like js very difficult.

For understanding, let's take a look at the flow during deck shuffling to get some fair idea on the complexity of the crypto operations involved:

![deck shuffling](https://user-images.githubusercontent.com/8114482/158166277-a0b107f1-90c4-4f74-a38c-7971ce93e46f.png)

In the above diagram if you see compared to the player node, the dealer and cashier nodes needs to perform more crpto intensive tasks during the process of deck shuffling, game play and disputes. In traditional mental poker protocols where in which as the number of players increases the computation burden on players increase significantly. We designed our poker protocols such that with the increase in number of players the more of computational burden goes to dealer and cashier nodes and mostly the computational bueden on playing nodes remains constant during deck shuffling and there is slight increase in computational complexity during the game play and which is obvious in any scenario where in which the number of players increases. 

When I abstracted just the player side crypto operations and ran it in the browser I see there isn't much difference in the performance. So that gives confidence that we can port player functionalities onto web, but it only solves the problem partly. 

![webapp_flow_diagram](https://user-images.githubusercontent.com/8114482/158166654-86adbeae-e5b3-4d62-a6fc-4d4b7bc8cb81.png)

If you look into the above flow diagram there are few things which still need clarity. e.g like from web how the communication should happen with the chips and ln nodes is not yet figured out. There are web wallets developed for BTC and web lightning networks for BTC(Chrome extension Joule) exist in place. Since CHIPS is a BTC fork, provided if we come up with the web wallet for CHIPS(like metamask) it solves the problem of interaction with the CHIPS blockchain from the browser, but for player node to make the payments from browser even with the existing approaches on BTC one must need to run the LN node in the backend. Since we already have poker without LN, so if we have web wallet for CHIPS we actually develop a full scale web app to play poker and any other games using bet.

To summarize here are two main tasks in developing the full scale web app for bet:
* Developing web wallet for CHIPS.
* Rewriting player side bet logic into browser.

## Bet without LN
So here comes the question: **Can we play poker without the Lightning Network?** The short answer is yes and we already implemented this.
Before we jump into the answer lets understand why need LN, in our case we using LN to record bets during the gameplay, and in any turn based games this game info is very crucial to record in the blockchain for non-repudiation purposes to resolve the conflicts. One approach i think of is to run a sidechain by the cashier nodes as validators where in which these nodes record the game info on the blockchain. 

Here is the flow diagram that depicts how things will work in the side chain scenario.

![bet_without_ln](https://user-images.githubusercontent.com/8114482/158166995-7f5f72a5-6e86-432d-8405-5643a387b53f.png)

But the good thing with chips is its blocktime is less than 10s and you can mine the chips block on your local computer. So that actually makes things much easier for us and we actually can use chips blockchain to record to record the game moves and we did the same thing in bet without ln. 

## Desktop app

In bet without ln, we already seen we can play poker without ln, now in that scenario we only have chips and bet repos. We now can bundle the chips and bet binaries in linux based environments and make a release. 
For windows, we are able to generate the cross compiled binaries and we make it avialable to the players so that one can simply download the binaries and play.
