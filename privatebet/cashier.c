#include "bet.h"
#include "client.h"
#include "cashier.h"
#include "network.h"
#include "common.h"
#include "commands.h"
#include "storage.h"
#include "misc.h"
#include "cards777.h"

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

struct cashier *cashier_info = NULL;
int32_t live_notaries = 0;
int32_t *notary_status = NULL;
int32_t notary_response = 0;

/***********************************************************************************************************
The values table_stack_in_chips and  chips_tx_fee are read from dealer_config.json file, table_stack_in_chips
is the amount of chips that are needed by the player in order to play the game, chips_tx_fee is chips n/w 
tx_fee set by the dealer.
Incase if these values are not mentioned in the dealer_config.json file, the default values read as
	table_stack_in_chips = 0.01;
	chips_tx_fee = 0.0005;
***********************************************************************************************************/

double table_stack_in_chips = 0.01;
double chips_tx_fee = 0.0005;

char *legacy_m_of_n_msig_addr = NULL;

int32_t bvv_state = 0;
char dealer_ip_for_bvv[128];

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
		legacy_m_of_n_msig_addr = (char *)malloc(strlen(jstr(msig_addr, "address")) + 1);
		memset(legacy_m_of_n_msig_addr, 0x00, strlen(jstr(msig_addr, "address")) + 1);
		strncpy(legacy_m_of_n_msig_addr, jstr(msig_addr, "address"), strlen(jstr(msig_addr, "address")));
		if (chips_iswatchonly(legacy_m_of_n_msig_addr) == 0) {
			dlg_info("Importing msig_address ::%s, it takes a while", legacy_m_of_n_msig_addr);
			chips_import_address(legacy_m_of_n_msig_addr);
		}
	}
}

void bet_check_cashier_nodes()
{
	bet_check_cashiers_status();

	if (live_notaries <= threshold_value) {
		dlg_warn("The cashier nodes available are :: %d  whic is less than the required number of cashier nodes i.e ::%d", live_notaries,threshold_value);
		exit(0);
	} else {
		dlg_info("Notary node status");
		for (int i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1)
				dlg_info("%d. %s active", i + 1, notary_node_ips[i]);
			else
				dlg_info("%d. %s not active", i + 1, notary_node_ips[i]);
		}
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
		}
	}
}

int32_t bet_send_status(struct cashier *cashier_info, char *id)
{
	int32_t retval = 1, bytes;
	cJSON *live_info = NULL;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	cJSON_AddStringToObject(live_info, "id", id);
	dlg_info("Sending::%s", cJSON_Print(live_info));
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
	if (bytes < 0) {
		dlg_info("Error occured in sending::%s", cJSON_Print(live_info));
		retval = -1;
	}

	return retval;
}

int32_t bet_process_lock_in_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t rc, retval = 1, bytes;
	cJSON *status = NULL;

	rc = bet_run_query(jstr(argjson, "sql_query"));
	status = cJSON_CreateObject();
	cJSON_AddStringToObject(status, "method", "query_status");
	cJSON_AddNumberToObject(status, "status", rc);
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(status), strlen(cJSON_Print(status)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_cashier_process_raw_msig_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = 1, bytes;
	cJSON *signed_tx = NULL;
	char *tx = NULL;

	signed_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(signed_tx, "method", "signed_tx");
	cJSON_AddStringToObject(signed_tx, "id", jstr(argjson, "id"));
	tx = cJSON_Print(cJSON_GetObjectItem(argjson, "tx"));
	cJSON_AddItemToObject(signed_tx, "signed_tx", chips_sign_raw_tx_with_wallet(tx));
	dlg_info("signed_tx::%s", cJSON_Print(signed_tx));
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(signed_tx), strlen(cJSON_Print(signed_tx)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_process_payout_tx(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t retval = -1;
	char *sql_query = NULL;

	sql_query = calloc(1, 400);
	sprintf(sql_query,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = %s where table_id = \"%s\";",
		cJSON_Print(cJSON_GetObjectItem(argjson, "tx_info")), jstr(argjson, "table_id"));
	dlg_info("sql_query::%s", sql_query);
	retval = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);

	return retval;
}

int32_t bet_process_game_info(cJSON *argjson, struct cashier *cashier_info)
{
	char *sql_query = NULL;
	cJSON *game_state = NULL;
	int32_t rc;

	sql_query = calloc(1, 2000);

	dlg_info("%s", cJSON_Print(argjson));
	game_state = cJSON_GetObjectItem(argjson, "game_state");
	sprintf(sql_query, "INSERT into cashier_game_state values(\"%s\", \'%s\');", jstr(argjson, "table_id"),
		cJSON_Print(game_state));
	dlg_info("sql_query::%s", sql_query);
	rc = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);

	return rc;
}

cJSON *bet_resolve_game_dispute(cJSON *game_info)
{
	cJSON *msig_addr_nodes = NULL;
	int min_cashiers, active_cashiers = 0;
	int *cashier_node_status = NULL;
	int *validation_status = NULL;
	char **cashier_node_ips = NULL;
	int32_t no_of_cashier_nodes, approved_cashiers_count = 0;

	msig_addr_nodes = cJSON_CreateArray();
	msig_addr_nodes = cJSON_Parse(jstr(game_info, "msig_addr_nodes"));

	no_of_cashier_nodes = cJSON_GetArraySize(msig_addr_nodes);
	cashier_node_status = (int *)malloc(no_of_cashier_nodes * sizeof(int));
	validation_status = (int *)malloc(no_of_cashier_nodes * sizeof(int));
	cashier_node_ips = (char **)malloc(no_of_cashier_nodes * sizeof(int *));

	min_cashiers = jint(game_info, "min_cashiers");

	for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
		cashier_node_status[i] = 0;
		cashier_node_ips[i] = calloc(1, 20);
		strcpy(cashier_node_ips[i], unstringify(cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i))));
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

		strcpy(tx_ids[0], jstr(game_info, "tx_id"));
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
			sql_query = calloc(1, sql_query_size);
			sprintf(sql_query, "UPDATE player_tx_mapping set status = 0 where tx_id = \'%s\';",
				jstr(game_info, "tx_id"));
			bet_run_query(sql_query);
			if (sql_query)
				free(sql_query);
			free_json(update_tx_info);
		}
	}

end:
	return NULL;
}

int32_t bet_process_solve(cJSON *argjson, struct cashier *cashier_info)
{
	int rc;
	cJSON *disputed_games_info = NULL;

	disputed_games_info = cJSON_CreateArray();
	disputed_games_info = cJSON_GetObjectItem(argjson, "disputed_games_info");

	bet_check_cashiers_status();
	for (int32_t i = 0; i < cJSON_GetArraySize(disputed_games_info); i++) {
		bet_resolve_game_dispute(cJSON_GetArrayItem(disputed_games_info, i));
	}
	return rc;
}

int32_t bet_validate_game_details(cJSON *argjson, struct cashier *cashier_info)
{
	int rc, bytes;
	cJSON *response_info = NULL;
	int value = 1;
	response_info = cJSON_CreateObject();
	/*
	Need to implement the validation logic here - sg777
	*/
	cJSON_AddStringToObject(response_info, "method", "game_validation_status");
	cJSON_AddStringToObject(response_info, "id", jstr(argjson, "id"));
	cJSON_AddNumberToObject(response_info, "status", value);
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(response_info), strlen(cJSON_Print(response_info)), 0);

	if (bytes < 0)
		rc = -1;
	return rc;
}

void bet_cashier_status_loop(void *_ptr)
{
	int32_t recvlen = 0, bytes;
	void *ptr = NULL;
	cJSON *argjson = NULL;
	struct cashier *cashier_info = _ptr;

	bytes = nn_send(cashier_info->c_pushsock, cJSON_Print(cashier_info->msg),
			strlen(cJSON_Print(cashier_info->msg)), 0);

	if (bytes < 0)
		dlg_error("Failed to send data");
	else {
		while (cashier_info->c_pushsock >= 0 && cashier_info->c_subsock >= 0) {
			ptr = 0;
			if ((recvlen = nn_recv(cashier_info->c_subsock, &ptr, NN_MSG, 0)) > 0) {
				char *tmp = clonestr(ptr);
				if ((argjson = cJSON_Parse(tmp)) != 0) {
					if (strcmp(jstr(argjson, "id"), unique_id) == 0) {
						if (strcmp(jstr(argjson, "method"), "live") == 0)
							live_notaries++;
						break;
					}
					free_json(argjson);
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
}

static int32_t bet_update_tx_spent(cJSON *argjson)
{
	char *sql_query = NULL;
	int rc;

	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query,
		"UPDATE c_tx_addr_mapping set payin_tx_id_status = 0, payout_tx_id = \'%s\' where payin_tx_id_status =\'%s\';",
		jstr(argjson, "payout_tx_id"), jstr(argjson, "payin_tx_id"));
	rc = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);
	return rc;
}

static cJSON *bet_reverse_disputed_tx(cJSON *game_info)
{
	cJSON *msig_addr_nodes = NULL;
	int min_cashiers, active_cashiers = 0;
	int *cashier_node_status = NULL;
	char **cashier_node_ips = NULL;
	int32_t no_of_cashier_nodes;
	cJSON *tx = NULL;

	msig_addr_nodes = cJSON_CreateArray();
	msig_addr_nodes = cJSON_Parse(jstr(game_info, "msig_addr_nodes"));

	no_of_cashier_nodes = cJSON_GetArraySize(msig_addr_nodes);
	cashier_node_status = (int *)malloc(no_of_cashier_nodes * sizeof(int));
	cashier_node_ips = (char **)malloc(no_of_cashier_nodes * sizeof(int *));

	min_cashiers = jint(game_info, "min_cashiers");

	for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
		cashier_node_status[i] = 0;
		cashier_node_ips[i] = calloc(1, 20);
		strcpy(cashier_node_ips[i], unstringify(cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i))));
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
		cJSON *hex = NULL;

		strcpy(tx_ids[0], jstr(game_info, "tx_id"));
		if (chips_iswatchonly(jstr(game_info, "msig_addr")) == 0) {
			dlg_info("Importing the msigaddress::%s", jstr(game_info, "msig_addr"));
			chips_import_address(jstr(game_info, "msig_addr"));
		}
		raw_tx = chips_create_tx_from_tx_list(unstringify(jstr(game_info, "dispute_addr")), no_of_in_txs,
						      tx_ids);
		if (raw_tx == NULL)
			return NULL;
		dlg_info("raw_tx::%s", cJSON_Print(raw_tx));
		for (int i = 0; i < no_of_cashier_nodes; i++) {
			if (cashier_node_status[i] == 1) {
				if (signers == 0) {
					cJSON *temp = chips_sign_msig_tx(cashier_node_ips[i], raw_tx);
					if (temp == NULL)
						continue;
					dlg_info("signed_tx::%s", cJSON_Print(temp));
					if (cJSON_GetObjectItem(temp, "signed_tx") != NULL) {
						hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"),
									  "hex");
						signers++;
					} else {
						dlg_error("error in signing at %s happened", cashier_node_ips[i]);
						goto end;
					}
				} else if (signers == 1) {
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
			}
		}
		if (tx) {
			dlg_info("Final payout tx::%s", cJSON_Print(tx));
		}
	}

end:
	return tx;
}

int32_t bet_process_dispute(cJSON *argjson, struct cashier *cashier_info)
{
	int rc = 1, bytes;
	char *hex_data = NULL, *data = NULL;
	cJSON *player_info = NULL, *tx = NULL;
	cJSON *dispute_response = NULL;

	dispute_response = cJSON_CreateObject();
	cJSON_AddStringToObject(dispute_response, "method", "dispute_response");
	cJSON_AddStringToObject(dispute_response, "id", jstr(argjson, "id"));

	hex_data = calloc(1, tx_data_size * 2);
	rc = chips_extract_data(jstr(argjson, "tx_id"), &hex_data);

	if (rc == 1) {
		data = calloc(1, tx_data_size);
		hexstr_to_str(hex_data, data);
		player_info = cJSON_CreateObject();
		player_info = cJSON_Parse(data);

		dlg_info("%s", cJSON_Print(player_info));
		cJSON_AddStringToObject(player_info, "tx_id", jstr(argjson, "tx_id"));
		bet_check_cashiers_status();
		tx = bet_reverse_disputed_tx(player_info);
		cJSON_AddItemToObject(dispute_response, "payout_tx", tx);
	}
	dlg_info("%s", cJSON_Print(dispute_response));
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(dispute_response), strlen(cJSON_Print(dispute_response)),
			0);

	if (bytes < 0)
		rc = -1;
	return rc;
}

static int32_t bet_process_dealer_info(cJSON *argjson)
{
	char *sql_query = NULL;
	int rc, retval = 0;

	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query, "INSERT into dealers_info values(\'%s\');", jstr(argjson, "ip"));
	rc = bet_run_query(sql_query);
	if((rc == SQLITE_OK) || (rc == SQLITE_CONSTRAINT)) {
		retval = 1;
	}
	if (sql_query)
		free(sql_query);
	
	return retval;
}

static int32_t bet_check_dealer_status(char *dealer_ip)
{
	cJSON *live_info = NULL;
	cJSON *response = NULL;
	int32_t rc = 0;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	cJSON_AddStringToObject(live_info, "id", unique_id);
	response = bet_msg_dealer_with_response_id(live_info, dealer_ip, "live");

	if (response) {
		rc = 1;
	} else {
		char *sql_query = NULL;
		sql_query = calloc(1, sql_query_size);
		sprintf(sql_query, "DELETE FROM dealers_info WHERE dealer_ip = \'%s\';", dealer_ip);
		bet_run_query(sql_query);
		if (sql_query)
			free(sql_query);
	}
	return rc;
}
static int32_t bet_process_rqst_dealer_info(cJSON *argjson, struct cashier *cashier_info)
{
	cJSON *response_info = NULL;
	int32_t bytes, rc = 0;
	cJSON *dealer_ips = NULL;
	cJSON *dcv_state_info = NULL;
	cJSON *dcv_state_rqst = NULL;
	cJSON *active_dealers = NULL;

	dealer_ips = cJSON_CreateArray();
	response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(response_info, "method", "rqst_dealer_info_response");
	cJSON_AddStringToObject(response_info, "id", jstr(argjson, "id"));
	dealer_ips = sqlite3_get_dealer_info_details();

	active_dealers = cJSON_CreateArray();

	dcv_state_rqst = cJSON_CreateObject();
	cJSON_AddStringToObject(dcv_state_rqst, "method", "dcv_state");
	cJSON_AddStringToObject(dcv_state_rqst, "id", unique_id);

	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ips); i++) {
		dcv_state_info = bet_msg_dealer_with_response_id(
			dcv_state_rqst, unstringify(cJSON_Print(cJSON_GetArrayItem(dealer_ips, i))), "dcv_state");
		if ((dcv_state_info) && (jint(dcv_state_info, "dcv_state") == 0)) {
			cJSON *temp = cJSON_CreateObject();
			temp = cJSON_CreateString(unstringify(cJSON_Print(cJSON_GetArrayItem(dealer_ips, i))));
			cJSON_AddItemToArray(active_dealers, temp);
		} else {
			rc = sqlite3_delete_dealer(unstringify(cJSON_Print(cJSON_GetArrayItem(dealer_ips, i))));
		}
	}

	cJSON_AddItemToObject(response_info, "dealer_ips", active_dealers);
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(response_info), strlen(cJSON_Print(response_info)), 0);
	if (bytes < 0) {
		dlg_error("There is a problem in sending the %s", cJSON_Print(response_info));
		rc = -1;
	}
	return rc;
}

static int32_t bet_process_find_bvv(cJSON *argjson, struct cashier *cashier_info)
{
	int32_t bytes, rc;
	cJSON *bvv_status = NULL;
	cJSON *dcv_state = 0;

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
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(bvv_status), strlen(cJSON_Print(bvv_status)), 0);
	if (bytes < 0)
		rc = -1;
	return rc;
}

static void bet_process_add_bvv(cJSON *argjson, struct cashier *cashier_info)
{
	uint16_t port = 7797;

	if (bvv_state == 0) {
		bvv_state = 1;
		bet_bvv_thrd(jstr(argjson, "dealer_ip"), port);
		memset(dealer_ip_for_bvv, 0x00, sizeof(dealer_ip_for_bvv));
		strcpy(dealer_ip_for_bvv, jstr(argjson, "dealer_ip"));
	}
}

void bet_cashier_backend_thrd(void *_ptr)
{
	struct cashier *cashier_info = _ptr;
	cJSON *argjson = NULL;
	char *method = NULL;
	int32_t retval = 1;

	argjson = cashier_info->msg;
	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("receiving::%s", method);
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
			retval = bet_process_solve(argjson, cashier_info);
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
}
void bet_cashier_server_loop(void *_ptr)
{
	int32_t recvlen = 0;
	void *ptr = NULL;
	cJSON *msgjson = NULL;
	struct cashier *cashier_info = _ptr;
	uint8_t flag = 1;

	dlg_info("cashier server node started");
	while (flag) {
		if (cashier_info->c_pubsock >= 0 && cashier_info->c_pullsock >= 0) {
			ptr = 0;
			char *tmp = NULL;
			recvlen = nn_recv(cashier_info->c_pullsock, &ptr, NN_MSG, 0);
			if (recvlen > 0) {
				tmp = clonestr(ptr);
			}
			if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
				pthread_t server_thrd;
				cashier_info->msg = msgjson;
				if (OS_thread_create(&server_thrd, NULL, (void *)bet_cashier_backend_thrd,
						     (void *)cashier_info) != 0) {
					dlg_error("Error in launching the bet_cashier_backend_thrd");
					exit(-1);
				}
				/*
				if (pthread_join(server_thrd, NULL)) {
					dlg_error("Error in joining the main thread for bet_cashier_backend_thrd");
				}
				*/
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
}

int32_t bet_submit_msig_raw_tx(cJSON *tx)
{
	cJSON *msig_raw_tx = NULL;
	int32_t bytes, retval = 0;

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", tx);

	if (cashier_info->c_pushsock > 0) {
		bytes = nn_send(cashier_info->c_pushsock, cJSON_Print(msig_raw_tx), strlen(cJSON_Print(msig_raw_tx)),
				0);
		if (bytes < 0)
			retval = -1;
	}
	return retval;
}

char *bet_send_message_to_notary(cJSON *argjson, char *notary_node_ip)
{
	int32_t c_subsock, c_pushsock;
	uint16_t cashier_pubsub_port = 7901, cashier_pushpull_port = 7902;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	pthread_t cashier_thrd;
	struct cashier *cashier_info = NULL;

	cashier_info = calloc(1, sizeof(struct cashier));

	memset(cashier_info, 0x00, sizeof(struct cashier));
	memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
	memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

	bet_tcp_sock_address(0, bind_sub_addr, notary_node_ip, cashier_pubsub_port);
	c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

	bet_tcp_sock_address(0, bind_push_addr, notary_node_ip, cashier_pushpull_port);
	c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

	cashier_info->c_subsock = c_subsock;
	cashier_info->c_pushsock = c_pushsock;
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

cJSON *bet_msg_cashier_with_response_id(cJSON *argjson, char *cashier_ip, char *method_name)
{
	int32_t c_subsock, c_pushsock, bytes, recvlen;
	uint16_t cashier_pubsub_port = 7901, cashier_pushpull_port = 7902;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	void *ptr;
	cJSON *response_info = NULL;

	memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
	memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

	bet_tcp_sock_address(0, bind_sub_addr, cashier_ip, cashier_pubsub_port);
	c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

	bet_tcp_sock_address(0, bind_push_addr, cashier_ip, cashier_pushpull_port);
	c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

	bytes = nn_send(c_pushsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
	if (bytes < 0) {
		return NULL;
	} else {
		while (c_pushsock >= 0 && c_subsock >= 0) {
			ptr = 0;
			if ((recvlen = nn_recv(c_subsock, &ptr, NN_MSG, 0)) > 0) {
				char *tmp = clonestr(ptr);
				if ((response_info = cJSON_Parse(tmp)) != 0) {
					if ((strcmp(jstr(response_info, "method"), method_name) == 0) &&
					    (strcmp(jstr(response_info, "id"), unique_id) == 0)) {
						break;
					}
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}

	nn_close(c_pushsock);
	nn_close(c_subsock);

	return response_info;
}

int32_t bet_msg_cashier(cJSON *argjson, char *cashier_ip)
{
	int32_t c_pushsock, bytes, retval = 1;
	uint16_t cashier_pushpull_port = 7902;
	char bind_push_addr[128] = { 0 };

	memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

	bet_tcp_sock_address(0, bind_push_addr, cashier_ip, cashier_pushpull_port);
	c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

	bytes = nn_send(c_pushsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
	if (bytes < 0) {
		retval = -1;
	}

	nn_close(c_pushsock);

	return retval;
}

int32_t *bet_msg_multiple_cashiers(cJSON *argjson, char **cashier_ips, int no_of_cashiers)
{
	int *sent_status = NULL;

	sent_status = (int *)malloc(sizeof(int) * no_of_cashiers);
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

	dlg_info("Dispute info::%s", cJSON_Print(dispute_info));
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			response_info =
				bet_msg_cashier_with_response_id(dispute_info, notary_node_ips[i], "dispute_response");
			dlg_info("Dispute Response Info::%s", cJSON_Print(response_info));
			if (response_info)
				break;
		}
	}

	dlg_info("Response info::%s", cJSON_Print(response_info));
	if ((response_info) && (jstr(response_info, "payout_tx"))) {
		sql_query = calloc(1, sql_query_size);
		sprintf(sql_query,
			"UPDATE player_tx_mapping set status = 0, payout_tx_id = \'%s\' where tx_id = \'%s\';",
			(jstr(response_info, "payout_tx")), tx_id);
		bet_run_query(sql_query);
		if (sql_query)
			free(sql_query);
	}
}

void bet_handle_game(int argc, char **argv)
{
	if (argc > 2) {
		if (strcmp(argv[2], "info") == 0) {
			int32_t opt = -1;
			if (argc == 4) {
				if ((strcmp(argv[3], "success") == 0) || (strcmp(argv[3], "0") == 0))
					opt = 0;
				else if ((strcmp(argv[3], "fail") == 0) || (strcmp(argv[3], "1") == 0))
					opt = 1;
			}
			cJSON *info = sqlite3_get_game_details(opt);
			dlg_info("info::%s", cJSON_Print(info));
		} else if (strcmp(argv[2], "solve") == 0) {
			bet_resolve_disputed_tx();
		} else if (strcmp(argv[2], "dispute") == 0) {
			if (argc == 4) {
				bet_raise_dispute(argv[3]);
			}
		} else if (strcmp(argv[2], "history") == 0) {
			cJSON *fail_info = bet_show_fail_history();
			dlg_info(
				"Below hands played unsuccessfully, you can raise dispute using \'.\\bet game dispute tx_id\'::\n %s\n",
				cJSON_Print(fail_info));
			cJSON *success_info = bet_show_success_history();
			dlg_info("Below hands are played successfully::\n%s\n", cJSON_Print(success_info));
		}
	}
}

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
				dlg_info("BVV node chosen for this hand is ::%s",
					 notary_node_ips[bvv_node_permutation[i]]);
				break;
			}
		}
	}
}
