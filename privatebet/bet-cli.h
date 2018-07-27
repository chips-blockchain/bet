#include "bet.h"

struct pair256 bet_player_create();
void bet_player_deck_create(int n,struct pair256 *cards);
bits256 bet_curve25519_rand256(int32_t privkeyflag,int8_t index);
void bet_blind_deck(char *deckStr,char *pubKeyStr);
void bet_player_join_req(char *pubKeyStr,char *srcAddr,char *destAddr);
int32_t bet_player_init(int32_t peerID,char *deckStr,char *pubKeyStr,char *destAddr);
void bet_dcv_init(int32_t n, int32_t r, char *dcvStr);
void bet_bvv_init(int32_t peerID,int32_t n, int32_t r,char *bvvStr);
struct enc_share bet_get_enc_share(cJSON *obj);
void bet_dcv_bvv_init(int32_t peerID,int32_t n, int32_t r,char *dcvStr, char *bvvStr);









