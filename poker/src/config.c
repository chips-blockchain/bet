#define _GNU_SOURCE
// Prevent narrow math functions from being declared (for compatibility)
// This must be defined before any system headers that might include math.h
#define __NO_MATH_NARROW_FUNCTIONS
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <libgen.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "bet.h"
#include "config.h"
#include "misc.h"
#include "err.h"
#include "commands.h"
#include "dealer.h"

// Config file paths - defaults initialized relative to executable, can be overridden with -c
static char dealer_config_buf[PATH_MAX];
static char player_config_buf[PATH_MAX];
static char cashier_config_buf[PATH_MAX];
static char blockchain_config_buf[PATH_MAX];
static char keys_config_buf[PATH_MAX];
static char rpc_credentials_buf[PATH_MAX];

// These are the main config paths used throughout the code
char *verus_dealer_config = dealer_config_buf;           // dealer.ini
char *verus_player_config_file = player_config_buf;      // player.ini (or p1.ini, p2.ini)
char *cashier_config_ini_file = cashier_config_buf;      // cashier.ini
char *blockchain_config_ini_file = blockchain_config_buf; // blockchain.ini

/* Native currency name used in sendcurrency. Configurable via
 * blockchain.ini → [blockchain] currency = ... . Defaults to CHIPS so
 * production behavior is unchanged when the key is absent. */
static char bet_currency_name[64] = CHIPS;
char *verus_ids_keys_config_file = keys_config_buf;      // keys.ini
char *rpc_credentials_file = rpc_credentials_buf;        // .rpccredentials

// Legacy aliases (for backward compatibility in code that still uses old names)
char *dealer_config_ini_file = dealer_config_buf;
char *player_config_ini_file = player_config_buf;

/**
 * Initialize config file paths relative to the executable location
 * This allows the program to be run from any directory
 */
void bet_init_config_paths(void)
{
	char exe_path[PATH_MAX] = { 0 };
	char exe_dir[PATH_MAX] = { 0 };
	char exe_dir_copy[PATH_MAX] = { 0 };
	ssize_t len = 0;

	// Get the executable path
	len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
	if (len == -1) {
		// Fallback: use current directory
		strncpy(exe_dir, ".", sizeof(exe_dir) - 1);
	} else {
		exe_path[len] = '\0';
		// Get directory from executable path (dirname modifies the string, so use a copy)
		strncpy(exe_dir_copy, exe_path, sizeof(exe_dir_copy) - 1);
		char *dir = dirname(exe_dir_copy);
		if (dir != NULL) {
			strncpy(exe_dir, dir, sizeof(exe_dir) - 1);
		}
	}

	// If executable is in bin/, go up one level to poker/ then to config/
	// Otherwise assume we're in poker/ directory
	char config_base[PATH_MAX];
	if (strstr(exe_dir, "/bin") != NULL || strstr(exe_dir, "bin") != NULL) {
		// Executable is in bin/, config is in ../config/
		snprintf(config_base, sizeof(config_base), "%s/../config", exe_dir);
	} else {
		// Executable is in poker/, config is in ./config/
		snprintf(config_base, sizeof(config_base), "%s/config", exe_dir);
	}

	// Initialize default config paths (can be overridden with -c flag)
	snprintf(verus_dealer_config, sizeof(dealer_config_buf), "%s/dealer.ini", config_base);
	snprintf(verus_player_config_file, sizeof(player_config_buf), "%s/player.ini", config_base);
	snprintf(cashier_config_ini_file, sizeof(cashier_config_buf), "%s/cashier.ini", config_base);
	snprintf(blockchain_config_ini_file, sizeof(blockchain_config_buf), "%s/blockchain.ini", config_base);
	snprintf(verus_ids_keys_config_file, sizeof(keys_config_buf), "%s/keys.ini", config_base);
	
	// RPC credentials file is in poker/ directory (not config/) since it contains secrets
	// Go up one level from config_base to get poker directory
	char poker_base[PATH_MAX];
	if (strstr(exe_dir, "/bin") != NULL || strstr(exe_dir, "bin") != NULL) {
		// Executable is in bin/, poker/ is ..
		snprintf(poker_base, sizeof(poker_base), "%s/..", exe_dir);
	} else {
		// Executable is in poker/
		snprintf(poker_base, sizeof(poker_base), "%s", exe_dir);
	}
	snprintf(rpc_credentials_file, sizeof(rpc_credentials_buf), "%s/.rpccredentials", poker_base);
}

struct verus_player_config player_config = { 0 };

bits256 game_id;

/* GUI WebSocket port - configurable, defaults to 9000 */
int gui_ws_port = DEFAULT_GUI_WS_PORT;

/* Globals declared in include/common.h */
int32_t is_table_private = 0;
char table_password[128] = { 0 };
char player_name[128] = { 0 };
char verus_pid[128] = { 0 };
// bet_ln_config removed - Lightning Network support removed, using CHIPS-only payments

cJSON *bet_read_json_file(char *file_name)
{
	FILE *fp = NULL;
	cJSON *json_data = NULL;
	char *data = NULL;
	char buf[256];
	size_t cap = 4096;
	size_t len = 0;

	data = calloc(cap, 1);
	if (!data)
		goto end;

	fp = fopen(file_name, "r");
	if (fp == NULL) {
		dlg_error("Failed to open file %s\n", file_name);
		goto end;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		size_t blen = strlen(buf);
		if (len + blen + 1 > cap) {
			size_t newcap = cap;
			while (len + blen + 1 > newcap)
				newcap *= 2;
			char *tmp = realloc(data, newcap);
			if (!tmp)
				goto end;
			data = tmp;
			/* Ensure newly-allocated tail is NUL (for safety) */
			memset(data + cap, 0x00, newcap - cap);
			cap = newcap;
		}
		memcpy(data + len, buf, blen);
		len += blen;
		data[len] = '\0';
	}

	json_data = cJSON_Parse(data);
end:
	if (fp)
		fclose(fp);
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
			/* INI value is in CHIPS (e.g. 0.02); store in table chips. */
			BB_in_table_chips = chips_to_table_chips(
				iniparser_getdouble(ini, "table:big_blind", 0));
			SB_in_table_chips = BB_in_table_chips / 2;
		}
		// min_stake and max_stake are read in CHIPS, stored in table chips
		if (0 != iniparser_getdouble(ini, "table:min_stake", 0)) {
			table_min_stake_in_table_chips = chips_to_table_chips(
				iniparser_getdouble(ini, "table:min_stake", 0));
		}
		if (0 != iniparser_getdouble(ini, "table:max_stake", 0)) {
			table_max_stake_in_table_chips = chips_to_table_chips(
				iniparser_getdouble(ini, "table:max_stake", 0));
		}

		if (0 != iniparser_getdouble(ini, "dealer:chips_tx_fee", 0)) {
			chips_tx_fee = iniparser_getdouble(ini, "dealer:chips_tx_fee", 0);
		}
		if (0 != iniparser_getdouble(ini, "dealer:dcv_commission", 0)) {
			dcv_commission_percentage = iniparser_getdouble(ini, "dealer:dcv_commission", 0);
		}
		if (NULL != iniparser_getstring(ini, "dealer:gui_host", NULL)) {
			/* Avoid overflowing fixed-size global buffer */
			snprintf(dcv_hosted_gui_url, sizeof(dcv_hosted_gui_url), "%s",
				 iniparser_getstring(ini, "dealer:gui_host", NULL));
		}
		// Read gui_ws_port from config, use default if not set or invalid
		int port = iniparser_getint(ini, "dealer:gui_ws_port", DEFAULT_DEALER_WS_PORT);
		if (port > 0 && port <= 65535) {
			gui_ws_port = port;
		} else {
			gui_ws_port = DEFAULT_DEALER_WS_PORT;
		}
		threshold_value = iniparser_getint(ini, "dealer:min_cashiers", threshold_value);
		if (-1 != iniparser_getboolean(ini, "private table:is_table_private", -1)) {
			is_table_private = iniparser_getboolean(ini, "private table:is_table_private", -1);
		}
		if (NULL != iniparser_getstring(ini, "private table:table_password", NULL)) {
			snprintf(table_password, sizeof(table_password), "%s",
				 iniparser_getstring(ini, "private table:table_password", NULL));
		}
		// bet_ln_config removed - Lightning Network support removed, using CHIPS-only payments
		iniparser_freedict(ini);
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
			snprintf(player_name, sizeof(player_name), "%s",
				 iniparser_getstring(ini, "player:name", NULL));
		}
		if (-1 != iniparser_getboolean(ini, "private table:is_table_private", -1)) {
			is_table_private = iniparser_getboolean(ini, "private table:is_table_private", -1);
		}
		if (NULL != iniparser_getstring(ini, "private table:table_password", NULL)) {
			snprintf(table_password, sizeof(table_password), "%s",
				 iniparser_getstring(ini, "private table:table_password", NULL));
		}
		// bet_ln_config removed - Lightning Network support removed, using CHIPS-only payments
		int port = iniparser_getint(ini, "player:gui_ws_port", DEFAULT_PLAYER_WS_PORT);
		if (port > 0 && port <= 65535) {
			gui_ws_port = port;
		} else {
			gui_ws_port = DEFAULT_PLAYER_WS_PORT;
		}
		iniparser_freedict(ini);
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
		char str[64];
		int i = 1;
		snprintf(str, sizeof(str), "cashier:node-%d", i);
		cashiers_info = cJSON_CreateArray();
		while (NULL != iniparser_getstring(ini, str, NULL)) {
			cJSON_AddItemToArray(cashiers_info, cJSON_Parse(iniparser_getstring(ini, str, NULL)));
			memset(str, 0x00, sizeof(str));
			snprintf(str, sizeof(str), "cashier:node-%d", ++i);
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
		int port = iniparser_getint(ini, "cashier:gui_ws_port", DEFAULT_CASHIER_WS_PORT);
		if (port > 0 && port <= 65535) {
			gui_ws_port = port;
		} else {
			gui_ws_port = DEFAULT_CASHIER_WS_PORT;
		}
		iniparser_freedict(ini);
	}
}

void bet_display_cashier_hosted_gui()
{
	dictionary *ini = NULL;

	ini = iniparser_load(player_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", player_config_ini_file);
	} else {
		char str[64];
		int i = 1;
		snprintf(str, sizeof(str), "gui:cashier-%d", i);
		while (NULL != iniparser_getstring(ini, str, NULL)) {
			if (check_url(iniparser_getstring(ini, str, NULL)))
				dlg_warn("%s", iniparser_getstring(ini, str, NULL));
			memset(str, 0x00, sizeof(str));
			snprintf(str, sizeof(str), "gui:cashier-%d", ++i);
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

bool bet_is_new_block_set()
{
	dictionary *ini = NULL;
	bool is_new_block_set = false;

	/* Use the same blockchain.ini that bet_parse_blockchain_config_ini_file()
	 * already resolved relative to the bet binary, instead of a hardcoded
	 * "$HOME/bet/privatebet/config/blockchain_config.ini" path that only
	 * exists on the canonical production layout. */
	ini = iniparser_load(blockchain_config_ini_file);
	if (ini == NULL) {
		dlg_error("error in parsing %s", blockchain_config_ini_file);
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
			/* Accept legacy values verbatim, otherwise accept any verus
			 * CLI invocation (any "-chain=...", regtest or PBaaS) so a
			 * local test chain like VRSCTEST works without a code edit. */
			bool is_legacy_chips = (strcmp(blockchain_cli, chips_cli) == 0) ||
					       (strcmp(blockchain_cli, verus_chips_cli) == 0);
			bool is_verus_cli = (strncmp(blockchain_cli, "verus", 5) == 0);
			if (!is_legacy_chips && !is_verus_cli) {
				dlg_warn(
					"The blockchain client configured in ./config/blockchain.ini is not recognised, falling back to '%s'",
					chips_cli);
				memset(blockchain_cli, 0x00, sizeof(blockchain_cli));
				strncpy(blockchain_cli, chips_cli, sizeof(blockchain_cli));
			}
			/* Mirror the configured CLI into the verus-cli pointer so all
			 * code paths that branch on it use the same chain. */
			if (is_verus_cli) {
				verus_chips_cli = strdup(blockchain_cli);
				dlg_info("verus_chips_cli set to '%s' (from blockchain.ini)", verus_chips_cli);
			}
		}
		const char *cur = iniparser_getstring(ini, "blockchain:currency", NULL);
		if (cur && *cur) {
			memset(bet_currency_name, 0x00, sizeof(bet_currency_name));
			strncpy(bet_currency_name, cur, sizeof(bet_currency_name) - 1);
			dlg_info("blockchain currency set to '%s' (from %s)", bet_currency_name,
				 blockchain_config_ini_file);
		}
	}
}

const char *bet_get_currency(void)
{
	return bet_currency_name;
}

int32_t bet_parse_verus_dealer()
{
	int32_t retval = OK;
	dictionary *ini = NULL;
	struct table t = { 0 };

	ini = iniparser_load(verus_dealer_config);
	if (!ini)
		return ERR_INI_PARSING;

	// Parse verus section
	if (NULL != iniparser_getstring(ini, "verus:dealer_id", NULL)) {
		strncpy(t.dealer_id, iniparser_getstring(ini, "verus:dealer_id", NULL), sizeof(t.dealer_id) - 1);
	}
	if (NULL != iniparser_getstring(ini, "verus:cashier_id", NULL)) {
		strncpy(t.cashier_id, iniparser_getstring(ini, "verus:cashier_id", NULL), sizeof(t.cashier_id) - 1);
	} else {
		// Default to main cashier ID if not specified
		strncpy(t.cashier_id, "cashier", sizeof(t.cashier_id) - 1);
	}

	// Parse table section
	if (-1 != iniparser_getint(ini, "table:max_players", -1)) {
		t.max_players = (uint8_t)iniparser_getint(ini, "table:max_players", -1);
	}
	if (0 != iniparser_getdouble(ini, "table:big_blind", 0)) {
		float_to_uint32_s(&t.big_blind, iniparser_getdouble(ini, "table:big_blind", 0));
	}
	// min_stake and max_stake are now in CHIPS directly
	if (0 != iniparser_getdouble(ini, "table:min_stake", 0)) {
		float_to_uint32_s(&t.min_stake, iniparser_getdouble(ini, "table:min_stake", 0));
	}
	if (0 != iniparser_getdouble(ini, "table:max_stake", 0)) {
		float_to_uint32_s(&t.max_stake, iniparser_getdouble(ini, "table:max_stake", 0));
	}
	if (NULL != iniparser_getstring(ini, "table:table_id", NULL)) {
		strncpy(t.table_id, iniparser_getstring(ini, "table:table_id", NULL), sizeof(t.table_id) - 1);
	}

	iniparser_freedict(ini);
	retval = dealer_init(t);
	return retval;
}

int32_t bet_parse_verus_dealer_with_reset(bool reset)
{
	int32_t retval = OK;
	dictionary *ini = NULL;
	struct table t = { 0 };

	ini = iniparser_load(verus_dealer_config);
	if (!ini)
		return ERR_INI_PARSING;

	// Parse verus section
	if (NULL != iniparser_getstring(ini, "verus:dealer_id", NULL)) {
		strncpy(t.dealer_id, iniparser_getstring(ini, "verus:dealer_id", NULL), sizeof(t.dealer_id) - 1);
	}
	if (NULL != iniparser_getstring(ini, "verus:cashier_id", NULL)) {
		strncpy(t.cashier_id, iniparser_getstring(ini, "verus:cashier_id", NULL), sizeof(t.cashier_id) - 1);
	} else {
		strncpy(t.cashier_id, "cashier", sizeof(t.cashier_id) - 1);
	}

	// Parse table section
	if (-1 != iniparser_getint(ini, "table:max_players", -1)) {
		t.max_players = (uint8_t)iniparser_getint(ini, "table:max_players", -1);
	}
	if (0 != iniparser_getdouble(ini, "table:big_blind", 0)) {
		float_to_uint32_s(&t.big_blind, iniparser_getdouble(ini, "table:big_blind", 0));
	}
	if (0 != iniparser_getdouble(ini, "table:min_stake", 0)) {
		float_to_uint32_s(&t.min_stake, iniparser_getdouble(ini, "table:min_stake", 0));
	}
	if (0 != iniparser_getdouble(ini, "table:max_stake", 0)) {
		float_to_uint32_s(&t.max_stake, iniparser_getdouble(ini, "table:max_stake", 0));
	}
	if (NULL != iniparser_getstring(ini, "table:table_id", NULL)) {
		strncpy(t.table_id, iniparser_getstring(ini, "table:table_id", NULL), sizeof(t.table_id) - 1);
	}

	iniparser_freedict(ini);
	
	if (reset) {
		retval = dealer_init_with_reset(t);
	} else {
		retval = dealer_init(t);
	}
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
	// Read WebSocket port for GUI mode (default: 9001)
	player_config.ws_port = iniparser_getint(ini, "verus:ws_port", DEFAULT_PLAYER_WS_PORT);
	
	//Check if all IDs are valid
	if ((player_config.dealer_id[0] == '\0') || (player_config.table_id[0] == '\0') || (player_config.verus_pid[0] == '\0') ||
	    !is_id_exists(player_config.dealer_id, 0) || !is_id_exists(player_config.table_id, 0)) {
		return ERR_CONFIG_PLAYER_ARGS;
	}
	//Check if the node has player IDs priv keys
	if (!id_cansignfor(player_config.verus_pid, 0, &retval)) {
		return retval;
	}

	return retval;
}

/* Verus IDs and Keys Configuration */
struct verus_ids_keys_config verus_config = { 0 };

void bet_parse_verus_ids_keys_config(void)
{
	dictionary *ini = NULL;
	const char *value = NULL;

	ini = iniparser_load(verus_ids_keys_config_file);
	if (ini == NULL) {
		dlg_warn("Could not load %s, using default values", verus_ids_keys_config_file);
		// Initialize with defaults (from #defines)
		strncpy(verus_config.parent_id, POKER_ID_FQN, sizeof(verus_config.parent_id) - 1);
		strncpy(verus_config.cashier_id, CASHIERS_ID_FQN, sizeof(verus_config.cashier_id) - 1);
		strncpy(verus_config.dealer_id, DEALERS_ID_FQN, sizeof(verus_config.dealer_id) - 1);
		strncpy(verus_config.cashiers_short, CASHIERS_ID, sizeof(verus_config.cashiers_short) - 1);
		strncpy(verus_config.dealers_short, DEALERS_ID, sizeof(verus_config.dealers_short) - 1);
		strncpy(verus_config.poker_short, POKER_ID, sizeof(verus_config.poker_short) - 1);
		verus_config.initialized = 1;
		return;
	}

	// Load from config file
	if ((value = iniparser_getstring(ini, "identities:parent_id", NULL)) != NULL) {
		strncpy(verus_config.parent_id, value, sizeof(verus_config.parent_id) - 1);
		verus_config.parent_id[sizeof(verus_config.parent_id) - 1] = '\0';
	} else {
		strncpy(verus_config.parent_id, POKER_ID_FQN, sizeof(verus_config.parent_id) - 1);
	}

	if ((value = iniparser_getstring(ini, "identities:cashier_id", NULL)) != NULL) {
		strncpy(verus_config.cashier_id, value, sizeof(verus_config.cashier_id) - 1);
		verus_config.cashier_id[sizeof(verus_config.cashier_id) - 1] = '\0';
	} else {
		strncpy(verus_config.cashier_id, CASHIERS_ID_FQN, sizeof(verus_config.cashier_id) - 1);
	}

	if ((value = iniparser_getstring(ini, "identities:dealer_id", NULL)) != NULL) {
		strncpy(verus_config.dealer_id, value, sizeof(verus_config.dealer_id) - 1);
		verus_config.dealer_id[sizeof(verus_config.dealer_id) - 1] = '\0';
	} else {
		strncpy(verus_config.dealer_id, DEALERS_ID_FQN, sizeof(verus_config.dealer_id) - 1);
	}

	if ((value = iniparser_getstring(ini, "identities:cashiers_short", NULL)) != NULL) {
		strncpy(verus_config.cashiers_short, value, sizeof(verus_config.cashiers_short) - 1);
	} else {
		strncpy(verus_config.cashiers_short, CASHIERS_ID, sizeof(verus_config.cashiers_short) - 1);
	}

	if ((value = iniparser_getstring(ini, "identities:dealers_short", NULL)) != NULL) {
		strncpy(verus_config.dealers_short, value, sizeof(verus_config.dealers_short) - 1);
	} else {
		strncpy(verus_config.dealers_short, DEALERS_ID, sizeof(verus_config.dealers_short) - 1);
	}

	if ((value = iniparser_getstring(ini, "identities:poker_short", NULL)) != NULL) {
		strncpy(verus_config.poker_short, value, sizeof(verus_config.poker_short) - 1);
	} else {
		strncpy(verus_config.poker_short, POKER_ID, sizeof(verus_config.poker_short) - 1);
	}

	verus_config.initialized = 1;
	iniparser_freedict(ini);
	dlg_info("Loaded Verus IDs and Keys configuration from %s", verus_ids_keys_config_file);
}

const char *bet_get_cashiers_id_fqn(void)
{
	if (verus_config.initialized) {
		return verus_config.cashier_id;
	}
	return CASHIERS_ID_FQN;
}

const char *bet_get_cashier_short_name(void)
{
	// Return just the short name (e.g., "cashier") for use with update_cmm
	// which adds the parent automatically
	if (verus_config.initialized && verus_config.cashier_id[0] != '\0') {
		// Extract short name from FQN (e.g., "cashier.sg777z.chips.vrsc@" -> "cashier")
		static char short_name[64];
		strncpy(short_name, verus_config.cashier_id, sizeof(short_name) - 1);
		short_name[sizeof(short_name) - 1] = '\0';
		char *dot = strchr(short_name, '.');
		if (dot) {
			*dot = '\0';  // Truncate at first dot
		}
		return short_name;
	}
	return "cashier";
}

const char *bet_get_dealers_id_fqn(void)
{
	if (verus_config.initialized) {
		return verus_config.dealer_id;
	}
	return DEALERS_ID_FQN;
}

const char *bet_get_poker_id_fqn(void)
{
	if (verus_config.initialized) {
		return verus_config.parent_id;
	}
	return POKER_ID_FQN;
}

/* RPC Credentials Configuration */
struct rpc_credentials rpc_config = { 0 };

void bet_parse_rpc_credentials(void)
{
	dictionary *ini = NULL;
	const char *value = NULL;

	// Default values
	strncpy(rpc_config.url, "http://127.0.0.1:22778", sizeof(rpc_config.url) - 1);
	strncpy(rpc_config.user, "", sizeof(rpc_config.user) - 1);
	strncpy(rpc_config.password, "", sizeof(rpc_config.password) - 1);
	rpc_config.use_rest_api = 0;  // Default to CLI mode for backwards compatibility
	rpc_config.initialized = 0;

	ini = iniparser_load(rpc_credentials_file);
	if (ini == NULL) {
		dlg_warn("Could not load %s, RPC credentials not configured", rpc_credentials_file);
		dlg_info("To use REST API, create %s with [rpc] section containing url, user, password", rpc_credentials_file);
		return;
	}

	// Load RPC URL
	if ((value = iniparser_getstring(ini, "rpc:url", NULL)) != NULL) {
		strncpy(rpc_config.url, value, sizeof(rpc_config.url) - 1);
		rpc_config.url[sizeof(rpc_config.url) - 1] = '\0';
	}

	// Load RPC user
	if ((value = iniparser_getstring(ini, "rpc:user", NULL)) != NULL) {
		strncpy(rpc_config.user, value, sizeof(rpc_config.user) - 1);
		rpc_config.user[sizeof(rpc_config.user) - 1] = '\0';
	}

	// Load RPC password
	if ((value = iniparser_getstring(ini, "rpc:password", NULL)) != NULL) {
		strncpy(rpc_config.password, value, sizeof(rpc_config.password) - 1);
		rpc_config.password[sizeof(rpc_config.password) - 1] = '\0';
	}

	// Load use_rest_api flag (default to 1 if credentials file exists)
	rpc_config.use_rest_api = iniparser_getboolean(ini, "rpc:use_rest_api", 1);

	rpc_config.initialized = 1;
	iniparser_freedict(ini);
	
	dlg_info("Loaded RPC credentials from %s", rpc_credentials_file);
	dlg_info("RPC URL: %s, User: %s, REST API: %s", 
		rpc_config.url, rpc_config.user, 
		rpc_config.use_rest_api ? "enabled" : "disabled");
}

const char *bet_get_rpc_url(void)
{
	if (rpc_config.initialized) {
		return rpc_config.url;
	}
	return "http://127.0.0.1:22778";
}

const char *bet_get_rpc_user(void)
{
	if (rpc_config.initialized) {
		return rpc_config.user;
	}
	return "";
}

const char *bet_get_rpc_password(void)
{
	if (rpc_config.initialized) {
		return rpc_config.password;
	}
	return "";
}

int bet_use_rest_api(void)
{
	return rpc_config.initialized && rpc_config.use_rest_api;
}
