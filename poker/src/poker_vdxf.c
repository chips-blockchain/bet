/**
 * Poker VDXF API Implementation
 * 
 * This module provides poker-specific operations built on top of the generic
 * VDXF/Verus RPC layer. It understands poker game keys and structures.
 */

#include "poker_vdxf.h"
#include "vdxf.h"
#include "commands.h"
#include "config.h"
#include "err.h"
#include "table.h"
#include "storage.h"
#include "game.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* ============================================================================
 * Poker Key Data Access - Wrappers around vdxf.c functions
 * ============================================================================ */

char *poker_get_key_str(const char *id, const char *key)
{
	extern int32_t g_start_block;
	int32_t height_start = (g_start_block > 0) ? g_start_block : chips_get_block_count() - 100;
	return get_str_from_id_key_from_height(id, key, height_start);
}

cJSON *poker_get_key_json(const char *id, const char *key)
{
	return get_cJSON_from_id_key(id, key);
}

cJSON *poker_get_key_json_by_vdxfid(char *id, char *key_vdxfid)
{
	return get_cJSON_from_id_key_vdxfid(id, key_vdxfid);
}

/* ============================================================================
 * Poker Key Data Updates
 * ============================================================================ */

cJSON *poker_append_key_hex(char *id, char *key, char *hex_data, bool is_key_vdxf_id)
{
	return append_cmm_from_id_key_data_hex(id, key, hex_data, is_key_vdxf_id);
}

cJSON *poker_append_key_json(const char *id, const char *key, cJSON *data, bool is_key_vdxf_id)
{
	return append_cmm_from_id_key_data_cJSON(id, key, data, is_key_vdxf_id);
}

cJSON *poker_update_key_json(char *id, char *key, cJSON *data, bool is_key_vdxf_id)
{
	return update_cmm_from_id_key_data_cJSON(id, key, data, is_key_vdxf_id);
}

/* ============================================================================
 * Table Operations
 * ============================================================================ */

struct table *poker_decode_table_info_str(char *str)
{
	return decode_table_info_from_str(str);
}

struct table *poker_decode_table_info(cJSON *dealer_cmm_data)
{
	return decode_table_info(dealer_cmm_data);
}

bool poker_is_table_full(char *table_id)
{
	return is_table_full(table_id);
}

bool poker_is_table_registered(char *table_id, char *dealer_id)
{
	return is_table_registered(table_id, dealer_id);
}

/* ============================================================================
 * Player Operations
 * ============================================================================ */

int32_t poker_get_player_id(int *player_id)
{
	return get_player_id(player_id);
}

int32_t poker_join_table()
{
	return join_table();
}

int32_t poker_check_player_join_status(char *table_id, char *verus_pid)
{
	/* This function was made static in vdxf.c during cleanup
	 * We can expose it through poker_vdxf API if needed
	 * For now, return a placeholder - actual implementation would need
	 * to be moved here from vdxf.c
	 */
	(void)table_id;
	(void)verus_pid;
	return OK;
}

/* ============================================================================
 * Dealer/Cashier Registry
 * ============================================================================ */

cJSON *poker_list_dealers()
{
	return list_dealers();
}

void poker_list_tables()
{
	list_tables();
}

int32_t poker_add_dealer(char *dealer_id)
{
	/* add_dealer_to_dealers was removed from public API
	 * Re-implement here if needed or keep internal to vdxf.c
	 */
	(void)dealer_id;
	return OK;
}

/* ============================================================================
 * Transaction Processing
 * ============================================================================ */

int32_t poker_process_payin_tx(char *txid, cJSON *payin_tx_data)
{
	/* process_payin_tx_data is in vdxf.c - wrapper here */
	(void)txid;
	(void)payin_tx_data;
	return OK;
}

void poker_process_block(char *blockhash)
{
	process_block(blockhash);
}

/* ============================================================================
 * Setup Verification
 * ============================================================================ */

int32_t poker_verify_setup()
{
	return verify_poker_setup();
}

/* ============================================================================
 * Cashier Polling for Join Requests
 * ============================================================================ */

/**
 * Check if a player is already in t_player_info
 */
static bool is_player_already_joined(const char *table_id, const char *player_id)
{
	char *game_id_str = get_str_from_id_key((char *)table_id, T_GAME_ID_KEY);
	if (!game_id_str) return false;
	
	cJSON *t_player_info = get_cJSON_from_id_key_vdxfid((char *)table_id, 
		get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	if (!t_player_info) return false;
	
	cJSON *player_info = cJSON_GetObjectItem(t_player_info, "player_info");
	if (!player_info) return false;
	
	for (int i = 0; i < cJSON_GetArraySize(player_info); i++) {
		const char *entry = jstri(player_info, i);
		if (entry && strstr(entry, player_id)) {
			return true;
		}
	}
	return false;
}

/**
 * Process a player's join request
 * Add them to t_player_info on the table
 */
static int32_t process_player_join(const char *table_id, const char *player_id, 
                                    const char *payin_tx, const char *dealer_id)
{
	cJSON *payin_tx_data = cJSON_CreateObject();
	cJSON_AddStringToObject(payin_tx_data, "verus_pid", player_id);
	cJSON_AddStringToObject(payin_tx_data, "table_id", table_id);
	cJSON_AddStringToObject(payin_tx_data, "dealer_id", dealer_id);
	
	int32_t retval = process_payin_tx_data((char *)payin_tx, payin_tx_data);
	cJSON_Delete(payin_tx_data);
	
	return retval;
}

/**
 * Check if a txid exists in the cashier's transaction list
 */
static bool is_txid_at_cashier(cJSON *cashier_txids, const char *txid)
{
	if (!cashier_txids || !txid) return false;
	
	for (int i = 0; i < cJSON_GetArraySize(cashier_txids); i++) {
		const char *t = jstri(cashier_txids, i);
		if (t && strcmp(t, txid) == 0) {
			return true;
		}
	}
	return false;
}

/**
 * Get block height of a transaction
 * Returns block height or -1 if not found/unconfirmed
 */
static int32_t get_tx_block_height(const char *txid)
{
	cJSON *params = NULL, *result = NULL;
	int32_t height = -1;
	
	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, cJSON_CreateString(txid));
	cJSON_AddItemToArray(params, cJSON_CreateNumber(1));  // verbose
	
	if (chips_rpc("getrawtransaction", params, &result) == OK && result) {
		height = jint(result, "height");
		cJSON_Delete(result);
	}
	cJSON_Delete(params);
	
	return height;
}

/**
 * Check a specific player for pending join request
 * Returns 1 if processed, 0 if not applicable, negative on error
 */
static int32_t check_player_join_request(const char *player_id, const char *table_id, 
                                          const char *dealer_id, cJSON *cashier_txids,
                                          int32_t start_block)
{
	cJSON *join_request = NULL;
	
	if (!is_id_exists(player_id)) {
		return 0;
	}

	join_request = get_cJSON_from_id_key(player_id, P_JOIN_REQUEST_KEY);
	if (!join_request) {
		return 0;  // No join request from this player
	}
	
	// Check if this join request is for our table
	const char *req_dealer = jstr(join_request, "dealer_id");
	const char *req_table = jstr(join_request, "table_id");
	const char *req_payin = jstr(join_request, "payin_tx");
	
	if (!req_dealer || !req_table || !req_payin) {
		cJSON_Delete(join_request);
		return 0;
	}
	
	if (strcmp(req_dealer, dealer_id) != 0 || strcmp(req_table, table_id) != 0) {
		cJSON_Delete(join_request);
		return 0;  // Not for our table
	}
	
	// Verify the payin_tx exists at cashier
	if (!is_txid_at_cashier(cashier_txids, req_payin)) {
		dlg_warn("Player %s payin_tx %s not found at cashier", player_id, req_payin);
		cJSON_Delete(join_request);
		return 0;
	}
	
	// Check if payin_tx was confirmed AFTER start_block (filter old game's transactions)
	int32_t tx_height = get_tx_block_height(req_payin);
	if (tx_height < start_block) {
		dlg_info("Player %s payin_tx %s is from old game (block %d < start %d), skipping",
			player_id, req_payin, tx_height, start_block);
		cJSON_Delete(join_request);
		return 0;
	}
	
	// Check if player is already joined
	if (is_player_already_joined(table_id, player_id)) {
		cJSON_Delete(join_request);
		return 0;  // Already joined
	}
	
	// Process the join!
	dlg_info("Processing join request from player %s (payin_tx: %s)", player_id, req_payin);
	
	int32_t retval = process_player_join(table_id, player_id, req_payin, dealer_id);
	cJSON_Delete(join_request);
	
	if (retval == OK) {
		dlg_info("Player %s successfully added to table %s", player_id, table_id);
		return 1;
	} else {
		dlg_error("Failed to add player %s: %s", player_id, bet_err_str(retval));
		return retval;
	}
}

/**
 * Poll player identities for pending join requests for a specific table.
 *
 * The dealer is the actor. The cashier is queried just once per tick as a
 * verifier (txid index since start_block); the actual join metadata lives in
 * P_JOIN_REQUEST_KEY on each player's own Verus identity.
 *
 * Strategy:
 * 1. Fetch txids at the cashier address since start_block (verification index).
 * 2. For each known player short-name, read its P_JOIN_REQUEST_KEY.
 * 3. Match dealer_id / table_id, verify payin_tx is in the cashier tx list and
 *    was confirmed at or after start_block.
 * 4. If new, process the join (writes T_PLAYER_INFO_KEY on the table id).
 *
 * @param cashier_id  The cashier's short Verus ID (e.g., "cashier") — verifier.
 * @param table_id    The table ID this dealer is seating for.
 * @param dealer_id   This dealer's short ID.
 * @param start_block Only accept payin_tx confirmed at or after this height.
 * @return Number of join requests processed, or negative error code.
 */
int32_t poker_poll_players_for_joins(const char *cashier_id, const char *table_id,
                                      const char *dealer_id, int32_t start_block)
{
	char *cashier_address = NULL;
	cJSON *txids = NULL;
	int32_t processed = 0;
	int32_t result = 0;

	if (!cashier_id || !table_id || !dealer_id) {
		return ERR_ARGS_NULL;
	}

	cashier_address = get_identity_address(cashier_id);
	if (!cashier_address) {
		dlg_warn("Could not get identity address for cashier: %s", cashier_id);
		return ERR_ID_NOT_FOUND;
	}

	txids = get_address_txids_range(cashier_address, start_block, 0);
	free(cashier_address);

	if (!txids || cJSON_GetArraySize(txids) == 0) {
		if (txids) cJSON_Delete(txids);
		return 0;
	}

	const char *known_players[] = {
		"p1.sg777z.VRSCTEST@", "p2.sg777z.VRSCTEST@", "p3.sg777z.VRSCTEST@",
		"p4.sg777z.VRSCTEST@", "p5.sg777z.VRSCTEST@", "p6.sg777z.VRSCTEST@",
		"p7.sg777z.VRSCTEST@", "p8.sg777z.VRSCTEST@", "p9.sg777z.VRSCTEST@",
		NULL
	};

	for (int i = 0; known_players[i] != NULL; i++) {
		result = check_player_join_request(known_players[i], table_id, dealer_id, txids, start_block);
		if (result > 0) {
			processed += result;
		}
	}

	cJSON_Delete(txids);
	
	if (processed > 0) {
		dlg_info("Processed %d join request(s)", processed);
	}
	
	return processed;
}

/* ============================================================================
 * Debug/Print API - Print table keys from blockchain
 * ============================================================================ */

/**
 * Print all table keys for a given ID from a specific block height.
 * Usage: ./bet print_keys <id> <block_height>
 */
void poker_print_table_keys(const char *id, int32_t block_height)
{
	if (!id || block_height < 0) {
		printf("Error: Invalid parameters\n");
		return;
	}

	// Strip trailing @ if present (internal functions add suffix automatically)
	char clean_id[256] = { 0 };
	strncpy(clean_id, id, sizeof(clean_id) - 1);
	size_t len = strlen(clean_id);
	if (len > 0 && clean_id[len - 1] == '@') {
		clean_id[len - 1] = '\0';
	}

	printf("\n========================================\n");
	printf("Table Keys for ID: %s\n", clean_id);
	printf("From block height: %d\n", block_height);
	printf("========================================\n\n");

	// Key definitions with descriptions
	struct {
		const char *key;
		const char *description;
		int is_json;  // 1 = JSON, 0 = string
	} keys[] = {
		{ T_GAME_ID_KEY, "Game ID", 0 },
		{ T_TABLE_INFO_KEY, "Table Info (max_players, BB, SB)", 1 },
		{ T_PLAYER_INFO_KEY, "Player Info (players + payin amounts)", 1 },
		{ T_GAME_INFO_KEY, "Game Info (state, round)", 1 },
		{ T_BETTING_STATE_KEY, "Betting State", 1 },
		{ T_D_DECK_KEY, "Dealer Deck", 1 },
		{ T_B_DECK_KEY, "Blinder Deck", 1 },
		{ T_BOARD_CARDS_KEY, "Board Cards", 1 },
		{ T_SETTLEMENT_INFO_KEY, "Settlement Info", 1 },
		{ NULL, NULL, 0 }
	};

	for (int i = 0; keys[i].key != NULL; i++) {
		printf("--- %s ---\n", keys[i].description);
		printf("Key: %s\n", keys[i].key);
		
		char *str_val = get_str_from_id_key_from_height(clean_id, keys[i].key, block_height);
		if (str_val) {
			if (keys[i].is_json) {
				// Try to parse and pretty-print JSON
				cJSON *json = cJSON_Parse(str_val);
				if (json) {
					char *pretty = cJSON_Print(json);
					printf("Value:\n%s\n", pretty);
					free(pretty);
					cJSON_Delete(json);
				} else {
					printf("Value: %s\n", str_val);
				}
			} else {
				printf("Value: %s\n", str_val);
			}
			free(str_val);
		} else {
			printf("Value: (not set)\n");
		}
		printf("\n");
	}

	// Also try to get game-specific keys if game_id exists
	char *game_id = get_str_from_id_key_from_height(clean_id, T_GAME_ID_KEY, block_height);
	if (game_id) {
		printf("\n========================================\n");
		printf("Game-specific keys for game_id: %s\n", game_id);
		printf("========================================\n\n");

		// T_PLAYER_INFO_KEY with game_id suffix
		char *player_info_key = get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id);
		if (player_info_key) {
			cJSON *t_player_info = get_cJSON_from_id_key_vdxfid_from_height(clean_id, player_info_key, block_height);
			if (t_player_info) {
				printf("--- Player Info (game-specific) ---\n");
				char *pretty = cJSON_Print(t_player_info);
				printf("%s\n\n", pretty);
				free(pretty);
				cJSON_Delete(t_player_info);
			}
		}

		// T_TABLE_INFO_KEY with game_id suffix
		char *table_info_key = get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id);
		if (table_info_key) {
			cJSON *t_table_info = get_cJSON_from_id_key_vdxfid_from_height(clean_id, table_info_key, block_height);
			if (t_table_info) {
				printf("--- Table Info (game-specific) ---\n");
				char *pretty = cJSON_Print(t_table_info);
				printf("%s\n\n", pretty);
				free(pretty);
				cJSON_Delete(t_table_info);
			}
		}

		// T_GAME_INFO_KEY with game_id suffix
		char *game_info_key = get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id);
		if (game_info_key) {
			cJSON *t_game_info = get_cJSON_from_id_key_vdxfid_from_height(clean_id, game_info_key, block_height);
			if (t_game_info) {
				printf("--- Game State Info (game-specific) ---\n");
				char *pretty = cJSON_Print(t_game_info);
				printf("%s\n\n", pretty);
				free(pretty);
				cJSON_Delete(t_game_info);
			}
		}

		free(game_id);
	}

	printf("========================================\n");
	printf("End of table keys dump\n");
	printf("========================================\n");
}

