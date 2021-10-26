The whole communication between various entities is broadly classified into parts.
1. Backend communication.
2. GUI communication.

## Backend communication
By now we already aware that their exists player, dealer and cashier nodes in the system. The following combination of backend communications are possible during and after the game.
1. Communication b/w dealer and players.
2. Communication b/w dealer and BVV(Blinding Value Vendor or Blinder).
3. Communication b/w players and cashiers.
4. Communication b/w dealer and cashiers.

### Communication b/w dealer and players
As per our design there isn't any direct communication exists b/w the playing nodes and all the players must communicate through the dealer. There exists two layers of communication that happens b/w the dealer and playing nodes. Dealer node binds on to the ports `7797` and `7798`. On port `7797` dealer node communicates with pub-sub mechanism with the players and on port `7798` the dealer node communicates with push-pull mechanism with the players. 

The typical pub-sub model b/w dealer and players is shown below:

<img src="../../assets/Messages.png" width="500">

The typical push-pull medel b/w dealer and players is shown below:

<img src="../../assets/PULL.png" width="500">

So as you see, players push messages to the dealer on port `7798` using nng push-pull sockets, and when dealer wants to communicate anyhting with the players it publishes the messages on port `7797`.

### Communication b/w dealer and BVV
As per our design the cashier nodes also play the role of BVV. During the game if enough number of players joined the table, then dealer randomly pick the cashier nodes and check if they can be part of deck shuffling. Upon accepting the dealer request from the cashier nodes starts new thread to communicate with the dealer during the deck shuffling process. 

### Communication b/w players and cashiers
Players can communicate with cashier nodes to resolve any disputes using the DRP(Dispute Resolution Protocol). Basically if the game didn't played successfully due to player disconnections or by any other reasons then cashier nodes will reverse the payin_tx's of all the players involved in the game.

### Communication b/w dealer and cashiers
There isn't much communication exist beween the dealer and cashier nodes. At the start of the hand dealer informs the cashier nodes about the payin_tx that players made and at the end of the hand, dealer evaluates the hand and inform about payouts.


## GUI Communication
For any backend node that is running there is a webserver running on ports 9000. From the web client using websockets the corresponding entity can connect to the backend server and interact with the backend. Players use this GUI interface to play game, withdraw funds and to view game statistics where as dealer can use this GUI to configure tables and to observe the gameplay. So basically to connect to backend from GUI one needs to enter the IP address of the backend node in the web client and can connect to backend.


