#include <libwebsockets.h>
struct privatebet_peerln *BET_peerln_find(char *peerid);
struct privatebet_peerln *BET_peerln_create(struct privatebet_rawpeerln *raw,int32_t maxplayers,int32_t maxchips,int32_t chipsize);
int32_t BET_host_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_host_gamestart(struct privatebet_info *bet,struct privatebet_vars *vars);
void BETS_players_update(struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_rawpeerln_parse(struct privatebet_rawpeerln *raw,cJSON *item);
cJSON *BET_hostrhashes(struct privatebet_info *bet);
int32_t BET_chipsln_update(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_hostloop(void *_ptr);
void* BET_hostdcv(void * _ptr);
int32_t BET_p2p_host_deck_init_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_host_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_host_start_init(struct privatebet_info *bet);
int32_t BET_p2p_client_join_req(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_dcv_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_highest_card(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_relay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_broadcast_table_info(struct privatebet_info *bet);
int32_t BET_p2p_check_player_ready(cJSON *playerReady,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_settle_game(cJSON *payInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_p2p_hostloop(void *_ptr);
int32_t BET_receive_card(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_evaluate_hand(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_DCV_reset(struct privatebet_info *bet,struct privatebet_vars *vars);

void BET_rest_hostloop(void * _ptr);

void BET_ws_dcvloop(void * _ptr);


/*
REST API's starts from here
*/
int32_t BET_rest_default(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_seats(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_game(struct lws *wsi, cJSON *argjson);
int32_t BET_process_rest_method(struct lws *wsi, cJSON *argjson);


