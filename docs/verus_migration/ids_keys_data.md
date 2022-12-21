Keys under a given namespace
----------------------------
Keys are ways to store the information under the identities, we encapsulate the info of a structure or JSON object and map them to the keys. For example we have an identifier named `cashiers` under `poker.chips10sec@` which holds all the cashiers info, we also define a `chips.vrsc::poker.cashier_key` where in which all the cashiers info is updated.

Our aim is to have as minimum ID's as possible to handle the data for two reasons.
1. Not everyone is authorized to create ID's that way we can prevent spamming and control over the namespaces.
2. Each ID creation needs some CHIPS and having the game design that creates ID's during the game creates burden on players in paying for the ID creation cost. 

We define all keys with the prefix `chips.vrsc::` in the entire CHIPS ecosystem, for the game specific keys we add the game name as prefix. For example, for all the keys that we use to play the poker name are defined with the prefix `chips.vrsc::poker.` and likewise for all the keys that we use to bet are defined with the prefix `chips.vrsc::bet.` and so on...

In the code for all the predefined keys, we compute their vdxfid's and map them for easy lookup. i.e, for key `chips.vrsc::poker.cashiers` the corresponding vdxfid is  `iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP` and we in the code we define them as follows.
```
//chips.vrsc::poker.cashiers --> iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
```

Some keys are predefined and some keys we create them on the fly during the game, all the keys that are created during the game are mapped to the predefined existing keys for easy reference and lookup. Some keys can be used across multiple ID's, for example the key `chips.vrsc::poker.t_player_info` is used to update the table info with both the dealer and table ID's.

Key Datatypes
-------------
Either its a structure or JSON object, we converting it to hex and using the datatype `vrsc::data.type.bytevector` we mapping it to the key and updating it to the ID, likewise to store string on to the ID we use the data type `vrsc::data.type.string` for the keys. We majorly been using bytevector for most of the keys defined int he CHIPS ecosystem, the definition of these data types in the code are as follows.
```
//vrsc::data.type.string -->  	iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c
#define STRING_VDXF_ID		 	"iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c"
//vrsc::data.type.byte -->  	iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t
#define BYTE_VDXF_ID 	   		"iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t"
//vrsc::data.type.bytevector --> iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3
#define BYTEVECTOR_VDXF_ID      "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3"
```

Identitites
-----------
We limit ourself to two levels of nesting under chips. At first level we mostly define identities are of game_types or any such things which are common across all the game types. For each game_type a corresponding identity is registered and we generate a token for it, further these tokens are used to register sub identities. All the transactions that happens in the game play use CHIPS for betting. The tokens we generate on the ID's registered has no significant value and they just be used to register subID's under that ID.

For example in the context of poker game we registered an identity named `poker` under chips as `poker.chips@`, and we generate a token named `poker` which is basically been used to create subID's under the poker ID.

Identities are often the addresses that can hold the tokens, ID's always ends with @. Any entity in the bet ecosystem can register the ID under chips, like for example  I registered an ID named sg777 under chips as `sg777.chips@` which basically been used to hold the tokens. Likewise `cashiers.poker.chips@` is an address players deposit funds to during the game. Even players are allowed to register the ID's and using which they can participate in the private tables.

Further, at high level we see what are all the keys that define under each ID in the namespace `poker.chips@`. Almost all the ID's in the poker ecosystem are of multiple signatures in nature, meaning they controlled by one or more primaryaddresses and some require one or more signatures to update them.

#### Cashiers ID
Address --> cashiers.poker.chips@
The keys that updates the data to this ID are
1. chips.vrsc::poker.cashiers

#### Dealers ID
Address --> dealers.poker.chips@
The keys that updates the data to this ID are
1. chips.vrsc::poker.dealers

#### Dealer ID
Address --> <dealer_name>.poker.chips@ //Dealer provides this name at the time of registration and all dealer names end with `_d` to avoid naming conflicts, e.g `sg777_d.poker.chips@`
The keys that updates the data to this ID are
1. chips.vrsc::poker.t_player_info

#### Table ID
Address --> <table_name>.poker.chips@ // Dealers can register upto 5 different table names and all table names ends with `_t` to avoid naming conflicts, e.g `sg777_t.poker.chips@`

The keys that updates the data to this ID are
1. chips.vrsc::poker.games --> Holds the info of all game id's [game1, game2, ...]

Table is the place where on the fly keys are created.  Each game is represented by a 32 byte random number calling it as gameID and this game ID is used to generate the on the fly keys during the game and these are used to update the table ID with the game info during the game. 

Lets say, for simplicity reasons we have gameID as `game1` then the following keys are generated during the game:
1. chips.vrsc::poker.t_table_info.game1
2. chips.vrsc::poker.t_player_info.game1
3. chips.vrsc::poker.t_player1.game1
4. chips.vrsc::poker.t_player2.game1
5. chips.vrsc::poker.t_player3.game1
6. chips.vrsc::poker.t_player4.game1
7. chips.vrsc::poker.t_player5.game1
8. chips.vrsc::poker.t_player6.game1
9. chips.vrsc::poker.t_player7.game1
10. chips.vrsc::poker.t_player8.game1
11. chips.vrsc::poker.t_player9.game1
12. chips.vrsc::poker.t_dealer.game1
13. chips.vrsc::poker.t_blinder.game1

While playing `game1` all the entities update the game info using these keys to the table ID.





