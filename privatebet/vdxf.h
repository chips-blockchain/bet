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

#define CASHIERS_KEY "chips.vrsc::poker.cashiers"
#define DEALERS_KEY "chips.vrsc::poker.dealers"

#define T_GAME_ID_KEY "chips.vrsc::poker.t_game_ids"
#define T_TABLE_INFO_KEY "chips.vrsc::poker.t_table_info"
#define T_PLAYER_INFO_KEY "chips.vrsc::poker.t_player_info"
#define T_PLAYER1_KEY "chips.vrsc::poker.t_player1"
#define T_PLAYER2_KEY "chips.vrsc::poker.t_player2"
#define T_PLAYER3_KEY "chips.vrsc::poker.t_player3"
#define T_PLAYER4_KEY "chips.vrsc::poker.t_player4"
#define T_PLAYER5_KEY "chips.vrsc::poker.t_player5"
#define T_PLAYER6_KEY "chips.vrsc::poker.t_player6"
#define T_PLAYER7_KEY "chips.vrsc::poker.t_player7"
#define T_PLAYER8_KEY "chips.vrsc::poker.t_player8"
#define T_PLAYER9_KEY "chips.vrsc::poker.t_player9"
#define T_DEALER_KEY "chips.vrsc::poker.t_dealer"
#define T_BLINDER_KEY "chips.vrsc::poker.t_blinder"


/*
Datatypes used
--------------
Since we are encapsulating the data and store using binary serialization, so we basically not needing much variety here. 
We majorly use bytevector defined as vrsc::data.type.bytevector in verus.
*/

#define STRING_VDXF_ID "vrsc::data.type.string"
#define BYTEVECTOR_VDXF_ID "vrsc::data.type.bytevector"

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
#define POKER_CHIPS_VDXF_ID "poker.chips10sec@"

#define ID_UPDATE_ESTIMATE_NO 50
#define RESERVE_AMOUNT ID_UPDATE_ESTIMATE_NO *chips_tx_fee

char *get_vdxf_id(char *key_name);
char *get_key_vdxf_id(char *key_name);
char *get_full_key(char *key_name);
char *get_key_data_type(char *key_name);
char *get_key_data_vdxf_id(char *key_name, char *data);
cJSON *update_cmm(char *id, cJSON *cmm);
cJSON *get_cmm(char *id, int16_t full_id);
cJSON *append_primaryaddresses(char *id, cJSON *primaryaddress);
cJSON *update_primaryaddresses(char *id, cJSON *primaryaddress);
cJSON *get_primaryaddresses(char *id, int16_t full_id);
cJSON *get_cmm_key_data(char *id, int16_t full_id, char *key);
cJSON *get_id_key_data(char *id, int16_t full_id, char *key);
void update_t_game_ids(char *id);
cJSON *update_t_table_info(char *dealer_id, char *key, struct table t);
struct table *get_dealers_config_table(char *dealer_id, char *table_id);
cJSON *get_cashiers_info(char *cashier_id);
cJSON *update_cashiers(char *ip);
cJSON *get_dealers();
bool is_dealer_exists(char *dealer_id);
int32_t get_player_id(int *player_id);
int32_t join_table();
int32_t get_table();
int32_t find_table();
int32_t is_id_exists(char *id, int16_t full_id);
int32_t check_player_join_status(char *table_id, char *pa);
cJSON *get_z_getoperationstatus(char *op_id);
cJSON *verus_sendcurrency_data(cJSON *data);
cJSON *getaddressutxos(char verus_addresses[][100], int n);
struct table *decode_table_info(cJSON *dealer_cmm_data);
struct table *get_available_t_of_d(char *dealer_id);
int32_t check_if_pa_exists(char *table_id);
bool check_if_enough_funds_avail(char *table_id);
bool check_if_d_t_available(char *dealer_id, char *table_id);
cJSON* get_cJSON_from_id_key(char *id, char *key);
struct table *get_t_table_info(char *id);
cJSON *get_t_player_info(char *table_id);
cJSON *update_t_player_info(char *id, cJSON *t_player_info);
int32_t do_payin_tx_checks(cJSON *payin_tx_data, char *txid);
void test_loop(char *block_hash);
