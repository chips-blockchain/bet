#include "bet.h"


char *enc_share_str(char hexstr[177],struct enc_share x);
bits256 BET_request_share(int32_t ofCardID,int32_t ofPlayerID,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key);
void BET_give_share(cJSON *shareInfo,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key);
struct enc_share get_API_enc_share(cJSON *obj);
int32_t BET_p2p_bvv_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_bvv_backend(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_BVV_reset(struct privatebet_info *bet,struct privatebet_vars *vars);




bits256 BET_p2p_decode_card(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t cardid);
int32_t BET_p2p_client_receive_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_give_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_get_own_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_bvv_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_dcv_init(cJSON *dcv_info,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_join_res(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_client_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_player_backend_loop(void * _ptr);

int32_t LN_get_channel_status(char *id);
int32_t BET_p2p_client_player_ready(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_p2p_table_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void display_cards(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_player_reset(struct privatebet_info *bet,struct privatebet_vars *vars);

/* REST API's */
cJSON* BET_rest_client_join(cJSON *argjson);
int32_t BET_rest_bvv(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player(struct lws *wsi, cJSON *argjson);

void BET_player_frontend_loop();
int32_t BET_player_backend(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars); // update game state based on host broadcast
int32_t BET_player_frontend(struct lws *wsi,cJSON *argjson);


void BET_bvv_backend_loop(void *_ptr);
int32_t BET_bvv_frontend(struct lws *wsi, cJSON *argjson);

void BET_bvv_frontend_loop(void* _ptr);

int32_t BET_rest_player_reset(struct lws *wsi,cJSON * argjson);
void BET_rest_BVV_reset();
bits256 BET_get_deckid(int32_t playerID);

void BET_push_client_blindInfo(cJSON *blindInfo);
void BET_push_client(cJSON *argjson);
int32_t BET_rest_player_join_res(cJSON *argjson);
int32_t BET_rest_player_init(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_listfunds();
void make_command(int argc, char **argv,cJSON **argjson);
int32_t BET_rest_uri(char **uri);
int32_t BET_rest_connect(char *uri);
int32_t BET_rest_fundChannel(char *channel_id);
int32_t BET_rest_pay(char *bolt11);
int32_t BET_rest_bvv_init(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_bvv_check_bvv_ready(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player_join(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_bvv_compute_init_b(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player_process_init_d(struct lws *wsi, cJSON *argjson,int32_t playerID);
int32_t BET_rest_player_process_init_b(struct lws *wsi, cJSON *argjson,int32_t playerID);
int32_t BET_rest_player_turn(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player_give_share(struct lws *wsi,cJSON *argjson);
int32_t BET_rest_player_receive_share(struct lws *wsi,cJSON *argjson);
int32_t BET_rest_player_invoice(struct lws *wsi,cJSON *argjson);
void rest_push_cards(struct lws *wsi,cJSON *argjson,int32_t this_playerID);
int32_t BET_p2p_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void player_lws_write(cJSON *data);
void rest_display_cards(cJSON *argjson,int32_t this_playerID);






void LN_connect_to_channel(char *id);
void LN_connect(char *id);







