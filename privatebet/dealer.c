#include "bet.h"
#include "dealer.h"
#include "vdxf.h"
#include "deck.h"
#include "cards777.h"

struct d_deck_info_struct d_deck_info;

cJSON *add_dealer(char *dealer_id)
{
	cJSON *dealers_info = NULL, *dealers = NULL, *out = NULL;

	if (!dealer_id)
		return NULL;

	dealers_info = cJSON_CreateObject();
	dealers = cJSON_CreateArray();
	jaddistr(dealers, dealer_id);

	cJSON_AddItemToObject(dealers_info, "dealers", dealers);
	out = update_cmm_from_id_key_data_cJSON("dealers", DEALERS_KEY, dealers_info, false);

	dlg_info("%s", cJSON_Print(out));
	return out;
}

int32_t dealer_sb_deck(bits256 *player_r, int32_t player_id)
{
	shuffle_deck_db(player_r, CARDS777_MAXCARDS, d_deck_info.d_permi);
	blind_deck_d(player_r, CARDS777_MAXCARDS, d_deck_info.dealer_r);
}

void dealer_init_deck()
{
	bet_permutation(d_deck_info.d_permi, CARDS777_MAXCARDS);
	gen_deck(d_deck_info.dealer_r, CARDS777_MAXCARDS);
		
}

void test_dealer_sb()
{
	
}
