#include "bet.h"
#include "blinder.h"
#include "vdxf.h"
#include "deck.h"
#include "cards777.h"

char all_t_b_p_keys[all_t_b_p_keys_no][128] = { T_B_DECK_KEY,    T_B_P1_DECK_KEY, T_B_P2_DECK_KEY, T_B_P3_DECK_KEY, T_B_P4_DECK_KEY,
			       T_B_P5_DECK_KEY, T_B_P6_DECK_KEY, T_B_P7_DECK_KEY, T_B_P8_DECK_KEY, T_B_P9_DECK_KEY };

char all_t_b_p_key_names[all_t_b_p_keys_no][128] = { "t_b_deck",    "t_b_p1_deck", "t_b_p2_deck", "t_b_p3_deck", "t_b_p4_deck",
				    "t_b_p5_deck", "t_b_p6_deck", "t_b_p7_deck", "t_b_p8_deck", "t_b_p9_deck" };


struct b_deck_info_struct b_deck_info;

int32_t cashier_sb_deck(char *id, bits256 *d_blinded_deck, int32_t player_id)
{
	char str[65], *game_id_str = NULL;
	cJSON *b_blinded_deck = NULL;

	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, d_blinded_deck[i]));
	}
	shuffle_deck_db(d_blinded_deck, CARDS777_MAXCARDS, b_deck_info.b_permi);
	blind_deck_b(d_blinded_deck, CARDS777_MAXCARDS, b_deck_info.cashier_r[player_id]);

	dlg_info("Cashier blinding values for the player ::%d", player_id);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, b_deck_info.cashier_r[player_id].priv));
	}

	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(b_blinded_deck, bits256_str(str, d_blinded_deck[i]));
	}

	cJSON *out = append_cmm_from_id_key_data_cJSON(id, get_key_data_vdxf_id(all_t_b_p_keys[player_id], game_id_str),
						       b_blinded_deck, true);

	dlg_info("%s", cJSON_Print(out));
}

void cashier_init_deck()
{
	bet_permutation(b_deck_info.b_permi, CARDS777_MAXCARDS);
	for(int32_t i=0; i<CARDS777_MAXPLAYERS; i++) {
		gen_deck(b_deck_info.cashier_r[i], CARDS777_MAXCARDS);
	}
}

void test_cashier_sb(char *id)
{
	char *game_id_str = NULL, str[65];
	cJSON *t_d_p1_deck_info = NULL, *t_d_p2_deck_info = NULL;
	bits256 t_d_p1_deck[CARDS777_MAXCARDS], t_d_p2_deck[CARDS777_MAXCARDS];

	cashier_init_deck();
	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);

	t_d_p1_deck_info= get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_D_P1_DECK_KEY, game_id_str));
	for (int32_t i = 0; i < cJSON_GetArraySize(t_d_p1_deck_info); i++) {
		t_d_p1_deck[i] = jbits256i(t_d_p1_deck_info, i);
	}
	cashier_sb_deck(id, t_d_p1_deck, 1);

	t_d_p2_deck_info = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_PLAYER2_KEY, game_id_str));
	for (int32_t i = 0; i < cJSON_GetArraySize(t_d_p2_deck_info); i++) {
		t_d_p2_deck[i] = jbits256i(t_d_p2_deck_info, i);
	}
	cashier_sb_deck(id, t_d_p2_deck, 2);

}
