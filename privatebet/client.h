#include "bet.h"


int32_t BET_client_onechip(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_gameeval(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_tablestatus(cJSON *msgjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_client_gamestart(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_client_gamestarted(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_perm(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_endround(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_client_MofN(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_senderid(cJSON *argjson,struct privatebet_info *bet);
int32_t BET_clientupdate(cJSON *argjson,uint8_t *ptr,int32_t recvlen,struct privatebet_info *bet,struct privatebet_vars *vars); // update game state based on host broadcast
void BET_clientloop(void *_ptr);
char *enc_share_str(char hexstr[177],struct enc_share x);
void* BET_request(void* _ptr);
void* BET_response(void* _ptr);
bits256 BET_request_share(int32_t ofCardID,int32_t ofPlayerID,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key);
void BET_give_share(cJSON *shareInfo,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key);
struct enc_share get_API_enc_share(cJSON *obj);
void* BET_clientplayer(void * _ptr);
void* BET_clientbvv(void * _ptr);
int32_t BET_p2p_bvv_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_bvvcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_p2p_bvvloop(void *_ptr);
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
int32_t BET_p2p_clientupdate(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars); // update game state based on host broadcast
void BET_p2p_clientloop(void * _ptr);
void BET_p2p_clientloop_test(void * _ptr);

int32_t LN_get_channel_status(char *id);
int32_t BET_p2p_client_player_ready(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_p2p_table_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
void display_cards(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_player_reset(struct privatebet_info *bet,struct privatebet_vars *vars);

/* REST API's */
cJSON* BET_rest_client_join(cJSON *argjson);
int32_t BET_rest_bvv(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player(struct lws *wsi, cJSON *argjson);

void BET_test_function();
int32_t BET_p2p_clientupdate_test(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars); // update game state based on host broadcast


void BET_p2p_bvvloop_test(void *_ptr);
int32_t BET_p2p_bvvcommand_test(struct lws *wsi, cJSON *argjson);

void BET_test_function_bvv(void* _ptr);

int32_t BET_rest_player_reset(struct lws *wsi,cJSON * argjson);
void BET_rest_BVV_reset();
bits256 BET_get_deckid(int32_t playerID);

void BET_push_client_blindInfo(cJSON *blindInfo);




