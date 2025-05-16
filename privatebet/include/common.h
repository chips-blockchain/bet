#ifndef COMMON_H
#define COMMON_H

// clang-format off
#define __FUNCTION__ __func__
#define CHANNELD_AWAITING_LOCKIN    2
#define CHANNELD_NORMAL             3

#define hand_size                   7
#define no_of_hole_cards            2
#define no_of_flop_cards            3
#define no_of_turn_card             1
#define no_of_river_card            1
#define no_of_community_cards       5

#define NSUITS 4
#define NFACES 13

#define table_stack 200


enum blinds { 
	small_blind_amount = 1, 
	big_blind_amount   = 2 
};

enum bet_dcv_state { 
	dealer_table_empty = 0, 
	dealer_table_full  = 1, 
	pos_on_table_empty = 2, 
	pos_on_table_full  = 3 
};


#define mchips_msatoshichips    1000000 // 0.01mCHIPS
#define channel_fund_satoshis   16777215 // 0.167CHIPS this is the mx limit set up the c lightning node
#define satoshis                100000000 //10^8 satoshis for 1 COIN
#define satoshis_per_unit       1000
#define normalization_factor    100

#define CARDS777_MAXCARDS       10 // 52    //
#define CARDS777_MAXPLAYERS     9 // 9   //
#define CARDS777_MAXROUNDS      4 // 9   //
#define CARDS777_MAXCHIPS       1000
#define CARDS777_CHIPSIZE       (SATOSHIDEN / CARDS777_MAXCHIPS)
#define BET_PLAYERTIMEOUT       15
#define BET_GAMESTART_DELAY     10
#define BET_RESERVERATE         1.025
#define LN_FUNDINGERROR         "\"Cannot afford funding transaction\""

#define tx_spent    0
#define tx_unspent 	1

#define dealer_pub_sub_port         7797
#define dealer_push_pull_port       7798
#define cashier_pub_sub_port        7901
#define cashier_push_pull_port      7902
#define dealer_bvv_pub_sub_port     7903
#define dealer_bvv_push_pull_port   7904
#define gui_ws_port                 9000

#define default_chips_tx_fee        0.0001
#define default_bb_in_chips         0.01
#define default_min_stake_in_bb     20
#define default_max_stake_in_bb     100

#define BET_WITHOUT_LN 0
#define BET_WITH_LN    1      

extern bits256 v_hash[CARDS777_MAXCARDS][CARDS777_MAXCARDS];
extern bits256 g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];

extern bits256 all_v_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXCARDS];
extern bits256 all_g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];

extern struct privatebet_info *bet_dcv;
extern struct privatebet_vars *dcv_vars;

extern int32_t no_of_signers, max_no_of_signers, is_signed[CARDS777_MAXPLAYERS];

extern struct privatebet_info *bet_bvv;
extern struct privatebet_vars *bvv_vars;
extern struct dcv_bvv_sock_info *bet_dcv_bvv;

extern struct privatebet_info *BET_player[CARDS777_MAXPLAYERS];

extern int32_t all_sharesflag[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

extern int32_t all_player_card_values[CARDS777_MAXPLAYERS][hand_size]; // where 7 is hand_size
extern int32_t all_number_cards_drawn[CARDS777_MAXPLAYERS];
extern int32_t all_player_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
extern int32_t all_no_of_player_cards[CARDS777_MAXPLAYERS];
extern bits256 all_playershares[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

extern int32_t permis_d[CARDS777_MAXCARDS], permis_b[CARDS777_MAXCARDS];
extern bits256 deckid;
extern uint8_t sharenrs[256];
extern bits256 playershares[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

extern struct lws *wsi_global_client;

extern struct cashier *cashier_info;

extern int32_t max_players;

extern int32_t no_of_notaries;
extern int32_t threshold_value;

extern char **notary_node_ips;
extern char **notary_node_pubkeys;

extern double SB_in_chips;
extern double BB_in_chips;
extern double table_stake_in_chips;
extern double chips_tx_fee;
extern double table_min_stake;
extern double table_max_stake;
extern double table_stack_in_bb;

extern char dev_fund_addr[64];
extern double dev_fund_commission;
extern char *legacy_m_of_n_msig_addr;

extern int32_t no_of_txs;
extern char tx_ids[CARDS777_MAXPLAYERS][100];
extern int32_t *notary_status;
extern int32_t live_notaries;

extern char table_id[65];
extern int32_t bvv_state;
extern int32_t dcv_state;

extern char dealer_ip[20];
extern char cashier_ip[20];
extern char unique_id[65];

extern double dcv_commission_percentage;
extern double max_allowed_dcv_commission;
extern char dcv_hosted_gui_url[128];

int32_t is_table_private;
char table_password[128];
char player_name[128];
char verus_pid[128]; // This is the verus ID owned by the player to which player updates during the game.

int32_t bet_ln_config; 
extern int64_t sc_start_block;

extern char dealer_ip_for_bvv[128];
#endif
