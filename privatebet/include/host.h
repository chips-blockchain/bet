#include "bet.h"

void bet_set_table_id();
int32_t bet_dcv_deck_init_info(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_start(struct privatebet_info *bet, int32_t peerid);
int32_t bet_player_join_req(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_relay(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_check_player_ready(cJSON *playerReady, struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_dcv_backend_loop(void *_ptr);
int32_t bet_receive_card(cJSON *playerCardInfo, struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_reset_all_dcv_params(struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_dcv_reset(struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_dcv_frontend_loop(void *_ptr);
void bet_chat(struct lws *wsi, cJSON *argjson);
void initialize_seat(cJSON *seat_info, char *name, int32_t seat, int32_t chips, int32_t empty, int32_t playing);
int32_t bet_seats(struct lws *wsi, cJSON *argjson);
int32_t bet_game(struct lws *wsi, cJSON *argjson);
int32_t bet_dcv_frontend(struct lws *wsi, cJSON *argjson);
int32_t bet_evaluate_hand(struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_push_dcv_to_gui(cJSON *argjson);
void bet_dcv_lws_write(cJSON *data);
void bet_init_player_seats_info();
cJSON *bet_get_seats_json(int32_t max_players);
void bet_dcv_bvv_backend_loop(void *_ptr);
int32_t bet_dcv_bvv_backend(cJSON *argjson, struct dcv_bvv_sock_info *bet);
