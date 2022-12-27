Deck Shuffling
---------------

The entire security of the game depends upon how the deck is shuffled and how the cards are revealed during the game. Here we discuss the process of deck shuffling and how we be using ID's to accomplish this task.

### Representation of Deck
The section 3 of the the [curve25519 paper](https://cr.yp.to/ecdh/curve25519-20060209.pdf) provides the hints about how to generate random numbers that are chosen to be the private keys.
```
The legitimate users are assumed to generate independent uniform random secret keys. A user can, for example, generate 32 uniform 
random bytes, clear bits 0, 1, 2 of the first byte, clear bit 7 of the last byte, and set bit 6 of the last byte.
```
In our case, each card is a 32 byte random number as per guideline given in the curve25519 paper, and we used prenultimate byte(i.e 31st byte) to store the index of the card and this index can be any number in the range `[1-52]`, these numbers are mapped to the corresponding cards in the deck. Likewise, we need one such random number to represent the each card in the deck, for a deck of 52 cards we need 52 different 32 byte random numbers, spacewise it need `52*32`, i.e `1664` bytes to represent the deck.

### Shuffling of the deck
In this section we see how we shuffling the deck at a high level:
1. Player choses the deck, blinds it and updates at table ID.
2. Dealer reads the deck, shuffles it, blinds it and encrypts it with cashiers pubkeys and updates at the table ID.
3. Cashier reads the deck, decrypts it, shuffles it, blinds it and updates back at the table ID. 

The above process repeats for all the players.

Note: If more cashiers inolved in the process of deck shuffling, then each cashier shuffles the deck and blinds it and updates back at the table ID. Atleast one cashier must be involved in the deck shuffling process, more the number of cashiers involved which adds up more trust to the deck shuffling process.
 
### Revealing of the deck
From the deck shuffling its clear that the all players having the deck with the same shuffled order. As we see the apart from player dealer and blinder also blinded the deck. So in order to reveal the deck the player has to get these blinded values. Here we see in detail about how the player reveals its card.

The values that are public after deck shulling process are below and these values are available at the table ID.
1. Shuffled deck.
2. The corresponding points for the blinding values of the dealer on the curve25519.
3. The merkel hash of the blinding values chosen by the cashier has to be published for each set of random numbers chosen for the players.

Lets get into the details further in revealing the card:
1. Dealer tells whose player turn it is along with the index in the deck.
2. Cashier reveals the corresponding blinding value used for that player for that index.
3. By knowing the blinding value, with the combination of public and private information that player had it can able to reveal the card.
4. If a card is a board card, then all players reveal that card to each other.

Few things like player shuffling/unshuffling and the use of shamir secret key sharing to reveal the cashiers blinding values are removed as they didn't add up anything to the security as per the ID design we been using.

### Some numbers
Here we try estimating the approx amout of data that gets updated to ID during the deck shuffling process. 
```
As we already seen that to represent a deck we need `52*32`, i.e `1664` bytes.
deck_size = 1664 bytes
For Players
-----------
Each player has their own deck, so for n players it will be n*deck_size bytes.

For Dealers
-----------
Dealer shuffles, blinds and encrypts each deck, so the size of all encrypted decks by the dealer is equal to n*deck_size bytes, 
along with that dealer publishes the corresponding points of its blinding values its of size deck_size. 
so bytes used by dealer = n*deck_size + deck_size = (n+1)*deck_size.

For Cashiers
------------
Cashiers shuffle, blinds each deck separately, and also publishes the merkelproof of the blinding values they chosen. 
In total the bytes used by cashier to do that = (n+1)*deck_size bytes.

In Total
--------
Total no of bytes needed = n*deck_size (n+1)*deck_size +(n+1)*deck_size = (3n+2)*deck_size bytes.

For a maximum of n=9 players, the min(also max) bytes needed  = (3n+2)*deck_size = (3*9+2) *1664 = 48,256 bytes = 47.124 KB.
```

### Handling deck shuffling with vdxf ID's
Here we see, how we store the deck shuffling data in the contentmultimap of table ID. For each player there is a dedidcated key on the table at which the corresponding player stores its data.
At the moment we identified multiple keys on the table, whose values gets updated at various instances during the process of the game. The key values are as follows:
1. `chips.vrsc::poker.t_table_info` --> Contains the table info, which is updated by the dealer when it starts the table.
2. `chips.vrsc::poker.t_player_info` --> Contains the info about the players, like its spot on the table, its joining status, etc.. and this key is updated by cashier the momemt it receives the payin_tx using blocknotify.
3. `chips.vrsc::poker.t_player1` --> Contains the info about the player `1` blinded deck, likewise there exists 9 different keys, one for each player from `chips.vrsc::poker.t_player1` to `chips.vrsc::poker.t_player9`
4. `chips.vrsc::poker.t_dealer` --> Contains the info about the encrypted dealers blinded deck along with the points on the curve25519 for the corresponding blinded values.
5. `chips.vrsc::poker.t_blinder` --> Contains the info about cashiers blinded deck along with the merkelproof of all the blinding values that are used to mask the deck.
