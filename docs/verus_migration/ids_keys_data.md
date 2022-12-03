Keys under a given namespace
----------------------------
Keys are ways to store the information under the identities, we encapsulate the info of a structure or a method and map them to the keys. For example we have an identifier named cashiers under poker which holds all the cashiers info, we also define a cashier_key where in which all the cashiers info is encapsulated and is mapped to the cashier_key and is stored under the cashier identifier. 
Our aim is to define as minimum identifiers as possible and as minimum keys as possible to represent the data.

We define every keys under the namespace `chips.vrsc::`. As we know that bet aim to support multiple games, so under the namespace `chips.vrsc::` we group the keys based on the game in which they used for. For example `chips.vrsc::poker` is the prefix to all the keys that are used in playing the poker game.
Likewise if any keys are common across the games then those are defined with the `prefix chips.vrsc::bet`.

Lets say the cashiers are specific to the context of poker, so we defined them with the prefix `chips.vrsc::poker` and the correponding key of it is represented as `chips.vrsc::poker.cashiers`.

Examples:
```
//chips.vrsc::poker.cashiers --> iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
```

Datatypes used
--------------
Since we are encapsulating the data and store using binary serialization, so we basically not needing much variety here. 
We majorly use bytevector defined as `vrsc::data.type.bytevector` in verus.

Examples:
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
We limit ourself to two levels of nesting under chips. At first level we mostly define identities are of game_types or any such things which are common across all the game types. For each game_type a corresponding identity is registered and we generate a token for it, further these tokens are used to register sub identities and and also needed in that specific game play.

For example in the context of poker game we registered an identity named `poker` under chips as `poker.chips@`, and we generate a token named `poker` which is basically be used to play the poker game. At the second level we register sub identities under the game_type identity which are very specific to the game.

Identifiers are often the addresses that can hold the tokens, so for that reason identifies always ends with @. Any entity in the bet ecosystem can register the identities under chips, like for example i registered an identiy named sg777 under chips as `sg777.chips@` which basically been used to hold the tokens. 

Examples:
```
#define CASHIERS_ID "cashiers.poker.chips10sec@"
#define POKER_CHIPS_VDXF_ID "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs"
```
