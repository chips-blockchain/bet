#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "bet.h"
#include "config.h"
#include "common.h"
#include "misc.h"
#include "err.h"
#include "commands.h"
#include "vdxf.h"
#include "dealer.h"

char *dealer_config_ini_file = "../config/dealer_config.ini";
char *player_config_ini_file = "../config/player_config.ini";
char *cashier_config_ini_file = "../config/cashier_config.ini";
char *bets_config_ini_file = "../config/bets.ini";
char *blockchain_config_ini_file = "../config/blockchain_config.ini";
char *verus_dealer_config = "../config/verus_dealer.ini";
char *verus_player_config_file = "../config/verus_player.ini";

struct verus_player_config player_config = { 0 };

bits256 game_id;

cJSON *bet_read_json_file(char *file_name)
{
	FILE *fp = NULL;
	cJSON *json_data = NULL;
	char *data = NULL, buf[256];
	unsigned long data_size = 1024, buf_size = 256, temp_size = 0;
	unsigned long new_size = data_size;

	data = calloc(data_size, sizeof(char));
	if (!data) {
		goto end;
	}

	fp = fopen(file_name, "r");
	if (fp == NULL) {
		dlg_error("Failed to open file %s\n", file_name);
		goto end;
	} else {
		while (fgets(buf, buf_size, fp) != NULL) {
			temp_size = temp_size + strlen(buf);
			if (temp_size >= new_size) {
				char *temp = calloc(new_size, sizeof(char));
				strncpy(temp, data, strlen(data));
				free(data);
				new_size = new_size * 2;
				data = calloc(new_size, sizeof(char));
				strncpy(data, temp, strlen(temp));
				free(temp);
			}
			strcat(data, buf);
			memset(buf, 0x00, buf_size);
		}
		json_data = cJSON_CreateObject();
		json_data = cJSON_Parse(data);
	}
end:
	if (data)
		free(data);
	return json_data;
}

void bet_parse_dealer_config_ini_file()
{
	dictionary *ini = NULL;

	ini = iniparser_load(dealer_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", dealer_config_ini_file);
	} else {
		if (-1 != iniparser_getint(ini, "table:max_players", -1)) {
			max_players = iniparser_getint(ini, "table:max_players", -1);
		}
		if (0 != iniparser_getdouble(ini, "table:big_blind", 0)) {
			BB_in_chips = iniparser_getdouble(ini, "table:big_blind", 0);
			SB_in_chips = BB_in_chips / 2;
		}
		if (0 != iniparser_getint(ini, "table:min_stake", 0)) {
			table_min_stake = iniparser_getint(ini, "table:min_stake", 0) * BB_in_chips;
		}
		if (0 != iniparser_getint(ini, "table:max_stake", 0)) {
			table_max_stake = iniparser_getint(ini, "table:max_stake", 0) * BB_in_chips;
		}

		if (0 != iniparser_getdouble(ini, "dealer:chips_tx_fee", 0)) {
			chips_tx_fee = iniparser_getdouble(ini, "dealer:chips_tx_fee", 0);
		}
		if (0 != iniparser_getdouble(ini, "dealer:dcv_commission", 0)) {
			dcv_commission_percentage = iniparser_getdouble(ini, "dealer:dcv_commission", 0);
		}
		if (NULL != iniparser_getstring(ini, "dealer:gui_host", NULL)) {
			strcpy(dcv_hosted_gui_url, iniparser_getstring(ini, "dealer:gui_host", NULL));
		}
		threshold_value = iniparser_getint(ini, "dealer:min_cashiers", threshold_value);
		if (-1 != iniparser_getboolean(ini, "private table:is_table_private", -1)) {
			is_table_private = iniparser_getboolean(ini, "private table:is_table_private", -1);
		}
		if (NULL != iniparser_getstring(ini, "private table:table_password", NULL)) {
			strcpy(table_password, iniparser_getstring(ini, "private table:table_password", NULL));
		}
		if (-1 != iniparser_getboolean(ini, "dealer:bet_ln_config", -1)) {
			bet_ln_config = iniparser_getboolean(ini, "dealer:bet_ln_config", -1);
		}
	}
}

void bet_parse_player_config_ini_file()
{
	dictionary *ini = NULL;

	ini = iniparser_load(player_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", player_config_ini_file);
	} else {
		if (0 != iniparser_getdouble(ini, "player:max_allowed_dcv_commission", 0)) {
			max_allowed_dcv_commission = iniparser_getdouble(ini, "player:max_allowed_dcv_commission", 0);
		}
		if (0 != iniparser_getint(ini, "player:table_stake_size", 0)) {
			table_stack_in_bb = iniparser_getint(ini, "player:table_stake_size", 0);
		}
		if (0 != iniparser_getstring(ini, "player:name", NULL)) {
			strcpy(player_name, iniparser_getstring(ini, "player:name", NULL));
		}
		if (-1 != iniparser_getboolean(ini, "private table:is_table_private", -1)) {
			is_table_private = iniparser_getboolean(ini, "private table:is_table_private", -1);
		}
		if (NULL != iniparser_getstring(ini, "private table:table_password", NULL)) {
			strcpy(table_password, iniparser_getstring(ini, "private table:table_password", NULL));
		}
		if (-1 != iniparser_getboolean(ini, "player:bet_ln_config", -1)) {
			bet_ln_config = iniparser_getboolean(ini, "player:bet_ln_config", -1);
		}
	}
}

void bet_parse_cashier_config_ini_file()
{
	cJSON *cashiers_info = NULL;
	dictionary *ini = NULL;

	ini = iniparser_load(cashier_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", cashier_config_ini_file);
	} else {
		char str[20];
		int i = 1;
		sprintf(str, "cashier:node-%d", i);
		cashiers_info = cJSON_CreateArray();
		while (NULL != iniparser_getstring(ini, str, NULL)) {
			cJSON_AddItemToArray(cashiers_info, cJSON_Parse(iniparser_getstring(ini, str, NULL)));
			memset(str, 0x00, sizeof(str));
			sprintf(str, "cashier:node-%d", ++i);
		}
		no_of_notaries = cJSON_GetArraySize(cashiers_info);
		notary_node_ips = (char **)malloc(no_of_notaries * sizeof(char *));
		notary_node_pubkeys = (char **)malloc(no_of_notaries * sizeof(char *));
		notary_status = (int *)malloc(no_of_notaries * sizeof(int));

		for (int32_t i = 0; i < no_of_notaries; i++) {
			cJSON *node_info = cJSON_CreateObject();
			node_info = cJSON_GetArrayItem(cashiers_info, i);

			notary_node_ips[i] = (char *)malloc(strlen(jstr(node_info, "ip")) + 1);
			memset(notary_node_ips[i], 0x00, strlen(jstr(node_info, "ip")) + 1);

			notary_node_pubkeys[i] = (char *)malloc(strlen(jstr(node_info, "pubkey")) + 1);
			memset(notary_node_pubkeys[i], 0x00, strlen(jstr(node_info, "pubkey")) + 1);

			strncpy(notary_node_ips[i], jstr(node_info, "ip"), strlen(jstr(node_info, "ip")));
			strncpy(notary_node_pubkeys[i], jstr(node_info, "pubkey"), strlen(jstr(node_info, "pubkey")));
		}
	}
}

void bet_display_cashier_hosted_gui()
{
	dictionary *ini = NULL;

	ini = iniparser_load(player_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", player_config_ini_file);
	} else {
		char str[20];
		int i = 1;
		sprintf(str, "gui:cashier-%d", i);
		while (NULL != iniparser_getstring(ini, str, NULL)) {
			if (check_url(iniparser_getstring(ini, str, NULL)))
				dlg_warn("%s", iniparser_getstring(ini, str, NULL));
			memset(str, 0x00, sizeof(str));
			sprintf(str, "gui:cashier-%d", ++i);
		}
	}
}

static int32_t ini_sec_exists(dictionary *ini, char *sec_name)
{
	int32_t n, retval = -1;

	n = iniparser_getnsec(ini);
	dlg_info("number of sections:: %d", n);
	for (int32_t i = 0; i < n; i++) {
		dlg_info("%s::%s", iniparser_getsecname(ini, i), sec_name);
		if (strcmp(iniparser_getsecname(ini, i), sec_name) == 0) {
			retval = OK;
			break;
		}
	}
	return retval;
}

int32_t bet_parse_bets()
{
	int32_t retval = OK, bet_no = 0;
	dictionary *ini = NULL;
	char key_name[40];
	cJSON *bets_info = NULL, *info = NULL;

	ini = iniparser_load(bets_config_ini_file);
	if (ini == NULL) {
		retval = ERR_INI_PARSING;
		dlg_error("error in parsing %s", bets_config_ini_file);
		return retval;
	}
	info = cJSON_CreateObject();
	cJSON_AddStringToObject(info, "method", "bets");
	cJSON_AddNumberToObject(info, "balance", chips_get_balance());
	bets_info = cJSON_CreateArray();
	while (1) {
		cJSON *bet = cJSON_CreateObject();

		cJSON_AddNumberToObject(bet, "bet_id", bet_no);
		memset(key_name, 0x00, sizeof(key_name));
		sprintf(key_name, "bets:%d:desc", bet_no);
		if (NULL != iniparser_getstring(ini, key_name, NULL)) {
			cJSON_AddStringToObject(bet, "desc", iniparser_getstring(ini, key_name, NULL));
		} else {
			break;
		}
		memset(key_name, 0x00, sizeof(key_name));
		sprintf(key_name, "bets:%d:predictions", bet_no);
		if (NULL != iniparser_getstring(ini, key_name, NULL)) {
			cJSON_AddStringToObject(bet, "predictions", iniparser_getstring(ini, key_name, NULL));
		} else {
			break;
		}
		memset(key_name, 0x00, sizeof(key_name));
		sprintf(key_name, "bets:%d:range", bet_no);
		if (NULL != iniparser_getstring(ini, key_name, NULL)) {
			cJSON_AddStringToObject(bet, "range", iniparser_getstring(ini, key_name, NULL));
		} else {
			break;
		}
		cJSON_AddItemToArray(bets_info, bet);
		bet_no++;
	}
	cJSON_AddItemToObject(info, "bets_info", bets_info);
	dlg_info("\n%s", cJSON_Print(info));
	return retval;
}

bool bet_is_new_block_set()
{
	dictionary *ini = NULL;
	bool is_new_block_set = false;

	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;

	char *config_file = NULL;
	config_file = calloc(1, 200);
	strcpy(config_file, homedir);
	strcat(config_file, "/bet/privatebet/config/blockchain_config.ini");
	ini = iniparser_load(config_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", config_file);
	} else {
		if (-1 != iniparser_getboolean(ini, "blockchain:new_block", -1)) {
			is_new_block_set = iniparser_getboolean(ini, "blockchain:new_block", -1);
		}
	}
	return is_new_block_set;
}

void bet_parse_blockchain_config_ini_file()
{
	dictionary *ini = NULL;

	ini = iniparser_load(blockchain_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", blockchain_config_ini_file);
	} else {
		if (NULL != iniparser_getstring(ini, "blockchain:blockchain_cli", NULL)) {
			memset(blockchain_cli, 0x00, sizeof(blockchain_cli));
			strncpy(blockchain_cli, iniparser_getstring(ini, "blockchain:blockchain_cli", "chips-cli"),
				sizeof(blockchain_cli));
			if (!((strcmp(blockchain_cli, chips_cli) == 0) ||
			      (strcmp(blockchain_cli, verus_chips_cli) == 0))) {
				dlg_warn(
					"The blockchain client configured in ./config/blockchain_config.ini is not in the supported list of clients, so setting it do default chips-cli");
				memset(blockchain_cli, 0x00, sizeof(blockchain_cli));
				strncpy(blockchain_cli, chips_cli, sizeof(blockchain_cli));
			}
		}
	}
}

int32_t bet_parse_verus_dealer()
{
	int32_t retval = OK;
	dictionary *ini = NULL;
	struct table t;

	ini = iniparser_load(verus_dealer_config);
	if (!ini)
		return ERR_INI_PARSING;

	if (NULL != iniparser_getstring(ini, "verus:dealer_id", NULL)) {
		strncpy(t.dealer_id, iniparser_getstring(ini, "verus:dealer_id", NULL), sizeof(t.dealer_id));
	}
	if (-1 != iniparser_getint(ini, "table:max_players", -1)) {
		t.max_players = (uint8_t)iniparser_getint(ini, "table:max_players", -1);
	}
	if (0 != iniparser_getdouble(ini, "table:big_blind", 0)) {
		float_to_uint32_s(&t.big_blind, iniparser_getdouble(ini, "table:big_blind", 0));
	}
	if (0 != iniparser_getint(ini, "table:min_stake", 0)) {
		float_to_uint32_s(&t.min_stake, (iniparser_getint(ini, "table:min_stake", 0) * BB_in_chips));
	}
	if (0 != iniparser_getint(ini, "table:max_stake", 0)) {
		float_to_uint32_s(&t.max_stake, (iniparser_getint(ini, "table:max_stake", 0) * BB_in_chips));
	}
	if (NULL != iniparser_getstring(ini, "table:table_id", NULL)) {
		strncpy(t.table_id, iniparser_getstring(ini, "table:table_id", NULL), sizeof(t.table_id));
	}

	retval = dealer_init(t);
	return retval;
}

int32_t bet_parse_verus_player()
{
	int32_t retval = OK;
	dictionary *ini = NULL;

	ini = iniparser_load(verus_player_config_file);
	if (!ini)
		return ERR_INI_PARSING;

	if (NULL != iniparser_getstring(ini, "verus:dealer_id", NULL)) {
		strncpy(player_config.dealer_id, iniparser_getstring(ini, "verus:dealer_id", NULL),
			sizeof(player_config.dealer_id));
	}
	if (NULL != iniparser_getstring(ini, "verus:table_id", NULL)) {
		strncpy(player_config.table_id, iniparser_getstring(ini, "verus:table_id", NULL),
			sizeof(player_config.table_id));
	}
	if (NULL != iniparser_getstring(ini, "verus:wallet_addr", NULL)) {
		strncpy(player_config.wallet_addr, iniparser_getstring(ini, "verus:wallet_addr", NULL),
			sizeof(player_config.wallet_addr));
	}
	if (NULL != iniparser_getstring(ini, "verus:player_id", NULL)) {
		strncpy(player_config.verus_pid, iniparser_getstring(ini, "verus:player_id", NULL),
			sizeof(player_config.verus_pid));
	}
	//Check if all IDs are valid
	if ((!player_config.dealer_id) || (!player_config.table_id) || (!player_config.verus_pid) ||
	    !is_id_exists(player_config.dealer_id, 0) || !is_id_exists(player_config.table_id, 0)) {
		return ERR_CONFIG_PLAYER_ARGS;
	}
	//Check if the node has player IDs priv keys
	if (!id_cansignfor(player_config.verus_pid, 0, &retval)) {
		return retval;
	}

	return retval;
}
