#include "bet-cli.h"

char* bet_strip(char *s) 
{
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
			bet_blind_deck(argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"join-req")==0)
		{
			bet_player_join_req(argv[2],argv[3],argv[4]);
		}
		else if(strcmp(argv[1],"init-player")==0)
		{
			bet_player_init(atoi(argv[2]),argv[3],argv[4],argv[5]);
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
	printf("\nPlayer PubKey: %s",cJSON_Print(cJSON_CreateString(cJSON_Print(playerInfo))));

	jaddbits256(playerInfo,"PrivKey",key.priv); 
	printf("\n%s",cJSON_Print(playerInfo));
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
	
    for (i=0; i<n; i++) 
	{
		temp=cJSON_CreateObject();
        tmp.priv = bet_curve25519_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        cards[i] = tmp;

		cJSON_AddNumberToObject(temp,"Card Number",i);	
		jaddbits256(temp,"PrivKey",cards[i].priv);
		jaddbits256(temp,"PubKey",cards[i].prod);
		cJSON_AddItemToArray(cardsInfo,temp);
    }
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

	deckInfo=cJSON_CreateObject();
	deckInfo=cJSON_Parse(bet_strip(deckStr));
	if(deckInfo)
	{
		n=jint(deckInfo,"Number Of Cards");
		blindCards=calloc(n,sizeof(struct pair256));
		cardsInfo=cJSON_GetObjectItem(deckInfo,"CardsInfo");
		for(i=0;i<n;i++)
		{
			card=cJSON_GetArrayItem(cardsInfo,i);
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
	cJSON_AddItemToObject(blindDeckInfo,"BlindDeck",cardsInfo=cJSON_CreateObject());
	for(i=0;i<n;i++)
	{
		card=cJSON_CreateObject();
		cJSON_AddNumberToObject(card,"Card Number",i);
		jaddbits256(card,"PrivKey",blindCards[i].priv);
		jaddbits256(card,"BlindPrivKey",blindCards[i].prod);
		cJSON_AddItemToArray(cardsInfo,card);
	}

	printf("\nBlinded Deck:\n%s",cJSON_Print(blindDeckInfo));
    printf("\n%s",cJSON_Print(cJSON_CreateString(cJSON_Print(blindDeckInfo))));

}

void bet_player_join_req(char *pubKeyStr,char *srcAddr,char *destAddr)
{
	int32_t pushSock,subSock,recvlen;
	char *recvBuf=NULL,*rendered=NULL;
	bits256 pubKey;
	cJSON *keyInfo=NULL,*joinInfo=NULL;
	
	keyInfo=cJSON_CreateObject();
	keyInfo=cJSON_Parse(bet_strip(pubKeyStr));
	if(keyInfo)
	{
		pubKey=jbits256(keyInfo,"PubKey");
	}

	 joinInfo=cJSON_CreateObject();
    cJSON_AddStringToObject(joinInfo,"method","join_req");
    jaddbits256(joinInfo,"pubkey",pubKey);    
    rendered=cJSON_Print(joinInfo);
   
	
	pushSock=BET_nanosock(0,destAddr,NN_PUSH);
	subSock=BET_nanosock(0,srcAddr,NN_SUB);
	nn_send(pushSock,rendered,strlen(rendered),0);
	while(1)
	{
		if ( (recvlen= nn_recv(subSock,&recvBuf,NN_MSG,0)) > 0 )
		{
			printf("\nResponse Received:%s",recvBuf);
			break;
		}
	}
}

int32_t bet_player_init(int32_t peerID,char *deckStr,char *pubKeyStr,char *destAddr)
{
	bits256 key,pubKey,privKey;
	cJSON *deckInfo=NULL,*cardsInfo=NULL,*keyInfo=NULL,*initInfo=NULL,*card;
    int32_t pushSock,i,n,bytes,retval=1;; 
	char str[65],*rendered=NULL;
	struct pair256 *cards=NULL;

	deckInfo=cJSON_CreateObject();
	deckInfo=cJSON_Parse(bet_strip(deckStr));
	if(deckInfo)
	{
		n=jint(deckInfo,"Number Of Cards");
		cards=calloc(n,sizeof(struct pair256));
		cardsInfo=cJSON_GetObjectItem(deckInfo,"CardsInfo");
		for(i=0;i<n;i++)
		{
			card=cJSON_GetArrayItem(cardsInfo,i);
			cards[i].priv=jbits256(card,"PrivKey");
			cards[i].prod=jbits256(card,"PubKey");
		}
	}
	keyInfo=cJSON_CreateObject();
	keyInfo=cJSON_Parse(bet_strip(pubKeyStr));
	if(keyInfo)
	{
		key=jbits256(keyInfo,"PubKey");
	}
	
   if(cardsInfo)
		cJSON_Delete(cardsInfo);
	initInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(initInfo,"command","init_p");
	cJSON_AddNumberToObject(initInfo,"peerid",peerID);
	jaddbits256(initInfo,"pubkey",key);
	cJSON_AddItemToObject(initInfo,"cardinfo",cardsInfo=cJSON_CreateObject());
	for(i=0;i<n;i++)
	{
		cJSON_AddItemToArray(cardsInfo,cJSON_CreateString(bits256_str(str,cards[i].prod)));
	}
	rendered=cJSON_Print(initInfo);
	pushSock=BET_nanosock(0,destAddr,NN_PUSH);
	bytes=nn_send(pushSock,rendered,strlen(rendered),0);
	printf("\nInit Deck Info:\n%s",cJSON_Print(cardsInfo));
    printf("\n%s",cJSON_Print(cJSON_CreateString(cJSON_Print(cardsInfo))));	
	if(bytes<0)
        retval=-1;
    
	return retval;
}




