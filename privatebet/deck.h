#include "bet.h"
struct pair256 gen_keypair();
void gen_deck(struct pair256 *r, int32_t n);
void shuffle_deck(struct pair256 *r, int32_t n, int32_t *perm);
void shuffle_deck_db(bits256 *r, int32_t n, int32_t *perm);
