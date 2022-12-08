#include "bet.h"
#include "common.h"

/* 
Keys under a given namespace
----------------------------
Keys are ways to store the information under the identities, we encapsulate the info of a structure or a method and 
map them to the keys. 
For example we have an identifier named cashiers under poker which holds all the cashiers info, we also define a cashier_key
where in which all the cashiers info is encapsulated map to the cashier_key and stored under the cashier identifier. 
Our aim is to define as minimum identifiers as possible and as minimum keys as possible to represent the data.

We define every keys under the namespace chips.vrsc::
As we know that bet aim to support multiple games, so under the namespace chips.vrsc:: we group the keys based on the game 
in which they used for. For example chips.vrsc::poker is the prefix to all the keys that are used in playing the poker game.
Likewise if any keys are common across the games then those are defined with the prefix chips.vrsc::bet.

Lets say the cashiers are specific to the context of poker, so we defined them with the prefix chips.vrsc::poker and the final
key is represented as chips.vrsc::poker.cashiers.
*/

//chips.vrsc::poker.cashiers --> iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP
#define CASHIERS_KEY "iH6n3SW9hpou8LW4nEAzJZXDb4AG4tLnQN"
//chips.vrsc::poker.dealers --> iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ
#define DEALERS_KEY "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ"

/*
Datatypes used
--------------
Since we are encapsulating the data and store using binary serialization, so we basically not needing much variety here. 
We majorly use bytevector defined as vrsc::data.type.bytevector in verus.
*/

//vrsc::data.type.string -->  	iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c
#define STRING_VDXF_ID "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c"
//vrsc::data.type.byte -->  	iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t
#define BYTE_VDXF_ID "iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t"
//vrsc::data.type.bytevector --> iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3
#define BYTEVECTOR_VDXF_ID "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3"

/*
Identitites
-----------
We limit ourself to two levels of nesting under chips. At first level we mostly define identities are of game_types or any 
such things which are common across all the game types. For each game_type identity we generate a token which is used to register 
sub identities and is needed to play that specific game. 
For example we registered the identity poker under chips as poker.chips@, and we generate a token named poker which is basically be
used to play the poker game.
At the second level we register sun identities under the game_type identity which are very specific to the game.

Identifiers are often the addresses that can hold the tokens, so for that reason identifies always ends with @. 
Any entity in the bet ecosystem can register the identities under chips, like for example i registered an identiy named sg777 
under chips as sg777.chips@ which basically been used to hold the tokens. 
*/
#define CASHIERS_ID "cashiers.poker.chips10sec@"
#define POKER_CHIPS_VDXF_ID "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs"

cJSON *update_cmm(char *id, cJSON *cmm);
cJSON *get_cmm(char *id, int16_t full_id);
cJSON *append_primaryaddresses(char *id, cJSON *primaryaddress);
cJSON *update_primaryaddresses(char *id, cJSON *primaryaddress);
cJSON *get_primaryaddresses(char *id, int16_t full_id);
cJSON *get_cmm_key_data(char *id, int16_t full_id, char *key);
cJSON *update_dealers_config_table(char *dealer_id, struct table t);
struct table *get_dealers_config_table(char *dealer_id);
cJSON *get_cashiers_info(char *cashier_id);
cJSON *update_cashiers(char *ip);
cJSON *get_dealers();
struct table *find_table();
bool is_id_exists(char *id, int16_t full_id);
void verus_sendcurrency_data(cJSON *data);
cJSON *getaddressutxos(char verus_addresses[][100], int n);
void test_loop(char *block_hash);
