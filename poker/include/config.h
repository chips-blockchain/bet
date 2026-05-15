#ifndef CONFIG_H
#define CONFIG_H

#include "bet.h"

/* Verus IDs and Keys Configuration Structure */
struct verus_ids_keys_config {
	char parent_id[128];
	char cashier_id[128];
	char dealer_id[128];
	char cashiers_short[64];
	char dealers_short[64];
	char poker_short[64];
	int32_t initialized;
};

extern struct verus_ids_keys_config verus_config;

/* RPC Credentials Configuration Structure */
struct rpc_credentials {
	char url[256];
	char user[128];
	char password[256];
	int use_rest_api;  /* 1 = use REST API, 0 = use CLI */
	int initialized;
};

extern struct rpc_credentials rpc_config;

/* RPC Credentials Functions */
void bet_parse_rpc_credentials(void);
const char *bet_get_rpc_url(void);
const char *bet_get_rpc_user(void);
const char *bet_get_rpc_password(void);
int bet_use_rest_api(void);

void bet_init_config_paths(void);
cJSON *bet_read_json_file(char *file_name);

/* Config file paths - declared in config.c */
extern char *dealer_config_ini_file;
extern char *player_config_ini_file;
extern char *cashier_config_ini_file;
extern char *verus_player_config_file;
extern char *verus_dealer_config;

void bet_parse_dealer_config_ini_file();
void bet_parse_player_config_ini_file();
void bet_parse_cashier_config_ini_file();
void bet_display_cashier_hosted_gui();
void bet_parse_blockchain_config_ini_file();
bool bet_is_new_block_set();
int32_t bet_parse_verus_dealer();
int32_t bet_parse_verus_dealer_with_reset(bool reset);
int32_t bet_parse_verus_player();
void bet_parse_verus_ids_keys_config(void);
const char *bet_get_cashiers_id_fqn(void);
const char *bet_get_cashier_short_name(void);
const char *bet_get_dealers_id_fqn(void);
const char *bet_get_poker_id_fqn(void);
const char *bet_get_currency(void);
#endif /* CONFIG_H */
