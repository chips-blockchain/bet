#include "bet.h"
#include "blinder.h"
#include "commands.h"
#include "config.h"
#include "deck.h"
#include "cards.h"
#include "err.h"
#include "game.h"
#include "poker_vdxf.h"
#include <stdbool.h>

extern int32_t g_start_block;

// Forward declaration - defined later in file
static int32_t ensure_cashier_game_id_initialized(char *table_id);

/*
 * Cashier-side join discovery state (single-table for now — see docs/TODO.md).
 *
 * Idle until the first valid P_JOIN_REQUEST_KEY targeting this cashier shows up
 * on a known player's identity. From that request we learn `cashier_table_id`
 * and seed `g_start_block` from T_TABLE_INFO_KEY.<game_id>.start_block read off
 * the table id. Reset back to idle on G_SETTLEMENT_COMPLETE.
 */
static char    cashier_table_id[64] = {0};
static int32_t cashier_active = 0;

static int32_t cashier_check_payin_join(const char *player_short,
                                        char *out_table_id, size_t out_size);
static int32_t cashier_poll_players_for_joins(void);

char all_t_b_p_keys[all_t_b_p_keys_no][128] = { T_B_P1_DECK_KEY, T_B_P2_DECK_KEY, T_B_P3_DECK_KEY, T_B_P4_DECK_KEY,
						T_B_P5_DECK_KEY, T_B_P6_DECK_KEY, T_B_P7_DECK_KEY, T_B_P8_DECK_KEY,
						T_B_P9_DECK_KEY, T_B_DECK_KEY };

char all_t_b_p_key_names[all_t_b_p_keys_no][128] = { "t_b_p1_deck", "t_b_p2_deck", "t_b_p3_deck", "t_b_p4_deck",
						     "t_b_p5_deck", "t_b_p6_deck", "t_b_p7_deck", "t_b_p8_deck",
						     "t_b_p9_deck", "t_b_deck" };

/* Cashier-owned mirror of all_t_b_p_keys[] used by cashier_sb_deck to write
 * blinded decks onto the cashier's own identity. The dealer's canonicalizer
 * (canonicalize_cashier_blinded_decks in dealer.c) reads these and copies
 * the values onto the corresponding T_B_P*_DECK_KEY.<gid> entries on the
 * table id, preserving the existing player.c reader path. */
char all_c_b_p_keys[all_t_b_p_keys_no][128] = { C_B_P1_DECK_KEY, C_B_P2_DECK_KEY, C_B_P3_DECK_KEY, C_B_P4_DECK_KEY,
						C_B_P5_DECK_KEY, C_B_P6_DECK_KEY, C_B_P7_DECK_KEY, C_B_P8_DECK_KEY,
						C_B_P9_DECK_KEY, C_B_DECK_KEY };

char all_c_b_p_key_names[all_t_b_p_keys_no][128] = { "c_b_p1_deck", "c_b_p2_deck", "c_b_p3_deck", "c_b_p4_deck",
						     "c_b_p5_deck", "c_b_p6_deck", "c_b_p7_deck", "c_b_p8_deck",
						     "c_b_p9_deck", "c_b_deck" };

struct b_deck_info_struct b_deck_info;

int32_t cashier_sb_deck(char *id, bits256 *d_blinded_deck, int32_t player_id)
{
	int32_t retval = OK;
	char str[65], *game_id_str = NULL;
	cJSON *b_blinded_deck = NULL;
	char dbg[65];

	dlg_info("[DBG-SBDECK] player_id=%d, using cashier_r[%d][0].priv=%s",
		 player_id, player_id, bits256_str(dbg, b_deck_info.cashier_r[player_id][0].priv));

	/* `id` (the table id) is still the source of truth for the game_id.
	 * The cashier published its own T_GAME_ID_KEY via
	 * ensure_cashier_game_id_initialized but reading from the table id keeps
	 * a single canonical source for which game we're shuffling for. */
	game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, d_blinded_deck[i]));
	}
	shuffle_deck_db(d_blinded_deck, CARDS_MAXCARDS, b_deck_info.b_permi);
	blind_deck_b(d_blinded_deck, CARDS_MAXCARDS, b_deck_info.cashier_r[player_id]);

	b_blinded_deck = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		dlg_info("%d::%s", i, bits256_str(str, d_blinded_deck[i]));
		jaddibits256(b_blinded_deck, d_blinded_deck[i]);
	}
	char *b_blinded_deck_str = cJSON_PrintUnformatted(b_blinded_deck);
	if (b_blinded_deck_str) {
		dlg_info("b_blinded_deck::%s", b_blinded_deck_str);
		free(b_blinded_deck_str);
	}

	/* Single-writer-per-identity (docs/TODO.md item 1.1):
	 * Cashier writes its blinded deck for this player slot to its OWN id
	 * under C_B_P*_DECK_KEY.<game_id>. The dealer's canonicalizer copies
	 * this onto T_B_P*_DECK_KEY.<game_id> on the table id before
	 * transitioning the table to G_DECK_SHUFFLING_B, so the existing
	 * player.c:255 reader path is unchanged. */
	const char *write_id = bet_get_cashiers_id_fqn();
	dlg_info("[DBG-SBDECK] writing C_B_P%d_DECK_KEY to id=\"%s\" game_id=%s deck[0]=%s",
		 player_id + 1, write_id, game_id_str, bits256_str(dbg, d_blinded_deck[0]));
	cJSON *out = poker_append_key_json((char *)write_id,
					   get_key_data_vdxf_id(all_c_b_p_keys[player_id], game_id_str),
					   b_blinded_deck, true);

	if (!out)
		retval = ERR_DECK_BLINDING_CASHIER;

	if (out) {
		char *out_str = cJSON_PrintUnformatted(out);
		if (out_str) {
			dlg_info("%s", out_str);
			free(out_str);
		}
	}

	return retval;
}

void cashier_init_deck(char *table_id)
{
	int32_t num_players;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL;
	static int32_t init_call_count = 0;
	char dbg[65];

	init_call_count++;
	dlg_info("[DBG-INIT] cashier_init_deck call #%d for table_id=%s", init_call_count, table_id);

	bet_permutation(b_deck_info.b_permi, CARDS_MAXCARDS);

	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid_from_height(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
	num_players = jint(t_player_info, "num_players");
	for (int32_t i = 0; i < num_players; i++) {
		gen_deck(b_deck_info.cashier_r[i], CARDS_MAXCARDS);
		dlg_info("[DBG-INIT] regenerated cashier_r[%d][0].priv=%s (call #%d, game_id=%s)",
			 i, bits256_str(dbg, b_deck_info.cashier_r[i][0].priv), init_call_count, game_id_str);
	}
}

int32_t cashier_shuffle_deck(char *id)
{
	int32_t num_players = 0, retval = OK;
	char *game_id_str = NULL;
	cJSON *t_d_p_deck_info = NULL, *t_player_info = NULL;
	bits256 t_d_p_deck[CARDS_MAXCARDS];

	cashier_init_deck(id);
	game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
	num_players = jint(t_player_info, "num_players");

	for (int32_t i = 0; i < num_players; i++) {
		t_d_p_deck_info =
			get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(all_t_d_p_keys[(i + 1)], game_id_str), g_start_block);
		int32_t deck_size = cJSON_GetArraySize(t_d_p_deck_info);
		if (deck_size > CARDS_MAXCARDS) {
			deck_size = CARDS_MAXCARDS;
		}
		for (int32_t j = 0; j < deck_size; j++) {
			t_d_p_deck[j] = jbits256i(t_d_p_deck_info, j);
		}
		retval = cashier_sb_deck(id, t_d_p_deck, i);
		if (retval)
			return retval;
	}

	return retval;
}

static char g_last_bv_phase[16];
static char g_last_bv_gid[80];

static int32_t reveal_bv_batch(char *table_id, cJSON *game_state_info, cJSON *requests)
{
	int32_t retval = OK;
	char *game_id_str = NULL, hexstr[65];
	cJSON *batch = NULL, *bvs = NULL, *out = NULL;
	const char *phase = jstr(game_state_info, "phase");
	if (!phase) phase = "unknown";

	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	if (!game_id_str) return ERR_GAME_ID_NOT_FOUND;

	int32_t local_num_players = num_of_players;
	if (local_num_players <= 0) {
		cJSON *tpi = get_cJSON_from_id_key_vdxfid_from_height(
			table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
		local_num_players = jint(tpi, "num_players");
		if (tpi) cJSON_Delete(tpi);
	}

	batch = cJSON_CreateObject();
	cJSON_AddStringToObject(batch, "phase", phase);
	bvs = cJSON_CreateArray();
	for (int i = 0; i < cJSON_GetArraySize(requests); i++) {
		cJSON *e = cJSON_GetArrayItem(requests, i);
		int32_t pid = jint(e, "player_id");
		int32_t cid = jint(e, "card_id");
		cJSON *entry = cJSON_CreateObject();
		cJSON_AddNumberToObject(entry, "player_id", pid);
		cJSON_AddNumberToObject(entry, "card_id", cid);
		cJSON *bv_info = cJSON_CreateArray();
		if (pid >= 0) {
			jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[pid][cid].priv));
		} else {
			for (int p = 0; p < local_num_players; p++)
				jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[p][cid].priv));
		}
		cJSON_AddItemToObject(entry, "bv", bv_info);
		cJSON_AddItemToArray(bvs, entry);
	}
	cJSON_AddItemToObject(batch, "bvs", bvs);

	dlg_info("Cashier writing batched BVs (%d entries) for %s phase",
		 cJSON_GetArraySize(bvs), phase);
	strncpy(g_last_bv_phase, phase, sizeof(g_last_bv_phase) - 1);
	g_last_bv_phase[sizeof(g_last_bv_phase) - 1] = 0;
	strncpy(g_last_bv_gid, game_id_str, sizeof(g_last_bv_gid) - 1);
	g_last_bv_gid[sizeof(g_last_bv_gid) - 1] = 0;
	out = poker_append_key_json((char *)bet_get_cashiers_id_fqn(),
				    get_key_data_vdxf_id(C_CARD_BV_KEY, game_id_str),
				    batch, true);
	if (!out) {
		retval = ERR_BV_UPDATE;
		g_last_bv_phase[0] = 0;
		g_last_bv_gid[0] = 0;
	}
	cJSON_Delete(batch);
	return retval;
}

int32_t reveal_bv(char *table_id)
{
	int32_t player_id, card_id, num_players, retval = OK;
	char *game_id_str = NULL, hexstr[65];
	cJSON *bv_info = NULL, *game_state_info = NULL, *t_player_info = NULL, *out = NULL, *card_bv = NULL;

	game_state_info = get_game_state_info(table_id);
	{
		cJSON *requests = cJSON_GetObjectItem(game_state_info, "requests");
		if (requests && requests->type == cJSON_Array)
			return reveal_bv_batch(table_id, game_state_info, requests);
	}
	player_id = jint(game_state_info, "player_id");
	card_id = jint(game_state_info, "card_id");

	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);

	bv_info = cJSON_CreateArray();
	if (player_id == -1) {
		t_player_info =
			get_cJSON_from_id_key_vdxfid_from_height(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
		num_players = jint(t_player_info, "num_players");
		for (int32_t i = 0; i < num_players; i++) {
			dlg_info("[DBG-BV] reveal_bv community: cashier_r[%d][%d].priv=%s",
				 i, card_id, bits256_str(hexstr, b_deck_info.cashier_r[i][card_id].priv));
			jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[i][card_id].priv));
		}
	} else {
		dlg_info("[DBG-BV] reveal_bv hole: player_id=%d card_id=%d cashier_r[%d][%d].priv=%s",
			 player_id, card_id, player_id, card_id,
			 bits256_str(hexstr, b_deck_info.cashier_r[player_id][card_id].priv));
		jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[player_id][card_id].priv));
	}

	card_bv = cJSON_CreateObject();
	jaddnum(card_bv, "player_id", player_id);
	jaddnum(card_bv, "card_id", card_id);
	jadd(card_bv, "bv", bv_info);

	{
		char *card_bv_str = cJSON_PrintUnformatted(card_bv);
		if (card_bv_str) {
			dlg_info("bv_info::%s", card_bv_str);
			free(card_bv_str);
		}
	}
	/* Single-writer-per-identity (docs/TODO.md item 1.2): cashier writes
	 * each (player_id, card_id) BV to its OWN id under C_CARD_BV_KEY.<gid>.
	 * Players read directly from cashier_id; no canonicalize step on
	 * table_id. The handle_bv_reveal_card idempotency check below also
	 * reads from cashier_id. */
	out = poker_append_key_json((char *)bet_get_cashiers_id_fqn(),
				    get_key_data_vdxf_id(C_CARD_BV_KEY, game_id_str), card_bv,
				    true);

	if (!out) {
		retval = ERR_BV_UPDATE;
	}
	return retval;
}

static int32_t handle_bv_reveal_card(char *table_id)
{
	int32_t retval = OK;
	char *game_id_str = NULL;
	cJSON *card_bv = NULL, *game_state_info = NULL;

	game_state_info = get_game_state_info(table_id);
	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	/* Single-writer-per-identity (docs/TODO.md item 1.2): cashier reads its
	 * own previously-published BV from its OWN id, not table_id. */
	card_bv = get_cJSON_from_id_key_vdxfid_from_height((char *)bet_get_cashiers_id_fqn(),
							   get_key_data_vdxf_id(C_CARD_BV_KEY, game_id_str),
							   g_start_block);

	cJSON *requests = cJSON_GetObjectItem(game_state_info, "requests");
	if (requests && requests->type == cJSON_Array) {
		const char *cur_phase = jstr(game_state_info, "phase");
		if (cur_phase && game_id_str &&
		    g_last_bv_phase[0] && g_last_bv_gid[0] &&
		    strcmp(cur_phase, g_last_bv_phase) == 0 &&
		    strcmp(game_id_str, g_last_bv_gid) == 0) {
			return retval;
		}
		const char *prev_phase = card_bv ? jstr(card_bv, "phase") : NULL;
		if (card_bv && cJSON_GetObjectItem(card_bv, "bvs") &&
		    cur_phase && prev_phase && strcmp(cur_phase, prev_phase) == 0) {
			dlg_info("Cashier already published BV batch for %s phase; skipping",
				 cur_phase);
			return retval;
		}
		return reveal_bv(table_id);
	}

	if (card_bv && ((jint(game_state_info, "player_id") == jint(card_bv, "player_id")) &&
			(jint(game_state_info, "card_id") == jint(card_bv, "card_id")))) {
		dlg_info("Cashier revealed its Blinding Value for the player_id::%d, card_id::%d...",
			 jint(card_bv, "player_id"), jint(card_bv, "card_id"));
		return retval;
	}
	retval = reveal_bv(table_id);
	return retval;
}

// Process settlement - cashier sends funds to players
static int32_t cashier_process_settlement(char *table_id)
{
	int32_t retval = OK;
	char *game_id_str = NULL;
	cJSON *settlement_info = NULL, *settle_amounts = NULL, *player_ids = NULL;
	cJSON *payout_txs = NULL;
	
	dlg_info("=== PROCESSING POT SETTLEMENT ===");
	
	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_error("Failed to get game_id");
		return ERR_GAME_ID_NOT_FOUND;
	}

	/* Re-pay idempotency guard (docs/TODO.md item 1.3): once we've
	 * migrated the result write off T_SETTLEMENT_INFO_KEY (which the
	 * cashier no longer mutates), the cashier's old "status==completed"
	 * check on the table-id record can never fire — that record stays
	 * "pending" until the dealer canonicalizes. Instead, gate on the
	 * cashier's own C_SETTLEMENT_RESULT_KEY.<gid>: if already present
	 * with status:completed, payouts have been issued in a prior tick
	 * (the dealer just hasn't canonicalized yet) and we must not re-pay. */
	{
		cJSON *prior_result = get_cJSON_from_id_key_vdxfid_from_height(
			(char *)bet_get_cashiers_id_fqn(),
			get_key_data_vdxf_id(C_SETTLEMENT_RESULT_KEY, game_id_str),
			g_start_block);
		if (prior_result) {
			const char *prior_status = jstr(prior_result, "status");
			if (prior_status && strcmp(prior_status, "completed") == 0) {
				dlg_info("Settlement already completed for game %s (C_SETTLEMENT_RESULT_KEY present); skipping re-pay",
					 game_id_str);
				cJSON_Delete(prior_result);
				return OK;
			}
			cJSON_Delete(prior_result);
		}
	}

	// Read settlement info from table ID
	settlement_info = get_cJSON_from_id_key_vdxfid_from_height(table_id,
		get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, game_id_str), g_start_block);
	
	if (!settlement_info) {
		dlg_error("No settlement info found");
		return ERR_INVALID_POS;
	}
	
	const char *status = jstr(settlement_info, "status");
	if (!status || strcmp(status, "pending") != 0) {
		dlg_info("Settlement not pending (status: %s)", status ? status : "null");
		return OK;
	}
	
	settle_amounts = cJSON_GetObjectItem(settlement_info, "settle_amounts");
	player_ids = cJSON_GetObjectItem(settlement_info, "player_ids");
	
	if (!settle_amounts || !player_ids) {
		dlg_error("Missing settle_amounts or player_ids in settlement info");
		return ERR_INVALID_POS;
	}
	
	int num_players = cJSON_GetArraySize(settle_amounts);
	payout_txs = cJSON_CreateArray();
	
	/* Send funds to each player. settle_amounts is in integer table chips
	 * (dealer wrote it that way in dealer_initiate_settlement). Convert
	 * back to CHIPS for the on-chain sendcurrency call - this is one of
	 * the only two table-chips -> CHIPS boundaries in the codebase
	 * (the other is the payin in vdxf.c::process_payin_tx_data). */
	for (int i = 0; i < num_players; i++) {
		cJSON *amount_item = cJSON_GetArrayItem(settle_amounts, i);
		cJSON *player_id_item = cJSON_GetArrayItem(player_ids, i);
		
		if (!amount_item || !player_id_item) continue;
		
		int64_t amount_table_chips = (int64_t)amount_item->valuedouble;
		const char *player_id = player_id_item->valuestring;
		
		if (amount_table_chips <= 0) {
			dlg_info("Player %s has no payout (amount: %lld table chips)",
				 player_id, (long long)amount_table_chips);
			cJSON_AddItemToArray(payout_txs, cJSON_CreateString(""));
			continue;
		}
		
		double amount_chips = table_chips_to_chips(amount_table_chips);
		dlg_info("Sending %.8f CHIPS (%lld table chips) to player %s",
			 amount_chips, (long long)amount_table_chips, player_id);
		
		// Create payout data
		cJSON *payout_data = cJSON_CreateObject();
		cJSON_AddStringToObject(payout_data, "type", "game_settlement");
		cJSON_AddStringToObject(payout_data, "game_id", game_id_str);
		cJSON_AddStringToObject(payout_data, "table_id", table_id);
		cJSON_AddNumberToObject(payout_data, "player_slot", i);
		
		// Send using sendcurrency
		cJSON *tx_result = verus_sendcurrency_data(player_id, amount_chips, payout_data);
		
		if (tx_result) {
			const char *txid = NULL;
			if (tx_result->type == cJSON_String) {
				txid = tx_result->valuestring;
			} else {
				txid = jstr(tx_result, "opid");
			}
			dlg_info("Payout TX for player %s: %s", player_id, txid ? txid : "pending");
			cJSON_AddItemToArray(payout_txs, cJSON_CreateString(txid ? txid : "pending"));
		} else {
			dlg_error("Failed to send payout to player %s", player_id);
			cJSON_AddItemToArray(payout_txs, cJSON_CreateString("failed"));
		}
		
		cJSON_Delete(payout_data);
	}
	
	/* Single-writer-per-identity (docs/TODO.md item 1.3): publish the
	 * cashier's settlement *result* (status + payout_txs) on the cashier's
	 * OWN id — the dealer canonicalizes status + payout_txs onto
	 * T_SETTLEMENT_INFO_KEY.<gid> on table_id once it observes this key.
	 * The dealer-written settlement *order* (winners, settle_amounts,
	 * player_ids, payin_txs, pot, cashier_id) is never mutated by us. */
	cJSON *cashier_result = cJSON_CreateObject();
	cJSON_AddStringToObject(cashier_result, "status", "completed");
	/* payout_txs ownership transfers to cashier_result here; do NOT
	 * cJSON_Delete it separately. */
	cJSON_AddItemToObject(cashier_result, "payout_txs", payout_txs);

	cJSON *out = poker_append_key_json((char *)bet_get_cashiers_id_fqn(),
		get_key_data_vdxf_id(C_SETTLEMENT_RESULT_KEY, game_id_str),
		cashier_result, true);

	if (!out) {
		dlg_error("Failed to publish C_SETTLEMENT_RESULT_KEY on cashier id");
		cJSON_Delete(cashier_result);
		return ERR_GAME_STATE_UPDATE;
	}

	dlg_info("Settlement complete - signalling G_SETTLEMENT_COMPLETE_BY_CASHIER on cashier id");

	/* Single-writer-per-identity (docs/TODO.md item 1.4): write the
	 * cashier-side terminal-state signal on the cashier's OWN id.
	 * dealer.c::handle_game_state polls cashier_id via
	 * is_cashier_settlement_complete and, on observing this value, writes
	 * the canonical G_SETTLEMENT_COMPLETE on table_id. Mirrors the
	 * existing G_DECK_SHUFFLING_B handshake exactly — the table id
	 * remains a dealer-only writer for game state. */
	append_game_state((char *)bet_get_cashiers_id_fqn(),
			  G_SETTLEMENT_COMPLETE_BY_CASHIER, NULL);

	cJSON_Delete(cashier_result);
	return retval;
}

/*
 * Inspect P_JOIN_REQUEST_KEY on `player_short`. If the request targets this
 * cashier and the claimed payin_tx actually credited this cashier's address,
 * copy the table_id into `out_table_id` and return OK.
 *
 * No on-chain writes. No height filter on the player ID read (P_JOIN_REQUEST_KEY
 * is cumulative-latest). Verification is cryptographic via
 * chips_get_balance_on_address_from_tx — independent of g_start_block, so this
 * function can run before the cashier has resolved a start_block at all.
 *
 * Returns OK on a verified match, ERR_ID_NOT_FOUND for any non-match
 * (no request, request not for us, payin not credited). The caller only
 * branches on OK vs non-OK, so the specific code is a sentinel.
 */
static int32_t cashier_check_payin_join(const char *player_short,
                                        char *out_table_id, size_t out_size)
{
	cJSON *join_request = NULL;
	int32_t retval = ERR_ID_NOT_FOUND;

	if (!player_short || !out_table_id || out_size == 0) {
		return ERR_ARGS_NULL;
	}

	if (!is_id_exists(player_short)) {
		return ERR_ID_NOT_FOUND;
	}

	join_request = get_cJSON_from_id_key(player_short, P_JOIN_REQUEST_KEY);
	if (!join_request) {
		return ERR_ID_NOT_FOUND;
	}

	const char *req_cashier = jstr(join_request, "cashier_id");
	const char *req_table   = jstr(join_request, "table_id");
	const char *req_payin   = jstr(join_request, "payin_tx");

	if (!req_cashier || !req_table || !req_payin) {
		cJSON_Delete(join_request);
		return ERR_ID_NOT_FOUND;
	}

	if (strcmp(req_cashier, bet_get_cashiers_id_fqn()) != 0) {
		cJSON_Delete(join_request);
		return ERR_ID_NOT_FOUND;
	}

	/* Verify cryptographically: the payin_tx actually credited our address. */
	double credited = chips_get_balance_on_address_from_tx(
		get_vdxf_id(bet_get_cashiers_id_fqn()), (char *)req_payin);
	if (credited <= 0.0) {
		dlg_warn("Player %s claims payin_tx %s but it did not credit cashier",
			 player_short, req_payin);
		cJSON_Delete(join_request);
		return ERR_ID_NOT_FOUND;
	}

	snprintf(out_table_id, out_size, "%s", req_table);
	dlg_info("Cashier verified join: player=%s table=%s payin_tx=%s amount=%f",
		 player_short, req_table, req_payin, credited);
	retval = OK;

	cJSON_Delete(join_request);
	return retval;
}

/*
 * Iterate known_players[]. On the first verified match, learn `cashier_table_id`
 * from the join request, then resolve the game's start_block from the table id
 * (T_GAME_ID_KEY -> T_TABLE_INFO_KEY.<game_id>.start_block) and seed
 * g_start_block. Marks `cashier_active = 1`.
 *
 * Both reads from the table id use plain getidentity (no height filter) because
 * those keys are cumulative-latest writes by the dealer at game-init time.
 *
 * Returns OK once the cashier is active, ERR_TABLE_IS_NOT_STARTED while idle.
 */
static int32_t cashier_poll_players_for_joins(void)
{
	char learned_table_id[64] = {0};

	if (!known_players) {
		dlg_error("cashier_poll_players_for_joins: known_players is NULL");
		return ERR_TABLE_IS_NOT_STARTED;
	}

	for (int i = 0; known_players[i] != NULL; i++) {
		if (cashier_check_payin_join(known_players[i],
					     learned_table_id,
					     sizeof(learned_table_id)) == OK) {
			break;
		}
	}

	if (learned_table_id[0] == '\0') {
		return ERR_TABLE_IS_NOT_STARTED;
	}

	/* Resolve game_id and start_block from the table id (cumulative reads). */
	char *game_id_str = get_str_from_id_key(learned_table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_warn("Cashier learned table %s but T_GAME_ID_KEY is missing",
			 learned_table_id);
		return ERR_GAME_ID_NOT_FOUND;
	}

	cJSON *t_table_info = get_cJSON_from_id_key_vdxfid(
		learned_table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str));
	if (!t_table_info) {
		dlg_warn("Cashier learned table %s game %s but T_TABLE_INFO_KEY.<game_id> is missing",
			 learned_table_id, game_id_str);
		return ERR_T_TABLE_INFO_NULL;
	}

	int32_t start_block = jint(t_table_info, "start_block");
	cJSON_Delete(t_table_info);

	if (start_block <= 0) {
		dlg_warn("Cashier learned table %s game %s but start_block is %d",
			 learned_table_id, game_id_str, start_block);
		return ERR_TABLE_IS_NOT_STARTED;
	}

	snprintf(cashier_table_id, sizeof(cashier_table_id), "%s", learned_table_id);
	g_start_block = start_block;
	cashier_active = 1;

	return OK;
}

// Ensure cashier's ID has T_GAME_ID_KEY set (required for append_game_state to work)
static int32_t ensure_cashier_game_id_initialized(char *table_id)
{
	static bool initialized = false;
	if (initialized) return OK;
	
	// Get game_id from the table
	char *game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_error("Cannot initialize cashier game_id - table has no game_id");
		return ERR_GAME_ID_NOT_FOUND;
	}
	
	// Set T_GAME_ID_KEY on the cashier's ID (use short name for update_cmm)
	dlg_info("Initializing cashier's T_GAME_ID_KEY: %s", game_id_str);
	cJSON *out = poker_append_key_hex((char *)bet_get_cashiers_id_fqn(), T_GAME_ID_KEY, game_id_str, false);
	if (!out) {
		dlg_error("Failed to set game_id on cashier's ID");
		return ERR_GAME_STATE_UPDATE;
	}
	
	initialized = true;
	dlg_info("Cashier game_id initialized successfully");
	return OK;
}

int32_t handle_game_state_cashier(char *table_id)
{
	int32_t game_state, retval = OK;
	static int32_t last_logged_state = -1;

	game_state = get_game_state(table_id);
	if (game_state != last_logged_state) {
		dlg_info("%s", game_state_str(game_state));
		last_logged_state = game_state;
	}
	switch (game_state) {
	case G_ZEROIZED_STATE:
	case G_TABLE_ACTIVE:
	case G_TABLE_STARTED:
	case G_PLAYERS_JOINED:
	case G_DECK_SHUFFLING_P:
	case G_DECK_SHUFFLING_B:
		break;
	case G_DECK_SHUFFLING_D:
		// Ensure cashier's game_id is set before updating game state
		retval = ensure_cashier_game_id_initialized(table_id);
		if (retval) break;

		/* Idempotency gate (docs/TODO.md item 1.1, post-direct-read race fix):
		 * if C_B_P1_DECK_KEY.<game_id> is already present on the cashier id,
		 * the shuffle for THIS game has already completed. Re-running
		 * cashier_shuffle_deck would re-call cashier_init_deck and regenerate
		 * b_deck_info.cashier_r in memory; the deck on-chain is frozen at the
		 * first shuffle's bytes, so BVs derived from the new cashier_r at
		 * reveal time would not match → decode_card returns -1.
		 *
		 * The gate is keyed by <game_id> so it's automatically per-game and
		 * self-resets when a new game_id is generated. */
		{
			char *gate_gid = poker_get_key_str(table_id, T_GAME_ID_KEY);
			if (gate_gid) {
				cJSON *existing = get_cJSON_from_id_key_vdxfid_from_height(
					(char *)bet_get_cashiers_id_fqn(),
					get_key_data_vdxf_id(all_c_b_p_keys[0], gate_gid),
					g_start_block);
				if (existing) {
					cJSON_Delete(existing);
					break;
				}
			}
		}

		retval = cashier_shuffle_deck(table_id);
		if (!retval) {
			dlg_info("Cashier shuffle complete, updating game state to G_DECK_SHUFFLING_B");
			append_game_state(bet_get_cashiers_id_fqn(), G_DECK_SHUFFLING_B, NULL);
		}
		break;
	case G_REVEAL_CARD:
		retval = handle_bv_reveal_card(table_id);
		break;
	case G_SETTLEMENT_PENDING:
		dlg_info("Processing pot settlement...");
		retval = cashier_process_settlement(table_id);
		break;
	case G_SETTLEMENT_COMPLETE:
		dlg_info("Settlement complete - game finished");
		/* Reset cashier-side join discovery state so the next game's
		 * payin will re-seed cashier_table_id and g_start_block. */
		cashier_active = 0;
		cashier_table_id[0] = '\0';
		g_start_block = 0;
		dlg_info("Cashier returning to idle — polling player IDs for next join");
		break;
	}
	return retval;
}

int32_t cashier_game_init(void)
{
	int32_t retval = OK;

	dlg_info("Cashier idle — polling player IDs for join requests");

	while (1) {
		if (!cashier_active) {
			if (cashier_poll_players_for_joins() != OK) {
				sleep(2);
				continue;
			}
			dlg_info("Cashier active for table %s, start_block=%d",
				 cashier_table_id, g_start_block);
		}

		retval = handle_game_state_cashier(cashier_table_id);
		if (retval)
			return retval;
		sleep(2);
	}
}
