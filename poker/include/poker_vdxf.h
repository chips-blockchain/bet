#ifndef __POKER_VDXF_H__
#define __POKER_VDXF_H__

#include "bet.h"
#include "common.h"
#include "vdxf.h"

/* ============================================================================
 * Poker VDXF API
 * ============================================================================
 * This module provides poker-specific operations built on top of the generic
 * VDXF/Verus RPC layer (vdxf.c). These functions understand poker game keys
 * and structures.
 *
 * Architecture:
 *   Application (dealer.c, player.c)
 *        |
 *        v
 *   poker_vdxf.c  <-- This layer (poker-specific key parsing)
 *        |
 *        v
 *   vdxf.c        <-- Generic VDXF operations (get_cmm, update_cmm)
 *        |
 *        v
 *   chips_rpc()   <-- RPC abstraction (REST or CLI)
 * ============================================================================ */

/* ============================================================================
 * Poker Key Data Access
 * ============================================================================ */

/**
 * Get string value from a poker key stored in an identity
 * @param id The identity name
 * @param key The poker key name (e.g., T_TABLE_INFO_KEY)
 * @return String value or NULL
 */
char *poker_get_key_str(const char *id, const char *key);

/**
 * Get cJSON value from a poker key stored in an identity
 * @param id The identity name
 * @param key The poker key name
 * @return cJSON object or NULL
 */
cJSON *poker_get_key_json(const char *id, const char *key);

/**
 * Get cJSON value using a pre-computed vdxfid
 * @param id The identity name
 * @param key_vdxfid The vdxfid of the key
 * @return cJSON object or NULL
 */
cJSON *poker_get_key_json_by_vdxfid(char *id, char *key_vdxfid);

/* ============================================================================
 * Poker Key Data Updates
 * ============================================================================ */

/**
 * Append hex data to a key in an identity's CMM
 * @param id The identity name
 * @param key The poker key name
 * @param hex_data The hex-encoded data to append
 * @param is_key_vdxf_id true if key is already a vdxfid
 * @return Updated CMM or NULL on error
 */
cJSON *poker_append_key_hex(char *id, char *key, char *hex_data, bool is_key_vdxf_id);

/**
 * Append cJSON data to a key in an identity's CMM
 * @param id The identity name
 * @param key The poker key name
 * @param data The cJSON data to append
 * @param is_key_vdxf_id true if key is already a vdxfid
 * @return Updated CMM or NULL on error
 */
cJSON *poker_append_key_json(const char *id, const char *key, cJSON *data, bool is_key_vdxf_id);

/**
 * Update (replace) data for a key in an identity's CMM
 * @param id The identity name
 * @param key The poker key name
 * @param data The cJSON data to set
 * @param is_key_vdxf_id true if key is already a vdxfid
 * @return Updated CMM or NULL on error
 */
cJSON *poker_update_key_json(char *id, char *key, cJSON *data, bool is_key_vdxf_id);

/* ============================================================================
 * Table Operations
 * ============================================================================ */

/**
 * Decode table info from a string
 * @param str The encoded table info string
 * @return Decoded table structure or NULL
 */
struct table *poker_decode_table_info_str(char *str);

/**
 * Decode table info from cJSON
 * @param dealer_cmm_data The dealer's CMM data
 * @return Decoded table structure or NULL
 */
struct table *poker_decode_table_info(cJSON *dealer_cmm_data);

/**
 * Check if a table is full
 * @param table_id The table identity
 * @return true if table is full, false otherwise
 */
bool poker_is_table_full(char *table_id);

/**
 * Check if a table is registered under a dealer
 * @param table_id The table identity
 * @param dealer_id The dealer identity
 * @return true if registered, false otherwise
 */
bool poker_is_table_registered(char *table_id, char *dealer_id);

/* ============================================================================
 * Player Operations
 * ============================================================================ */

/**
 * Get the player ID assigned by the dealer
 * @param player_id Output parameter for player ID
 * @return OK on success, error code on failure
 */
int32_t poker_get_player_id(int *player_id);

/**
 * Join a poker table
 * @return OK on success, error code on failure
 */
int32_t poker_join_table();

/**
 * Check player join status
 * @param table_id The table identity
 * @param verus_pid The player's Verus ID
 * @return Join status code
 */
int32_t poker_check_player_join_status(char *table_id, char *verus_pid);

/* ============================================================================
 * Dealer/Cashier Registry
 * ============================================================================ */

/**
 * List all registered dealers
 * @return cJSON array of dealers or NULL
 */
cJSON *poker_list_dealers();

/**
 * List all available tables
 */
void poker_list_tables();

/**
 * Add a dealer to the dealers registry
 * @param dealer_id The dealer identity to add
 * @return OK on success, error code on failure
 */
int32_t poker_add_dealer(char *dealer_id);

/* ============================================================================
 * Transaction Processing
 * ============================================================================ */

/**
 * Process a payin transaction
 * @param txid The transaction ID
 * @param payin_tx_data The transaction data
 * @return OK on success, error code on failure
 */
int32_t poker_process_payin_tx(char *txid, cJSON *payin_tx_data);

/**
 * Process a new block for poker-related transactions
 * @param blockhash The block hash to process
 */
void poker_process_block(char *blockhash);

/* ============================================================================
 * Setup Verification
 * ============================================================================ */

/**
 * Verify the poker environment is properly configured
 * @return OK if setup is valid, error code otherwise
 */
int32_t poker_verify_setup();

/* ============================================================================
 * Cashier Polling for Join Requests
 * ============================================================================ */

/**
 * Poll player identities for pending join requests against a specific table.
 *
 * Called by the dealer. The cashier is queried only as a verifier — its txid
 * list at start_block is fetched once per tick and used to confirm that each
 * player's claimed payin_tx actually landed on-chain. The discovery target is
 * the player identity itself (P_JOIN_REQUEST_KEY).
 *
 * Steps per tick:
 * 1. Fetch txids at the cashier address since start_block (verification index).
 * 2. For each known player short-name:
 *    a. Read P_JOIN_REQUEST_KEY from that player's identity.
 *    b. Match dealer_id / table_id; verify payin_tx is in the cashier txid list
 *       and confirmed at or after start_block.
 *    c. Skip if the player is already in t_player_info, otherwise process the
 *       join (writes T_PLAYER_INFO_KEY on the table id).
 *
 * @param cashier_id  The cashier's short ID (e.g., "cashier") — used as a verifier.
 * @param table_id    The table ID this dealer is seating for.
 * @param dealer_id   This dealer's short ID.
 * @param start_block Earliest block height accepted for a payin_tx.
 * @return Number of join requests processed, or negative error code.
 */
int32_t poker_poll_players_for_joins(const char *cashier_id, const char *table_id, 
                                      const char *dealer_id, int32_t start_block);

/* ============================================================================
 * Debug/Print API
 * ============================================================================ */

/**
 * Print all table keys for debugging/inspection
 * 
 * This function prints all poker-related keys stored in an identity,
 * formatted as JSON for easy inspection.
 * 
 * Usage: ./bet print_keys <id> <block_height>
 * 
 * @param id The identity to inspect (e.g., "t1@", "d1@", "p1@")
 * @param block_height Starting block height to search from
 */
void poker_print_table_keys(const char *id, int32_t block_height);

#endif /* __POKER_VDXF_H__ */

