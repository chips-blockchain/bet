#include "bet.h"
#include "dealer.h"
#include "deck.h"
#include "cards.h"
#include "game.h"
#include "err.h"
#include "misc.h"
#include "commands.h"
#include "dealer_registration.h"
#include "config.h"
#include "poker_vdxf.h"
#include "storage.h"
#include "poker.h"

struct d_deck_info_struct d_deck_info;
struct game_meta_info_struct game_meta_info;

char all_t_d_p_keys[all_t_d_p_keys_no][128] = { T_D_DECK_KEY,    T_D_P1_DECK_KEY, T_D_P2_DECK_KEY, T_D_P3_DECK_KEY,
						T_D_P4_DECK_KEY, T_D_P5_DECK_KEY, T_D_P6_DECK_KEY, T_D_P7_DECK_KEY,
						T_D_P8_DECK_KEY, T_D_P9_DECK_KEY };

char all_t_d_p_key_names[all_t_d_p_keys_no][128] = { "t_d_deck",    "t_d_p1_deck", "t_d_p2_deck", "t_d_p3_deck",
						     "t_d_p4_deck", "t_d_p5_deck", "t_d_p6_deck", "t_d_p7_deck",
						     "t_d_p8_deck", "t_d_p9_deck" };

char all_game_keys[all_game_keys_no][128] = { T_GAME_INFO_KEY };

char all_game_key_names[all_game_keys_no][128] = { "t_game_info" };

int32_t num_of_players;
char player_ids[CARDS_MAXPLAYERS][MAX_ID_LEN];

int32_t add_dealer(char *dealer_id, const char *aggregator_fqn)
{
	int32_t retval = OK;
	cJSON *dealers_info = NULL, *dealers = NULL, *existing = NULL, *out = NULL;
	const char *fqn = aggregator_fqn ? aggregator_fqn : bet_get_dealers_id_fqn();

	if (!dealer_id) {
		return ERR_NULL_ID;
	}
	if (!strchr(dealer_id, '@')) {
		dlg_error("add_dealer: dealer_id must be a fully-qualified Verus ID; got '%s'", dealer_id);
		return ERR_ARGS_NULL;
	}
	if (!strchr(fqn, '@')) {
		dlg_error("add_dealer: aggregator must be a fully-qualified Verus ID; got '%s'", fqn);
		return ERR_ARGS_NULL;
	}
	if (!is_id_exists(dealer_id)) {
		return ERR_ID_NOT_FOUND;
	}

	if (!id_cansignfor(fqn, &retval)) {
		return retval;
	}

	dealers_info = cJSON_CreateObject();
	existing = get_cJSON_from_id_key(fqn, DEALERS_KEY);
	if (existing) {
		dealers = cJSON_DetachItemFromObject(existing, "dealers");
		cJSON_Delete(existing);
	}
	if (!dealers) {
		dealers = cJSON_CreateArray();
	}
	jaddistr(dealers, dealer_id);
	cJSON_AddItemToObject(dealers_info, "dealers", dealers);
	out = poker_update_key_json((char *)fqn, DEALERS_KEY, dealers_info, false);

	if (!out) {
		return ERR_UPDATEIDENTITY;
	}
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

int32_t dealer_sb_deck(char *id, bits256 *player_r, int32_t player_id)
{
	int32_t retval = OK;
	char str[65], *game_id_str = NULL;
	cJSON *d_blinded_deck = NULL;

	game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);

	dlg_info("Player::%d deck...", player_id);
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}
	shuffle_deck_db(player_r, CARDS_MAXCARDS, d_deck_info.d_permi);
	blind_deck_d(player_r, CARDS_MAXCARDS, d_deck_info.dealer_r);

	dlg_info("Player::%d deck blinded with dealer secret key...", player_id);
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}

	d_blinded_deck = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		jaddistr(d_blinded_deck, bits256_str(str, player_r[i]));
	}
	dlg_info("Updating Player ::%d blinded deck at the key ::%s by dealer..", player_id, all_t_d_p_keys[player_id]);
	cJSON *out = poker_append_key_json(id, get_key_data_vdxf_id(all_t_d_p_keys[player_id], game_id_str),
						       d_blinded_deck, true);

	if (!out)
		retval = ERR_DECK_BLINDING_DEALER;
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

void dealer_init_deck()
{
	bet_permutation(d_deck_info.d_permi, CARDS_MAXCARDS);
	gen_deck(d_deck_info.dealer_r, CARDS_MAXCARDS);
}

int32_t dealer_table_init(struct table *t)
{
	int32_t game_state = G_ZEROIZED_STATE, retval = OK;
	char hexstr[65], *hex_data = NULL;
	cJSON *out = NULL, *t_game_info = NULL, *t_table_info = NULL;
	cJSON *cmm = NULL, *data_obj = NULL;
	char *game_id_vdxfid = NULL, *table_info_vdxfid = NULL;
	char *bytevector_vdxfid = NULL, *game_id_key_vdxfid = NULL;

	if (!is_id_exists(t->table_id))
		return ERR_ID_NOT_FOUND;

	game_state = get_game_state(t->table_id);

	switch (game_state) {
	case G_ZEROIZED_STATE:
	case G_TABLE_ACTIVE:
		// Generate new game ID (or use existing if resuming)
		if (game_state == G_ZEROIZED_STATE) {
			game_id = rand256(0);
		} else {
			char *game_id_str = poker_get_key_str(t->table_id, T_GAME_ID_KEY);
			if (!game_id_str) {
				dlg_error("Failed to get game_id from chain");
				return ERR_GAME_ID_NOT_FOUND;
			}
			game_id = bits256_conv(game_id_str);
		}
		bits256_str(hexstr, game_id);
		dlg_info("Game ID: %s", hexstr);
		
		// Set start_block
		t->start_block = chips_get_block_count();
		g_start_block = t->start_block;  // Set global for CMM reads
		dlg_info("Table start_block: %d", t->start_block);

		// Build ALL keys in a single CMM for atomic update
		cmm = cJSON_CreateObject();
		bytevector_vdxfid = get_vdxf_id(BYTEVECTOR_VDXF_ID);
		
		// Key 1: T_GAME_ID_KEY
		game_id_key_vdxfid = get_vdxf_id(T_GAME_ID_KEY);
		data_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hexstr);
		cJSON_AddItemToObject(cmm, game_id_key_vdxfid, data_obj);

		// Key 2: T_TABLE_INFO_KEY.<game_id>
		table_info_vdxfid = get_key_data_vdxf_id(T_TABLE_INFO_KEY, hexstr);
		t_table_info = struct_table_to_cJSON(t);
		cJSON_hex(t_table_info, &hex_data);
		data_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hex_data);
		cJSON_AddItemToObject(cmm, table_info_vdxfid, data_obj);
		free(hex_data); hex_data = NULL;
		cJSON_Delete(t_table_info);

		// Key 3: T_GAME_INFO_KEY.<game_id> = {game_state: G_TABLE_STARTED}
		game_id_vdxfid = get_key_data_vdxf_id(T_GAME_INFO_KEY, hexstr);
		t_game_info = cJSON_CreateObject();
		cJSON_AddNumberToObject(t_game_info, "game_state", G_TABLE_STARTED);
		cJSON_hex(t_game_info, &hex_data);
		data_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hex_data);
		cJSON_AddItemToObject(cmm, game_id_vdxfid, data_obj);
		free(hex_data);
		cJSON_Delete(t_game_info);

		// Single atomic update with all keys
		dlg_info("Updating table with all init data in single transaction...");
		out = update_cmm(t->table_id, cmm);
		cJSON_Delete(cmm);
		
		if (!out) {
			dlg_error("Failed to update table CMM");
			return ERR_TABLE_LAUNCH;
		}
		dlg_info("Table initialized successfully");
		break;
	default:
		// Table is already started - attempt REJOIN
		dlg_info("═══════════════════════════════════════════");
		dlg_info("  🔄 DEALER REJOIN - Resuming game...      ");
		dlg_info("═══════════════════════════════════════════");
		{
			char *game_id_str = poker_get_key_str(t->table_id, T_GAME_ID_KEY);
			if (!game_id_str) {
				dlg_error("Cannot rejoin: game_id not found on chain");
				return ERR_GAME_ID_NOT_FOUND;
			}
			
			// Bootstrap read with fallback height to get start_block
			int32_t bootstrap_height = chips_get_block_count() - 1000;
			cJSON *t_table_info = get_cJSON_from_id_key_vdxfid_from_height(t->table_id, 
				get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str), bootstrap_height);
			if (t_table_info) {
				t->start_block = jint(t_table_info, "start_block");
				g_start_block = t->start_block;
				dlg_info("Loaded start_block from chain: %d", t->start_block);
			} else {
				dlg_error("Cannot rejoin: t_table_info not found");
				return ERR_TABLE_LAUNCH;
			}
			
			// Load player_ids from t_player_info
			cJSON *t_player_info = get_cJSON_from_id_key_vdxfid_from_height(t->table_id,
				get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
			if (t_player_info) {
				num_of_players = jint(t_player_info, "num_players");
				cJSON *player_info_arr = cJSON_GetObjectItem(t_player_info, "player_info");
				if (player_info_arr && player_info_arr->type == cJSON_Array) {
					for (int32_t i = 0; i < num_of_players && i < CARDS_MAXPLAYERS; i++) {
						cJSON *item = cJSON_GetArrayItem(player_info_arr, i);
						if (item && item->valuestring) {
							char *player_info_str = item->valuestring;
							char *underscore = strchr(player_info_str, '_');
							if (underscore) {
								size_t id_len = underscore - player_info_str;
								if (id_len < MAX_ID_LEN) {
									strncpy(player_ids[i], player_info_str, id_len);
									player_ids[i][id_len] = '\0';
									dlg_info("Loaded player %d ID: %s", i, player_ids[i]);
								}
							}
						}
					}
				}
			}
			
			// If game is past dealer shuffle, load deck info from local DB
			if (game_state >= G_DECK_SHUFFLING_D) {
				int32_t load_result = load_dealer_deck_info(game_id_str);
				if (load_result == OK) {
					dlg_info("✓ Dealer deck info loaded from local DB - rejoin successful");
				} else {
					dlg_error("✗ Cannot rejoin: dealer deck info not found in local DB");
					dlg_error("  Game cannot continue - deck keys lost");
					return ERR_DECK_BLINDING_DEALER;
				}
			}
			
			// Set game_id global
			game_id = bits256_conv(game_id_str);
		}
		dlg_info("Rejoin successful. Game state: %s", game_state_str(game_state));
	}
	return retval;
}

bool is_players_shuffled_deck(char *table_id)
{
	int32_t game_state, num_players = 0, count = 0;
	;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL;

	game_state = get_game_state(table_id);

	if (game_state == G_DECK_SHUFFLING_P) {
		return true;
	} else if (game_state == G_PLAYERS_JOINED) {
		game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
		t_player_info =
			get_cJSON_from_id_key_vdxfid_from_height(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
		num_players = jint(t_player_info, "num_players");
		for (int32_t i = 0; i < num_players; i++) {
			if (G_DECK_SHUFFLING_P == get_game_state(player_ids[i]))
				count++;
		}
		if (count == num_players)
			return true;
	}
	return false;
}

static bool is_cashier_shuffled_deck(char *cashier_id)
{
	int32_t game_state = get_game_state(cashier_id);
	return (game_state == G_DECK_SHUFFLING_B);
}

/*
 * Mirror of is_cashier_shuffled_deck for the settlement handshake
 * (docs/TODO.md item 1.4). The cashier signals payout completion by
 * writing G_SETTLEMENT_COMPLETE_BY_CASHIER on its own id; the dealer
 * canonicalizes G_SETTLEMENT_COMPLETE on table_id once this fires.
 */
static bool is_cashier_settlement_complete(char *cashier_id)
{
	int32_t game_state = get_game_state(cashier_id);
	return (game_state == G_SETTLEMENT_COMPLETE_BY_CASHIER);
}

/*
 * Showdown hand evaluation.
 *
 * Reads each non-folded player's hole cards from
 * P_HOLECARDS_REVEAL_KEY.<gid> on the player id (single-writer per
 * identity), reads the 5 community cards from T_BOARD_CARDS_KEY.<gid>
 * on the table id, builds a 7-card hand per player, and runs
 * poker.c::seven_card_draw_score to pick winners. Sets
 * vars->winners[i] = 1 for tied/best players and credits
 * vars->win_funds[i] = pot/num_winners (integer split; remainder is
 * dropped today — TODO: side-pot handling via det_dcv_pot_split).
 *
 * Returns:
 *   OK                         all non-folded players have published
 *                              hole cards and winners have been set;
 *   ERR_T_BOARD_CARDS_NULL     community cards not yet on the table id
 *                              (caller should retry on next tick);
 *   ERR_PLAYER_HOLECARDS_PEND  one or more players haven't published
 *                              hole cards yet (caller should retry).
 */
static int32_t dealer_evaluate_showdown(char *table_id, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	int32_t hole_cards[CARDS_MAXPLAYERS][2];
	int32_t board[5];
	int32_t folded[CARDS_MAXPLAYERS] = { 0 };
	int32_t active_players = 0;
	char *game_id_str = NULL;

	for (int32_t i = 0; i < CARDS_MAXPLAYERS; i++) {
		hole_cards[i][0] = -1;
		hole_cards[i][1] = -1;
	}
	for (int32_t i = 0; i < 5; i++) board[i] = -1;

	game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_error("Showdown: unable to resolve T_GAME_ID for table %s", table_id);
		return ERR_GAME_ID_NOT_FOUND;
	}

	/* Mark folded vs active across all rounds. A player is considered
	 * active if no fold action was recorded in any round. */
	for (int32_t i = 0; i < num_of_players; i++) {
		for (int32_t r = 0; r < CARDS_MAXROUNDS; r++) {
			if (vars->bet_actions[i][r] == fold) {
				folded[i] = 1;
				break;
			}
		}
		if (!folded[i]) active_players++;
	}

	if (active_players == 0) {
		dlg_warn("Showdown: no active players (all folded?) — skipping hand eval");
		return OK;
	}

	/* Edge case: only one active player — wins by default, no hand
	 * eval needed (and they would not have decoded all 5 community
	 * cards anyway since dealer aborts dealing once a single player
	 * remains; see G_REVEAL_CARD timeout path). */
	if (active_players == 1) {
		for (int32_t i = 0; i < num_of_players; i++) {
			if (!folded[i]) {
				vars->winners[i] = 1;
				vars->win_funds[i] = vars->pot;
				dlg_info("Showdown: only player %d active — wins pot %lld table chips",
					 i, (long long)vars->pot);
			}
		}
		return OK;
	}

	/* Read community cards from the table id (dealer-canonical). */
	{
		cJSON *board_cards = get_cJSON_from_id_key_vdxfid_from_height(
			table_id, get_key_data_vdxf_id(T_BOARD_CARDS_KEY, game_id_str), g_start_block);
		if (!board_cards) {
			dlg_info("Showdown: T_BOARD_CARDS_KEY not yet on table id");
			return ERR_T_BOARD_CARDS_NULL;
		}
		cJSON *flop = cJSON_GetObjectItem(board_cards, "flop");
		if (flop && cJSON_GetArraySize(flop) >= 3) {
			board[0] = jinti(flop, 0);
			board[1] = jinti(flop, 1);
			board[2] = jinti(flop, 2);
		}
		board[3] = jint(board_cards, "turn");
		board[4] = jint(board_cards, "river");
		cJSON_Delete(board_cards);
	}
	for (int32_t i = 0; i < 5; i++) {
		if (board[i] < 0) {
			dlg_info("Showdown: community card slot %d not yet revealed (got %d)", i, board[i]);
			return ERR_T_BOARD_CARDS_NULL;
		}
	}

	/* Read each active player's hole-card reveal from their own id. */
	for (int32_t i = 0; i < num_of_players; i++) {
		if (folded[i]) continue;
		cJSON *reveal = get_cJSON_from_id_key_vdxfid_from_height(
			player_ids[i],
			get_key_data_vdxf_id(P_HOLECARDS_REVEAL_KEY, game_id_str),
			g_start_block);
		if (!reveal) {
			dlg_info("Showdown: player %d (%s) has not published hole cards yet",
				 i, player_ids[i]);
			return ERR_PLAYER_HOLECARDS_PEND;
		}
		cJSON *hc = cJSON_GetObjectItem(reveal, "hole_cards");
		if (!hc || cJSON_GetArraySize(hc) < 2) {
			dlg_warn("Showdown: malformed hole-card reveal from player %d", i);
			cJSON_Delete(reveal);
			return ERR_PLAYER_HOLECARDS_PEND;
		}
		hole_cards[i][0] = jinti(hc, 0);
		hole_cards[i][1] = jinti(hc, 1);
		cJSON_Delete(reveal);
	}

	/* Build the 7-card hand per active player and score it.
	 * card values land in 0..51 via card_rand256(privkeyflag, i)
	 * which packs the deck index into priv.bytes[30] — that index is
	 * what reveal_card writes into card_value, and what
	 * poker.c::CardMask[52] expects. */
	unsigned long scores[CARDS_MAXPLAYERS] = { 0 };
	unsigned long max_score = 0;
	int32_t num_winners = 0;

	for (int32_t i = 0; i < num_of_players; i++) {
		if (folded[i]) continue;
		unsigned char h[7];
		h[0] = (unsigned char)hole_cards[i][0];
		h[1] = (unsigned char)hole_cards[i][1];
		h[2] = (unsigned char)board[0];
		h[3] = (unsigned char)board[1];
		h[4] = (unsigned char)board[2];
		h[5] = (unsigned char)board[3];
		h[6] = (unsigned char)board[4];
		scores[i] = seven_card_draw_score(h);
		dlg_info("Showdown: player %d (%s) hand=[%d,%d,%d,%d,%d,%d,%d] score=%lu",
			 i, player_ids[i], h[0], h[1], h[2], h[3], h[4], h[5], h[6], scores[i]);
		if (scores[i] > max_score) max_score = scores[i];
	}

	for (int32_t i = 0; i < num_of_players; i++) {
		if (folded[i]) continue;
		if (scores[i] == max_score) {
			vars->winners[i] = 1;
			num_winners++;
		}
	}

	/* Pot split. Integer division — remainder is dropped (no odd-chip
	 * rule yet). Side-pot generalization for all-in scenarios is
	 * tracked in docs/TODO.md. */
	if (num_winners > 0) {
		int64_t share = vars->pot / num_winners;
		for (int32_t i = 0; i < num_of_players; i++) {
			if (vars->winners[i] == 1) {
				vars->win_funds[i] = share;
				dlg_info("Showdown winner: player %d (%s) score=%lu wins %lld table chips",
					 i, player_ids[i], scores[i], (long long)share);
			}
		}
	}

	return retval;
}

int32_t dealer_shuffle_deck(char *id)
{
	int32_t retval = OK;
	char *game_id_str = NULL, str[65];
	cJSON *t_d_deck_info = NULL;
	bits256 t_p_r[CARDS_MAXCARDS];

	dealer_init_deck();
	game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);

	for (int32_t i = 0; i < num_of_players; i++) {
		cJSON *player_deck =
			get_cJSON_from_id_key_vdxfid_from_height(player_ids[i], get_key_data_vdxf_id(PLAYER_DECK_KEY, game_id_str), g_start_block);
		cJSON *cardinfo = cJSON_GetObjectItem(player_deck, "cardinfo");
		for (int32_t j = 0; j < cJSON_GetArraySize(cardinfo); j++) {
			t_p_r[j] = jbits256i(cardinfo, j);
		}
		retval = dealer_sb_deck(id, t_p_r, (i + 1));
		if (retval)
			return retval;
	}

	t_d_deck_info = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		jaddistr(t_d_deck_info, bits256_str(str, d_deck_info.dealer_r[i].prod));
	}
	dlg_info("Updating the key :: %s, which contains public points of dealer blinded values..", T_D_DECK_KEY);
	cJSON *out = poker_append_key_json(id, get_key_data_vdxf_id(T_D_DECK_KEY, game_id_str),
						       t_d_deck_info, true);
	if (!out)
		retval = ERR_DECK_BLINDING_DEALER;
	dlg_info("%s", cJSON_Print(out));

	// Save dealer deck info to local DB for rejoin capability
	if (retval == OK) {
		save_dealer_deck_info(game_id_str);
	}

	return retval;
}

// Initiate pot settlement after showdown
// Writes settlement info to table ID for cashier to process
int32_t dealer_initiate_settlement(struct table *t, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	char *game_id_str = NULL, hexstr[65];
	cJSON *settlement_info = NULL, *winners_arr = NULL, *amounts_arr = NULL;
	cJSON *player_ids_arr = NULL, *payin_txs_arr = NULL;
	
	dlg_info("=== INITIATING POT SETTLEMENT ===");
	
	game_id_str = poker_get_key_str(t->table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_error("Failed to get game_id for settlement");
		return ERR_GAME_ID_NOT_FOUND;
	}
	
	/* Build settlement info. All chip amounts on this CMM are integer
	 * table chips - the cashier converts back to CHIPS at sendcurrency
	 * time (see blinder.c::cashier_process_settlement). */
	settlement_info = cJSON_CreateObject();
	cJSON_AddStringToObject(settlement_info, "game_id", game_id_str);
	cJSON_AddStringToObject(settlement_info, "status", "pending");
	cJSON_AddNumberToObject(settlement_info, "pot", (double)vars->pot);
	
	// Winners array
	winners_arr = cJSON_CreateArray();
	for (int32_t i = 0; i < num_of_players; i++) {
		if (vars->winners[i] == 1) {
			cJSON_AddItemToArray(winners_arr, cJSON_CreateNumber(i));
		}
	}
	cJSON_AddItemToObject(settlement_info, "winners", winners_arr);
	
	/* Settlement amount per player (table chips):
	 * settle_amount = remaining stack + winnings from pot. */
	amounts_arr = cJSON_CreateArray();
	for (int32_t i = 0; i < num_of_players; i++) {
		int64_t amount = vars->win_funds[i] + vars->funds[i];
		cJSON_AddItemToArray(amounts_arr, cJSON_CreateNumber((double)amount));
	}
	cJSON_AddItemToObject(settlement_info, "settle_amounts", amounts_arr);
	
	// Player IDs (Verus IDs for payouts)
	player_ids_arr = cJSON_CreateArray();
	for (int32_t i = 0; i < num_of_players; i++) {
		cJSON_AddItemToArray(player_ids_arr, cJSON_CreateString(player_ids[i]));
	}
	cJSON_AddItemToObject(settlement_info, "player_ids", player_ids_arr);
	
	// Get payin TXs from t_player_info
	cJSON *t_player_info = get_cJSON_from_id_key_vdxfid_from_height(t->table_id,
		get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str), g_start_block);
	if (t_player_info) {
		cJSON *player_info_arr = cJSON_GetObjectItem(t_player_info, "player_info");
		payin_txs_arr = cJSON_CreateArray();
		if (player_info_arr) {
			for (int32_t i = 0; i < cJSON_GetArraySize(player_info_arr); i++) {
				cJSON *item = cJSON_GetArrayItem(player_info_arr, i);
				if (item && item->valuestring) {
					// Format: "p1_<payin_tx>_<slot>" - extract payin_tx
					char *str = item->valuestring;
					char *first_underscore = strchr(str, '_');
					if (first_underscore) {
						char *second_underscore = strchr(first_underscore + 1, '_');
						if (second_underscore) {
							size_t tx_len = second_underscore - (first_underscore + 1);
							char tx[128] = {0};
							strncpy(tx, first_underscore + 1, tx_len < 127 ? tx_len : 127);
							cJSON_AddItemToArray(payin_txs_arr, cJSON_CreateString(tx));
						}
					}
				}
			}
		}
		cJSON_AddItemToObject(settlement_info, "payin_txs", payin_txs_arr);
	}
	
	// Add cashier ID
	cJSON_AddStringToObject(settlement_info, "cashier_id", t->cashier_id);
	
	dlg_info("Settlement info: %s", cJSON_Print(settlement_info));
	
	// Write to table ID
	cJSON *out = poker_update_key_json(t->table_id, 
		get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, game_id_str),
		settlement_info, true);
	
	if (!out) {
		dlg_error("Failed to write settlement info to table ID");
		return ERR_GAME_STATE_UPDATE;
	}
	
	dlg_info("Settlement info written to table ID, waiting for cashier to process");
	
	// Update game state to settlement pending
	append_game_state(t->table_id, G_SETTLEMENT_PENDING, NULL);
	
	return retval;
}

int32_t handle_game_state(struct table *t)
{
	int32_t game_state, retval = OK;
	static int32_t last_logged_state = -1;

	if (!t) {
		return ERR_ARGS_NULL;
	}

	game_state = get_game_state(t->table_id);
	if (game_state != last_logged_state) {
		dlg_info("%s", game_state_str(game_state));
		last_logged_state = game_state;
	}
	switch (game_state) {
	case G_TABLE_STARTED:
		// Poll player identities for pending join requests (since start_block).
		// The cashier id is passed as a verifier — used only to confirm that
		// each player's claimed payin_tx actually landed on-chain.
		if (t->cashier_id[0] != '\0') {
			int32_t start = (t->start_block > 0) ? t->start_block : 1;
			int32_t joins = poker_poll_players_for_joins(t->cashier_id, t->table_id, 
			                                              t->dealer_id, start);
			if (joins > 0) {
				dlg_info("Processed %d join requests", joins);
			}
		}
		// Check if enough players have joined
		if (poker_is_table_full(t->table_id)) {
			/* Final join-phase write to t1: do a merge-mode (full
			 * snapshot) update so the table id ends G_TABLE_STARTED
			 * with all four bootstrap keys (T_GAME_ID, T_TABLE_INFO,
			 * T_GAME_INFO, T_PLAYER_INFO) visible in the latest
			 * snapshot. After this, single-key append_game_state
			 * writes are safe because every participant already
			 * knows start_block.
			 */
			char *game_id_str = poker_get_key_str(t->table_id, T_GAME_ID_KEY);
			if (game_id_str) {
				cJSON *t_game_info = cJSON_CreateObject();
				cJSON_AddNumberToObject(t_game_info, "game_state", G_PLAYERS_JOINED);
				merge_cmm_from_id_key_data_cJSON(t->table_id, t->start_block,
								 get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str),
								 t_game_info, true);
				cJSON_Delete(t_game_info);
			} else {
				dlg_warn("Could not read T_GAME_ID for merge-mode G_PLAYERS_JOINED write; "
					 "falling back to append-mode");
				append_game_state(t->table_id, G_PLAYERS_JOINED, NULL);
			}
		}
		break;
	case G_PLAYERS_JOINED:
		if (is_players_shuffled_deck(t->table_id))
			append_game_state(t->table_id, G_DECK_SHUFFLING_P, NULL);
		break;
	case G_DECK_SHUFFLING_P:
		retval = dealer_shuffle_deck(t->table_id);
		if (!retval)
			append_game_state(t->table_id, G_DECK_SHUFFLING_D, NULL);
		break;
	case G_DECK_SHUFFLING_D:
		// Wait for cashier to finish shuffling
		if (is_cashier_shuffled_deck(t->cashier_id)) {
			dlg_info("Cashier shuffle complete, updating table state");
			append_game_state(t->table_id, G_DECK_SHUFFLING_B, NULL);
		}
		break;
	case G_DECK_SHUFFLING_B:
		dlg_info("Its time for game");
		retval = init_game_state(t->table_id);
		break;
	case G_REVEAL_CARD: {
		cJSON *gsi = get_game_state_info(t->table_id);
		int32_t is_batch = (gsi && cJSON_GetObjectItem(gsi, "requests")) ? 1 : 0;
		if (is_batch) {
			retval = is_reveal_batch_complete(t->table_id);
			if (retval == OK) {
				const char *phase = jstr(gsi, "phase");
				dlg_info("%s reveal batch complete", phase ? phase : "?");
				retval = verus_receive_reveal_batch(t->table_id, dcv_vars);
			} else {
				retval = OK;
			}
			break;
		}
		retval = is_card_drawn(t->table_id);
		if (retval == OK) {
			dlg_info("Card is drawn");
			retval = verus_receive_card(t->table_id, dcv_vars);
		} else if (retval == ERR_PLAYER_TIMEOUT) {
			// Player timed out - continue game with remaining players
			dlg_warn("Player timed out during card reveal - continuing with remaining players");
			// Check if enough players remain
			int32_t players_left = 0;
			for (int i = 0; i < num_of_players; i++) {
				if (dcv_vars->bet_actions[i][0] != fold) players_left++;
			}
			if (players_left < 2) {
				dlg_info("Only 1 player remaining - they win by default");
				append_game_state(t->table_id, G_SHOWDOWN, NULL);
			} else {
				// Continue dealing to next player
				retval = verus_receive_card(t->table_id, dcv_vars);
			}
			retval = OK;  // Don't propagate timeout as error - game continues
		}
		break;
	}
	case G_ROUND_BETTING:
		// Poll current player for their betting action
		retval = verus_handle_round_betting(t->table_id, dcv_vars);
		break;
	case G_SHOWDOWN:
		/* Showdown — collect each non-folded player's hole-card reveal
		 * (P_HOLECARDS_REVEAL_KEY on player id, single-writer-per-id)
		 * + community cards (T_BOARD_CARDS_KEY on table id), score
		 * 7-card hands via poker.c::seven_card_draw_score, set
		 * winners[] / win_funds[], and initiate settlement.
		 *
		 * If any input is not yet on-chain (board incomplete or some
		 * player hasn't published yet), return OK so the dealer loop
		 * retries on the next tick rather than escalating into a
		 * failed-game path. Settlement is only initiated once eval
		 * succeeds, so this gating also prevents emitting a stale/
		 * incomplete settlement record. */
		{
			static int32_t last_pending_log = -1;
			int32_t eval_rc = dealer_evaluate_showdown(t->table_id, dcv_vars);
			if (eval_rc == ERR_T_BOARD_CARDS_NULL ||
			    eval_rc == ERR_PLAYER_HOLECARDS_PEND) {
				if (last_pending_log != eval_rc) {
					dlg_info("Showdown: %s — retrying", bet_err_str(eval_rc));
					last_pending_log = eval_rc;
				}
				retval = OK;
				break;
			}
			last_pending_log = -1;
			if (eval_rc != OK) {
				dlg_error("Showdown evaluation failed: %s", bet_err_str(eval_rc));
				retval = eval_rc;
				break;
			}
			retval = dealer_initiate_settlement(t, dcv_vars);
		}
		break;
	case G_SETTLEMENT_PENDING:
		/* docs/TODO.md items 1.3 + 1.4: dealer canonicalizes the cashier's
		 * settlement result onto the table id once it observes the
		 * cashier-side terminal-state signal. Two-step canonicalize:
		 *   (1) Read C_SETTLEMENT_RESULT_KEY.<gid> from cashier_id;
		 *       copy {status, payout_txs} onto T_SETTLEMENT_INFO_KEY.<gid>
		 *       on table_id (preserving dealer-written order fields).
		 *   (2) Append canonical G_SETTLEMENT_COMPLETE on table_id.
		 * Mirrors the existing G_DECK_SHUFFLING_B handshake. */
		if (is_cashier_settlement_complete(t->cashier_id)) {
			char *gid = poker_get_key_str(t->table_id, T_GAME_ID_KEY);
			if (gid) {
				cJSON *c_result = get_cJSON_from_id_key_vdxfid_from_height(
					t->cashier_id,
					get_key_data_vdxf_id(C_SETTLEMENT_RESULT_KEY, gid),
					g_start_block);
				cJSON *t_settlement = get_cJSON_from_id_key_vdxfid_from_height(
					t->table_id,
					get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, gid),
					g_start_block);
				if (c_result && t_settlement) {
					cJSON *updated = cJSON_Duplicate(t_settlement, 1);
					cJSON_DeleteItemFromObject(updated, "status");
					const char *new_status = jstr(c_result, "status");
					cJSON_AddStringToObject(updated, "status",
								new_status ? new_status : "completed");
					cJSON *payout_txs = cJSON_GetObjectItem(c_result, "payout_txs");
					if (payout_txs) {
						cJSON_DeleteItemFromObject(updated, "payout_txs");
						cJSON_AddItemToObject(updated, "payout_txs",
								      cJSON_Duplicate(payout_txs, 1));
					}
					cJSON *out = poker_update_key_json(t->table_id,
						get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, gid),
						updated, true);
					if (!out) {
						dlg_error("Failed to canonicalize T_SETTLEMENT_INFO_KEY on table id");
					}
					cJSON_Delete(updated);
				} else {
					dlg_warn("Cashier signalled but could not read %s on cashier or %s on table",
						 c_result ? "T_SETTLEMENT_INFO_KEY" : "C_SETTLEMENT_RESULT_KEY",
						 t_settlement ? "(other)" : "T_SETTLEMENT_INFO_KEY");
				}
				if (c_result) cJSON_Delete(c_result);
				if (t_settlement) cJSON_Delete(t_settlement);
			} else {
				dlg_warn("Cashier signalled but no T_GAME_ID on table id; cannot canonicalize result");
			}
			dlg_info("Cashier settlement complete signal observed, advancing table to G_SETTLEMENT_COMPLETE");
			append_game_state(t->table_id, G_SETTLEMENT_COMPLETE, NULL);
		}
		break;
	case G_SETTLEMENT_COMPLETE:
		dlg_info("Settlement complete - game finished!");
		// Could reset table for next game here
		break;
	}
	return retval;
}

int32_t register_table(struct table t)
{
	int32_t retval = OK;
	cJSON *d_table_info = NULL, *out = NULL;

	d_table_info = poker_get_key_json(t.dealer_id, T_TABLE_INFO_KEY);
	if (d_table_info == NULL) {
		out = poker_update_key_json(t.dealer_id, get_vdxf_id(T_TABLE_INFO_KEY),
							struct_table_to_cJSON(&t), true);
		if (!out)
			retval = ERR_RESERVED;
	}
	return retval;
}

// Reset the table to start a fresh game
// This generates a new game_id and sets start_block to current block
// All updates are done in a SINGLE transaction to avoid UTXO conflicts
int32_t dealer_reset_table(struct table *t)
{
	int32_t retval = OK;
	char hexstr[65], *hex_data = NULL;
	cJSON *cmm = NULL, *t_game_info = NULL, *t_table_info = NULL, *out = NULL;
	cJSON *data_obj = NULL;
	char *game_id_vdxfid = NULL, *table_info_vdxfid = NULL;
	char *bytevector_vdxfid = NULL, *game_id_key_vdxfid = NULL;

	dlg_info("=== RESETTING TABLE FOR FRESH GAME ===");

	// Generate new game_id
	game_id = rand256(0);
	bits256_str(hexstr, game_id);
	dlg_info("New game_id: %s", hexstr);

	// Set new start_block
	t->start_block = chips_get_block_count();
	g_start_block = t->start_block;  // Set global for CMM reads
	dlg_info("New start_block: %d", t->start_block);

	// Build a single CMM with all keys for atomic update
	cmm = cJSON_CreateObject();
	bytevector_vdxfid = get_vdxf_id(BYTEVECTOR_VDXF_ID);
	
	// Key 1: T_GAME_ID_KEY = hexstr (game_id)
	game_id_key_vdxfid = get_vdxf_id(T_GAME_ID_KEY);
	data_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hexstr);
	cJSON_AddItemToObject(cmm, game_id_key_vdxfid, data_obj);

	// Key 2: T_TABLE_INFO_KEY.<game_id> = table info JSON
	table_info_vdxfid = get_key_data_vdxf_id(T_TABLE_INFO_KEY, hexstr);
	t_table_info = struct_table_to_cJSON(t);
	cJSON_hex(t_table_info, &hex_data);
	data_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hex_data);
	cJSON_AddItemToObject(cmm, table_info_vdxfid, data_obj);
	free(hex_data);
	hex_data = NULL;
	cJSON_Delete(t_table_info);

	// Key 3: T_GAME_INFO_KEY.<game_id> = {game_state: G_TABLE_STARTED}
	game_id_vdxfid = get_key_data_vdxf_id(T_GAME_INFO_KEY, hexstr);
	t_game_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(t_game_info, "game_state", G_TABLE_STARTED);
	cJSON_hex(t_game_info, &hex_data);
	data_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(data_obj, bytevector_vdxfid, hex_data);
	cJSON_AddItemToObject(cmm, game_id_vdxfid, data_obj);
	free(hex_data);
	cJSON_Delete(t_game_info);

	// Single atomic update
	dlg_info("Updating table with all reset data in single transaction...");
	out = update_cmm(t->table_id, cmm);
	cJSON_Delete(cmm);
	
	if (!out) {
		dlg_error("Failed to update table CMM");
		return ERR_TABLE_LAUNCH;
	}
	dlg_info("Table CMM updated successfully");

	// Clear player_ids
	for (int32_t i = 0; i < CARDS_MAXPLAYERS; i++) {
		memset(player_ids[i], 0, MAX_ID_LEN);
	}
	num_of_players = 0;

	dlg_info("Table reset complete. Waiting for players to join...");
	return retval;
}

int32_t dealer_init(struct table t)
{
	int32_t retval = OK;
	double balance = 0;

	balance = chips_get_balance();
	if (balance < RESERVE_AMOUNT) {
		dlg_info("Wallet balance ::%f, Minimum funds needed to host a table :: %f", balance, RESERVE_AMOUNT);
		return ERR_CHIPS_INSUFFICIENT_FUNDS;
	}
	if ((!id_cansignfor(t.dealer_id, &retval)) || (!id_cansignfor(t.table_id, &retval))) {
		return retval;
	}

	/* TEMP_BYPASS_DEALER_REGISTRATION: Uncomment when dealer auto-registration is ready
	if (!is_dealer_registered(t.dealer_id)) {
		// TODO:: An automated mechanism to register the dealer with dealer.sg777z.chips.vrsc@ need to be worked out
		return ERR_DEALER_UNREGISTERED;
	}
	*/

	if (is_table_registered(t.table_id, t.dealer_id)) {
		dlg_info("Table::%s is already registered with the dealer ::%s", t.table_id, t.dealer_id);
	} else {
		// TODO:: At the moment only one table we are registering with the dealer, if any other table exists it will be replaced with new table info
		retval = register_table(t);
		if (retval) {
			dlg_error("Table::%s, registration at dealer::%s is failed", t.table_id, t.dealer_id);
			return retval;
		}
	}

	retval = dealer_table_init(&t);
	if (retval != OK) {
		dlg_info("Table Init is failed");
		return retval;
	}

	dlg_info("Dealer ready. Table: %s, Dealer: %s, Cashier: %s", t.table_id, t.dealer_id, t.cashier_id);
	dlg_info("Waiting for players to join via cashier...");
	
	while (1) {
		retval = handle_game_state(&t);
		if (retval)
			return retval;
		sleep(2);
	}
	return retval;
}

// Dealer init with table reset - starts a fresh game
int32_t dealer_init_with_reset(struct table t)
{
	int32_t retval = OK;
	double balance = 0;

	balance = chips_get_balance();
	if (balance < RESERVE_AMOUNT) {
		dlg_info("Wallet balance ::%f, Minimum funds needed to host a table :: %f", balance, RESERVE_AMOUNT);
		return ERR_CHIPS_INSUFFICIENT_FUNDS;
	}
	if ((!id_cansignfor(t.dealer_id, &retval)) || (!id_cansignfor(t.table_id, &retval))) {
		return retval;
	}

	/* TEMP_BYPASS_DEALER_REGISTRATION: Uncomment when dealer auto-registration is ready
	if (!is_dealer_registered(t.dealer_id)) {
		return ERR_DEALER_UNREGISTERED;
	}
	*/

	if (!is_table_registered(t.table_id, t.dealer_id)) {
		retval = register_table(t);
		if (retval) {
			dlg_error("Table::%s, registration at dealer::%s is failed", t.table_id, t.dealer_id);
			return retval;
		}
	}

	// Reset the table for a fresh game
	retval = dealer_reset_table(&t);
	if (retval != OK) {
		dlg_error("Table reset failed");
		return retval;
	}

	dlg_info("Dealer ready (RESET). Table: %s, Dealer: %s, Cashier: %s", t.table_id, t.dealer_id, t.cashier_id);
	dlg_info("Waiting for players to join via cashier...");
	
	while (1) {
		retval = handle_game_state(&t);
		if (retval)
			return retval;
		sleep(2);
	}
	return retval;
}
