#include "bet-cli.h"

int main(int argc, char **argv)
{
	struct pair256 *cards=NULL,key;
	cJSON *cardsInfo=NULL;
	int32_t n;
	if(argc>=2)
	{
		if(strcmp(argv[1],"create-player")==0)
		{
			bet_player_create();
		}
		else if(strcmp(argv[1],"create-deck")==0)
		{
			if(argc!=3)
				goto end;
			n=atoi(argv[2]);
			cards=calloc(n,sizeof(struct pair256));
			bet_player_deck_create(n,cards);
		}
		else if(strcmp(argv[1],"player-deck-blind")==0)
		{
			// bet_player_deck_blind(cards,key,n);
			cardsInfo=cJSON_Parse(argv[2]);
			n=jint(cardsInfo,"Number Of Cards");
			printf("\nNumber of Cards %d",n);
		}
		else
		{
			printf("\nCommand Not Found");
		}
	}
	end:
		printf("\nInvalid Arguments");
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
	cJSON_Delete(playerInfo);

	return(key);
}

void bet_player_deck_create(int n,struct pair256 *cards)
{
	cJSON *deckInfo=NULL,*cardsInfo=NULL,*temp=NULL;
	int32_t i; 
	struct pair256 tmp;
	

	deckInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(deckInfo,"command","create-deck");
	cJSON_AddNumberToObject(deckInfo,"Number Of Cards",n);
	
	cJSON_AddItemToObject(deckInfo,"CardsInfo",cardsInfo=cJSON_CreateArray());
	
	
	temp=cJSON_CreateObject();
    for (i=0; i<n; i++) {
        tmp.priv = bet_curve25519_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        cards[i] = tmp;

		cJSON_AddNumberToObject(temp,"Card Number",i);	
		jaddbits256(temp,"PrivKey",cards[i].priv);
		jaddbits256(temp,"PubKey",cards[i].prod);

		cJSON_AddItemToArray(cardsInfo,temp);
    }
	

	printf("%s",cJSON_Print(deckInfo));
	cJSON_Delete(deckInfo);
    
}
void bet_player_deck_blind(struct pair256 *cards,struct pair256 key,int32_t n)
{
    int32_t i; 
    for (i=0; i<n; i++)
    {
		cards[i].prod=curve25519(cards[i].priv,key.prod);
	}
    
}

