#include "bet.h"
#include "test.h"

void test_permutations()
{
	int cards[52];

	printf("\nOriginal Cards\n");
	for(int i=0; i<CARDS777_MAXCARDS; i++) {
		cards[i] = i;
		printf("%d ", cards[i]);
	}	
	
	bet_permutation(player_info.permis, CARDS777_MAXCARDS);

	printf("\nPermuted Cards\n");
	for(int i=0; i<CARDS777_MAXCARDS; i++) {
		cards[i] = player_info.permis[i];
		printf("%d ", cards[i]);
	}	
	
	bet_r_permutation(player_info.permis, CARDS777_MAXCARDS, player_info.r_permis);
	printf("\nReverse Permuted Cards\n");
	for(int i=0; i<CARDS777_MAXCARDS; i++) {
		cards[i] = player_info.permis[player_info.r_permis[i]];
		printf("%d ", cards[i]);
	}	
	
}

void test_crypto()
{
	struct pair256 key;
	bits256 r1, r2, r3;
	bits256 p, d, b, r1_dup, d_pub;
	bits256 p_dup, d_dup;
	char hexstr[65];

	key.priv = curve25519_keypair(&key.prod);
	r1 = card_rand256(1, 10);
	r2 = curve25519_keypair(&d_pub);
	r3 = rand256(1);

	p = fmul_donna(r1, key.prod);
	d = fmul_donna(p, r2);
	b = fmul_donna(d, r3);

	d_dup = fmul_donna(b, crecip_donna(r3));
	p_dup = fmul_donna(d_dup, crecip_donna(r2));
	r1_dup = fmul_donna(p_dup, crecip_donna(key.prod));

	dlg_info("%s::%d::p::%s\n", __func__, __LINE__, bits256_str(hexstr, p));
	dlg_info("%s::%d::d::%s\n", __func__, __LINE__, bits256_str(hexstr, d));
	dlg_info("%s::%d::b::%s\n", __func__, __LINE__, bits256_str(hexstr, b));

	dlg_info("%s::%d::p_dup %s\n", __func__, __LINE__, bits256_str(hexstr, p_dup));
	dlg_info("%s::%d::r1_dup::%s\n", __func__, __LINE__, bits256_str(hexstr, r1_dup));

	dlg_info("%s::%d::card r1 :: %d r1_dup::%d\n", __func__, __LINE__, r1.bytes[30], r1_dup.bytes[30]);
}

void test_append_t_key()
{
	char *id = "sg777_t";
	cJSON *key_info = NULL;

	key_info = cJSON_CreateString("Hello World");
	append_t_key(id, T_PLAYER1_KEY, key_info);
}
