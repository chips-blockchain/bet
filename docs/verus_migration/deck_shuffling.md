Deck Shuffling
---------------

The entire security of the game depends upon how the deck is shuffled and how the cards are revealed during the game. Here we discuss the process involved in deck shuffling and how we be using ID's to accomplish this task.

### Representation of Deck
We represent each card in the deck as a field element of the curve25519, to represent a point on the curve we need 256 bits so likewise the representation of each card need 256 bits or 32 bytes. The space needed to represent the deck is `52*32` bytes, i.e `1664` bytes.

### The shuffling
In simple terms here is how the deck shuffling happens.
1. Each player generates a deck of cards, shuffles them, blinds them(so that no other can read the deck except the player who generated it).
2. Dealer reads each player deck separately and shuffles them, blinds them.
3. Cashiers reads the each of the dealer shuffled blicded desk, shuffles them and blinds them.

So on a high level we can see that for each player deck is shuffled by player, dealer and blinder. So that way the collusion of any two entities is not possible to reveal the deck.

### Some numbers
Here we see what's the size of the data that we going to be updated on to the ID during the deck shuffling process.

Space to represent the deck = `52*32` = `1664` bytes.<br/>
If there are n players, then total update space on ID  = `n * 1664` bytes.<br/>
The space needed to store the dealers blinded desk on ID = `n * 1664` bytes. [since dealer blinds all the player desks individually]<br/>
The space needed to store the blinders blinded desk on ID = `n * 1664` bytes. <br/>
Blinder reveals its share using shamir secret sharing, the number of shamir shards possible for n players = `n * 1634` bytes.<br/>

Total space that is needed = `1664 * (4n)` bytes [By combining all the above equations]<br/>
The maximum number of players that can be accomodated is 9 players, so by substituting n=9 in the above equation, 
i,e   `1664 *4n` = `1664*4*9` = `59904` bytes. So minimum amount of data that is getting updated on to the ID for deck shuffling is of `59904` bytes.

### Handling deck shuffling with vdxf ID's
Here we see, how we store the deck shuffling data in the contentmultimap of table ID. For each player there is a dedidcated key on the table at which the corresponding player stores its data.
At the moment we identified multiple keys on the table, whose values gets updated at various instances during the process of the game. The key values are as follows:
1. `chips.vrsc::poker.t_table_info` --> Contains the table info, which is updated by the dealer when it starts the table.
2. `chips.vrsc::poker.t_player_info` --> Contains the info about the players, like its spot on the table, its joining status, etc.. and this key is updated by cashier the momemt it receives the payin_tx using blocknotify.
3. `chips.vrsc::poker.t_player1` --> Contains the info about the player `1` blinded deck, likewise there exists 9 different keys, one for each player from `chips.vrsc::poker.t_player1` to `chips.vrsc::poker.t_player9`
4. `chips.vrsc::poker.t_dealer` --> Contains the info about the dealers blinded deck.
5. `chips.vrsc::poker.t_blinder` --> Contains the info about blinders blinded deck along with the encrypted shamir shards.
