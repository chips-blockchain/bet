# All about ID's and Keys

## ID Registration
Can we can restrict the ID registration process to few authorized addresses or can it can made open so that anyone can register ID's? Here is the insights from Mike(Verus fouder/dev) on why ID registration should be open.
```
    First one is that I think you should not restrict the ID registration capabilities, as they will be a primary source of 
    economy and since you do not have block rewards, could make the difference between a functional chain or not. There is no 
    spam because all resources used on chain are paid for to miners, stakers, LPs in baskets that register decentralized IDs, 
    and those selling IDs on-chain. All of this is economic value for the on-chain economy that is not spam. It is exactly the 
    opposite. If your chain is busy with that, everyone should be happy. If it is too busy, make another. Also, people have IDs 
    on different chains that they should be able to send over and use. IMO, each player should have to have an ID, and if some 
    games need KYC, you will be able to do that as well on any IDs using the Value service, which will use VerusID to enable 
    non-DOXed KYC verification. I really recommend that the general model is to consider IDs just on-chain addresses and let 
    people use them as they will. Enable poker and other games this way, but have all of the on-chain activity, including 
    currencies, to benefit miners and stakers (CHIPS holders) through the fee pool as part of normal use.
```
Earlier in some sections we talked about only few can be cashiers and few can be dealers once they met certain conditions. 
We can actually seperate this process into two steps:
1. ID creation
2. ID Registration
Anyone can create any numer of ID's, but to register an ID as either dealer or cashier certain conditions to be met, we streamline this process and provide an API for it`(TO DO)`.

## Key - Value pair
Under IDs information is stored in key-value pairs. We encapsulate the info of a structure or JSON object and map them to the keys. For example we have an identifier named `cashiers` under `poker.chips10sec@` which holds all the cashiers info, we also define a `chips.vrsc::poker.cashier_key` where in which all the cashiers info is updated.

We define all keys with the prefix `chips.vrsc::` in the entire CHIPS ecosystem, for the game specific keys we add the game name as prefix. For example, for all the keys that we use to play the poker name are defined with the prefix `chips.vrsc::poker.` and likewise for all the keys that we use to bet are defined with the prefix `chips.vrsc::bet.` and so on...

In the code for all the predefined keys, we compute their vdxfid's and map them for easy lookup. i.e, for key `chips.vrsc::poker.cashiers` the corresponding vdxfid is  `iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP` and we in the code we define them as follows.
```
//chips.vrsc::poker.cashiers --> iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
```

Some keys are predefined and some keys we create them on the fly during the game, all the keys that are created during the game are mapped to the predefined existing keys for easy reference and lookup. Some keys can be used across multiple ID's, for example the key `chips.vrsc::poker.t_player_info` is used to update the table info with both the dealer and table ID's.

## Key Datatypes
Either its a structure or JSON object, we converting it to hex and using the datatype `vrsc::data.type.bytevector` we mapping it to the key and updating it to the ID, likewise to store string on to the ID we use the data type `vrsc::data.type.string` for the keys. We majorly been using bytevector for most of the keys defined int he CHIPS ecosystem, the definition of these data types in the code are as follows.
```
//vrsc::data.type.string -->  	iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c
#define STRING_VDXF_ID		 	"iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c"
//vrsc::data.type.byte -->  	iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t
#define BYTE_VDXF_ID 	   		"iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t"
//vrsc::data.type.bytevector --> iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3
#define BYTEVECTOR_VDXF_ID      "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3"
```

## Identitites
We limit ourself to two levels of nesting under chips. At first level we mostly define identities are of game_types or any such things which are common across all the game types. For each game_type a corresponding identity is registered and we generate a token for it, further these tokens are used to register sub identities. All the transactions that happens in the game play use CHIPS for betting. The tokens we generate on the ID's registered has no significant value and they just be used to register subID's under that ID.

For example in the context of poker game we registered an identity named `poker` under chips as `poker.chips@`, and we generate a token named `poker` which is basically been used to create subID's under the poker ID.

Identities are often the addresses that can hold the tokens, ID's always ends with @. Any entity in the bet ecosystem can register the ID under chips, like for example  I registered an ID named sg777 under chips as `sg777.chips@` which basically been used to hold the tokens. Likewise `cashiers.poker.chips@` is an address players deposit funds to during the game. Even players are allowed to register the ID's and using which they can participate in the private tables.

Further, at high level we see what are all the keys that define under each ID in the namespace `poker.chips@`. Almost all the ID's in the poker ecosystem are of multiple signatures in nature, meaning they controlled by one or more primaryaddresses and some require one or more signatures to update them.

### Cashiers ID
Address --> `cashiers.poker.chips@`
The keys that updates the data to this ID are
```
1. chips.vrsc::poker.cashiers
```

### Dealers ID

Address --> `dealers.poker.chips@`
The keys that updates the data to this ID are
```
1. chips.vrsc::poker.dealers
```
#### 1. chips.vrsc::poker.dealers
This key holds the array of dealers info, i.e it basically a string array that holds all dealer_id names.
```
{
        "dealers":      ["sg777_d"]
}
```


### Dealer ID
Address --> `<dealer_name>.poker.chips@` //Dealer provides this name at the time of registration and all dealer names end with `_d` to avoid naming conflicts, e.g `sg777_d.poker.chips@`
The keys that updates the data to this ID are
```
1. chips.vrsc::poker.t_player_info
```

#### 1. chips.vrsc::poker.t_player_info
Contains the table info, dealer updates this info from the values read from `verus_dealer.ini`
```
{
        "max_players":  2,
        "big_blind":    0.00100000,
        "min_stake":    0.20000000,
        "max_stake":    1,
        "table_id":     "sg777_t",
        "dealer_id":    "sg777_d"
}
```

### Table ID
Address --> `<table_name>.poker.chips@` // Dealers can register upto 5 different table names and all table names ends with `_t` to avoid naming conflicts, e.g `sg777_t.poker.chips@`

The keys that updates the data to this ID are
```
1. chips.vrsc::poker.game_ids --> Holds the info of the active game_id that is attached to the table, this is a 32 byte random string.
```
#### 1. chips.vrsc::poker.game_ids
```
game_id::47be72cf1267b85c1ad07b38dfaab2b38ab036ec867ec95a08ff440af2bb4dc9
```

Table is the place where on the fly keys are created.  Each game is represented by a 32 byte random number calling it as gameID and this game ID is used to generate the on the fly keys during the game and these are used to update the table ID with the game info during the game. 

Lets say, for simplicity reasons we have gameID as `game_id1` then the following keys are generated during the game:
```
1. chips.vrsc::poker.t_table_info.game_id1
2. chips.vrsc::poker.t_player_info.game_id1
3. chips.vrsc::poker.t_player1.game_id1
4. chips.vrsc::poker.t_player2.game_id1
5. chips.vrsc::poker.t_player3.game_id1
6. chips.vrsc::poker.t_player4.game_id1
7. chips.vrsc::poker.t_player5.game_id1
8. chips.vrsc::poker.t_player6.game_id1
9. chips.vrsc::poker.t_player7.game_id1
10. chips.vrsc::poker.t_player8.game_id1
11. chips.vrsc::poker.t_player9.game_id1
12. chips.vrsc::poker.t_dealer.game_id1
13. chips.vrsc::poker.t_blinder.game_id1
```
While playing game with ID `game_id1` during which all the entities that participate in the game update the game info using these keys at the corresponding table ID.

#### 1. chips.vrsc::poker.t_table_info.game_id1
Contains the talbe info.
```
{
        "max_players":  2,
        "big_blind":    0.00100000,
        "min_stake":    0.20000000,
        "max_stake":    1,
        "table_id":     "sg777_t",
        "dealer_id":    "sg777_d"
}
```
#### 2. chips.vrsc::poker.t_player_info.game_id1
Contains the info about the players that joined the table
```
{
        "num_players":  2,
        "player_info":  ["RLqZtcUkqCHWe5t35zjEXLS1ubqKnbUtJW_e1eb_1", "RPP74xK9HRZGAS5Ynn4EbfLrKGyak3fJrJ_9597_2"]
}
```
#### 3. chips.vrsc::poker.t_player1.game_id1
Contains the info about players shuffled deck
```
{
        "id":   1,
        "pubkey":       "1ea418903532637eedddc7918fb648332b59e4a2cda42db7493861529e42de66",
        "cardinfo":     ["afe92a18c5dacc314be0eb16f7f1d83a4eaebaab06874ed78a487e7a4ea03a0d", "b14de6c06ff1fac3f0b698d55718712521c12d2e6c4f633494abf019a8e92f25", "8ce9ca33bdd0cddf0929b5a631a33a82b03a5e3ffb4c22d9744ee3b488297336"]
}
```


