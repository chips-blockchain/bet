#include "bet-cli.h"

int main(int argc, char **argv)
{
	struct pair256 *cards=NULL;
	int32_t n;
	if(argc>=2)
	{
		if(strcmp(argv[1],"create-player")==0)
		{
			bet_player_create();
		}
		else if(strcmp(argv[1],"create-deck")==0)
		{
			n=atoi(argv[2]);
			cards=calloc(n,sizeof(struct pair256));
			bet_player_deck_create(n,cards);
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
	cJSON *playerInfo=NULL;
	struct pair256 key;

    key.priv=curve25519_keypair(&key.prod);
	playerInfo=cJSON_CreateObject();

	cJSON_AddStringToObject(playerInfo,"command","create-player");
    jaddbits256(playerInfo,"PrivKey",key.priv);    
    jaddbits256(playerInfo,"PubKey",key.prod);

	printf("%s",cJSON_Print(playerInfo));

	return(key);
}

void bet_player_deck_create(int n,struct pair256 *cards)
{
	int32_t i; 
	struct pair256 tmp;
    char str[65];
    for (i=0; i<n; i++) {
        tmp.priv = bet_curve25519_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        cards[i] = tmp;
		printf("\n%d card:",i);
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

