#include "bet.h"

struct pair256 bet_player_create();
void bet_player_deck_create(int n,struct pair256 *cards);
bits256 bet_curve25519_rand256(int32_t privkeyflag,int8_t index);
void bet_blind_deck(char *deckStr,char *pubKeyStr);
void bet_player_join_req(char *pubKeyStr,char *srcAddr,char *destAddr);



