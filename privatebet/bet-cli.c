#include "bet-cli.h"

int main()
{
	return 0;
}

struct pair256 bet_player_create()
{
	struct pair256 key;
    key.priv=curve25519_keypair(&key.prod);
    return(key);
}

struct pair256 bet_deck_init(bits256 *privkeys,bits256 *pubprod,int32_t *permis,int32_t numcards)
{
    int32_t i; struct pair256 key,randcards[256];
    key = deckgen_common(randcards,numcards);
    BET_permutation(permis,numcards);
    for (i=0; i<numcards; i++)
    {
        playerprivs[i] = randcards[i].priv; //permis[i]
        playercards[i]=curve25519(playerprivs[i],key.prod);
    }
    return(key);
}

