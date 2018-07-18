#include "bet-cli.h"

int main(int argc, char *argv)
{
	if(argc>2)
	{
		if(strcmp(argv[1],"create-player")==0)
		{
			bet_player_create();
		}
	}
	return 0;
}
bits256 bet_curve25519_rand256(int32_t privkeyflag,int8_t index)
{
    bits256 randval;
    OS_randombytes(randval.bytes,sizeof(randval));
    if ( privkeyflag != 0 )
        randval.bytes[0] &= 0xf8, randval.bytes[31] &= 0x7f, randval.bytes[31] |= 0x40;
    randval.bytes[30] = index;
    return(randval);
}

struct pair256 bet_player_create()
{
	struct pair256 key;
	char str[65];
    key.priv=curve25519_keypair(&key.prod);
	printf("\nPlayer priv key:%s",bits256_str(str,key.priv));
	printf("\nPlayer pub key:%s",bits256_str(str,key.prod));
    return(key);
}

void bet_player_deck_create(bits256 *privkeys,bits256 *pubprod,int n,struct pair256 *cards)
{
	int32_t i; 
	struct pair256 tmp;
    char str[65];
    for (i=0; i<n; i++) {
        tmp.priv = bet_curve25519_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        cards[i] = tmp;
		printf("\n%d card:");
		printf("\n privkey:%s",bits256_str(str,cards[i].priv));
		printf("\n privkey:%s",bits256_str(str,cards[i].prod));
    }
    
}
void bet_player_deck_blind(struct pair256 *cards,struct pair256 key,int32_t n)
{
    int32_t i; 
    for (i=0; i<n; i++)
    {
		cards[i].prod=curve25519(cards[i].priv,key.prod);
	}
    
}

