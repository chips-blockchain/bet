#include "bet.h"

char *enc_share_str(char hexstr[177], struct enc_share x);
struct enc_share get_API_enc_share(cJSON *obj);
int32_t bet_bvv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_bvv_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_bvv_reset(struct privatebet_info *bet, struct privatebet_vars *vars);

bits256 bet_decode_card(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int32_t cardid,
			int32_t *retval);
int32_t bet_client_receive_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_ask_share(struct privatebet_info *bet, int32_t cardid, int32_t playerid, int32_t card_type,
			     int32_t other_player);
int32_t bet_client_give_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_get_own_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_bvv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_dcv_init(cJSON *dcv_info, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_join_res(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_client_join(cJSON *argjson, struct privatebet_info *bet);
void bet_player_backend_loop(void *_ptr);

int32_t bet_player_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
void display_cards();
int32_t bet_player_reset(struct privatebet_info *bet, struct privatebet_vars *vars);

void bet_player_frontend_read_loop();
void bet_player_frontend_write_loop();

void bet_player_frontend_loop();
int32_t bet_player_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_frontend(struct lws *wsi, cJSON *argjson);
void bet_bvv_backend_loop(void *_ptr);
bits256 bet_get_deckid(int32_t playerID);
void bet_push_client(cJSON *argjson);

void rest_push_cards(struct lws *wsi, cJSON *argjson, int32_t this_playerID);
int32_t ln_pay_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
void player_lws_write(cJSON *data);
void rest_display_cards(cJSON *argjson, int32_t this_playerID);
cJSON *bet_get_available_dealers();
int32_t bet_player_stack_info_req(struct privatebet_info *bet);
void bet_handle_player_error(struct privatebet_info *bet, int32_t err_no);
