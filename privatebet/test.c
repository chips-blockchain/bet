#include "bet.h"
#include "test.h"
#include "cards777.h"
#include "vdxf.h"
#include "deck.h"

int32_t deck_size = 3;

void test_sg()
{
	char hexstr[65];
	struct pair256 k1, k2;	
	bits256 p1, p2, r;
	
	k1.priv = curve25519_keypair(&k1.prod);
	k2.priv = curve25519_keypair(&k2.prod);
	
	dlg_info("k1.priv::%s", bits256_str(hexstr,k1.priv));
	dlg_info("k1.pub::%s", bits256_str(hexstr,k1.prod));

	dlg_info("k2.priv::%s", bits256_str(hexstr,k2.priv));
	dlg_info("k2.pub::%s", bits256_str(hexstr,k2.prod));

	p1 = curve25519(k1.priv,k2.prod);
	p2 = curve25519(k2.priv,k1.prod);
	dlg_info("p1::%s", bits256_str(hexstr,p1));
	dlg_info("p2::%s", bits256_str(hexstr,p2));

	r= rand256(1);
	
	p1 = curve25519(r,p1);
	p1 = fmul_donna(crecip_donna(r),p1);
	dlg_info("p1 ::%s", bits256_str(hexstr,p1));
}

void test_permutations()
{
	int cards[52];

	printf("\nOriginal Cards\n");
	for (int i = 0; i < CARDS777_MAXCARDS; i++) {
		cards[i] = i;
		printf("%d ", cards[i]);
	}

	bet_permutation(player_info.permis, CARDS777_MAXCARDS);

	printf("\nPermuted Cards\n");
	for (int i = 0; i < CARDS777_MAXCARDS; i++) {
		cards[i] = player_info.permis[i];
		printf("%d ", cards[i]);
	}

	bet_r_permutation(player_info.permis, CARDS777_MAXCARDS, player_info.r_permis);
	printf("\nReverse Permuted Cards\n");
	for (int i = 0; i < CARDS777_MAXCARDS; i++) {
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
	//append_t_key(id, T_PLAYER1_KEY, key_info);
}

void test_deck_shuffling()
{
	int p_permi[deck_size],p_r_permi[deck_size],d_permi[deck_size],b_permi[deck_size];
	struct pair256 player_kp;
	struct pair256 player_r[deck_size];
	char pubstr[65], privstr[65];

	bits256 player_r_inv[deck_size];
		
	struct pair256 dealer_r[deck_size];
	bits256 dealer_b[deck_size], dealer_r_inv[deck_size];
	
	struct pair256 blinder_r[deck_size];
	bits256 blinder_b[deck_size], blinder_r_inv[deck_size];

	bits256 final_deck[deck_size], unshuffled_deck[deck_size];
		
	bet_permutation(p_permi, deck_size);
	bet_r_permutation(p_permi, deck_size, p_r_permi);
	bet_permutation(d_permi, deck_size);
	bet_permutation(b_permi, deck_size);

	player_kp = gen_keypair();
	
	dlg_info("Player Keys:: priv ::%s pub::%s", bits256_str(privstr,player_kp.priv),bits256_str(pubstr,player_kp.prod));

	gen_deck(player_r,deck_size);
	dlg_info("Deck");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("priv ::%s pub::%s", bits256_str(privstr,player_r[i].priv),bits256_str(pubstr,player_r[i].prod));
	}
	dlg_info("Player permutation");
	for(int32_t i=0; i<deck_size; i++){
		printf("%d ", p_permi[i]+1);
	}

	shuffle_deck(player_r,deck_size,p_permi);
	dlg_info("Shuffled deck Player");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,player_r[i].prod));
	}

	//From here on dealer
	for(int32_t i=0; i<deck_size; i++){
		dealer_b[i] = player_r[i].prod; //rG
	}
	shuffle_deck_db(dealer_b,deck_size,d_permi);

	dlg_info("Dealer permutation");
	for(int32_t i=0; i<deck_size; i++){
		printf("%d ", d_permi[i]+1);
	}
	
	dlg_info("Shuffled deck Dealer");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,dealer_b[i]));
	}
		
	gen_deck(dealer_r,deck_size);
	blind_deck(dealer_b,deck_size,dealer_r);

	dlg_info("Blinded deck Dealer");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,dealer_b[i]));
	}
		
	//From here on blinder
	for(int32_t i=0; i<deck_size; i++){
		blinder_b[i] = dealer_b[i];
	}
	shuffle_deck_db(blinder_b,deck_size,b_permi);

	dlg_info("Blinder permutation");
	for(int32_t i=0; i<deck_size; i++){
		printf("%d ", b_permi[i]+1);
	}
	
	dlg_info("Shuffled deck at Blinder");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,blinder_b[i]));
	}

	gen_deck(blinder_r,deck_size);
	blind_deck(blinder_b,deck_size,blinder_r);

	dlg_info("Blinded deck at Blinder");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,blinder_b[i]));
	}

	//From here on player
	for(int32_t i=0; i<deck_size; i++){
		final_deck[i] = blinder_b[i];
		unshuffled_deck[i] = blinder_b[i];
	}
		
	dlg_info("\nPlayer reverse permutation");
	for(int32_t i=0; i<deck_size; i++){
		printf("%d ", p_r_permi[i]+1);
	}

	shuffle_deck_db(unshuffled_deck,deck_size,p_r_permi);
	
	dlg_info("UnShuffled blinded deck Player");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,unshuffled_deck[i]));
	}

	//Computing blinder_r_inverse
	for(int32_t i=0; i<deck_size; i++){
		blinder_r_inv[i] = crecip_donna(blinder_r[i].priv);
	}

	//Remove blinders blind
	for(int32_t i=0; i<deck_size; i++){
		final_deck[i] = fmul_donna(final_deck[i],blinder_r_inv[i]);
	}

	dlg_info("Deck after removing blinders blind at Player");
	for(int32_t i=0; i<deck_size; i++){
		dlg_info("pub::%s", bits256_str(pubstr,final_deck[i]));
	}
	bits256 card = final_deck[0];

	//Computing player_r_inverse
	for(int32_t i=0; i<deck_size; i++){
		player_r_inv[i] = crecip_donna(player_r[i].priv);
	}
	dlg_info("Finding card");
	for(int32_t i=0; i<deck_size; i++){
		bits256 card = final_deck[0];
		card = fmul_donna(player_r_inv[i],card);
		for(int32_t j=0; j<deck_size; j++){
			dlg_info("%s::%s", bits256_str(pubstr,card), bits256_str(privstr,dealer_r[j].prod));
		}
	}
	
}

