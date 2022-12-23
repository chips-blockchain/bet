#include "bet.h"
#include "deck.h"

struct pair256 gen_keypair()
{
	struct pair256 key;
	key.priv = curve25519_keypair(&key.prod);
	return key;
}

void gen_deck(struct pair256 *r, int32_t n)
{
	struct pair256 tmp;
	for (int32_t i = 0; i < n; i++) {
		tmp.priv = card_rand256(1, i);
		tmp.prod = curve25519(tmp.priv, curve25519_basepoint9());
		r[i] = tmp;
	}	
}