#include "bet.h"
int32_t bet_dcv_deck_init_info(cJSON *argjson, struct privatebet_info *bet,
			       struct privatebet_vars *vars);
int32_t bet_dcv_init(cJSON *argjson, struct privatebet_info *bet,
		     struct privatebet_vars *vars);
int32_t bet_dcv_start(struct privatebet_info *bet, int32_t peerid);
int32_t bet_player_join_req(cJSON *argjson, struct privatebet_info *bet,
			    struct privatebet_vars *vars);
int32_t bet_dcv_turn(cJSON *argjson, struct privatebet_info *bet,
		     struct privatebet_vars *vars);
int32_t bet_relay(cJSON *argjson, struct privatebet_info *bet,
		  struct privatebet_vars *vars);
int32_t bet_check_player_ready(cJSON *playerReady, struct privatebet_info *bet,
			       struct privatebet_vars *vars);
int32_t bet_dcv_backend(cJSON *argjson, struct privatebet_info *bet,
			struct privatebet_vars *vars);
void bet_dcv_backend_loop(void *_ptr);
int32_t bet_receive_card(cJSON *playerCardInfo, struct privatebet_info *bet,
			 struct privatebet_vars *vars);
void bet_dcv_reset(struct privatebet_info *bet, struct privatebet_vars *vars);
void bet_dcv_force_reset(struct privatebet_info *bet,
			 struct privatebet_vars *vars);
void bet_dcv_frontend_loop(void *_ptr);
void bet_dcv_live_loop(void *_ptr);

int32_t bet_chat(struct lws *wsi, cJSON *argjson);
int32_t bet_seats(struct lws *wsi, cJSON *argjson);
int32_t bet_game(struct lws *wsi, cJSON *argjson);
int32_t bet_dcv_frontend(struct lws *wsi, cJSON *argjson);

int32_t bet_evaluate_hand(cJSON *playerCardInfo, struct privatebet_info *bet,
			  struct privatebet_vars *vars);

void bet_push_dcv_to_gui(cJSON *argjson);
void dcv_lws_write(cJSON *data);
