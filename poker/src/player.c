#include "bet.h"
#include "player.h"
#include "client.h"
#include "err.h"
#include "misc.h"
#include "commands.h"
#include "deck.h"
#include "game.h"
#include "config.h"
#include "print.h"
#include "poker_vdxf.h"
#include "storage.h"
#include "vdxf.h"
#include "gui.h"
#include <time.h>
#include <errno.h>

extern int32_t g_start_block;
struct p_deck_info_struct p_deck_info;

// Card names for display
static const char *card_names[52] = {
	"2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "10♣", "J♣", "Q♣", "K♣", "A♣",
	"2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "10♦", "J♦", "Q♦", "K♦", "A♦",
	"2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "10♥", "J♥", "Q♥", "K♥", "A♥",
	"2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "10♠", "J♠", "Q♠", "K♠", "A♠"
};

static const char *get_card_name(int32_t card_value) {
	if (card_value >= 0 && card_value < 52) return card_names[card_value];
	return "??";
}

static const char *get_card_type_name(int32_t card_type) {
	switch (card_type) {
		case 1: return "HOLE CARD";
		case 2: return "FLOP 1";
		case 3: return "FLOP 2";
		case 4: return "FLOP 3";
		case 5: return "TURN";
		case 6: return "RIVER";
		default: return "UNKNOWN";
	}
}

// Player initialization state tracking
int32_t player_init_state = 0;

const char *player_init_state_str(int32_t state)
{
	switch (state) {
		case P_INIT_WALLET_READY: return "WALLET_READY";
		case P_INIT_TABLE_FOUND:  return "TABLE_FOUND";
		case P_INIT_WAIT_JOIN:    return "WAIT_JOIN";
		case P_INIT_JOINING:      return "JOINING";
		case P_INIT_JOINED:       return "JOINED";
		case P_INIT_DECK_READY:   return "DECK_READY";
		case P_INIT_IN_GAME:      return "IN_GAME";
		default:                  return "UNKNOWN";
	}
}

int32_t player_init_deck()
{
	int32_t retval = OK;
	char str[65], game_id_str[65];
	cJSON *cjson_player_cards = NULL, *player_deck = NULL;

	// Player ID numbering range from [0...8]
	if ((p_deck_info.player_id < 0) || (p_deck_info.player_id > 8))
		return ERR_INVALID_PLAYER_ID;

	p_deck_info.p_kp = gen_keypair();

	gen_deck(p_deck_info.player_r, CARDS_MAXCARDS);

	// Save deck info to local DB immediately after generation (for rejoin support)
	bits256_str(game_id_str, p_deck_info.game_id);
	retval = save_player_deck_info(game_id_str, player_config.table_id, p_deck_info.player_id);
	if (retval != OK) {
		dlg_error("Failed to save player deck info to local DB");
		// Continue anyway - this is non-fatal for first run
	}

	player_deck = cJSON_CreateObject();
	jaddnum(player_deck, "id", p_deck_info.player_id);
	jaddbits256(player_deck, "pubkey", p_deck_info.p_kp.prod);
	jadd(player_deck, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		jaddistr(cjson_player_cards, bits256_str(str, p_deck_info.player_r[i].prod));
	}

	dlg_info("Updating %s key...", T_GAME_ID_KEY);
	cJSON *out = poker_append_key_hex(player_config.verus_pid, T_GAME_ID_KEY,
					     bits256_str(str, p_deck_info.game_id), false);
	if (!out) {
		dlg_error("Failed to write %s to %s", T_GAME_ID_KEY, player_config.verus_pid);
		return ERR_UPDATEIDENTITY;
	}

	dlg_info("Updating %s key...", PLAYER_DECK_KEY);
	out = poker_append_key_json(
		player_config.verus_pid, get_key_data_vdxf_id(PLAYER_DECK_KEY, bits256_str(str, p_deck_info.game_id)),
		player_deck, true);
	if (!out) {
		dlg_error("Failed to write %s to %s", PLAYER_DECK_KEY, player_config.verus_pid);
		return ERR_UPDATEIDENTITY;
	}
	dlg_info("%s", cJSON_Print(out));

	dlg_info("Updating player game state to player_id...");
	out = append_game_state(player_config.verus_pid, G_DECK_SHUFFLING_P, NULL);
	if (!out)
		return ERR_GAME_STATE_UPDATE;
	dlg_info("%s", cJSON_Print(out));

	return OK;
}

int32_t decode_card(bits256 b_blinded_card, bits256 blinded_value, cJSON *dealer_blind_info)
{
	int32_t card_value = -1;
	char str1[65], str2[65];
	bits256 blinded_value_inv, d_blinded_card;

	blinded_value_inv = crecip_donna(blinded_value);
	d_blinded_card = fmul_donna(blinded_value_inv, b_blinded_card);

	dlg_info("Dealer blinded card :: %s", bits256_str(str1, d_blinded_card));

	for (int32_t i = 0; i < CARDS_MAXCARDS; i++) {
		for (int32_t j = 0; j < CARDS_MAXCARDS; j++) {
			if (strcmp(bits256_str(str1, d_blinded_card),
				   bits256_str(str2, curve25519(p_deck_info.player_r[i].priv,
								jbits256i(dealer_blind_info, j)))) == 0) {
				card_value = p_deck_info.player_r[i].priv.bytes[30];
				dlg_info("card::%x\n", p_deck_info.player_r[i].priv.bytes[30]);
			}
		}
	}
	return card_value;
}

// Check if card_type is a community card (board card)
static bool is_community_card(int32_t card_type)
{
	return (card_type == flop_card_1 || card_type == flop_card_2 || card_type == flop_card_3 ||
		card_type == turn_card || card_type == river_card);
}

/*
 * Report a freshly decoded community card to the player's own identity
 * under P_DECODED_CARD_KEY.<game_id>.
 *
 * The CMM entry is a cumulative snapshot of every community card this
 * player has decoded so far for the current game (schema documented
 * on P_DECODED_CARD_KEY in vdxf.h). On each call we:
 *   1. Read the latest existing snapshot for this game (if any).
 *   2. Replace any pre-existing entry for the same card_type with the
 *      new value, otherwise append.
 *   3. Re-publish the full snapshot.
 *
 * This lets the dealer's update_board_cards consensus check find any
 * card_type by scanning the latest entry's `decoded_cards` array,
 * rather than having to walk multiple per-card CMM entries.
 */
static int32_t report_decoded_card(int32_t card_id, int32_t card_type, int32_t card_value)
{
	char str[65];
	cJSON *snapshot = NULL, *decoded_arr = NULL, *out = NULL;
	char *gid = bits256_str(str, p_deck_info.game_id);
	const char *key_vdxfid = get_key_data_vdxf_id(P_DECODED_CARD_KEY, gid);

	/* Pull the existing snapshot (or build a fresh one). */
	snapshot = get_cJSON_from_id_key_vdxfid_from_height(
		player_config.verus_pid, key_vdxfid, g_start_block);
	if (!snapshot) {
		snapshot = cJSON_CreateObject();
		cJSON_AddStringToObject(snapshot, "game_id", gid);
		decoded_arr = cJSON_CreateArray();
		cJSON_AddItemToObject(snapshot, "decoded_cards", decoded_arr);
	} else {
		decoded_arr = cJSON_GetObjectItem(snapshot, "decoded_cards");
		if (!decoded_arr) {
			decoded_arr = cJSON_CreateArray();
			cJSON_AddItemToObject(snapshot, "decoded_cards", decoded_arr);
		}
		/* If a previous entry exists for this card_type, drop it so
		 * the new value supersedes (idempotent re-decode). */
		for (int i = cJSON_GetArraySize(decoded_arr) - 1; i >= 0; i--) {
			cJSON *e = cJSON_GetArrayItem(decoded_arr, i);
			if (e && jint(e, "card_type") == card_type) {
				cJSON_DeleteItemFromArray(decoded_arr, i);
			}
		}
	}

	cJSON *new_entry = cJSON_CreateObject();
	cJSON_AddNumberToObject(new_entry, "card_id", card_id);
	cJSON_AddNumberToObject(new_entry, "card_type", card_type);
	cJSON_AddNumberToObject(new_entry, "card_value", card_value);
	cJSON_AddItemToArray(decoded_arr, new_entry);

	out = poker_append_key_json(player_config.verus_pid, key_vdxfid, snapshot, true);
	if (!out) {
		dlg_error("Failed to report decoded card to player ID");
		cJSON_Delete(snapshot);
		return ERR_UPDATEIDENTITY;
	}

	dlg_info("Reported decoded card snapshot to player ID: card_id=%d, card_type=%d, value=%d (snapshot now has %d card(s))",
		 card_id, card_type, card_value, cJSON_GetArraySize(decoded_arr));
	cJSON_Delete(snapshot);
	return OK;
}

static cJSON *find_bv_in_batch(cJSON *bv_info, int32_t player_id, int32_t card_id)
{
	cJSON *bvs = bv_info ? cJSON_GetObjectItem(bv_info, "bvs") : NULL;
	if (!bvs || bvs->type != cJSON_Array) return NULL;
	for (int i = 0; i < cJSON_GetArraySize(bvs); i++) {
		cJSON *e = cJSON_GetArrayItem(bvs, i);
		if (e && jint(e, "player_id") == player_id &&
		    jint(e, "card_id") == card_id)
			return e;
	}
	return NULL;
}

int32_t reveal_card_for_ex(char *table_id, int32_t player_id, int32_t card_id, int32_t card_type,
			   int32_t do_report, int32_t *out_value)
{
	int32_t retval = OK, card_value = -1;
	char *game_id_str = NULL, str[65];
	cJSON *bv_info = NULL, *b_blinded_deck = NULL, *dealer_blind_info = NULL, *bv = NULL;
	bits256 b_blinded_card, blinded_value;

	if ((player_id == p_deck_info.player_id) || (player_id == -1)) {
		// Check if we already decoded this card (from local state)
		if (card_id < hand_size && p_local_state.decoded_cards[card_id] >= 0) {
			dlg_info("Card %d already decoded from local state: value=%d", 
				card_id, p_local_state.decoded_cards[card_id]);
			return OK;
		}

		game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);

		/* Resolve cashier_id once via T_TABLE_INFO_KEY on table_id (dealer
		 * writes it at game setup). Used by both the BV read (item 1.2)
		 * and the deck read (item 1.1) below — single-writer-per-identity
		 * means both live on the cashier id, not on the table id. */
		char cashier_id[128] = {0};
		{
			cJSON *t_table_info = get_cJSON_from_id_key_vdxfid_from_height(
				table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str), g_start_block);
			const char *cid = jstr(t_table_info, "cashier_id");
			if (!cid) {
				dlg_error("reveal_card: cashier_id missing in T_TABLE_INFO_KEY for game %s",
					  game_id_str);
				if (t_table_info) cJSON_Delete(t_table_info);
				return ERR_GAME_STATE_UPDATE;
			}
			strncpy(cashier_id, cid, sizeof(cashier_id) - 1);
			cJSON_Delete(t_table_info);
		}

		// For community cards, first check if already revealed on table ID
		if (is_community_card(card_type)) {
			cJSON *board_cards = get_cJSON_from_id_key_vdxfid_from_height(table_id,
				get_key_data_vdxf_id(T_BOARD_CARDS_KEY, game_id_str), g_start_block);
			if (board_cards) {
				int32_t existing_value = -1;
				switch (card_type) {
				case flop_card_1:
					existing_value = jinti(jobj(board_cards, "flop"), 0);
					break;
				case flop_card_2:
					existing_value = jinti(jobj(board_cards, "flop"), 1);
					break;
				case flop_card_3:
					existing_value = jinti(jobj(board_cards, "flop"), 2);
					break;
				case turn_card:
					existing_value = jint(board_cards, "turn");
					break;
				case river_card:
					existing_value = jint(board_cards, "river");
					break;
				}
				if (existing_value >= 0) {
					dlg_info("Board card already revealed on table: type=%d, value=%d",
						card_type, existing_value);
					// Save to local state
					if (card_id < hand_size) {
						update_player_decoded_card(card_id, existing_value);
					}
					/* Mirror in community_cards[] keyed by board position
					 * so the GUI emit path doesn't need to interpret the
					 * global card_id (see comment on community_cards in
					 * struct p_local_state_struct). */
					int32_t pos = card_type - flop_card_1;
					if (pos >= 0 && pos < 5) {
						p_local_state.community_cards[pos] = existing_value;
					}
					return OK;
				}
			}
		}

		while (1) {
			bv_info = get_cJSON_from_id_key_vdxfid_from_height(cashier_id,
							       get_key_data_vdxf_id(C_CARD_BV_KEY, game_id_str), g_start_block);
			if (!bv_info) {
				dlg_info("BV INFO hasn't revealed its secret yet");
				wait_for_a_blocktime();
				continue;
			}
			cJSON *batch_entry = find_bv_in_batch(bv_info, player_id, card_id);
			if (batch_entry) {
				bv = cJSON_GetObjectItem(batch_entry, "bv");
				if (bv) break;
			}
			bv = jobj(bv_info, "bv");
			if (bv && jint(bv_info, "card_id") == card_id &&
			    jint(bv_info, "player_id") == player_id)
				break;
			wait_for_a_blocktime();
		}

		/* Read the blinded deck for this player slot directly from the
		 * cashier id under C_B_P*_DECK_KEY.<gid> (docs/TODO.md item 1.1).
		 * cashier_id was resolved once at the top of reveal_card. */
		int32_t deck_slot = (player_id == -1) ? p_deck_info.player_id : player_id;
		dlg_info("[DBG-PREAD] reading C_B_P%d_DECK_KEY from cashier_id=\"%s\" game_id=%s player_id=%d card_id=%d g_start_block=%d",
			 deck_slot + 1, cashier_id, game_id_str, player_id, card_id, g_start_block);
		b_blinded_deck = get_cJSON_from_id_key_vdxfid_from_height(
			cashier_id,
			get_key_data_vdxf_id(all_c_b_p_keys[deck_slot], game_id_str),
			g_start_block);
		if (b_blinded_deck) {
			char *deck_dbg = cJSON_PrintUnformatted(b_blinded_deck);
			if (deck_dbg) {
				dlg_info("[DBG-PREAD] b_blinded_deck=%s", deck_dbg);
				free(deck_dbg);
			}
		} else {
			dlg_error("[DBG-PREAD] b_blinded_deck is NULL");
		}
		b_blinded_card = jbits256i(b_blinded_deck, card_id);
		if (player_id == -1)
			blinded_value = jbits256i(bv, p_deck_info.player_id);
		else
			blinded_value = jbits256i(bv, 0);

		dlg_info("blinded_value::%s", bits256_str(str, blinded_value));
		dlg_info("blinded_card::%s", bits256_str(str, b_blinded_card));
		dealer_blind_info =
			get_cJSON_from_id_key_vdxfid_from_height(table_id, get_key_data_vdxf_id(T_D_DECK_KEY, game_id_str), g_start_block);
		dlg_info("dealer_blind_info::%s", cJSON_Print(dealer_blind_info));
		card_value = decode_card(b_blinded_card, blinded_value, dealer_blind_info);
		dlg_info("card_value ::%d", card_value);
		if (card_value == -1) {
			retval = ERR_CARD_DECODING_FAILED;
		} else {
			// ========== CARD REVEALED ==========
			dlg_info("╔════════════════════════════════════════╗");
			dlg_info("║  🃏 CARD REVEALED: %-20s ║", get_card_name(card_value));
			dlg_info("║  Type: %-10s  Card ID: %-10d ║", get_card_type_name(card_type), card_id);
			dlg_info("╚════════════════════════════════════════╝");
			
			// Save decoded card to local state
			if (card_id < hand_size) {
				update_player_decoded_card(card_id, card_value);
				dlg_info("Saved decoded card %d (type=%d) with value %d to local DB", 
					card_id, card_type, card_value);
			}

			// Send GUI message for each card reveal
			{
				if (card_type == hole_card) {
					/* Hole-card global card_id is (hole_index * num_players) + player_index,
					 * which is NOT the same as the per-player slot. Store hole cards by
					 * per-player slot in hole_cards[0..1] so the GUI deal message has both
					 * cards regardless of which player we are. Fire the GUI emit only once,
					 * when the second hole card lands.
					 */
					int32_t slot = -1;
					if (p_local_state.hole_cards[0] < 0)
						slot = 0;
					else if (p_local_state.hole_cards[1] < 0)
						slot = 1;

					if (slot >= 0) {
						p_local_state.hole_cards[slot] = card_value;
						dlg_info("Stored hole card slot %d = %s (global card_id %d)",
							slot, get_card_name(card_value), card_id);
					} else {
						dlg_warn("Both hole-card slots already filled; ignoring extra hole card_id %d",
							card_id);
					}

					if (p_local_state.hole_cards[0] >= 0 && p_local_state.hole_cards[1] >= 0) {
						dlg_info("Sending hole cards to GUI: %s, %s",
							get_card_name(p_local_state.hole_cards[0]),
							get_card_name(p_local_state.hole_cards[1]));
						cJSON *deal_msg = gui_build_deal_holecards(p_local_state.hole_cards[0],
											   p_local_state.hole_cards[1],
											   0.0);
						gui_send_message(deal_msg);
						cJSON_Delete(deal_msg);
					}
				} else if (is_community_card(card_type)) {
					/* Store this card by board position (0..4 for
					 * flop_1, flop_2, flop_3, turn, river) into the
					 * dedicated community_cards[] mirror. We CANNOT
					 * use decoded_cards[card_id] for the board because
					 * card_id is a global id whose slots overlap with
					 * hole-card slots for multi-player games (e.g. p1's
					 * second hole card lands at decoded_cards[2], which
					 * is also the implied "board slot 0" if you index
					 * by card_id - 2). See community_cards docs in
					 * struct p_local_state_struct. */
					int32_t pos = card_type - flop_card_1;
					if (pos >= 0 && pos < 5) {
						p_local_state.community_cards[pos] = card_value;
					}

					/* Build board[] from contiguous prefix of
					 * community_cards[]. The flop is dealt as three
					 * cards in order (positions 0..2), then the turn
					 * (3), then the river (4) - so we always emit a
					 * dense prefix without holes, in the order the GUI
					 * expects to render them on the felt. */
					int32_t board[5];
					int32_t board_count = 0;
					for (int i = 0; i < 5; i++) {
						if (p_local_state.community_cards[i] >= 0) {
							board[board_count++] = p_local_state.community_cards[i];
						} else {
							break;
						}
					}
					if (board_count > 0) {
						cJSON *deal_msg = gui_build_deal_board(board, board_count);
						gui_send_message(deal_msg);
						cJSON_Delete(deal_msg);
					}
				}
			}

			if (do_report && is_community_card(card_type)) {
				report_decoded_card(card_id, card_type, card_value);
			}
			if (out_value) *out_value = card_value;
		}
	}
	return retval;
}

int32_t reveal_card_for(char *table_id, int32_t player_id, int32_t card_id, int32_t card_type)
{
	return reveal_card_for_ex(table_id, player_id, card_id, card_type, 1, NULL);
}

int32_t reveal_card(char *table_id)
{
	cJSON *game_state_info = get_game_state_info(table_id);
	if (!game_state_info) return OK;
	int32_t player_id = jint(game_state_info, "player_id");
	int32_t card_id = jint(game_state_info, "card_id");
	int32_t card_type = jint(game_state_info, "card_type");
	return reveal_card_for(table_id, player_id, card_id, card_type);
}

struct decoded_entry {
	int32_t card_id;
	int32_t card_type;
	int32_t card_value;
};

static int32_t append_decoded_to_snapshot(struct decoded_entry *entries, int32_t n)
{
	char str[65];
	cJSON *snapshot = NULL, *decoded_arr = NULL, *out = NULL;
	char *gid = bits256_str(str, p_deck_info.game_id);
	const char *key_vdxfid = get_key_data_vdxf_id(P_DECODED_CARD_KEY, gid);

	snapshot = get_cJSON_from_id_key_vdxfid_from_height(
		player_config.verus_pid, key_vdxfid, g_start_block);
	if (!snapshot) {
		snapshot = cJSON_CreateObject();
		cJSON_AddStringToObject(snapshot, "game_id", gid);
		decoded_arr = cJSON_CreateArray();
		cJSON_AddItemToObject(snapshot, "decoded_cards", decoded_arr);
	} else {
		decoded_arr = cJSON_GetObjectItem(snapshot, "decoded_cards");
		if (!decoded_arr) {
			decoded_arr = cJSON_CreateArray();
			cJSON_AddItemToObject(snapshot, "decoded_cards", decoded_arr);
		}
	}

	int32_t added = 0;
	for (int k = 0; k < n; k++) {
		int32_t cid = entries[k].card_id;
		int32_t ctype = entries[k].card_type;
		int32_t cval = entries[k].card_value;
		int32_t already_idx = -1;
		for (int i = 0; i < cJSON_GetArraySize(decoded_arr); i++) {
			cJSON *e = cJSON_GetArrayItem(decoded_arr, i);
			if (e && jint(e, "card_type") == ctype &&
			    jint(e, "card_id") == cid) {
				already_idx = i;
				break;
			}
		}
		if (already_idx >= 0) {
			cJSON *e = cJSON_GetArrayItem(decoded_arr, already_idx);
			int32_t prev = jint(e, "card_value");
			if (cval >= 0 && prev != cval) {
				cJSON_DeleteItemFromObject(e, "card_value");
				cJSON_AddNumberToObject(e, "card_value", cval);
				added++;
			}
			continue;
		}
		cJSON *entry = cJSON_CreateObject();
		cJSON_AddNumberToObject(entry, "card_id", cid);
		cJSON_AddNumberToObject(entry, "card_type", ctype);
		if (cval >= 0)
			cJSON_AddNumberToObject(entry, "card_value", cval);
		cJSON_AddItemToArray(decoded_arr, entry);
		added++;
	}
	if (added == 0) {
		cJSON_Delete(snapshot);
		return OK;
	}
	out = poker_append_key_json(player_config.verus_pid, key_vdxfid, snapshot, true);
	cJSON_Delete(snapshot);
	return out ? OK : ERR_UPDATEIDENTITY;
}

static char g_last_decoded_phase[16];
static char g_last_decoded_gid[80];

static int32_t reveal_request_batch(char *table_id, cJSON *requests)
{
	int32_t my_id = p_deck_info.player_id;
	struct decoded_entry mine[CARDS_MAXCARDS];
	int32_t n = 0;

	for (int i = 0; i < cJSON_GetArraySize(requests); i++) {
		cJSON *e = cJSON_GetArrayItem(requests, i);
		int32_t epid = jint(e, "player_id");
		if (epid != my_id && epid != -1) continue;
		int32_t cid = jint(e, "card_id");
		int32_t ctype = jint(e, "card_type");
		int32_t cval = -1;
		int32_t r = reveal_card_for_ex(table_id, epid, cid, ctype, 0,
					       (ctype == hole_card) ? NULL : &cval);
		if (r != OK) return r;
		if (n < CARDS_MAXCARDS) {
			mine[n].card_id = cid;
			mine[n].card_type = ctype;
			mine[n].card_value = (ctype == hole_card) ? -1 : cval;
			n++;
		}
	}
	if (n == 0) return OK;
	return append_decoded_to_snapshot(mine, n);
}

static int32_t handle_player_reveal_card(char *table_id)
{
	int32_t retval = OK;
	cJSON *game_state_info = NULL, *player_game_state_info = NULL;

	game_state_info = get_game_state_info(table_id);
	if (!game_state_info)
		return retval;

	cJSON *requests = cJSON_GetObjectItem(game_state_info, "requests");
	if (requests && requests->type == cJSON_Array) {
		const char *cur_phase = jstr(game_state_info, "phase");
		char str[65];
		char *gid = bits256_str(str, p_deck_info.game_id);
		if (cur_phase && gid &&
		    g_last_decoded_phase[0] && g_last_decoded_gid[0] &&
		    strcmp(cur_phase, g_last_decoded_phase) == 0 &&
		    strcmp(gid, g_last_decoded_gid) == 0) {
			return OK;
		}
		if (cur_phase) {
			strncpy(g_last_decoded_phase, cur_phase, sizeof(g_last_decoded_phase) - 1);
			g_last_decoded_phase[sizeof(g_last_decoded_phase) - 1] = 0;
			strncpy(g_last_decoded_gid, gid, sizeof(g_last_decoded_gid) - 1);
			g_last_decoded_gid[sizeof(g_last_decoded_gid) - 1] = 0;
		}
		retval = reveal_request_batch(table_id, requests);
		if (retval != OK) {
			g_last_decoded_phase[0] = 0;
			g_last_decoded_gid[0] = 0;
		}
		return retval;
	}

	player_game_state_info = get_game_state_info(player_config.verus_pid);
	if (jint(game_state_info, "player_id") != p_deck_info.player_id) {
		dlg_info("Not this players turn...");
		return retval;
	}
	if ((!player_game_state_info) || (jint(game_state_info, "card_id") > jint(player_game_state_info, "card_id"))) {
		retval = reveal_card(table_id);
		if (retval == OK) {
			append_game_state(player_config.verus_pid, G_REVEAL_CARD_P_DONE, game_state_info);
			dlg_info("Updating player's revealed card info :: %s", cJSON_Print(game_state_info));
		}
	}
	return retval;
}

/**
 * Read betting state from table ID
 */
cJSON *player_read_betting_state(char *table_id)
{
	char *game_id_str = poker_get_key_str(table_id, T_GAME_ID_KEY);
	if (!game_id_str) return NULL;
	
	return get_cJSON_from_id_key_vdxfid_from_height(
		table_id,
		get_key_data_vdxf_id(T_BETTING_STATE_KEY, game_id_str),
		g_start_block);
}

/**
 * Write betting action to player ID
 * Amount is in CHIPS (e.g., 0.01, 0.02)
 */
int32_t player_write_betting_action(char *table_id, const char *action, int64_t amount, int32_t round)
{
	int32_t retval = OK;
	char game_id_str[65];
	cJSON *action_obj = NULL, *out = NULL;

	(void)table_id;  /* writes go to player's own verus_pid, not the table */

	bits256_str(game_id_str, p_deck_info.game_id);

	action_obj = cJSON_CreateObject();
	if (!action_obj) {
		dlg_error("cJSON_CreateObject failed for betting action");
		return ERR_MEMORY_ALLOC;
	}
	cJSON_AddStringToObject(action_obj, "action", action);
	/* amount is integer table chips. cJSON serializes integer-valued
	 * doubles without a decimal point, so the on-chain CMM payload is
	 * e.g. {"amount": 2} rather than {"amount": 0.02}. */
	cJSON_AddNumberToObject(action_obj, "amount", (double)amount);
	/* round MUST come from the dealer's T_BETTING_STATE_KEY (caller
	 * extracts it from betting_state.round). Previously this was set to
	 * p_local_state.last_game_state which is only assigned AFTER the
	 * action is written, so it always lagged by one round - the dealer
	 * then dropped the action as "old round" and auto-folded the player
	 * after 60 s. Pre-flop happened to work by accident because both
	 * values were 0 on the first hand. */
	cJSON_AddNumberToObject(action_obj, "round", round);

	/* Echo turn_start_time the player observed when reading the prompt.
	 * Dealer's verus_poll_player_action compares this against its current
	 * vars->turn_start_time and ignores stale actions. Without this echo
	 * the dealer would re-process the same on-chain action on every 2 s
	 * poll, leaking the bet amount into the pot indefinitely. */
	cJSON_AddNumberToObject(action_obj, "turn_start_time",
				(double)p_local_state.last_betting_turn_start_time);

	dlg_info("Writing betting action: %s, amount=%lld table chips (%.4f CHIPS), round=%d, tst=%lld",
		 action, (long long)amount, table_chips_to_chips(amount), round,
		 (long long)p_local_state.last_betting_turn_start_time);
	
	out = poker_update_key_json(player_config.verus_pid,
		get_key_data_vdxf_id(P_BETTING_ACTION_KEY, game_id_str),
		action_obj, true);
	
	cJSON_Delete(action_obj);
	
	if (!out) {
		return ERR_GAME_STATE_UPDATE;
	}
	
	return retval;
}

/**
 * Display betting options and get player input
 */
int32_t player_handle_betting(char *table_id)
{
	int32_t retval = OK;
	cJSON *betting_state = NULL;
	
	betting_state = player_read_betting_state(table_id);
	if (!betting_state) {
		return OK;  // No betting state yet, wait
	}
	
	int32_t current_turn = jint(betting_state, "current_turn");
	const char *action = jstr(betting_state, "action");
	int32_t round = jint(betting_state, "round");
	/* Dealer writes betting_state in integer table chips - cast through
	 * jdouble because cJSON only stores numbers as double. */
	int64_t pot = (int64_t)jdouble(betting_state, "pot");
	int64_t min_amount = (int64_t)jdouble(betting_state, "min_amount");
	int64_t turn_start_time = (int64_t)jdouble(betting_state, "turn_start_time");

	/* Change-detector gate: this function is called on every poll
	 * iteration of the main game loop. Without this gate, every iteration
	 * (a) re-prints the YOUR TURN / Waiting-for-Player block on the
	 * console, (b) re-emits the betting prompt to the GUI WebSocket - and
	 * the GUI then re-renders the betting controls every poll, causing
	 * visible flicker and stale messages piling up in the WS write queue.
	 *
	 * The dealer stamps a fresh turn_start_time into betting_state every
	 * time it writes a NEW prompt (verus_write_betting_state, game.c).
	 * So a turn_start_time we have NOT yet emitted == new prompt; same
	 * turn_start_time as last time == nothing changed, skip silently.
	 *
	 * Static is fine because player_handle_betting is called from a
	 * single thread/loop. Reset to 0 on game end is unnecessary - a new
	 * game will produce a strictly larger turn_start_time. */
	static int64_t last_emit_turn_start_time = 0;
	if (turn_start_time == last_emit_turn_start_time) {
		return OK;
	}
	last_emit_turn_start_time = turn_start_time;

	/* Cache for player_write_betting_action() to echo back. The dealer
	 * uses this to dedup on-chain actions: an action whose
	 * turn_start_time doesn't match the dealer's current betting_state
	 * is treated as stale and ignored. */
	p_local_state.last_betting_turn_start_time = turn_start_time;

	{
		const char *last_str = jstr(betting_state, "last_action_str");
		if (last_str && last_str[0]) {
			int64_t last_amt = (int64_t)jdouble(betting_state, "last_action_amount");
			int32_t last_pid = jint(betting_state, "last_action_player");
			cJSON *gm = NULL;
			if (strcmp(last_str, "small_blind") == 0) {
				gm = gui_build_blind_post(last_pid, "small_blind_bet", last_amt);
			} else if (strcmp(last_str, "big_blind") == 0) {
				gm = gui_build_blind_post(last_pid, "big_blind_bet", last_amt);
			} else if (strcmp(last_str, "check") == 0 ||
				   strcmp(last_str, "fold") == 0) {
				gm = gui_build_betting_action(last_pid, last_str, 0);
			} else if (strcmp(last_str, "call") == 0 ||
				   strcmp(last_str, "raise") == 0 ||
				   strcmp(last_str, "allin") == 0) {
				gm = gui_build_betting_action(last_pid, last_str, last_amt);
			}
			if (gm) {
				gui_send_message(gm);
				cJSON_Delete(gm);
			}
		}
	}

	// Check if it's our turn
	if (current_turn != p_deck_info.player_id) {
		// Not our turn, just display status (once per dealer prompt)
		dlg_info("Waiting for Player %d to act (pot: %lld table chips / %.4f CHIPS)",
			 current_turn, (long long)pot, table_chips_to_chips(pot));
		return OK;
	}
	
	// Calculate remaining time
	int32_t timeout_secs = jint(betting_state, "timeout_secs");
	int64_t current_time = (int64_t)time(NULL);
	int64_t elapsed = current_time - turn_start_time;
	int64_t remaining = timeout_secs - elapsed;
	if (remaining < 0) remaining = 0;
	
	// It's our turn!
	dlg_info("═══════════════════════════════════════════");
	dlg_info("  🎯 YOUR TURN - Player %d (%s)             ", p_deck_info.player_id, player_config.verus_pid);
	dlg_info("  ⏰ Time remaining: %lld seconds           ", (long long)remaining);
	dlg_info("═══════════════════════════════════════════");
	dlg_info("  Action: %s", action);
	dlg_info("  Round: %d, Pot: %lld table chips (%.4f CHIPS)",
		 round, (long long)pot, table_chips_to_chips(pot));
	dlg_info("  Minimum to call: %lld table chips (%.4f CHIPS)",
		 (long long)min_amount, table_chips_to_chips(min_amount));
	
	// Display possibilities
	cJSON *possibilities = cJSON_GetObjectItem(betting_state, "possibilities");
	if (possibilities) {
		printf("\n  Options: ");
		for (int i = 0; i < cJSON_GetArraySize(possibilities); i++) {
			cJSON *opt = cJSON_GetArrayItem(possibilities, i);
			if (opt && opt->valuestring) {
				printf("[%s] ", opt->valuestring);
			}
		}
		printf("\n");
	}
	
	// Display player funds
	cJSON *player_funds = cJSON_GetObjectItem(betting_state, "player_funds");
	if (player_funds && p_deck_info.player_id < cJSON_GetArraySize(player_funds)) {
		int64_t my_funds = (int64_t)cJSON_GetArrayItem(player_funds, p_deck_info.player_id)->valuedouble;
		dlg_info("  Your funds: %lld table chips (%.4f CHIPS)",
			 (long long)my_funds, table_chips_to_chips(my_funds));
	}
	
	dlg_info("═══════════════════════════════════════════");
	
	// Handle based on betting mode
	extern int g_betting_mode;

	/* Blinds are forced bets - the only legal action is to post.
	 * Auto-post in every mode (CLI/AUTO/GUI) so the GUI player is not
	 * required to click anything for SB/BB; the GUI will see the next
	 * betting_state push from the dealer with the updated pot.
	 *
	 * Write the LITERAL action name ("small_blind" / "big_blind") so the
	 * dealer routes through the correct branch in
	 * verus_process_betting_action - which assigns the right enum value
	 * to bet_actions[playerid][round] (small_blind vs big_blind, not
	 * conflated). Writing "bet" here would mis-route through the SB-or-
	 * bet aliased branch and (a) label p2's BB as small_blind, (b) leak
	 * pot via repeated assignment if the dedup gate failed. */
	if (strcmp(action, "small_blind") == 0 || strcmp(action, "big_blind") == 0) {
		dlg_info("[AUTO-BLIND] Posting %s: %lld table chips (%.4f CHIPS)",
			 action, (long long)min_amount, table_chips_to_chips(min_amount));
		retval = player_write_betting_action(table_id, action, min_amount, round);
		return retval;
	}

	// Send GUI message for betting round (always log for testing)
	{
		/* Translate the dealer's legal-action list (string array on
		 * the chain, e.g. ["check","raise","fold","allin"] or
		 * ["call","raise","fold","allin"]) into the integer codes the
		 * GUI expects. The mapping is fixed by both sides:
		 *
		 *   backend  states.c::action_str    GUI  constants.ts::Possibilities
		 *     1   small_blind                 1   smallBlind
		 *     2   big_blind                   2   bigBlind
		 *     3   check                       3   check
		 *     4   raise                       4   raise
		 *     5   call                        5   call
		 *     6   allin                       6   allIn
		 *     7   fold                        7   fold
		 *
		 * Same indices, so it's a plain lookup. "bet" is dealer-side
		 * only - it appears for the SB/BB prompt which never reaches
		 * this block (AUTO-BLIND returns above). Map it defensively
		 * to raise(4) anyway since the GUI renders the Raise button
		 * with label "Bet" when toCall is 0.
		 *
		 * Why this matters: previously the array was hardcoded to
		 * [0,1,2,3,4,5] which meant processControls() in the GUI
		 * always set canCheck=true. The user always saw a Check
		 * button - even when there was a bet to face - and clicking
		 * it produced an illegal "check" action that the dealer
		 * rejected, leading to auto-fold. */
		static const struct {
			const char *str;
			int32_t code;
		} action_map[] = {
			{ "small_blind", 1 },
			{ "big_blind",   2 },
			{ "check",       3 },
			{ "raise",       4 },
			{ "call",        5 },
			{ "allin",       6 },
			{ "fold",        7 },
			{ "bet",         4 },  /* alias - see comment above */
		};
		const int32_t action_map_size =
			(int32_t)(sizeof(action_map) / sizeof(action_map[0]));

		int32_t poss_arr[8] = {0};
		int32_t poss_count = 0;
		if (possibilities) {
			int32_t n = cJSON_GetArraySize(possibilities);
			for (int32_t i = 0; i < n && poss_count < (int32_t)(sizeof(poss_arr) / sizeof(poss_arr[0])); i++) {
				cJSON *opt = cJSON_GetArrayItem(possibilities, i);
				if (!opt || !opt->valuestring) continue;
				for (int32_t j = 0; j < action_map_size; j++) {
					if (strcmp(opt->valuestring, action_map[j].str) == 0) {
						poss_arr[poss_count++] = action_map[j].code;
						break;
					}
				}
			}
		}
		if (poss_count == 0) {
			/* Dealer's possibilities was missing or contained only
			 * unknown strings. Fall back to the conservative full set
			 * so the GUI isn't completely locked out, but warn loudly
			 * because something is wrong upstream. */
			dlg_warn("betting_state.possibilities missing or unrecognised - "
				 "GUI fallback to full action set");
			poss_arr[0] = 3; poss_arr[1] = 4; poss_arr[2] = 5;
			poss_arr[3] = 6; poss_arr[4] = 7;
			poss_count = 5;
		}

		// Build player funds array (table chips)
		int64_t funds_arr[CARDS_MAXPLAYERS];
		cJSON *player_funds_json = cJSON_GetObjectItem(betting_state, "player_funds");
		int32_t num_players = player_funds_json ? cJSON_GetArraySize(player_funds_json) : 0;
		for (int i = 0; i < num_players && i < CARDS_MAXPLAYERS; i++) {
			funds_arr[i] = (int64_t)cJSON_GetArrayItem(player_funds_json, i)->valuedouble;
		}
		
		/* Min raise: typically 2x the call amount, or one big blind if no
		 * one has bet yet. BB_in_table_chips is the canonical fallback
		 * (default 2 table chips = 0.02 CHIPS). */
		int64_t min_raise = (min_amount > 0) ? (min_amount * 2) : BB_in_table_chips;
		
		cJSON *gui_msg = gui_build_betting_round(
			p_deck_info.player_id,
			pot,
			min_amount,
			min_raise,
			poss_arr,
			poss_count,
			funds_arr,
			num_players
		);
		gui_send_message(gui_msg);
		cJSON_Delete(gui_msg);
		
		// In GUI mode, wait for WebSocket message from GUI
		if (g_betting_mode == BET_MODE_GUI) {
			dlg_info("Waiting for GUI action...");
			return OK;
		}
	}
	
	if (g_betting_mode == BET_MODE_CLI) {
		// CLI mode - get input from user
		char input[256] = {0};
		
		// For blinds, auto-post (mandatory)
		if (strcmp(action, "small_blind") == 0 || strcmp(action, "big_blind") == 0) {
			printf("\n  [AUTO] Posting %s: %lld table chips (%.4f CHIPS)\n",
			       action, (long long)min_amount, table_chips_to_chips(min_amount));
			retval = player_write_betting_action(table_id, "bet", min_amount, round);
		} else {
			// Regular betting round - two-step input
			// Step 1: Get action
			printf("\n  Enter action (fold/check/call/raise/allin): ");
			fflush(stdout);
			
			if (fgets(input, sizeof(input), stdin) != NULL) {
				// Remove newline
				input[strcspn(input, "\n")] = 0;
				
				// Convert to lowercase for easier comparison
				for (int i = 0; input[i]; i++) {
					if (input[i] >= 'A' && input[i] <= 'Z') input[i] += 32;
				}
				
				if (strcmp(input, "fold") == 0 || strcmp(input, "f") == 0) {
					printf("  → Folding...\n");
					retval = player_write_betting_action(table_id, "fold", 0, round);
					
				} else if (strcmp(input, "check") == 0 || strcmp(input, "x") == 0) {
					if (min_amount > 0) {
						printf("  ⚠ Cannot check, must call %lld table chips (%.4f CHIPS) or fold\n",
						       (long long)min_amount, table_chips_to_chips(min_amount));
						printf("  Enter action (call/fold): ");
						fflush(stdout);
						if (fgets(input, sizeof(input), stdin) != NULL) {
							input[strcspn(input, "\n")] = 0;
							if (strcmp(input, "call") == 0 || strcmp(input, "c") == 0) {
								printf("  → Calling %lld table chips (%.4f CHIPS)...\n",
								       (long long)min_amount, table_chips_to_chips(min_amount));
								retval = player_write_betting_action(table_id, "call", min_amount, round);
							} else {
								printf("  → Folding...\n");
								retval = player_write_betting_action(table_id, "fold", 0, round);
							}
						}
					} else {
						printf("  → Checking...\n");
						retval = player_write_betting_action(table_id, "check", 0, round);
					}
					
				} else if (strcmp(input, "call") == 0 || strcmp(input, "c") == 0) {
					printf("  → Calling %lld table chips (%.4f CHIPS)...\n",
					       (long long)min_amount, table_chips_to_chips(min_amount));
					retval = player_write_betting_action(table_id, "call", min_amount, round);
					
				} else if (strcmp(input, "raise") == 0 || strcmp(input, "r") == 0) {
					/* Step 2: Get raise amount. CLI accepts CHIPS for human
					 * convenience (player types "0.04" not "4"); we convert
					 * to table chips before writing. */
					int64_t raise_amount = 0;
					printf("  Enter raise amount in CHIPS (min %.4f): ",
					       table_chips_to_chips(min_amount * 2));
					fflush(stdout);
					if (fgets(input, sizeof(input), stdin) != NULL) {
						input[strcspn(input, "\n")] = 0;
						double raise_chips = atof(input);
						if (raise_chips <= 0.0) {
							raise_amount = min_amount * 2;  // Default to min raise
						} else {
							raise_amount = chips_to_table_chips(raise_chips);
						}
					}
					printf("  → Raising to %lld table chips (%.4f CHIPS)...\n",
					       (long long)raise_amount, table_chips_to_chips(raise_amount));
					retval = player_write_betting_action(table_id, "raise", raise_amount, round);
					
				} else if (strcmp(input, "bet") == 0 || strcmp(input, "b") == 0) {
					// Step 2: Get bet amount (CLI input in CHIPS, see raise above)
					int64_t bet_amt = 0;
					printf("  Enter bet amount in CHIPS: ");
					fflush(stdout);
					if (fgets(input, sizeof(input), stdin) != NULL) {
						input[strcspn(input, "\n")] = 0;
						double bet_chips = atof(input);
						if (bet_chips <= 0.0) {
							bet_amt = (min_amount > 0) ? min_amount : SB_in_table_chips;
						} else {
							bet_amt = chips_to_table_chips(bet_chips);
						}
					}
					printf("  → Betting %lld table chips (%.4f CHIPS)...\n",
					       (long long)bet_amt, table_chips_to_chips(bet_amt));
					retval = player_write_betting_action(table_id, "bet", bet_amt, round);
					
				} else if (strcmp(input, "allin") == 0 || strcmp(input, "a") == 0) {
					// Get player funds and go all-in
					cJSON *pf = cJSON_GetObjectItem(betting_state, "player_funds");
					int64_t my_funds = 0;
					if (pf && p_deck_info.player_id < cJSON_GetArraySize(pf)) {
						my_funds = (int64_t)cJSON_GetArrayItem(pf, p_deck_info.player_id)->valuedouble;
					}
					printf("  → Going ALL-IN with %lld table chips (%.4f CHIPS)!\n",
					       (long long)my_funds, table_chips_to_chips(my_funds));
					retval = player_write_betting_action(table_id, "allin", my_funds, round);
					
				} else if (strlen(input) == 0) {
					// Empty input - auto check/call
					if (min_amount > 0) {
						printf("  → Auto-calling %lld table chips (%.4f CHIPS)...\n",
						       (long long)min_amount, table_chips_to_chips(min_amount));
						retval = player_write_betting_action(table_id, "call", min_amount, round);
					} else {
						printf("  → Auto-checking...\n");
						retval = player_write_betting_action(table_id, "check", 0, round);
					}
					
				} else {
					printf("  ⚠ Unknown action '%s'.\n", input);
					printf("  Valid actions: fold, check, call, raise, bet, allin\n");
					// Don't send anything, let timeout handle it or user can retry
					return OK;
				}
			}
		}
		p_local_state.last_game_state = round;
	} else {
		// AUTO mode - auto-respond based on action type
		if (strcmp(action, "small_blind") == 0) {
			dlg_info("Posting small blind: %lld table chips (%.4f CHIPS)",
				 (long long)min_amount, table_chips_to_chips(min_amount));
			retval = player_write_betting_action(table_id, "bet", min_amount, round);
			p_local_state.last_game_state = round;
		} else if (strcmp(action, "big_blind") == 0) {
			dlg_info("Posting big blind: %lld table chips (%.4f CHIPS)",
				 (long long)min_amount, table_chips_to_chips(min_amount));
			retval = player_write_betting_action(table_id, "bet", min_amount, round);
			p_local_state.last_game_state = round;
		} else {
			// Regular betting - for testing, auto-call or check
			if (min_amount > 0) {
				dlg_info("Auto-calling %lld table chips (%.4f CHIPS)...",
					 (long long)min_amount, table_chips_to_chips(min_amount));
				retval = player_write_betting_action(table_id, "call", min_amount, round);
			} else {
				dlg_info("Auto-checking...");
				retval = player_write_betting_action(table_id, "check", 0, round);
			}
			p_local_state.last_game_state = round;
		}
	}
	
	return retval;
}

static cJSON *card_int_to_json_str(int32_t v)
{
	if (v < 0 || v > 51)
		return cJSON_CreateNull();
	char buf[4];
	strncpy(buf, card_value_to_string(v), sizeof(buf));
	return cJSON_CreateString(buf);
}

static int32_t try_send_final_info_to_gui(char *table_id)
{
	extern int g_betting_mode;
	char str[65];
	char *gid = bits256_str(str, p_deck_info.game_id);
	cJSON *settlement = NULL, *board = NULL;
	cJSON *msg = NULL, *show_info = NULL, *win_arr = NULL;
	cJSON *board_arr = NULL, *all_holes = NULL;
	cJSON *winners_in = NULL, *player_ids_in = NULL, *flop = NULL;
	int32_t np = 0, ready = 1;

	if (g_betting_mode != BET_MODE_GUI)
		return 0;

	settlement = get_cJSON_from_id_key_vdxfid_from_height(table_id,
		get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, gid), g_start_block);
	if (!settlement)
		return 0;

	board = get_cJSON_from_id_key_vdxfid_from_height(table_id,
		get_key_data_vdxf_id(T_BOARD_CARDS_KEY, gid), g_start_block);
	if (!board) {
		cJSON_Delete(settlement);
		return 0;
	}

	player_ids_in = cJSON_GetObjectItem(settlement, "player_ids");
	np = player_ids_in ? cJSON_GetArraySize(player_ids_in) : 0;

	all_holes = cJSON_CreateArray();
	for (int p = 0; p < np; p++) {
		cJSON *pid = cJSON_GetArrayItem(player_ids_in, p);
		cJSON *cards_arr = cJSON_CreateArray();
		cJSON *reveal = NULL;
		if (pid && pid->valuestring) {
			reveal = get_cJSON_from_id_key_vdxfid_from_height(pid->valuestring,
				get_key_data_vdxf_id(P_HOLECARDS_REVEAL_KEY, gid), g_start_block);
		}
		if (!reveal) {
			ready = 0;
			cJSON_Delete(cards_arr);
			break;
		}
		cJSON *hc = cJSON_GetObjectItem(reveal, "hole_cards");
		if (!hc || cJSON_GetArraySize(hc) < 2) {
			ready = 0;
			cJSON_Delete(reveal);
			cJSON_Delete(cards_arr);
			break;
		}
		for (int c = 0; c < cJSON_GetArraySize(hc); c++) {
			cJSON *hcv = cJSON_GetArrayItem(hc, c);
			cJSON_AddItemToArray(cards_arr, card_int_to_json_str(hcv ? hcv->valueint : -1));
		}
		cJSON_Delete(reveal);
		cJSON_AddItemToArray(all_holes, cards_arr);
	}

	if (!ready) {
		cJSON_Delete(all_holes);
		cJSON_Delete(settlement);
		cJSON_Delete(board);
		return 0;
	}

	msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "method", "finalInfo");

	winners_in = cJSON_GetObjectItem(settlement, "winners");
	win_arr = cJSON_CreateArray();
	if (winners_in) {
		for (int i = 0; i < cJSON_GetArraySize(winners_in); i++) {
			cJSON *w = cJSON_GetArrayItem(winners_in, i);
			cJSON_AddItemToArray(win_arr, cJSON_CreateNumber(w ? w->valueint : 0));
		}
	}
	cJSON_AddItemToObject(msg, "winners", win_arr);
	cJSON_AddNumberToObject(msg, "win_amount", (double)jint(settlement, "pot"));

	show_info = cJSON_CreateObject();

	board_arr = cJSON_CreateArray();
	flop = cJSON_GetObjectItem(board, "flop");
	for (int i = 0; i < 3; i++) {
		cJSON *c = flop ? cJSON_GetArrayItem(flop, i) : NULL;
		cJSON_AddItemToArray(board_arr, card_int_to_json_str(c ? c->valueint : -1));
	}
	cJSON_AddItemToArray(board_arr, card_int_to_json_str(jint(board, "turn")));
	cJSON_AddItemToArray(board_arr, card_int_to_json_str(jint(board, "river")));
	cJSON_AddItemToObject(show_info, "boardCardInfo", board_arr);

	cJSON_AddItemToObject(show_info, "allHoleCardsInfo", all_holes);
	cJSON_AddItemToObject(msg, "showInfo", show_info);

	dlg_info("Sending finalInfo to GUI");
	player_lws_write(msg);
	cJSON_Delete(msg);
	cJSON_Delete(settlement);
	cJSON_Delete(board);
	return 1;
}

int32_t handle_game_state_player(char *table_id)
{
	int32_t game_state, retval = OK;
	static int32_t last_logged_state = -1;

	game_state = get_game_state(table_id);
	if (game_state != last_logged_state) {
		dlg_info("%s", game_state_str(game_state));
		last_logged_state = game_state;
	}
	switch (game_state) {
	case G_REVEAL_CARD:
		retval = handle_player_reveal_card(table_id);
		break;
	case G_ROUND_BETTING:
		retval = player_handle_betting(table_id);
		break;
	case G_SHOWDOWN:
		dlg_info("═══════════════════════════════════════════");
		dlg_info("  🏆 SHOWDOWN - Game Complete!              ");
		dlg_info("═══════════════════════════════════════════");
		// Display final cards
		for (int i = 0; i < hand_size && i < p_local_state.cards_decoded_count; i++) {
			if (p_local_state.decoded_cards[i] >= 0) {
				dlg_info("  Card %d: %s", i + 1, get_card_name(p_local_state.decoded_cards[i]));
			}
		}
		/* One-shot reveal of our two hole cards onto our own player id
		 * under P_HOLECARDS_REVEAL_KEY.<gid>. Single-writer-per-identity:
		 * only this player writes this key, the dealer reads it at
		 * showdown to score 7-card hands. Guard prevents re-publishing
		 * on every poll tick of the player loop while the table sits in
		 * G_SHOWDOWN. */
		{
			static int32_t hole_cards_published = 0;
			if (!hole_cards_published &&
			    p_local_state.hole_cards[0] >= 0 &&
			    p_local_state.hole_cards[1] >= 0) {
				char str[65];
				char *gid = bits256_str(str, p_deck_info.game_id);
				cJSON *reveal = cJSON_CreateObject();
				cJSON_AddStringToObject(reveal, "game_id", gid);
				cJSON *hc_arr = cJSON_CreateArray();
				cJSON_AddItemToArray(hc_arr, cJSON_CreateNumber(p_local_state.hole_cards[0]));
				cJSON_AddItemToArray(hc_arr, cJSON_CreateNumber(p_local_state.hole_cards[1]));
				cJSON_AddItemToObject(reveal, "hole_cards", hc_arr);

				dlg_info("Publishing hole cards [%s, %s] for showdown",
					 get_card_name(p_local_state.hole_cards[0]),
					 get_card_name(p_local_state.hole_cards[1]));

				cJSON *out = poker_append_key_json(
					player_config.verus_pid,
					get_key_data_vdxf_id(P_HOLECARDS_REVEAL_KEY, gid),
					reveal, true);
				if (out) {
					hole_cards_published = 1;
				} else {
					dlg_warn("Hole-card reveal write failed; will retry on next tick");
				}
				cJSON_Delete(reveal);
			}
		}
		{
			static int32_t final_info_sent = 0;
			if (!final_info_sent && try_send_final_info_to_gui(table_id))
				final_info_sent = 1;
		}
		break;
	case G_SETTLEMENT_COMPLETE:
		dlg_info("═══════════════════════════════════════════");
		dlg_info("  💰 PAYOUT RECEIVED - Game Finished!       ");
		dlg_info("═══════════════════════════════════════════");
		break;
	}
	return retval;
}

int32_t handle_verus_player()
{
	int32_t retval = OK;
	char game_id_str[65];

	// Initialize local state
	init_p_local_state();

	// Check if poker is ready
	if ((retval = verify_poker_setup()) != OK) {
		dlg_error("Poker not ready: %s", bet_err_str(retval));
		return retval;
	}
	// Parse Verus player configuration
	if ((retval = bet_parse_verus_player()) != OK) {
		dlg_error("Failed to parse Verus player configuration: %s", bet_err_str(retval));
		return retval;
	}
	
	// State 1: Wallet + ID ready, can read blockchain
	send_init_state_to_gui(P_INIT_WALLET_READY);
	dlg_info("Player init state: %s", player_init_state_str(P_INIT_WALLET_READY));

	// In GUI mode, wait for GUI to request table info before trying to find a table
	if (g_betting_mode == BET_MODE_GUI) {
		dlg_info("GUI mode: waiting for GUI to send table_info request...");
		pthread_mutex_lock(&gui_table_mutex);
		while (gui_table_requested == 0) {
			pthread_cond_wait(&gui_table_cond, &gui_table_mutex);
		}
		// Reset for next request
		gui_table_requested = 0;
		pthread_mutex_unlock(&gui_table_mutex);
		dlg_info("GUI triggered table finding");
	}

	// Find a table
	bool already_joined = false;
	if ((retval = find_table()) != OK) {
		if (retval == ERR_DUPLICATE_PLAYERID) {
			dlg_info("Player already exists in the table. Skipping join, proceeding to deck shuffling.");
			already_joined = true;
			retval = OK;  // Clear the error since we're handling it
		} else {
			dlg_error("Failed to find table: %s", bet_err_str(retval));
			// In GUI mode, send error to GUI instead of returning
			if (g_betting_mode == BET_MODE_GUI) {
				cJSON *error_msg = cJSON_CreateObject();
				cJSON_AddStringToObject(error_msg, "method", "error");
				cJSON_AddStringToObject(error_msg, "error", bet_err_str(retval));
				cJSON_AddStringToObject(error_msg, "message", "Failed to find table");
				player_lws_write(error_msg);
				cJSON_Delete(error_msg);
			}
			return retval;
		}
	}
	dlg_info("Table found");
	print_struct_table(&player_t);
	
	// State 2: Found table, have table info
	send_init_state_to_gui(P_INIT_TABLE_FOUND);
	dlg_info("Player init state: %s", player_init_state_str(P_INIT_TABLE_FOUND));
	
	// If GUI mode, wait for GUI to approve join
	if (g_betting_mode == BET_MODE_GUI) {
		// State 3: Waiting for GUI approval
		send_init_state_to_gui(P_INIT_WAIT_JOIN);
		dlg_info("Player init state: %s - waiting for GUI approval...", player_init_state_str(P_INIT_WAIT_JOIN));
		
		// Wait for GUI to send join_table message, refreshing table info every 5 seconds
		extern void bet_player_table_info(void);
		struct timespec ts;
		
		pthread_mutex_lock(&gui_join_mutex);
		dlg_info("Waiting for GUI join signal (gui_join_approved=%d)...", gui_join_approved);
		while (gui_join_approved == 0) {
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 5;  // 5 second timeout
			
			int wait_result = pthread_cond_timedwait(&gui_join_cond, &gui_join_mutex, &ts);
			if (wait_result == ETIMEDOUT && gui_join_approved == 0) {
				// Timeout - refresh table info for GUI
				pthread_mutex_unlock(&gui_join_mutex);
				bet_player_table_info();
				pthread_mutex_lock(&gui_join_mutex);
			}
		}
		// Reset for next join
		gui_join_approved = 0;
		pthread_mutex_unlock(&gui_join_mutex);
		
		dlg_info("GUI join approved, proceeding to join table");

		cJSON *join_ack = cJSON_CreateObject();
		cJSON_AddStringToObject(join_ack, "method", "join_ack");
		cJSON_AddStringToObject(join_ack, "status", "approved");
		cJSON_AddStringToObject(join_ack, "message", "Join approved, proceeding with payin transaction");
		player_lws_write(join_ack);
		cJSON_Delete(join_ack);
	}

	// Join the table (skip if already joined)
	if (!already_joined) {
		// State 4: Joining table (executing payin_tx)
		send_init_state_to_gui(P_INIT_JOINING);
		dlg_info("Player init state: %s", player_init_state_str(P_INIT_JOINING));
		
		if ((retval = poker_join_table()) != OK) {
			dlg_error("Failed to join table: %s", bet_err_str(retval));
			return retval;
		}
		dlg_info("Table Joined");
		
		// State 5: Successfully joined (have seat)
		send_init_state_to_gui(P_INIT_JOINED);
		dlg_info("Player init state: %s", player_init_state_str(P_INIT_JOINED));
	}

	// In GUI mode, wait for all players to join (G_PLAYERS_JOINED state)
	// Periodically send table_info updates so GUI can show who else has joined
	if (g_betting_mode == BET_MODE_GUI) {
		extern void bet_player_table_info(void);
		int32_t game_state;
		
		dlg_info("Waiting for all players to join...");
		while ((game_state = get_game_state(player_config.table_id)) < G_PLAYERS_JOINED) {
			bet_player_table_info();  // Send updated table info to GUI
			sleep(5);  // Check every 5 seconds
		}
		dlg_info("All players joined, game state: %s", game_state_str(game_state));
	}

	// Get player ID
	if ((retval = get_player_id(&p_deck_info.player_id)) != OK) {
		dlg_error("Failed to get player ID: %s", bet_err_str(retval));
		return retval;
	}
	dlg_info("Player ID: %d", p_deck_info.player_id);

	// Get the current game_id from the table
	char *game_id_from_table = poker_get_key_str(player_config.table_id, T_GAME_ID_KEY);
	if (game_id_from_table) {
		p_deck_info.game_id = bits256_conv(game_id_from_table);
		strncpy(game_id_str, game_id_from_table, sizeof(game_id_str) - 1);
		game_id_str[sizeof(game_id_str) - 1] = '\0';
		dlg_info("Game ID from table: %s", game_id_str);
	} else {
		// No game_id yet - will be set during deck init
		memset(game_id_str, 0, sizeof(game_id_str));
	}

	// Check if game is already past deck shuffling phase
	int32_t current_game_state = get_game_state(player_config.table_id);
	if (current_game_state > G_DECK_SHUFFLING_B) {
		// Game is already in progress - try to load state from local DB
		dlg_info("Game is in progress (state: %s). Attempting to load from local DB...",
			 game_state_str(current_game_state));
		
		if (strlen(game_id_str) > 0) {
			// Load deck info
			retval = load_player_deck_info(game_id_str);
			if (retval != OK) {
				dlg_error("Failed to load deck info from local DB: %s", bet_err_str(retval));
				dlg_error("Cannot rejoin - deck keys not found. Please wait for a new game.");
				return ERR_GAME_ALREADY_STARTED;
			}
			dlg_info("Successfully loaded player deck info from local DB!");

			// Load local state (payin_tx, decoded cards)
			retval = load_player_local_state(game_id_str);
			if (retval == OK) {
				dlg_info("Loaded local state: payin_tx=%s, cards_decoded=%d", 
					p_local_state.payin_tx, p_local_state.cards_decoded_count);
				
				// Check if payin_tx is still unspent (game still active)
				if (strlen(p_local_state.payin_tx) > 0) {
					dlg_info("Payin TX: %s - can be used for dispute if needed", p_local_state.payin_tx);
				}
			} else {
				dlg_info("No local state found, starting fresh tracking");
				// Initialize local state for this game
				strncpy(p_local_state.game_id, game_id_str, sizeof(p_local_state.game_id) - 1);
				strncpy(p_local_state.table_id, player_config.table_id, sizeof(p_local_state.table_id) - 1);
				p_local_state.player_id = p_deck_info.player_id;
			}

			dlg_info("Rejoining game with player_id=%d", p_deck_info.player_id);
		} else {
			dlg_error("No game_id found on table. Cannot rejoin.");
			return ERR_GAME_ALREADY_STARTED;
		}
	} else if (current_game_state >= G_PLAYERS_JOINED && current_game_state <= G_DECK_SHUFFLING_B) {
		// Game is in deck shuffling phase - check if we have local deck info
		if (already_joined && strlen(game_id_str) > 0) {
			retval = load_player_deck_info(game_id_str);
			if (retval == OK) {
				dlg_info("Loaded existing deck info for this game from local DB");
				// Also load local state
				load_player_local_state(game_id_str);
				// Skip deck init since we already have our deck
			} else {
				// No local deck info - need to initialize
				dlg_info("No saved deck info found, initializing new deck...");
				if ((retval = player_init_deck()) != OK) {
					dlg_error("Failed to initialize player deck: %s", bet_err_str(retval));
					return retval;
				}
				// Initialize and save local state
				strncpy(p_local_state.game_id, game_id_str, sizeof(p_local_state.game_id) - 1);
				strncpy(p_local_state.table_id, player_config.table_id, sizeof(p_local_state.table_id) - 1);
				p_local_state.player_id = p_deck_info.player_id;
				save_player_local_state(player_config.txid);  // Save with payin_tx
				dlg_info("Player deck shuffling info updated to table");
			}
		} else {
			// First join - initialize deck
			if ((retval = player_init_deck()) != OK) {
				dlg_error("Failed to initialize player deck: %s", bet_err_str(retval));
				return retval;
			}
			// Initialize and save local state with payin_tx
			strncpy(p_local_state.game_id, game_id_str, sizeof(p_local_state.game_id) - 1);
			strncpy(p_local_state.table_id, player_config.table_id, sizeof(p_local_state.table_id) - 1);
			p_local_state.player_id = p_deck_info.player_id;
			save_player_local_state(player_config.txid);
			dlg_info("Player deck shuffling info updated to table");
		}
	} else {
		// Game not started yet or waiting for players
		if ((retval = player_init_deck()) != OK) {
			dlg_error("Failed to initialize player deck: %s", bet_err_str(retval));
			return retval;
		}
		// Initialize and save local state with payin_tx
		bits256_str(game_id_str, p_deck_info.game_id);
		strncpy(p_local_state.game_id, game_id_str, sizeof(p_local_state.game_id) - 1);
		strncpy(p_local_state.table_id, player_config.table_id, sizeof(p_local_state.table_id) - 1);
		p_local_state.player_id = p_deck_info.player_id;
		save_player_local_state(player_config.txid);
		dlg_info("Player deck shuffling info updated to table");
	}
	
	// State 6: Deck initialized/loaded, ready for game
	send_init_state_to_gui(P_INIT_DECK_READY);
	dlg_info("Player init state: %s", player_init_state_str(P_INIT_DECK_READY));

	// State 7: Entering game loop
	send_init_state_to_gui(P_INIT_IN_GAME);
	dlg_info("Player init state: %s - entering game loop", player_init_state_str(P_INIT_IN_GAME));

	// Main game loop
	while (1) {
		retval = handle_game_state_player(player_config.table_id);
		if (retval != OK) {
			dlg_error("Error in game state handling: %s", bet_err_str(retval));
			return retval;
		}
		sleep(2);
	}

	return retval;
}
