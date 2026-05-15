#include "bet.h"
#include "client.h"
#include "cashier.h"
#include "network.h"
#include "common.h"
#include "commands.h"
#include "storage.h"
#include "misc.h"
#include "cards.h"
#include "err.h"
#include "switchs.h"
#include "payment.h"
// Legacy pub-sub functions removed - use Verus ID communication instead

int32_t no_of_notaries;

/***********************************************************************************************************
At the moment it was defined as, atleast there should exist two cashier/notary nodes in order to play the 
game. As the number of notary nodes gets increased this value will be increased in the future. More 
threshold_value means more trust.
***********************************************************************************************************/

int32_t threshold_value = 2;

/***********************************************************************************************************
The notary_node_ips and notary_node_pubkeys values are read from the config file named cashier_nodes.json. 
Since the sg777's nodes are trusted cashier nodes, the information regarding them configured into the 
cashier_nodes.json file as follow:

[{
    "pubkey":       "034d2b213240cfb4efcc24cc21a237a2313c0c734a4f0efc30087c095fd010385f",
    "ip":   "159.69.23.28"
}, {
    "pubkey":       "02137b5400ace827c225238765d4661a1b4fe589b9b625b10469c69f0867f7bc53",
    "ip":   "159.69.23.29"
}, {
    "pubkey":       "03b020866c9efae106e3c086a640e8b50cce7ae91cb30996ecf0f8816ce5ed8f49",
    "ip":   "159.69.23.30"
}, {
    "pubkey":       "0274ae1ce244bd0f9c52edfb6b9e60dc5d22f001dd74af95d1297edbcc8ae39568",
    "ip":   "159.69.23.31"
}]
***********************************************************************************************************/

char **notary_node_ips = NULL;
char **notary_node_pubkeys = NULL;

/***********************************************************************************************************
This address has been given by jl777, 0.25% of every pot will go to this address. These funds are used for 
development purpose.
***********************************************************************************************************/
char dev_fund_addr[64] = { "RSdMRYeeouw3hepxNgUzHn34qFhn1tsubb" };
double dev_fund_commission = 1;

struct cashier *cashier_info = NULL;
int32_t live_notaries = 0;
int32_t *notary_status = NULL;
int32_t notary_response = 0;

/* All chip-amount globals live in the integer "table chips" unit.
 * default_bb_in_chips, default_min_stake, default_max_stake are CHIPS
 * constants from common.h, so feed them through chips_to_table_chips()
 * once at init. */
int64_t BB_in_table_chips = 0;             /* set in initializer below */
int64_t SB_in_table_chips = 0;
int64_t table_stake_in_table_chips = 0;
int64_t table_min_stake_in_table_chips = 0;
int64_t table_max_stake_in_table_chips = 0;

double table_stack_in_bb = default_min_stake_in_bb;
double chips_tx_fee = default_chips_tx_fee;

/* Static initializer can't call chips_to_table_chips(), so seed the
 * table-chip globals from common.h's CHIPS macros via a constructor that
 * runs before main(). Keeps the "1 CHIP = 100 table chips" rule centralized
 * in chips_to_table_chips() rather than hardcoded here. */
__attribute__((constructor)) static void cashier_init_table_chip_globals(void)
{
	BB_in_table_chips = chips_to_table_chips(default_bb_in_chips);
	SB_in_table_chips = chips_to_table_chips(default_sb_in_chips);
	table_min_stake_in_table_chips = chips_to_table_chips(default_min_stake);
	table_max_stake_in_table_chips = chips_to_table_chips(default_max_stake);
	table_stake_in_table_chips = chips_to_table_chips(default_min_stake_in_bb * default_bb_in_chips);
}

char *legacy_m_of_n_msig_addr = NULL;

int32_t bvv_state = 0;
char dealer_ip_for_bvv[128];

char bvv_unique_id[65];

void bet_game_multisigaddress()
{
	cJSON *msig_info = NULL;
	cJSON *addr_list = NULL;

	msig_info = cJSON_CreateObject();
	addr_list = cJSON_CreateArray();
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1)
			cJSON_AddItemToArray(addr_list, cJSON_CreateString_Length(notary_node_pubkeys[i], 67));
	}
	cJSON_AddStringToObject(msig_info, "method", "game_multisigaddress");
	cJSON_AddItemToObject(msig_info, "threshold_value", cJSON_CreateNumber(threshold_value));
	cJSON_AddItemToObject(msig_info, "pubkeys", addr_list);

	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			bet_msg_cashier(msig_info, notary_node_ips[i]);
		}
	}
}
void bet_compute_m_of_n_msig_addr()
{
	cJSON *msig_addr = NULL;
	msig_addr = chips_add_multisig_address();
	dlg_info("The msig_address of the cashier nodes for payin tx \n %s", cJSON_Print(msig_addr));
	if (msig_addr) {
		const char *addr = jstr(msig_addr, "address");
		if (addr == NULL) {
			dlg_error("Invalid address in msig_addr");
			return;
		}
		size_t addr_len = strlen(addr);
		legacy_m_of_n_msig_addr = (char *)malloc(addr_len + 1);
		if (legacy_m_of_n_msig_addr == NULL) {
			dlg_error("Memory allocation failed for legacy_m_of_n_msig_addr");
			return;
		}
		memcpy(legacy_m_of_n_msig_addr, addr, addr_len);
		legacy_m_of_n_msig_addr[addr_len] = '\0';
		//Import the multisig address incase if its not 1-of-1 and not belong to the local wallet.
		if ((!chips_ismine(legacy_m_of_n_msig_addr)) && (chips_iswatchonly(legacy_m_of_n_msig_addr) == 0)) {
			dlg_info("Importing msig_address ::%s, it takes a while", legacy_m_of_n_msig_addr);
			chips_import_address(legacy_m_of_n_msig_addr);
		}
	} else {
		dlg_error("Error in computation of m-of-n multisig address");
		exit(0);
	}
}

void bet_check_cashier_nodes()
{
	bet_check_cashiers_status();

	if (live_notaries < 1) {
		dlg_warn(
			"The player node must be able to reach atleast one notary node, otherwise it can't be having BVV during deck shuffling, so exiting");
		exit(0);
	}

	dlg_info("Notary node status");
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			dlg_info("%d. %s active", i + 1, notary_node_ips[i]);
		} else {
			dlg_info("%d. %s not active", i + 1, notary_node_ips[i]);
		}
	}

	if (live_notaries < threshold_value) {
		dlg_warn("Cashier available:: %d \n Cashier needed ::%d\n so readjusting cashiers needed to :: %d",
			 live_notaries, threshold_value, live_notaries);
		threshold_value = live_notaries;
	}
}

void bet_check_cashiers_status()
{
	cJSON *live_info = NULL;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	cJSON_AddStringToObject(live_info, "id", unique_id);

	live_notaries = 0;
	for (int32_t i = 0; i < no_of_notaries; i++) {
		cJSON *temp = bet_msg_cashier_with_response_id(live_info, notary_node_ips[i], "live");
		if ((temp) && (strcmp(jstr(temp, "method"), "live") == 0)) {
			notary_status[i] = 1;
			live_notaries++;
		} else {
			notary_status[i] = 0;
		}
	}
}

int32_t bet_send_status(struct cashier *cashier_info, char *id)
{
	int32_t retval = OK;
	cJSON *live_info = NULL;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	cJSON_AddStringToObject(live_info, "id", id);
// Nanomsg removed - no longer used
	return retval;
}

int32_t bet_process_lock_in_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	cJSON *status = NULL;

	retval = bet_run_query(jstr(argjson, "sql_query"));
	status = cJSON_CreateObject();
	cJSON_AddStringToObject(status, "method", "query_status");
	cJSON_AddNumberToObject(status, "status", retval);
// Nanomsg removed - no longer used
	return retval;
}

int32_t bet_cashier_process_raw_msig_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	cJSON *signed_tx = NULL;
	char *tx = NULL;

	signed_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(signed_tx, "method", "signed_tx");
	cJSON_AddStringToObject(signed_tx, "id", jstr(argjson, "id"));
	tx = cJSON_Print(cJSON_GetObjectItem(argjson, "tx"));
	cJSON_AddItemToObject(signed_tx, "signed_tx", chips_sign_raw_tx_with_wallet(tx));
	dlg_info("signed_tx::%s", cJSON_Print(signed_tx));
// Nanomsg removed - no longer used
	return retval;
}

int32_t bet_process_payout_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	char *sql_query = NULL;
	const char *tx_info_str = NULL;
	const char *table_id_str = NULL;
	int required_size;

	tx_info_str = cJSON_Print(cJSON_GetObjectItem(argjson, "tx_info"));
	table_id_str = jstr(argjson, "table_id");
	if (tx_info_str == NULL || table_id_str == NULL) {
		dlg_error("Invalid tx_info or table_id");
		return ERR_ARGS_NULL;
	}

	required_size = snprintf(NULL, 0,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = %s where table_id = \"%s\";",
		tx_info_str, table_id_str) + 1;

	if (required_size > sql_query_size) {
		sql_query = calloc(required_size, sizeof(char));
	} else {
		sql_query = calloc(sql_query_size, sizeof(char));
	}
	if (sql_query == NULL) {
		if (tx_info_str) free((void *)tx_info_str);
		return ERR_MEMORY_ALLOC;
	}

	snprintf(sql_query, required_size,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = %s where table_id = \"%s\";",
		tx_info_str, table_id_str);
	dlg_info("sql_query::%s", sql_query);
	retval = bet_run_query(sql_query);
	if (tx_info_str) free((void *)tx_info_str);
	if (sql_query)
		free(sql_query);

	return retval;
}

int32_t bet_process_game_info(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	char *sql_query = NULL;
	cJSON *game_state = NULL;

	game_state = cJSON_GetObjectItem(argjson, "game_state");
	const char *table_id_str = jstr(argjson, "table_id");
	const char *game_state_str = cJSON_Print(game_state);
	int required_size;

	if (table_id_str == NULL || game_state_str == NULL) {
		dlg_error("Invalid table_id or game_state");
		return ERR_ARGS_NULL;
	}

	required_size = snprintf(NULL, 0, "INSERT into cashier_game_state values(\"%s\", \'%s\');",
		table_id_str, game_state_str) + 1;

	if (required_size > sql_query_size) {
		sql_query = calloc(required_size, sizeof(char));
	} else {
		sql_query = calloc(sql_query_size, sizeof(char));
	}
	if (sql_query == NULL) {
		if (game_state_str) free((void *)game_state_str);
		return ERR_MEMORY_ALLOC;
	}

	snprintf(sql_query, required_size, "INSERT into cashier_game_state values(\"%s\", \'%s\');",
		table_id_str, game_state_str);
	retval = bet_run_query(sql_query);
	if (game_state_str) free((void *)game_state_str);
	if (sql_query)
		free(sql_query);
	return retval;
}

cJSON *bet_resolve_game_dispute(cJSON *game_info)
{
	cJSON *msig_addr_nodes = NULL;
	int min_cashiers, active_cashiers = 0;
	int *cashier_node_status = NULL;
	int *validation_status = NULL;
	char **cashier_node_ips = NULL;
	int32_t no_of_cashier_nodes, approved_cashiers_count = 0, ip_length = 20;

	msig_addr_nodes = cJSON_CreateArray();
	msig_addr_nodes = cJSON_Parse(jstr(game_info, "msig_addr_nodes"));

	no_of_cashier_nodes = cJSON_GetArraySize(msig_addr_nodes);
	cashier_node_status = calloc(no_of_cashier_nodes, sizeof(int));
	validation_status = calloc(no_of_cashier_nodes, sizeof(int));
	cashier_node_ips = calloc(no_of_cashier_nodes, sizeof(char *));

	if (cashier_node_status == NULL || validation_status == NULL || cashier_node_ips == NULL) {
		if (cashier_node_status) free(cashier_node_status);
		if (validation_status) free(validation_status);
		if (cashier_node_ips) free(cashier_node_ips);
		return NULL;
	}

	min_cashiers = jint(game_info, "min_cashiers");

	for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
		cashier_node_status[i] = 0;
		cashier_node_ips[i] = calloc(ip_length, sizeof(char));
		if (cashier_node_ips[i] == NULL) {
			// Cleanup all previous allocations
			for (int32_t j = 0; j < i; j++) {
				free(cashier_node_ips[j]);
			}
			free(cashier_node_status);
			free(validation_status);
			free(cashier_node_ips);
			return NULL;
		}
		char *ip_json = cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i));
		if (ip_json != NULL) {
			const char *ip_str = unstringify(ip_json);
			if (ip_str != NULL) {
				strncpy(cashier_node_ips[i], ip_str, ip_length - 1);
				cashier_node_ips[i][ip_length - 1] = '\0';
			}
			free(ip_json);
		}
		for (int32_t j = 0; j < no_of_notaries; j++) {
			if (notary_status[j] == 1) {
				if (strcmp(notary_node_ips[j], cashier_node_ips[i]) == 0) {
					cashier_node_status[i] = 1;
					active_cashiers++;
					break;
				}
			}
		}
	}
	dlg_info("min_cashiers::%d,active_cashiers::%d", min_cashiers, active_cashiers);
	if (active_cashiers >= min_cashiers) {
		cJSON *send_game_info = cJSON_CreateObject();
		cJSON_AddStringToObject(send_game_info, "method", "validate_game_details");
		cJSON_AddStringToObject(send_game_info, "id", unique_id);
		cJSON_AddItemToObject(send_game_info, "game_details", game_info);
		for (int32_t i = 0; i < no_of_cashier_nodes; i++) {
			if (cashier_node_status[i] == 1) {
				cJSON *temp = bet_msg_cashier_with_response_id(send_game_info, cashier_node_ips[i],
									       "game_validation_status");
				if (temp) {
					validation_status[i] = jint(temp, "status");
					if (validation_status[i] == 1)
						approved_cashiers_count++;
				}
			}
		}
	}
	dlg_info("approved_cashiers::%d", approved_cashiers_count);
	if (approved_cashiers_count >= min_cashiers) {
		dlg_info(" Make a request to reverse the tx_id::%s", jstr(game_info, "tx_id"));
		char tx_ids[1][100];
		int no_of_in_txs = 1;
		cJSON *raw_tx = NULL;
		int signers = 0;
		cJSON *hex = NULL, *tx = NULL;

		const char *tx_id_str = jstr(game_info, "tx_id");
		if (tx_id_str != NULL) {
			strncpy(tx_ids[0], tx_id_str, sizeof(tx_ids[0]) - 1);
			tx_ids[0][sizeof(tx_ids[0]) - 1] = '\0';
		} else {
			tx_ids[0][0] = '\0';
		}
		raw_tx = chips_create_tx_from_tx_list(jstr(game_info, "addr"), no_of_in_txs, tx_ids);
		dlg_info("raw_tx::%s", cJSON_Print(raw_tx));
		for (int i = 0; i < no_of_cashier_nodes; i++) {
			if (cashier_node_status[i] == 1) {
				if (signers == 0) {
					cJSON *temp = chips_sign_msig_tx_of_table_id(cashier_node_ips[i], raw_tx,
										     jstr(game_info, "table_id"));
					if (temp == NULL) {
						continue;
					}
					if (cJSON_GetObjectItem(temp, "signed_tx") != NULL) {
						hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"),
									  "hex");
						signers++;
					} else {
						dlg_error("error::%s", jstr(temp, "err_str"));
						goto end;
					}
				} else if (signers == 1) {
					cJSON *temp1 = chips_sign_msig_tx_of_table_id(cashier_node_ips[i], hex,
										      jstr(game_info, "table_id"));
					if (temp1 == NULL) {
						continue;
					}
					if (cJSON_GetObjectItem(temp1, "signed_tx") != NULL) {
						cJSON *status = cJSON_GetObjectItem(
							cJSON_GetObjectItem(temp1, "signed_tx"), "complete");
						if (strcmp(cJSON_Print(status), "true") == 0) {
							tx = chips_send_raw_tx(cJSON_GetObjectItem(temp1, "signed_tx"));
							signers++;
							break;
						}
					} else {
						dlg_error("error::%s", jstr(temp1, "err_str"));
						goto end;
					}
				}
			}
		}
		if (tx) {
			dlg_info("Final payout tx::%s", cJSON_Print(tx));
			cJSON *update_tx_info = NULL;
			update_tx_info = cJSON_CreateObject();
			cJSON_AddStringToObject(update_tx_info, "method", "tx_spent");
			cJSON_AddStringToObject(update_tx_info, "payin_tx_id", jstr(game_info, "tx_id"));
			cJSON_AddStringToObject(update_tx_info, "payout_tx_id", unstringify(cJSON_Print(tx)));
			bet_msg_multiple_cashiers(update_tx_info, cashier_node_ips, no_of_cashier_nodes);
			char *sql_query = NULL;
			const char *tx_id_str = jstr(game_info, "tx_id");
			int required_size;

			if (tx_id_str == NULL) {
				dlg_error("Invalid tx_id in game_info");
				free_json(update_tx_info);
			} else {
				required_size = snprintf(NULL, 0, "UPDATE player_tx_mapping set status = 0 where tx_id = \'%s\';",
					tx_id_str) + 1;

				if (required_size > sql_query_size) {
					sql_query = calloc(required_size, sizeof(char));
				} else {
					sql_query = calloc(sql_query_size, sizeof(char));
				}
				if (sql_query == NULL) {
					dlg_error("Memory allocation failed for sql_query");
					free_json(update_tx_info);
				} else {
					snprintf(sql_query, required_size, "UPDATE player_tx_mapping set status = 0 where tx_id = \'%s\';",
						tx_id_str);
					bet_run_query(sql_query);
					if (sql_query)
						free(sql_query);
					free_json(update_tx_info);
				}
			}
		}
	}

end:
	return NULL;
}

void bet_process_solve(cJSON *argjson, struct cashier *cashier_info)
{
	cJSON *disputed_games_info = NULL;

	disputed_games_info = cJSON_CreateArray();
	disputed_games_info = cJSON_GetObjectItem(argjson, "disputed_games_info");

	bet_check_cashiers_status();
	for (int32_t i = 0; i < cJSON_GetArraySize(disputed_games_info); i++) {
		bet_resolve_game_dispute(cJSON_GetArrayItem(disputed_games_info, i));
	}
}

int32_t bet_validate_game_details(cJSON *argjson, struct cashier *cashier_info)
{
	int retval = OK;
	cJSON *response_info = NULL;
	int value = 1;

	response_info = cJSON_CreateObject();
	/*
	Need to implement the validation logic here - sg777
	*/
	cJSON_AddStringToObject(response_info, "method", "game_validation_status");
	cJSON_AddStringToObject(response_info, "id", jstr(argjson, "id"));
	cJSON_AddNumberToObject(response_info, "status", value);

	// Nanomsg removed - no longer used
	retval = OK;
	return retval;
}

void bet_cashier_status_loop(void *_ptr)
{
	// Nanomsg removed - communication now via websockets only
	// The cashier status loop is handled through websocket callbacks
	(void)_ptr; // Suppress unused parameter warning
}

static int32_t bet_update_tx_spent(cJSON *argjson)
{
	int retval = OK;
	char *sql_query = NULL;

	const char *payout_tx_id_str = jstr(argjson, "payout_tx_id");
	const char *payin_tx_id_str = jstr(argjson, "payin_tx_id");
	int required_size;

	if (payout_tx_id_str == NULL || payin_tx_id_str == NULL) {
		dlg_error("Invalid payout_tx_id or payin_tx_id");
		return ERR_ARGS_NULL;
	}

	required_size = snprintf(NULL, 0,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = \'%s\' where payin_tx_id_status =\'%s\';",
		payout_tx_id_str, payin_tx_id_str) + 1;

	if (required_size > sql_query_size) {
		sql_query = calloc(required_size, sizeof(char));
	} else {
		sql_query = calloc(sql_query_size, sizeof(char));
	}
	if (sql_query == NULL) {
		return ERR_MEMORY_ALLOC;
	}

	snprintf(sql_query, required_size,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = \'%s\' where payin_tx_id_status =\'%s\';",
		payout_tx_id_str, payin_tx_id_str);
	retval = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);

	return retval;
}

static cJSON *bet_reverse_disputed_tx(cJSON *game_info)
{
	cJSON *msig_addr_nodes = NULL;
	int min_cashiers, active_cashiers = 0, fully_signed = 0;
	int *cashier_node_status = NULL;
	char **cashier_node_ips = NULL;
	int32_t no_of_cashier_nodes, ip_addr_len = 20;
	cJSON *tx = NULL;

	msig_addr_nodes = cJSON_CreateArray();
	msig_addr_nodes = cJSON_Parse(jstr(game_info, "msig_addr_nodes"));

	no_of_cashier_nodes = cJSON_GetArraySize(msig_addr_nodes);
	cashier_node_status = calloc(no_of_cashier_nodes, sizeof(int));
	cashier_node_ips = calloc(no_of_cashier_nodes, sizeof(char *));

	if (cashier_node_status == NULL || cashier_node_ips == NULL) {
		if (cashier_node_status) free(cashier_node_status);
		if (cashier_node_ips) free(cashier_node_ips);
		return NULL;
	}

	min_cashiers = jint(game_info, "min_cashiers");

	for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
		cashier_node_status[i] = 0;
		cashier_node_ips[i] = calloc(ip_addr_len, sizeof(char));
		if (cashier_node_ips[i] == NULL) {
			// Cleanup all previous allocations
			for (int32_t j = 0; j < i; j++) {
				free(cashier_node_ips[j]);
			}
			free(cashier_node_status);
			free(cashier_node_ips);
			return NULL;
		}
		char *ip_json = cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i));
		if (ip_json != NULL) {
			const char *ip_str = unstringify(ip_json);
			if (ip_str != NULL) {
				strncpy(cashier_node_ips[i], ip_str, ip_addr_len - 1);
				cashier_node_ips[i][ip_addr_len - 1] = '\0';
			}
			free(ip_json);
		}
		for (int32_t j = 0; j < no_of_notaries; j++) {
			if (notary_status[j] == 1) {
				if (strcmp(notary_node_ips[j], cashier_node_ips[i]) == 0) {
					cashier_node_status[i] = 1;
					active_cashiers++;
					break;
				}
			}
		}
	}
	dlg_info("active_cashier::%d::min_cashiers::%d", active_cashiers, min_cashiers);
	if (active_cashiers >= min_cashiers) {
		char tx_ids[1][100];
		int no_of_in_txs = 1;
		cJSON *raw_tx = NULL;
		int signers = 0;
		cJSON *tx_to_sign = NULL;

		const char *tx_id_str = jstr(game_info, "tx_id");
		if (tx_id_str != NULL) {
			strncpy(tx_ids[0], tx_id_str, sizeof(tx_ids[0]) - 1);
			tx_ids[0][sizeof(tx_ids[0]) - 1] = '\0';
		} else {
			tx_ids[0][0] = '\0';
		}
		if (chips_iswatchonly(jstr(game_info, "msig_addr")) == 0) {
			dlg_info("Importing the msigaddress::%s", jstr(game_info, "msig_addr"));
			chips_import_address(jstr(game_info, "msig_addr"));
		}
		raw_tx = chips_create_tx_from_tx_list(unstringify(jstr(game_info, "dispute_addr")), no_of_in_txs,
						      tx_ids);
		if (raw_tx == NULL)
			return NULL;
		tx_to_sign = raw_tx;
		dlg_info("tx_to_sign::%s", cJSON_Print(tx_to_sign));

		for (int i = 0; i < no_of_cashier_nodes; i++) {
			if (cashier_node_status[i] == 1) {
				cJSON *temp = chips_sign_msig_tx(cashier_node_ips[i], tx_to_sign);
				if (temp == NULL)
					continue;
				dlg_info("signed_tx::%s", cJSON_Print(temp));
				if (cJSON_GetObjectItem(temp, "signed_tx") != NULL) {
					tx_to_sign = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "hex");
					signers++;
					if (signers == min_cashiers) {
						cJSON *status = cJSON_GetObjectItem(
							cJSON_GetObjectItem(temp, "signed_tx"), "complete");
						if (strcmp(cJSON_Print(status), "true") == 0) {
							tx = chips_send_raw_tx(cJSON_GetObjectItem(temp, "signed_tx"));
						}
						if (tx)
							fully_signed = 1;
						break;
					}
				} else {
					dlg_error("error in signing at %s happened", cashier_node_ips[i]);
					goto end;
				}
				/*	
				else if (signers == 1) {
					cJSON *temp1 = chips_sign_msig_tx(cashier_node_ips[i], hex);
					if (temp1 == NULL)
						continue;
					dlg_info("signed_tx::%s", cJSON_Print(temp1));
					if (cJSON_GetObjectItem(temp1, "signed_tx") != NULL) {
						cJSON *status = cJSON_GetObjectItem(
							cJSON_GetObjectItem(temp1, "signed_tx"), "complete");
						if (strcmp(cJSON_Print(status), "true") == 0) {
							tx = chips_send_raw_tx(cJSON_GetObjectItem(temp1, "signed_tx"));
							signers++;
							break;
						}
					} else {
						dlg_error("error in signing at %s happened", cashier_node_ips[i]);
						goto end;
					}
				}
				*/
			}
		}
		if (1 == fully_signed) {
			dlg_info("Final payout tx::%s", cJSON_Print(tx));
		}
	}

end:
	return tx;
}

int32_t bet_process_dispute(cJSON *argjson, struct cashier *cashier_info)
{
	int retval = OK;
	char *hex_data = NULL, *data = NULL;
	cJSON *player_info = NULL, *tx = NULL, *dispute_response = NULL;

	dispute_response = cJSON_CreateObject();
	cJSON_AddStringToObject(dispute_response, "method", "dispute_response");
	cJSON_AddStringToObject(dispute_response, "id", jstr(argjson, "id"));

	hex_data = calloc(tx_data_size * 2, sizeof(char));
	if (hex_data == NULL) {
		dlg_error("Memory allocation failed for hex_data");
		return ERR_MEMORY_ALLOC;
	}

	retval = chips_extract_data(jstr(argjson, "tx_id"), &hex_data);

	if (retval == OK && hex_data != NULL) {
		data = calloc(tx_data_size, sizeof(char));
		if (data == NULL) {
			free(hex_data);
			dlg_error("Memory allocation failed for data");
			return ERR_MEMORY_ALLOC;
		}

		hexstr_to_str(hex_data, data);
		player_info = cJSON_Parse(data);
		if (player_info == NULL) {
			free(hex_data);
			free(data);
			dlg_error("Failed to parse player_info from data");
			return ERR_JSON_PARSING;
		}

		dlg_info("%s", cJSON_Print(player_info));
		cJSON_AddStringToObject(player_info, "tx_id", jstr(argjson, "tx_id"));
		bet_check_cashiers_status();
		tx = bet_reverse_disputed_tx(player_info);
		cJSON_AddItemToObject(dispute_response, "payout_tx", tx);
	}

	// Cleanup
	if (hex_data) free(hex_data);
	if (data) free(data);
	if (player_info) cJSON_Delete(player_info);

	dlg_info("%s", cJSON_Print(dispute_response));
	// Nanomsg removed - no longer used
	retval = OK;
	return retval;
}

static int32_t bet_process_dealer_info(cJSON *argjson)
{
	int32_t retval = OK;
	char *sql_query = NULL;

	const char *ip_str = jstr(argjson, "ip");
	int required_size;

	if (ip_str == NULL) {
		dlg_error("Invalid ip in argjson");
		return ERR_ARGS_NULL;
	}

	required_size = snprintf(NULL, 0, "INSERT OR IGNORE into dealers_info values(\'%s\');", ip_str) + 1;

	if (required_size > sql_query_size) {
		sql_query = calloc(required_size, sizeof(char));
	} else {
		sql_query = calloc(sql_query_size, sizeof(char));
	}
	if (sql_query == NULL) {
		return ERR_MEMORY_ALLOC;
	}

	snprintf(sql_query, required_size, "INSERT OR IGNORE into dealers_info values(\'%s\');", ip_str);
	retval = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);

	return retval;
}

static int32_t bet_process_rqst_dealer_info(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	cJSON *dealer_ips = NULL, *response_info = NULL, *dcv_state_info = NULL, *dcv_state_rqst = NULL,
	      *active_dealers = NULL;

	dealer_ips = cJSON_CreateArray();
	response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(response_info, "method", "rqst_dealer_info_response");
	cJSON_AddStringToObject(response_info, "id", jstr(argjson, "id"));
	dealer_ips = sqlite3_get_dealer_info_details();
	dlg_info("%s", cJSON_Print(dealer_ips));
	active_dealers = cJSON_CreateArray();

	dcv_state_rqst = cJSON_CreateObject();
	cJSON_AddStringToObject(dcv_state_rqst, "method", "dcv_state");
	cJSON_AddStringToObject(dcv_state_rqst, "id", unique_id);

	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ips); i++) {
		dcv_state_info = bet_msg_dealer_with_response_id(
			dcv_state_rqst, unstringify(cJSON_Print(cJSON_GetArrayItem(dealer_ips, i))), "dcv_state");
		dlg_info("dcv_state_info::%s", cJSON_Print(dcv_state_info));
		if (dcv_state_info) {
			cJSON *temp = cJSON_CreateObject();
			cJSON_AddStringToObject(temp, "ip", jstri(dealer_ips, i));
			cJSON_AddNumberToObject(temp, "dcv_state", jint(dcv_state_info, "dcv_state"));
			cJSON_AddItemToArray(active_dealers, temp);
		}
	}

	cJSON_AddItemToObject(response_info, "dealers_info", active_dealers);
	// Nanomsg removed - no longer used
	retval = OK;
	return retval;
}

static int32_t bet_process_find_bvv(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = OK;
	cJSON *bvv_status = NULL, *dcv_state = NULL;

	dlg_info("dealer_ip_for_bvv::%s", dealer_ip_for_bvv);

	if (bvv_state == 1) {
		dcv_state = cJSON_CreateObject();
		cJSON_AddStringToObject(dcv_state, "method", "live");
		cJSON_AddStringToObject(dcv_state, "id", unique_id);
		cJSON *temp = bet_msg_dealer_with_response_id(dcv_state, dealer_ip_for_bvv, "live");
		if (temp == NULL) {
			bvv_state = 0;
			bet_bvv_reset(bet_bvv, bvv_vars);
			memset(dealer_ip_for_bvv, 0x00, sizeof(dealer_ip_for_bvv));
		}
	}

	bvv_status = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_status, "method", "bvv_status");
	cJSON_AddNumberToObject(bvv_status, "bvv_state", bvv_state);
	cJSON_AddStringToObject(bvv_status, "id", jstr(argjson, "id"));
	cJSON_AddStringToObject(bvv_status, "bvv_unique_id", unique_id);
// Nanomsg removed - no longer used
	return retval;
}

static void bet_process_add_bvv(cJSON *argjson, struct cashier *cashier_info)
{
	// Legacy nanomsg/pub-sub BVV flow removed - BVV is now handled through Verus ID updates
	// This method is kept for API compatibility but does nothing
	(void)argjson;
	(void)cashier_info;
	dlg_warn("add_bvv method called but nanomsg/pub-sub flow is no longer used");
}

void bet_cashier_backend_thrd(void *_ptr)
{
	struct cashier *cashier_info = _ptr;
	cJSON *argjson = NULL;
	char *method = NULL;
	int32_t retval = OK;

	argjson = cashier_info->msg;
	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("recv::%s", method);
		if (strcmp(method, "live") == 0) {
			retval = bet_send_status(cashier_info, jstr(argjson, "id"));
		} else if (strcmp(method, "raw_msig_tx") == 0) {
			retval = bet_cashier_process_raw_msig_tx(argjson, cashier_info);
		} else if (strcmp(method, "lock_in_tx") == 0) {
			retval = bet_process_lock_in_tx(argjson, cashier_info);
		} else if (strcmp(method, "payout_tx") == 0) {
			retval = bet_process_payout_tx(argjson, cashier_info);
		} else if (strcmp(method, "game_info") == 0) {
			retval = bet_process_game_info(argjson, cashier_info);
		} else if (strcmp(method, "solve") == 0) {
			bet_process_solve(argjson, cashier_info);
		} else if (strcmp(method, "validate_game_details") == 0) {
			retval = bet_validate_game_details(argjson, cashier_info);
		} else if (strcmp(method, "tx_spent") == 0) {
			retval = bet_update_tx_spent(argjson);
		} else if (strcmp(method, "dispute") == 0) {
			retval = bet_process_dispute(argjson, cashier_info);
		} else if (strcmp(method, "dealer_info") == 0) {
			retval = bet_process_dealer_info(argjson);
		} else if (strcmp(method, "rqst_dealer_info") == 0) {
			retval = bet_process_rqst_dealer_info(argjson, cashier_info);
		} else if (strcmp(method, "find_bvv") == 0) {
			retval = bet_process_find_bvv(argjson, cashier_info);
		} else if (strcmp(method, "add_bvv") == 0) {
			bet_process_add_bvv(argjson, cashier_info);
		} else if (strcmp(method, "game_multisigaddress") == 0) {
			cJSON *msig_info = chips_add_multisig_address_from_list(
				jint(argjson, "threshold_value"), cJSON_GetObjectItem(argjson, "pubkeys"));
			dlg_info("msig_info::%s", cJSON_Print(msig_info));
		}
	}
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
}
int32_t bet_submit_msig_raw_tx(cJSON *tx)
{
	cJSON *msig_raw_tx = NULL;
	int32_t retval = 0;

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", tx);

	// Nanomsg removed - no longer used
	return retval;
}

char *bet_send_message_to_notary(cJSON *argjson, char *notary_node_ip)
{
// Nanomsg removed - no longer used
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	pthread_t cashier_thrd;
	struct cashier *cashier_info = NULL;

	cashier_info = calloc(1, sizeof(struct cashier));

	memset(cashier_info, 0x00, sizeof(struct cashier));
	memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
	memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

	// Nanomsg removed - no longer used
	cashier_info->msg = argjson;

	if (OS_thread_create(&cashier_thrd, NULL, (void *)bet_cashier_status_loop, (void *)cashier_info) != 0) {
		dlg_error("Error in launching cashier");
		exit(-1);
	}

	if (pthread_join(cashier_thrd, NULL)) {
		dlg_error("Error in joining the main thread for cashier");
	}

	return NULL;
}

// Legacy pub-sub functions - stubs (to be replaced with Verus ID communication)
cJSON *bet_msg_cashier_with_response_id(cJSON *argjson, char *cashier_ip, char *method_name)
{
	dlg_warn("bet_msg_cashier_with_response_id: Legacy pub-sub removed - use Verus ID updates instead");
	dlg_warn("  Replace with: get_cJSON_from_id_key() or update_cmm_from_id_key_data_cJSON()");
	(void)argjson; (void)cashier_ip; (void)method_name;
	return NULL;
}

int32_t bet_msg_cashier(cJSON *argjson, char *cashier_ip)
{
	dlg_warn("bet_msg_cashier: Legacy pub-sub removed - use Verus ID updates instead");
	(void)argjson; (void)cashier_ip;
	return -1;
}

int32_t *bet_msg_multiple_cashiers(cJSON *argjson, char **cashier_ips, int no_of_cashiers)
{
	int *sent_status = NULL;
	dlg_warn("bet_msg_multiple_cashiers: Legacy pub-sub removed - use Verus ID updates instead");
	
	sent_status = (int *)malloc(sizeof(int) * no_of_cashiers);
	if (sent_status == NULL) {
		dlg_error("bet_msg_multiple_cashiers: Memory allocation failed");
		return NULL;
	}
	
	// Mark all as failed since function is not functional
	for (int32_t i = 0; i < no_of_cashiers; i++) {
		sent_status[i] = bet_msg_cashier(argjson, cashier_ips[i]);
	}
	
	return sent_status;
}

void bet_resolve_disputed_tx()
{
	cJSON *argjson = NULL;
	cJSON *disputed_games_info = NULL;

	argjson = cJSON_CreateObject();
	cJSON_AddStringToObject(argjson, "method", "solve");

	disputed_games_info = cJSON_CreateObject();
	disputed_games_info = sqlite3_get_game_details(1);
	cJSON_AddItemToObject(argjson, "disputed_games_info", disputed_games_info);

	dlg_info("Disputed games info::%s", cJSON_Print(argjson));

	for (int32_t i = 0; i < cJSON_GetArraySize(disputed_games_info); i++) {
		bet_raise_dispute(unstringify(jstr(cJSON_GetArrayItem(disputed_games_info, i), "tx_id")));
	}
}

void bet_raise_dispute(char *tx_id)
{
	cJSON *dispute_info = NULL;
	cJSON *response_info = NULL;
	char *sql_query = NULL;

	dispute_info = cJSON_CreateObject();
	cJSON_AddStringToObject(dispute_info, "method", "dispute");
	cJSON_AddStringToObject(dispute_info, "tx_id", tx_id);
	cJSON_AddStringToObject(dispute_info, "id", unique_id);

	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			response_info =
				bet_msg_cashier_with_response_id(dispute_info, notary_node_ips[i], "dispute_response");
			if ((response_info) && (jstr(response_info, "payout_tx")))
				break;
		}
	}

	const char *payout_tx_str = NULL;
	const char *tx_id_str = jstr(dispute_info, "tx_id");
	int required_size;

	if ((response_info) && (jstr(response_info, "payout_tx"))) {
		payout_tx_str = jstr(response_info, "payout_tx");
		dlg_info("The tx::%s has been reversed with the payout_tx::%s", tx_id_str, payout_tx_str);
		if (payout_tx_str == NULL || tx_id_str == NULL) {
			dlg_error("Invalid payout_tx or tx_id");
			return;
		}
		required_size = snprintf(NULL, 0,
			"UPDATE player_tx_mapping set status = 0, payout_tx_id = \'%s\' where tx_id = \'%s\';",
			payout_tx_str, tx_id_str) + 1;
	} else {
		dlg_info("Notaries are failed to recover this disputed tx :: %s", tx_id_str);
		payout_tx_str = jstr(response_info, "payout_tx");
		if (payout_tx_str == NULL || tx_id_str == NULL) {
			dlg_error("Invalid payout_tx or tx_id");
			return;
		}
		required_size = snprintf(NULL, 0,
			"UPDATE player_tx_mapping set status = 2, payout_tx_id = \'%s\' where tx_id = \'%s\';",
			payout_tx_str, tx_id_str) + 1;
	}

	if (required_size > sql_query_size) {
		sql_query = calloc(required_size, sizeof(char));
	} else {
		sql_query = calloc(sql_query_size, sizeof(char));
	}
	if (sql_query == NULL) {
		dlg_error("Memory allocation failed for sql_query");
		return;
	}

	if ((response_info) && (jstr(response_info, "payout_tx"))) {
		snprintf(sql_query, required_size,
			"UPDATE player_tx_mapping set status = 0, payout_tx_id = \'%s\' where tx_id = \'%s\';",
			payout_tx_str, tx_id_str);
	} else {
		snprintf(sql_query, required_size,
			"UPDATE player_tx_mapping set status = 2, payout_tx_id = \'%s\' where tx_id = \'%s\';",
			payout_tx_str, tx_id_str);
	}
	bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);
}

// clang-format off
void bet_handle_game(int argc, char **argv)
{
	switchs(argv[2]) {
		cases(

"dispute")
			if (argc == 4) {
				bet_raise_dispute(argv[3]);
			}
			break;
		cases("history")
			cJSON *fail_info = bet_show_fail_history();
			dlg_info(
				"Below hands played unsuccessfully, you can raise dispute using \'.\\bet game dispute tx_id\'::\n %s\n",
				cJSON_Print(fail_info));
			cJSON *success_info = bet_show_success_history();
			dlg_info("Below hands are played successfully::\n%s\n", cJSON_Print(success_info));
			break;	
		cases("info")
			int32_t opt = -1;
			if (argc == 4) {
				if ((strcmp(argv[3], "success") == 0) || (strcmp(argv[3], "0") == 0))
					opt = 0;
				else if ((strcmp(argv[3], "fail") == 0) || (strcmp(argv[3], "1") == 0))
					opt = 1;
			}
			cJSON *info = sqlite3_get_game_details(opt);
			dlg_info("info::%s", cJSON_Print(info));
			break;
		cases("solve")
			bet_resolve_disputed_tx();
			break;
		defaults
			dlg_info("command %s is not handled", argv[2]);
	}switchs_end;
}
// clang-format on

void find_bvv()
{
	cJSON *bvv_rqst_info = NULL;
	cJSON *response_info = NULL;
	cJSON *bvv_info = NULL;
	int32_t bvv_node_permutation[no_of_notaries];

	bet_permutation(bvv_node_permutation, no_of_notaries);

	bvv_rqst_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_rqst_info, "method", "find_bvv");
	cJSON_AddStringToObject(bvv_rqst_info, "id", unique_id);
	dlg_warn("If its stuck here stop the node by pressing CTRL+C and start again");
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[bvv_node_permutation[i]] == 1) {
			response_info = bet_msg_cashier_with_response_id(
				bvv_rqst_info, notary_node_ips[bvv_node_permutation[i]], "bvv_status");
			if ((response_info) && (jint(response_info, "bvv_state") == 0)) {
				bvv_info = cJSON_CreateObject();
				cJSON_AddStringToObject(bvv_info, "method", "add_bvv");
				cJSON_AddStringToObject(bvv_info, "dealer_ip", dealer_ip);
				bet_msg_cashier(bvv_info, notary_node_ips[bvv_node_permutation[i]]);
				const char *bvv_id_str = jstr(response_info, "bvv_unique_id");
				if (bvv_id_str != NULL) {
					strncpy(bvv_unique_id, bvv_id_str, sizeof(bvv_unique_id) - 1);
					bvv_unique_id[sizeof(bvv_unique_id) - 1] = '\0';
				} else {
					bvv_unique_id[0] = '\0';
				}
				dlg_info("BVV node IP is ::%s, its unique id is :: %s",
					 notary_node_ips[bvv_node_permutation[i]], bvv_unique_id);
				break;
			}
		}
	}
}

int32_t bet_clear_tables()
{
	int32_t retval = -1;
	char *sql_query = NULL;

	// This query is fixed size, safe to use sql_query_size
	sql_query = calloc(sql_query_size, sizeof(char));
	if (sql_query == NULL) {
		return ERR_MEMORY_ALLOC;
	}
	snprintf(sql_query, sql_query_size, "DELETE from dealers_info");
	retval = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);

	return retval;
}

/***************************************************************
Cashier frontend loop for GUI communication
Handles wallet operations common to all node types
****************************************************************/

static char lws_cashier_buf[4096];
static int lws_cashier_buf_length = 0;
static struct lws *wsi_global_cashier = NULL;
static int ws_cashier_connection_status = 0;

/**
 * Cashier frontend handler - handles wallet operations using common wallet functions
 */
int32_t bet_cashier_frontend(struct lws *wsi, cJSON *argjson)
{
	int32_t retval = OK;
	char *method = NULL;
	cJSON *response = NULL;

	if (argjson == NULL) {
		return ERR_ARGS_NULL;
	}

	method = jstr(argjson, "method");
	if (method == NULL) {
		dlg_warn("Cashier frontend: No method specified");
		return ERR_ARGS_NULL;
	}

	dlg_info("Cashier frontend method::%s", method);

	// Handle common wallet operations
	if (strcmp(method, "get_bal_info") == 0) {
		response = bet_wallet_get_bal_info();
		if (response != NULL) {
			char *response_str = cJSON_Print(response);
			lws_write(wsi, (unsigned char *)response_str, strlen(response_str), 0);
			free(response_str);
			cJSON_Delete(response);
		}
	} else if (strcmp(method, "get_addr_info") == 0) {
		response = bet_wallet_get_addr_info();
		if (response != NULL) {
			char *response_str = cJSON_Print(response);
			lws_write(wsi, (unsigned char *)response_str, strlen(response_str), 0);
			free(response_str);
			cJSON_Delete(response);
		}
	} else if (strcmp(method, "withdrawRequest") == 0) {
		response = bet_wallet_withdraw_request();
		if (response != NULL) {
			char *response_str = cJSON_Print(response);
			lws_write(wsi, (unsigned char *)response_str, strlen(response_str), 0);
			free(response_str);
			cJSON_Delete(response);
		}
	} else if (strcmp(method, "withdraw") == 0) {
		response = bet_wallet_withdraw(argjson);
		if (response != NULL) {
			char *response_str = cJSON_Print(response);
			lws_write(wsi, (unsigned char *)response_str, strlen(response_str), 0);
			free(response_str);
			cJSON_Delete(response);
		}
	} else {
		dlg_warn("Cashier frontend: Unknown method::%s", method);
		retval = ERR_JSON_PARSING;
	}

	return retval;
}

/**
 * WebSocket callback for cashier frontend
 */
int lws_callback_http_cashier(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_cashier_buf + lws_cashier_buf_length, in, len);
		lws_cashier_buf_length += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_cashier_buf);
		if (bet_cashier_frontend(wsi, argjson) != OK) {
			dlg_warn("Failed to process cashier frontend command");
		}
		if (argjson != NULL) {
			cJSON_Delete(argjson);
		}
		memset(lws_cashier_buf, 0x00, sizeof(lws_cashier_buf));
		lws_cashier_buf_length = 0;
		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_cashier = wsi;
		dlg_info("Cashier WebSocket connection established");
		ws_cashier_connection_status = 1;
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		// Handle writeable events if needed
		break;
	default:
		break;
	}
	return 0;
}

static struct lws_protocols cashier_protocols[] = {
	{ "http", lws_callback_http_cashier, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static const struct lws_http_mount cashier_mount = {
	/* .mount_next */ NULL,
	/* .mountpoint */ "/",
	/* .origin */ "./mount-origin",
	/* .def */ "index.html",
	/* .protocol */ NULL,
	/* .cgienv */ NULL,
	/* .extra_mimetypes */ NULL,
	/* .interpret */ NULL,
	/* .cgi_timeout */ 0,
	/* .cache_max_age */ 0,
	/* .auth_mask */ 0,
	/* .cache_reusable */ 0,
	/* .cache_revalidate */ 0,
	/* .cache_intermediaries */ 0,
	/* .origin_protocol */ LWSMPRO_FILE,
	/* .mountpoint_len */ 1,
	/* .basic_auth_login_file */ NULL,
};

static int cashier_interrupted = 0;

/**
 * Cashier frontend loop - WebSocket server for cashier GUI
 */
void bet_cashier_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info cashier_info;
	struct lws_context *cashier_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	(void)_ptr; /* unused parameter */

	lws_set_log_level(logs, NULL);
	dlg_info("[GUI] Cashier WebSocket server starting on port %d | visit http://localhost:%d", gui_ws_port, gui_ws_port);
	lwsl_user("[GUI] Cashier WebSocket server | visit http://localhost:%d", gui_ws_port);

	memset(&cashier_info, 0, sizeof cashier_info);
	cashier_info.port = gui_ws_port;
	cashier_info.mounts = &cashier_mount;
	cashier_info.protocols = cashier_protocols;
	cashier_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
	cashier_info.uid = -1;  /* Don't drop privileges */
	cashier_info.gid = -1;  /* Don't drop privileges */

	if (cashier_info.port <= 0 || cashier_info.port > 65535) {
		dlg_error("[GUI] Invalid port %d, using default %d", cashier_info.port, DEFAULT_CASHIER_WS_PORT);
		cashier_info.port = DEFAULT_CASHIER_WS_PORT;
	}

	cashier_context = lws_create_context(&cashier_info);
	if (!cashier_context) {
		lwsl_err("[GUI] Cashier lws init failed");
		dlg_error("[GUI] Cashier lws_context error");
		return;
	}

	dlg_info("[GUI] Cashier WebSocket server started successfully");
	
	// Keep running even if main thread exits
	while (n >= 0) {
		n = lws_service(cashier_context, 1000);
	}
	
	dlg_info("[GUI] Cashier WebSocket server shutting down");

	if (cashier_context) {
		lws_context_destroy(cashier_context);
	}
}
