#ifndef __VDXF_H__
#define __VDXF_H__

#include "bet.h"
#include "common.h"

/* 
Keys under a given namespace
----------------------------
Keys are ways to store the information under the identities, we encapsulate the info of a structure or a method and 
map them to the keys. 
For example we have an identifier named cashiers under poker which holds all the cashiers info, we also define a cashier_key
where in which all the cashiers info is encapsulated map to the cashier_key and stored under the cashier identifier. 
Our aim is to define as minimum identifiers as possible and as minimum keys as possible to represent the data.

We define every keys under the namespace chips.vrsc::
As we know that bet aim to support multiple games, so under the namespace chips.vrsc:: we group the keys based on the game 
in which they used for. For example chips.vrsc::poker is the prefix to all the keys that are used in playing the poker game.
Likewise if any keys are common across the games then those are defined with the prefix chips.vrsc::bet.

Lets say the cashiers are specific to the context of poker, so we defined them with the prefix chips.vrsc::poker and the final
key is represented as chips.vrsc::poker.cashiers.
*/

#define VDXF_POKER_KEYS_PREFIX "chips.vrsc::poker."

#define CASHIERS_KEY VDXF_POKER_KEYS_PREFIX "cashiers"
/*
* The DEALERS_KEY holds information about registered dealers in the poker system.
* It stores an array of dealer identities (IDs) that are authorized to run poker tables.
* 
* The structure of the data stored under this key is:
* {
*   "dealers": [
*     "dealer_id_1",
*     "dealer_id_2",
*     ...
*   ]
* }
* 
* Where each "dealer_id_x" is a unique identifier (likely a VerusID) for a registered dealer.
* This allows the system to maintain a list of approved dealers that can host poker games.
*/
#define DEALERS_KEY VDXF_POKER_KEYS_PREFIX "dealers"

/*
* t_game_ids - Tracks the current active game ID
* {
*   "hexdata" (string representing the 256-bit game ID)
* }
*/
#define T_GAME_ID_KEY VDXF_POKER_KEYS_PREFIX "t_game_ids"

/*
* t_table_info - Stored on dealer identity (as advertisement) and table identity (for active game)
* {
*   max_players: maximum number of players allowed
*   big_blind: big blind amount in CHIPS
*   min_stake: minimum buy-in amount in CHIPS
*   max_stake: maximum buy-in amount in CHIPS
*   table_id: the table identity string
*   dealer_id: the dealer identity string
*   cashier_id: the cashier identity string
*   start_block: block height when the table/game was initialized
* }
*/
#define T_TABLE_INFO_KEY VDXF_POKER_KEYS_PREFIX "t_table_info"
/*
* t_player_info {
* num_players : 
* player_info : [veruspid_txid_playerid]
* }
*/
#define T_PLAYER_INFO_KEY VDXF_POKER_KEYS_PREFIX "t_player_info"

/*
* p_join_request - Stored on player identity when requesting to join a table
* {
*   dealer_id: dealer the player wants to join
*   table_id: table to join
*   cashier_id: cashier where payin was sent
*   payin_tx: transaction ID of payin
* }
* Player checks t_player_info to know if accepted (no status field needed)
*/
#define P_JOIN_REQUEST_KEY VDXF_POKER_KEYS_PREFIX "p_join_request"

/*
* t_betting_state - Dealer writes to TABLE ID during betting
* {
*   current_turn: player index whose turn it is
*   round: betting round (0=preflop, 1=flop, 2=turn, 3=river)
*   pot: total pot amount
*   action: "small_blind" | "big_blind" | "round_betting"
*   min_amount: minimum amount to call/bet
*   possibilities: array of allowed actions [call, raise, fold, check, allin]
*   bet_amounts: array of current bets per player this round
*   player_funds: array of remaining funds per player
* }
*/
#define T_BETTING_STATE_KEY VDXF_POKER_KEYS_PREFIX "t_betting_state"

/*
* p_betting_action - Player writes to PLAYER ID when taking action
* {
*   round: betting round
*   action: "fold" | "call" | "raise" | "check" | "allin"
*   amount: bet amount (if raise/allin)
*   card_id: card turn this action is for
* }
*/
#define P_BETTING_ACTION_KEY VDXF_POKER_KEYS_PREFIX "p_betting_action"

#define T_D_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_deck"
#define T_D_P1_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p1_deck"
#define T_D_P2_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p2_deck"
#define T_D_P3_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p3_deck"
#define T_D_P4_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p4_deck"
#define T_D_P5_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p5_deck"
#define T_D_P6_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p6_deck"
#define T_D_P7_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p7_deck"
#define T_D_P8_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p8_deck"
#define T_D_P9_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_d_p9_deck"

#define T_B_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_deck"
#define T_B_P1_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p1_deck"
#define T_B_P2_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p2_deck"
#define T_B_P3_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p3_deck"
#define T_B_P4_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p4_deck"
#define T_B_P5_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p5_deck"
#define T_B_P6_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p6_deck"
#define T_B_P7_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p7_deck"
#define T_B_P8_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p8_deck"
#define T_B_P9_DECK_KEY VDXF_POKER_KEYS_PREFIX "t_b_p9_deck"

/*
 * Cashier-side mirrors of T_B_*_DECK_KEY.
 *
 * Per the single-writer-per-identity rule (docs/TODO.md item 1.1), the cashier
 * writes its blinded-deck output to its OWN identity under these C_B_* keys.
 * The dealer polls the cashier id, sees them populated, and canonicalizes them
 * onto the table id under the existing T_B_* keys (which players read from).
 *
 * Wire format and per-game scoping (`<game_id>` suffix) are identical to the
 * T_B_* keys; only the writer + storage location differ.
 */
#define C_B_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_deck"
#define C_B_P1_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p1_deck"
#define C_B_P2_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p2_deck"
#define C_B_P3_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p3_deck"
#define C_B_P4_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p4_deck"
#define C_B_P5_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p5_deck"
#define C_B_P6_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p6_deck"
#define C_B_P7_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p7_deck"
#define C_B_P8_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p8_deck"
#define C_B_P9_DECK_KEY VDXF_POKER_KEYS_PREFIX "c_b_p9_deck"

/*
 * Cashier-side mirror of T_CARD_BV_KEY (docs/TODO.md item 1.2).
 *
 * The cashier writes one entry per (player_id, card_id) reveal request to
 * its OWN identity under C_CARD_BV_KEY.<game_id>. The player reader
 * (player.c:reveal_card) reads directly from the cashier id resolved via
 * T_TABLE_INFO_KEY — no canonicalize-onto-table-id step. Same payload
 * shape as T_CARD_BV_KEY (`{player_id, card_id, bv: [...]}`); only the
 * writer/storage location differ.
 */
#define C_CARD_BV_KEY VDXF_POKER_KEYS_PREFIX "c_card_bv"

/*
* card_bv {
* player_id : If player id is -1, then blinding values of all the players for that card to be revealed.
* card_id
* bv or bv[] : blinding value(s)
* }
*/
#define T_CARD_BV_KEY VDXF_POKER_KEYS_PREFIX "card_bv"

/*
* t_board_cards - Community cards revealed on the table (updated by dealer after confirming all players decoded)
* {
*   flop: [card1, card2, card3] or [-1, -1, -1] if not revealed
*   turn: card4 or -1
*   river: card5 or -1
*   last_card_id: last board card_id processed
* }
* On rejoin, player reads this to get already-revealed community cards
*/
#define T_BOARD_CARDS_KEY VDXF_POKER_KEYS_PREFIX "t_board_cards"

/*
 * p_decoded_card — Player publishes a cumulative snapshot of every
 * community card it has decoded so far for the current game. Schema:
 *
 *   {
 *     "game_id":  "<32-byte hex>",
 *     "decoded_cards": [
 *       { "card_id": <int>, "card_type": <enum>, "card_value": <0..51> },
 *       ...
 *     ]
 *   }
 *
 * Why a cumulative snapshot (rather than one CMM entry per card):
 *   `get_cJSON_from_id_key_vdxfid_from_height` returns only the LATEST
 *   CMM entry for a key. With a per-card schema the dealer's consensus
 *   check for, say, flop_card_1 fails once the player has appended
 *   flop_card_2 — the latest entry's card_type no longer matches. Each
 *   write here re-publishes the full array so the latest entry always
 *   holds the player's complete view, and the dealer's consensus
 *   read finds any requested card_type by scanning the array within
 *   that single entry.
 *
 * Single-writer-per-identity: only the player owning verus_pid writes
 * this key. The dealer reads it to verify community cards.
 *
 * Hole-card *secrecy* during play: this key is currently used only for
 * community-card consensus. Each player's two private hole cards are
 * revealed in a separate one-shot write at G_SHOWDOWN — see
 * P_HOLECARDS_REVEAL_KEY below.
 */
#define P_DECODED_CARD_KEY VDXF_POKER_KEYS_PREFIX "p_decoded_card"

/*
 * p_holecards_reveal - Player reveals its two hole cards at showdown.
 *
 * Player publishes a single CMM entry on its OWN identity at G_SHOWDOWN
 * (single-writer-per-identity rule):
 *   {
 *     game_id:    "<32-byte hex>",
 *     hole_cards: [v0, v1]    // card values (deck face indices)
 *   }
 *
 * Dealer reads this from each non-folded player's id at G_SHOWDOWN,
 * combines with the 5 community cards from T_BOARD_CARDS_KEY, and runs
 * 7-card hand evaluation (poker.c::seven_card_draw_score) to determine
 * winners and pot distribution.
 *
 * Why a separate key instead of reusing P_DECODED_CARD_KEY:
 *   - keeps community-card consensus reads deterministic — they read the
 *     latest entry of P_DECODED_CARD_KEY without having to filter by
 *     card_type.
 *   - one-shot write per game — simpler dealer-side reasoning than
 *     iterating an append-mode CMM and filtering hole-card entries.
 *   - revealed strictly at showdown, never earlier — prevents accidental
 *     leaking of hole cards mid-hand (P_DECODED_CARD_KEY entries that
 *     get appended throughout the hand would otherwise expose them as
 *     soon as a player decodes).
 */
#define P_HOLECARDS_REVEAL_KEY VDXF_POKER_KEYS_PREFIX "p_holecards_reveal"

/*
* t_game_info {
* t_game_ids : 256 bit unique string in hex
* game_info : Holds the info of the gaming state
* }
*/
#define T_GAME_INFO_KEY VDXF_POKER_KEYS_PREFIX "t_game_info"

/*
* t_settlement_info {
*   game_id: The game ID this settlement is for
*   status: "pending" | "completed" | "failed"
*   winners: Array of winner player indices
*   settle_amounts: Amount each player gets back (in CHIPS)
*   player_ids: Player Verus IDs for payout
*   payin_txs: Original payin transaction IDs
*   pot: Total pot amount
*   payout_txs: Array of payout transaction IDs (filled by cashier)
* }
*/
#define T_SETTLEMENT_INFO_KEY VDXF_POKER_KEYS_PREFIX "t_settlement_info"

/*
 * Cashier-side settlement result (docs/TODO.md item 1.3).
 *
 * The dealer writes the settlement *order* (winners, settle_amounts,
 * player_ids, payin_txs, pot, cashier_id, status:pending) into
 * T_SETTLEMENT_INFO_KEY.<gid> on the table id. The cashier MUST NOT
 * mutate that key. Instead, after issuing all sendcurrency payouts the
 * cashier writes its receipt — {status:"completed", payout_txs:[...]}
 * — to its OWN identity under C_SETTLEMENT_RESULT_KEY.<gid>.
 *
 * The dealer polls cashier_id, observes the result, and canonicalizes
 * status + payout_txs onto T_SETTLEMENT_INFO_KEY.<gid> on the table id
 * (preserving the dealer-written order fields). Same handshake pattern
 * as item 1.4's G_SETTLEMENT_COMPLETE_BY_CASHIER → G_SETTLEMENT_COMPLETE.
 *
 * The cashier also uses this key for its own re-pay idempotency: at
 * the top of cashier_process_settlement, if C_SETTLEMENT_RESULT_KEY for
 * this game already exists with status:completed, the cashier returns
 * OK without re-issuing payouts.
 */
#define C_SETTLEMENT_RESULT_KEY VDXF_POKER_KEYS_PREFIX "c_settlement_result"

/*
* p_game_history - Stored on player ID after joining a game
* {
*   payin_tx: Player's payin transaction ID
*   table_id: The table joined
*   dealer_id: The dealer
*   game_id: The game ID
*   join_block: Block height when joined
*   amount: Amount paid in (CHIPS)
* }
*/
#define P_GAME_HISTORY_KEY VDXF_POKER_KEYS_PREFIX "p_game_history"

/*
* p_dispute_request - Player raises a dispute
* {
*   payin_tx: The payin transaction to dispute
*   table_id: The table
*   game_id: The game ID
*   reason: "no_payout" | "game_aborted" | "timeout"
*   request_block: Block when dispute was raised
* }
*/
#define P_DISPUTE_REQUEST_KEY VDXF_POKER_KEYS_PREFIX "p_dispute_request"

/*
* c_dispute_result - Cashier's response to a dispute (stored on cashier ID)
* {
*   player_id: The player who raised dispute
*   game_id: The game ID
*   status: "refunded" | "paid" | "rejected"
*   payout_tx: Payout/refund transaction ID
*   reason: Explanation
*   resolved_block: Block when resolved
* }
*/
#define C_DISPUTE_RESULT_KEY VDXF_POKER_KEYS_PREFIX "c_dispute_result"

// Number of blocks after which a pending game is considered aborted
#define DISPUTE_TIMEOUT_BLOCKS 100

/*
* player_deck {
* id: Players ID assigned by dealer, this is fecthed using get_playerid after player join.
* pubkey: This is players pubkey pG
* cardinfo:Array of card pubkeys r1G, r2G, ..., r52G
* }
*/
#define PLAYER_DECK_KEY VDXF_POKER_KEYS_PREFIX "player_deck"

/*
Datatypes used
--------------
Since we are encapsulating the data and store using binary serialization, so we basically not needing much variety here. 
We majorly use bytevector defined as vrsc::data.type.bytevector in verus.
*/

#define STRING_VDXF_ID "vrsc::data.type.string"
#define BYTEVECTOR_VDXF_ID "vrsc::data.type.bytevector"

/*
Identitites
-----------
We limit ourself to two levels of nesting under chips. At first level we mostly define identities are of game_types or any 
such things which are common across all the game types. For each game_type identity we generate a token which is used to register 
sub identities and is needed to play that specific game. 
For example we registered the identity sg777z under chips as sg777z.chips.vrsc@, and we generate a token named sg777z which is basically be
used to play the poker game.
At the second level we register sun identities under the game_type identity which are very specific to the game.

Identifiers are often the addresses that can hold the tokens, so for that reason identifies always ends with @. 
Any entity in the bet ecosystem can register the identities under chips, like for example i registered an identiy named sg777 
under chips as sg777.chips@ which basically been used to hold the tokens. 
*/

#define CASHIERS_ID_FQN "cashier.sg777z.chips.vrsc@"
#define DEALERS_ID_FQN "dealers.sg777z.chips.vrsc@"

/*
Currencies
----------
Bet supports various tokens that launch on Verus and CHIPS is the token which we use to play poker.
*/
#define CHIPS "chips"

/* Every node that is part of the poker make updates to the IDs, so to pay tx_fee for the ID updates we keeping this reserve
*  amount to be 1 CHIP which is sufficient to accomodate all gaming updates in poker.
*/
#define ID_UPDATE_ESTIMATE_NO 1000
#define RESERVE_AMOUNT ID_UPDATE_ESTIMATE_NO *chips_tx_fee

#define all_t_d_p_keys_no 10
extern char all_t_d_p_keys[all_t_d_p_keys_no][128];
extern char all_t_d_p_key_names[all_t_d_p_keys_no][128];

#define all_t_b_p_keys_no 10
extern char all_t_b_p_keys[all_t_b_p_keys_no][128];
extern char all_t_b_p_key_names[all_t_b_p_keys_no][128];

/* Cashier-owned mirrors of all_t_b_p_keys[] / all_t_b_p_key_names[].
 * Same indexing convention: index 0..8 = P1..P9, index 9 = bare deck. */
extern char all_c_b_p_keys[all_t_b_p_keys_no][128];
extern char all_c_b_p_key_names[all_t_b_p_keys_no][128];

#define all_game_keys_no 1
extern char all_game_keys[all_game_keys_no][128];
extern char all_game_key_names[all_game_keys_no][128];

#define MAX_ID_LEN 128

/* ============================================================================
 * VDXF Key Operations
 * ============================================================================ */
char *get_vdxf_id(const char *key_name);
char *get_key_vdxf_id(char *key_name);
char *get_full_key(char *key_name);
char *get_key_data_vdxf_id(char *key_name, char *data);

/* ============================================================================
 * Identity CMM (ContentMultiMap) Operations
 * ============================================================================ */
cJSON *update_cmm(const char *id, cJSON *cmm);
cJSON *get_cmm(const char *id);

/* Fetch the contentmultimap (combined view from height_start) for a single
 * vdxfkey. Uses the daemon's vdxfkey filter, which bypasses the response-size
 * truncation that affects unfiltered getidentitycontent 0 -1 calls on
 * accumulated identities. Returned cmm is owned by the caller. */
cJSON *get_cmm_from_height_filtered(const char *id, const char *key_vdxfid, int32_t height_start);

/* Walk the identity's update history (getidentityhistory) to find the most
 * recent block where `key_vdxfid` was written to `id` with value
 * `expected_hex`. Used by print_id paths on identities (e.g. cashier) that
 * do not stamp a start_block in any CMM payload. Returns -1 if not found. */
int32_t find_cmm_key_write_block(const char *id, const char *key_vdxfid, const char *expected_hex);

/* Split a fully-qualified Verus identity (e.g. "p1.sg777z.VRSCTEST@") into
 * its leading short name ("p1") and parent suffix ("sg777z.VRSCTEST@"). On
 * failure (NULL input, no dot, missing trailing '@') returns false and the
 * out buffers are not modified. */
bool split_fqn(const char *fqn, char *name_out, size_t name_size,
	       char *parent_out, size_t parent_size);

/* ============================================================================
 * Identity Verification
 * ============================================================================ */
bool is_id_exists(const char *id);
int32_t id_canspendfor(const char *id, int32_t *err_no);
int32_t id_cansignfor(const char *id, int32_t *err_no);

/* ============================================================================
 * Currency/Transaction Operations
 * ============================================================================ */
cJSON *verus_sendcurrency_data(const char *id, double amount, cJSON *data);
cJSON *getaddressutxos(char verus_addresses[][100], int n);

/* ============================================================================
 * Poker Key Data Access (internal - use poker_vdxf.h for public API)
 * ============================================================================ */
char *get_str_from_id_key(char *id, char *key);
char *get_str_from_id_key_from_height(const char *id, const char *key, int32_t height_start);
cJSON *get_cJSON_from_id_key(const char *id, const char *key);
cJSON *get_cJSON_from_id_key_vdxfid(char *id, char *key_vdxfid);
cJSON *get_cJSON_from_id_key_vdxfid_from_height(const char *id, const char *key_vdxfid, int32_t height_start);
/* Like get_cJSON_from_id_key_vdxfid_from_height but returns ALL CMM
 * entries for the key (height-merged), not just the latest. Each
 * element of the returned array is a decoded cJSON object (the original
 * payload that was hex-encoded into the CMM entry). Caller owns the
 * returned array and must cJSON_Delete it. Returns NULL on no entries. */
cJSON *get_all_cJSON_from_id_key_vdxfid_from_height(const char *id, const char *key_vdxfid, int32_t height_start);
cJSON *append_cmm_from_id_key_data_hex(const char *id, const char *key, char *hex_data, bool is_key_vdxf_id);
cJSON *append_cmm_from_id_key_data_cJSON(const char *id, const char *key, cJSON *data, bool is_key_vdxf_id);
cJSON *update_cmm_from_id_key_data_cJSON(const char *id, const char *key, cJSON *data, bool is_key_vdxf_id);
/*
 * Merge-mode CMM writes: read combined view from `start_block`, fold in the
 * supplied key/value, rewrite the whole snapshot in one updateidentity. Use
 * this for table-id writes during the join phase so that fresh players (who
 * do not have start_block yet) can still discover the full game-bootstrap
 * state via plain getidentity. After all players have joined, switch back to
 * the cheaper append_cmm_from_id_key_data_* writes.
 */
cJSON *merge_cmm_from_id_key_data_hex(const char *id, int32_t start_block,
				      const char *key, char *hex_data, bool is_key_vdxf_id);
cJSON *merge_cmm_from_id_key_data_cJSON(const char *id, int32_t start_block,
					const char *key, cJSON *data, bool is_key_vdxf_id);

/* ============================================================================
 * Poker Table Operations (internal - use poker_vdxf.h for public API)
 * ============================================================================ */
struct table *decode_table_info_from_str(char *str);
struct table *decode_table_info(cJSON *dealer_cmm_data);
bool is_table_full(char *table_id);
bool is_table_registered(char *table_id, char *dealer_id);

/* ============================================================================
 * Poker Game State (internal - use poker_vdxf.h for public API)
 * ============================================================================ */
int32_t get_player_id(int *player_id);
int32_t join_table();
int32_t find_table();
void process_block(char *block_hash);
int32_t process_payin_tx_data(char *txid, cJSON *payin_tx_data);

/* ============================================================================
 * Dealer/Table Registry (internal - use poker_vdxf.h for public API)
 * ============================================================================ */
cJSON *list_dealers();
void list_tables();
int32_t verify_poker_setup();

/* ============================================================================
 * Dispute Resolution
 * ============================================================================ */

/**
 * Player raises a dispute for a game
 * @param game_id The game ID to dispute
 * @param reason The reason: "no_payout", "game_aborted", "timeout"
 * @return OK on success, error code on failure
 */
int32_t player_raise_dispute(const char *game_id, const char *reason);

/**
 * Cashier polls for and processes pending disputes
 * @param known_players Array of known player IDs to check
 * @param num_players Number of players
 * @return Number of disputes processed
 */
int32_t cashier_poll_disputes(const char **known_players, int32_t num_players);

/**
 * Player checks dispute result
 * @param game_id The game ID that was disputed
 * @return cJSON with dispute result or NULL
 */
cJSON *player_check_dispute_result(const char *game_id);

/* ============================================================================
 * NOTE: For poker-specific operations, prefer using poker_vdxf.h which
 * provides a cleaner API with the poker_* prefix functions.
 * ============================================================================ */

#endif
