#include "bet-cli.h"

void strip(char *s) {
	    char *p2 = s;
	        while(*s != '\0') {
			        if(*s != '\t' && *s != '\n') {
					            *p2++ = *s++;
						            } else {
								                ++s;
										        }
				    }
		    *p2 = '\0';
		    printf("\n%s:%d:%s",__FUNCTION__,__LINE__,p2);
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
		else if(strcmp(argv[1],"player-deck-blind")==0)
		{
			for(int i=0;i<strlen(argv[2]);i++)
			{
				if(((i+2)<strlen(argv[2]))&&(strncmp(argv[2]+i,"\\n",2)==0))
				{
					t[l++]=0x0A;
					i+=1;
				}
				else if(((i+1)<strlen(argv[2]))&&(strncmp(argv[2]+i,"\\t",2)==0))
				{
					t[l++]=0x20;
					i+=1;
				}
				else if(((i+1)<strlen(argv[2]))&&(strncmp(argv[2]+i,"\"",2)==0))
				{
					t[l++]=0x22;
				}
				else 	
					t[l++]=argv[2][i];
			}
			t[l]='\0';
			printf("%s",t);
			cardsInfo=cJSON_Parse(t);
			n=jint(cardsInfo,"Number Of Cards");
			printf("\nNumber of Cards %d",n);
		
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
void bet_player_deck_blind(struct pair256 *cards,struct pair256 key,int32_t n)
{
    int32_t i; 
    for (i=0; i<n; i++)
    {
		cards[i].prod=curve25519(cards[i].priv,key.prod);
	}
    
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

