struct pair256 deckgen_player(bits256 *playerprivs,bits256 *playercards,int32_t *permis,int32_t numcards)
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

