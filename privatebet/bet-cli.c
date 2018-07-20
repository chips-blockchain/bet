#include "bet-cli.h"

char* bet_strip(char *s) {
	    char *t = NULL;
	    int32_t l=0;
		t=calloc(strlen(s),sizeof(char));
		for(int i=0;i<strlen(s);i++)
		{
			if(((i+1)<strlen(s))&&(strncmp(s+i,"\\n",2)==0))
			{
				t[l++]=0x0A;
				i+=1;
			}
			else if(((i+1)<strlen(s))&&(strncmp(s+i,"\\t",2)==0))
			{
				t[l++]=0x20;
				i+=1;
			}
			else if(((i+1)<strlen(s))&&(strncmp(s+i,"\"",2)==0))
			{
				t[l++]=0x22;
			}
			else	
				t[l++]=s[i];
		}
		t[l]='\0';
		return t;
}

int main(int argc, char **argv)
{
	struct pair256 *cards=NULL,key;
	cJSON *cardsInfo=NULL;
	char t[1024];
	int32_t n,l=0;
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
		else if(strcmp(argv[1],"blind-deck")==0)
		{
			bet_player_deck_blind(argv[2],argv[3]);
		}
		else
		{
			printf("\nCommand Not Found");
		}
	}
	end:
		//printf("\nInvalid Arguments");
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
    jaddbits256(playerInfo,"PubKey",key.prod);
	printf("Player PubKey: %s",cJSON_Print(cJSON_CreateString(cJSON_Print(playerInfo))));

	jaddbits256(playerInfo,"PrivKey",key.priv); 
	printf("%s",cJSON_Print(playerInfo));
	cJSON_Delete(playerInfo);

	return(key);
}

void bet_player_deck_create(int n,struct pair256 *cards)
{
	cJSON *deckInfo=NULL,*cardsInfo=NULL,*temp=NULL;
	int32_t i; 
	struct pair256 tmp;
	char *rendered=NULL;	

	deckInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(deckInfo,"command","create-deck");
	cJSON_AddNumberToObject(deckInfo,"Number Of Cards",n);
	
	cJSON_AddItemToObject(deckInfo,"CardsInfo",cardsInfo=cJSON_CreateArray());
	
	
    for (i=0; i<n; i++) {
	temp=cJSON_CreateObject();
        tmp.priv = bet_curve25519_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        cards[i] = tmp;

		cJSON_AddNumberToObject(temp,"Card Number",i);	
		jaddbits256(temp,"PrivKey",cards[i].priv);
		jaddbits256(temp,"PubKey",cards[i].prod);
		cJSON_AddItemToArray(cardsInfo,temp);
    }
    	printf("\ncards:%d",jint(deckInfo,"Number Of Cards"));
    	printf("\n%s",cJSON_Print(deckInfo));
	printf("\n%s",cJSON_Print(cJSON_CreateString(cJSON_Print(deckInfo))));
	cJSON_Delete(deckInfo);
	
    
}
void bet_blind_deck(char *deckStr,char *pubKeyStr)
{
	bits256 key,pubKey,privKey;
	cJSON *deckInfo=NULL,*cardsInfo=NULL,*card,*keyInfo=NULL,*blindDeckInfo=NULL;
    int32_t i,n; 
	char str[65];
	struct pair256 *blindCards=NULL;

	printf("\n%s",bet_strip(deckStr));
	
	deckInfo=cJSON_CreateObject();
	deckInfo=cJSON_Parse(bet_strip(deckStr));
	if(deckInfo)
	{
		n=jint(deckInfo,"Number Of Cards");
		printf("\nNumber Of Cards:%d",n);
		blindCards=calloc(n,sizeof(struct pair256));
		cardsInfo=cJSON_GetObjectItem(deckInfo,"CardsInfo");
		for(i=0;i<n;i++)
		{
			card=cJSON_GetArrayItem(cardsInfo,i);
			printf("\nCard Number:%d",jint(card,"Card Number"));
			printf("\nPrivKey:%s",bits256_str(str,jbits256(card,"PrivKey")));
			printf("\nPubKey:%s",bits256_str(str,jbits256(card,"PubKey")));
			blindCards[i].priv=jbits256(card,"PrivKey");
			
		}
	}

	keyInfo=cJSON_CreateObject();
	keyInfo=cJSON_Parse(bet_strip(pubKeyStr));
	if(keyInfo)
	{
		key=jbits256(keyInfo,"PubKey");
	}
	
    for (i=0; i<n; i++)
    {
		blindCards[i].prod=curve25519(blindCards[i].priv,key);
	}
	if(cardsInfo)
		cJSON_Delete(cardsInfo);
	blindDeckInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(blindDeckInfo,"command","blind-deck");
	cJSON_AddItemToObject(blindDeckInfo,"BlindDeck",cardsInfo=cJSON_CreateObject())
	for(i=0;i<n;i++)
	{
		card=cJSON_CreateObject();
		cJSON_AddNumberToObject(card,"Card Number",i);
		jaddbits256(card,"PrivKey",blindCards[i].priv);
		jaddbits256(card,"BlindPrivKey",blindCards[i].prod);
		cJSON_AddItemToArray(blindDeckInfo,card);
	}

	printf("\nBlinded Deck:\n%s",cJSON_Print(blindDeckInfo));
	
   
}

int32_t bet_player_join_req(char *pubKey)
{
	cJSON *joinInfo=NULL;
	char *rendered=NULL;
	joinInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(joinInfo,"command","player-join-req");
	jaddstr(joinInfo,"PubKey",pubKey);
	printf("\n%s",cJSON_Print(joinInfo));

	
	
	return 1;
	
	
}

