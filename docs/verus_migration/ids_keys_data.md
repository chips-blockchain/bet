# All about ID's and Keys
## ID Registration

The ID registration process can either be restricted to a few authorized addresses or made open for anyone to register IDs. Mike, the founder/developer of Verus, suggests keeping the registration open to foster economic activity on the chain. Here is his insight:

```
  I think you should not restrict the ID registration capabilities, as they will be a primary source of economy. Since you do not have block rewards, this could make the difference between a functional chain or not. There is no spam because all resources used on-chain are paid for to miners, stakers, LPs in baskets that register decentralized IDs, and those selling IDs on-chain. All of this is economic value for the on-chain economy that is not spam. It is exactly the opposite. If your chain is busy with that, everyone should be happy. If it is too busy, make another. Also, people have IDs on different chains that they should be able to send over and use. Each player should have an ID, and if some games need KYC, you will be able to do that as well on any IDs using the Value service, which will use VerusID to enable non-DOXed KYC verification. I recommend considering IDs just on-chain addresses and letting people use them as they will. Enable poker and other games this way, but have all of the on-chain activity, including currencies, benefit miners and stakers (CHIPS holders) through the fee pool as part of normal use.
```

The process can be divided into two steps:
1. ID Creation
2. ID Registration

Anyone can create any number of IDs, but to register an ID as either a dealer or cashier, certain conditions must be met. We streamline this process and provide an API for it.

## Key-Value Pair

Information under IDs is stored in key-value pairs. We encapsulate the structure or JSON object info and map them to keys. For example, the identifier `cashiers` under `poker.chips10sec@` defines a key named `chips.vrsc::poker.cashiers` to store all information related to cashiers. 

Keys are defined with the prefix `chips.vrsc::` in the entire CHIPS ecosystem. For game-specific keys, we add the game type as a suffix, e.g., `chips.vrsc::poker.` for poker-related data and `chips.vrsc::bet.` for betting-related data.

In the code, predefined keys are mapped to their `vdxfid` for easy lookup. For example:
```
#verus -chain=chips10sec getvdxfid chips.vrsc::poker.cashiers
{
  "vdxfid": "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN",
  "indexid": "xMvtWEwEZ92ZkWP6duq9Gx3kciBGyf9cB6",
  "hash160result": "ec2bdc0eab94ab8f0d4a9ca6e29afe4b2ece7495",
  "qualifiedname": {
  "namespace": "iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP",
  "name": "chips.vrsc::poker.cashiers"
  }
}
```
These `vdxfid` are defined in the code using macro variables:
```
//chips.vrsc::poker.cashiers 
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
```

Some keys are predefined, while others are created on the fly during the game. Keys created during the game are mapped to predefined keys for easy reference and lookup. Some keys can be used across multiple IDs, e.g., `chips.vrsc::poker.t_player_info` is used across dealer and table IDs to store player info.

## Key Datatypes

Data is converted to hex and mapped to keys using specific datatypes. For example, `vrsc::data.type.bytevector` is used for most keys in the CHIPS ecosystem. The definitions in the code are as follows:
```
//vrsc::data.type.string -->  	iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c
#define STRING_VDXF_ID		 	"iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c"
//vrsc::data.type.byte -->  	iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t
#define BYTE_VDXF_ID 	   		"iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t"
//vrsc::data.type.bytevector --> iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3
#define BYTEVECTOR_VDXF_ID      "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3"
```

## Identities

We limit to two levels of nesting under chips. At the first level, we define identities of game types and player names. Each player is encouraged to have an ID, which helps in participating in private tables or betting. Users can also reserve IDs for future use and trade them, increasing chain activity. 

For each game type, a corresponding identity is registered, and a token is generated for it. These tokens are used to register sub-identities. All transactions in the gameplay use CHIPS for betting. Tokens generated on registered IDs have no significant value and are used to register subIDs.

For example, an identity named `poker` under chips is `poker.chips@`, and a token named `poker` is generated to create subIDs under the poker ID.

Identities are addresses that can hold tokens and always end with `@`. Any entity in the bet ecosystem can create an ID under chips, e.g., `sg777.chips@` is used to hold tokens and acts as a wallet address. Similarly, `cashiers.poker.chips@` is an address where players deposit funds during the game. Players can also create IDs to participate in private tables.

### Cashiers ID

Address: `cashiers.poker.chips@`

Keys:
```
1. chips.vrsc::poker.cashiers
```

### Dealers ID

Address: `dealers.poker.chips@`

Keys:
```
1. chips.vrsc::poker.dealers
```

Dealers info is a JSON object converted to hex and mapped to `chips.vrsc::poker.dealers` key. 

Example:
```
{
  "dealers": ["sg777_d"]
}
```
Hex string: `7b0a09226465616c657273223a095b2273673737375f64225d0a7d`

To simplify, use the `add_dealer` command:
```
./bet add_dealer <dealer_name>
```
Parsing commands to display dealer info:
```
1. ./bet print_id dealers dealers
2. ./bet print dealers dealers
```

### Dealer ID

ID: `<dealer_name>.poker.chips10sec@`

Keys:
```
1. chips.vrsc::poker.t_player_info
```

Example:
```
{
  "max_players": 2,
  "big_blind": 0.001,
  "min_stake": 0.2,
  "max_stake": 1,
  "table_id": "sg777_t",
  "dealer_id": "sg777_d"
}
```

### Table ID

ID: `<table_name>.poker.chips10sec@`

Keys:
```
1. chips.vrsc::poker.t_game_ids
```

Example:
```
game_id::47be72cf1267b85c1ad07b38dfaab2b38ab036ec867ec95a08ff440af2bb4dc9
```

During gameplay, keys are generated on the fly, e.g., `chips.vrsc::poker.t_table_info.game_id1`. These keys update the table ID with game info.

Example keys:
```
1. chips.vrsc::poker.t_table_info.game_id1
2. chips.vrsc::poker.t_player_info.game_id1
3. chips.vrsc::poker.t_player1.game_id1
...
```

Example table info:
```
{
  "max_players": 2,
  "big_blind": 0.001,
  "min_stake": 0.2,
  "max_stake": 1,
  "table_id": "sg777_t",
  "dealer_id": "sg777_d"
}
```

Example player info:
```
{
  "num_players": 2,
  "player_info": ["RLqZtcUkqCHWe5t35zjEXLS1ubqKnbUtJW_e1eb_1", "RPP74xK9HRZGAS5Ynn4EbfLrKGyak3fJrJ_9597_2"]
}
```

Example player shuffled deck:
```
{
  "id": 1,
  "pubkey": "1ea418903532637eedddc7918fb648332b59e4a2cda42db7493861529e42de66",
  "cardinfo": ["afe92a18c5dacc314be0eb16f7f1d83a4eaebaab06874ed78a487e7a4ea03a0d", "b14de6c06ff1fac3f0b698d55718712521c12d2e6c4f633494abf019a8e92f25", "8ce9ca33bdd0cddf0929b5a631a33a82b03a5e3ffb4c22d9744ee3b488297336"]
}
```

Example game info:
```
{
  game_state: game_state_no,
  game_state_info: "Info about the game based on the state that is updated"
}
```

More details about game_state are [here](https://github.com/sg777/bet/blob/verus_test/docs/verus_migration/game_state.md)
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
Earlier in some sections we talked about that to be dealer or cashier certain conditions needs to be met. 
We can actually seperate this process into two steps:
1. ID creation
2. ID Registration
Anyone can create any numer of ID's, but to register an ID as either dealer or cashier certain conditions to be met, we streamline this process and provide an API for it`(TO DO)`.

## Key - Value pair
Under IDs information is stored in key-value pairs. We encapsulate the info of a structure or JSON object and map them to the keys. For example we have an identifier named `cashiers` under `poker.chips10sec@` in which we define a key named `chips.vrsc::poker.cashiers` and map all the information related to cashiers to this key and store in the cashiers ID. It's important to identity and define key to each type of data that we store in the ID, using these keys we store and retrieve the data from ID, so the type and content of data that is to be mapped to these keys should be predefined.

We define all keys with the prefix `chips.vrsc::` in the entire CHIPS ecosystem, for the game specific keys we add game type as suffix to `chips.vrsc::` and followed by it we define keys. For example, for all the keys that maps to the data used to play poker are defined with the prefix `chips.vrsc::poker.` and likewise for all the keys that are used to to store the data related to betting are defined with the prefix `chips.vrsc::bet.` and so on...

In the code for all the predefined keys, we compute their vdxfid's and map them for easy lookup. i.e, for key `chips.vrsc::poker.cashiers` the corresponding vdxfid is  
`iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN` which is obtained using getvdxfid as follows: 
```
#verus -chain=chips10sec getvdxfid chips.vrsc::poker.cashiers
{
  "vdxfid": "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN",
  "indexid": "xMvtWEwEZ92ZkWP6duq9Gx3kciBGyf9cB6",
  "hash160result": "ec2bdc0eab94ab8f0d4a9ca6e29afe4b2ece7495",
  "qualifiedname": {
    "namespace": "iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP",
    "name": "chips.vrsc::poker.cashiers"
  }
}

```
We define these `vdxfid` in the code using macro variables. 
```
//chips.vrsc::poker.cashiers 
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
```

Some keys are predefined and some keys we create them on fly during the game, all the keys that are created during the game are mapped to the predefined existing keys for easy reference and lookup. Some keys can be used across multiple ID's, for example the key `chips.vrsc::poker.t_player_info` that maps to player info is used across the IDs dealer and table to store the player info. Aim should be less or no redundancy, but in some scenarios where different IDs need some common data but these IDs are having different priveleges then in such cases we going to have some redundancy in keys across IDs.

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
We limit ourself to two levels of nesting under chips. At first level we mostly define identities that are of game types and player names. As Mike suggested we encourage to have an ID for each player, having an ID for the player helps to be part of private tables in poker or in private betting, also as Mike suggested users can also reserve the IDs for the future purposes and they can trade those identities that inturn increases the activity on the chain. For each game_type a corresponding identity is registered and we generate a token for it, further these tokens are used to register sub identities. All the transactions that happens in the game play use CHIPS for betting. The tokens we generate on the ID's registered has no significant value and they just be used to register subID's under that ID.

For example in the context of poker game lets say we created an identity named `poker` under chips as `poker.chips@`, and we generate a token named `poker` which is basically used to create subID's under the poker ID.

Identities are often the addresses that can hold the tokens, ID's always ends with `@`. Any entity in the bet ecosystem can create the ID under chips, like for example  I created an ID named sg777 under chips as `sg777.chips@` which basically been used to hold the tokens and that is like my wallet address. Likewise `cashiers.poker.chips@` is an address players deposit funds to during the game. Even players are allowed to create the ID's and using which they can participate in the private tables.

Further, at high level we see what are all the keys that define under each ID are in the namespace `poker.chips@`. Almost all the ID's in the poker ecosystem are of multiple signatures in nature, meaning that they controlled by one or more primaryaddresses that inturn means it requires multiple parties to cosign for the tx to be processed.

### Cashiers ID
Address --> `cashiers.poker.chips@`
The keys that updates the data to this ID are
```
1. chips.vrsc::poker.cashiers
```

### Dealers ID
The dealers ID under the namespace `poker.chips10sec@` is `dealers.poker.chips@` which contains the list of dealers that are registered. The dealers info is mapped to the key `chips.vrsc::poker.dealers`. Unless specified explicitly all the key types are byte vectors. 
The list of keys that stores the information on dealers ID are:
```
1. chips.vrsc::poker.dealers
```
The dealers info is a JSON object which is converted to hex and is mapped to the `chips.vrsc::poker.dealers` key and stored in the ID. 
Here is step by step explanation of it.
1. For example, lets we have the following dealers info
```
{
        "dealers":      ["sg777_d"]
}
```
2. Converting this to hex string `7b0a09226465616c657273223a095b2273673737375f64225d0a7d`
3. The vdxfID of `chips.vrsc::poker.dealers` is `iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ` and vdxfID of byte vector type is `iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3`
4. The contentmultimap of dealers ID looks as follows:
 ```
    "contentmultimap": {
      "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ": [
        {
          "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3": "7b0a09226465616c657273223a095b2273673737375f64225d0a7d"
        }
      ]
    }
```
To avoid these multiple steps, we provided `add_dealer` command, using which the dealer can be added by simply running `./bet add_dealer <dealer_name>`. Since the dealers info on ID is in hex format, we provided parsers to display the info of dealers in a readable format. 
Following are the parsing commands that displays dealers info of dealers ID:
```
1. ./bet print_id dealers dealers
2. ./bet print dealers dealers
```

### Dealer ID
ID --> `<dealer_name>.poker.chips10sec@` //Dealer provides this name at the time of registration and all dealer names end with `_d` to avoid naming conflicts, e.g `sg777_d.poker.chips@`
The keys that updates the data to this ID are
```
1. chips.vrsc::poker.t_player_info
```

#### 1. chips.vrsc::poker.t_player_info
The value mapped to this key is the table info, dealer updates this info from the values read from `verus_dealer.ini`
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
ID --> `<table_name>.poker.chips10sec@` // Dealers can register upto any number of table names and all table names ends with `_t` to avoid naming conflicts, e.g `sg777_t.poker.chips@`

The keys that updates the data to this ID are
```
1. chips.vrsc::poker.t_game_ids --> Holds the info of the active game_id that is attached to the table, this is a 32 byte random string.
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
#### 4. chips.vrsc::poker.t_game_info
This is an adhoc key, the `game_id_str` is appended to this base key, and a new key is generated during the game play i.e `chips.vrsc::poker.t_game_info.game_id_str` and this key holds all the game related information information.
This key mainly has the information about the game state and information associated with that game state if any.
```
{
    game_state : game_state_no
    game_state_info :"Info about the game based on the state that is updated"
}
```
More details about game_state are [here](https://github.com/sg777/bet/blob/verus_test/docs/verus_migration/game_state.md)
