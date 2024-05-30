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

void shuffle_deck(struct pair256 *r, int32_t n, int32_t *perm)
{
	struct pair256 *tmp = NULL;
	tmp = calloc(n, sizeof(struct pair256));
	for (int32_t i = 0; i < n; i++) {
		tmp[i] = r[perm[i]];
	}
	for (int32_t i = 0; i < n; i++) {
		r[i] = tmp[i];
	}
}

void shuffle_deck_db(bits256 *r, int32_t n, int32_t *perm)
{
	bits256 *tmp = NULL;
	tmp = calloc(n, sizeof(bits256));
	for (int32_t i = 0; i < n; i++) {
		tmp[i] = r[perm[i]];
	}
	for (int32_t i = 0; i < n; i++) {
		r[i] = tmp[i];
	}
}

void blind_deck_d(bits256 *r, int32_t n, struct pair256 *blinder)
{
	for (int32_t i = 0; i < n; i++) {
		r[i] = curve25519(blinder[i].priv, r[i]);
	}
}

void blind_deck_b(bits256 *r, int32_t n, struct pair256 *blinder)
{
	char hexstr[65];

	dlg_info("Cashier blinding the deck");
	for (int32_t i = 0; i < n; i++) {
		dlg_info("Dealer card::%s", bits256_str(hexstr, r[i]));
		dlg_info("BV::%s", bits256_str(hexstr, blinder[i].priv));
		r[i] = fmul_donna(blinder[i].priv, r[i]);
		dlg_info("Blinded card::%s", bits256_str(hexstr, r[i]));
	}
}
