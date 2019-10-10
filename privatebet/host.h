int32_t BET_p2p_host_deck_init_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_host_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_host_start_init(struct privatebet_info *bet,int32_t peerid);
int32_t BET_p2p_client_join_req(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_dcv_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_highest_card(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_relay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_broadcast_table_info(struct privatebet_info *bet);
int32_t BET_p2p_check_player_ready(cJSON *playerReady,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_settle_game(cJSON *payInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_dcv_backend(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_dcv_backend_loop(void *_ptr);
int32_t BET_receive_card(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_evaluate_hand(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_DCV_reset(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_DCV_force_reset(struct privatebet_info *bet,struct privatebet_vars *vars);


void BET_dcv_frontend_loop(void * _ptr);

/*
REST API's starts from here
*/
int32_t BET_rest_default(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_chat(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_seats(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_game(struct lws *wsi, cJSON *argjson);
int32_t BET_dcv_frontend(struct lws *wsi, cJSON *argjson);


int32_t BET_evaluate_hand_test(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars);

void BET_p2p_host_blinds_info(struct lws *wsi);
void BET_push_host(cJSON *argjson);
void dcv_lws_write(cJSON *data);
int32_t BET_rest_evaluate_hand(struct lws *wsi);
int32_t BET_rest_dcv_turn(struct lws *wsi);
int32_t BET_rest_relay(struct lws *wsi, cJSON *argjson);




