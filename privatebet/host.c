/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
#include "bet.h"
#include "common.h"
#include "host.h"
#include "cards777.h"
#include "table.h"
#include "client.h"
#include "network.h"
#include "oracle.h"
#include "commands.h"
#include "payment.h"
#include "states.h"
#include "../log/macrologger.h"
#include "poker.h"

#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#if 1 //this is for websockets

#include <string.h>


#define LWS_PLUGIN_STATIC


struct lws *wsi_global_host=NULL;


cJSON *dcvDataToWrite=NULL;
int32_t dcvDataExists=0;

struct privatebet_rawpeerln Rawpeersln[CARDS777_MAXPLAYERS+1],oldRawpeersln[CARDS777_MAXPLAYERS+1];
struct privatebet_peerln Peersln[CARDS777_MAXPLAYERS+1];
int32_t Num_rawpeersln,oldNum_rawpeersln,Num_peersln,Numgames;
int32_t players_joined=0;
int32_t turn=0,no_of_cards=0,no_of_rounds=0,no_of_bets=0;
int32_t card_matrix[CARDS777_MAXPLAYERS][hand_size];
int32_t card_values[CARDS777_MAXPLAYERS][hand_size];
int32_t all_player_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
struct deck_dcv_info dcv_info;
int32_t player_ready[CARDS777_MAXPLAYERS];
int32_t hole_cards_drawn=0,community_cards_drawn=0,flop_cards_drawn=0,turn_card_drawn=0,river_card_drawn=0;
int32_t bet_amount[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS];
int32_t eval_game_p[CARDS777_MAXPLAYERS],eval_game_c[CARDS777_MAXPLAYERS];


char player_chips_address[CARDS777_MAXPLAYERS][64];

int32_t invoiceID;

char* suit[NSUITS]= {"clubs","diamonds","hearts","spades"};
char* face[NFACES]= {"two","three","four","five","six","seven","eight","nine",
					 "ten","jack","queen","king","ace"};

struct privatebet_info *BET_dcv;
struct privatebet_vars *DCV_VARS;


static int dealerPosition;

int32_t no_of_signers,max_no_of_signers=2,is_signed[CARDS777_MAXPLAYERS];


/*
Below are the API's which are written to support REST
*/

void dcv_lws_write(cJSON *data)
{
	if(!dcvDataToWrite)
		dcvDataToWrite=cJSON_CreateObject();

	memset(dcvDataToWrite,0,sizeof(struct cJSON));
	dcvDataToWrite=data;
	dcvDataExists=1;
	lws_callback_on_writable(wsi_global_host);
	
}


int32_t BET_rest_dcv_init(struct lws *wsi, cJSON *argjson)
{
	int32_t range=52;
	cJSON *dcvInfo=NULL,*dcvKeyInfo=NULL;
	int32_t Maxplayers=2;
	char *uri=NULL;

	if(BET_dcv == NULL)
		BET_dcv=(struct privatebet_info*)malloc(sizeof(struct privatebet_info)); //   calloc(1,sizeof(struct privatebet_info));
	else
		memset(BET_dcv,0x00,sizeof(struct privatebet_info));

	if(DCV_VARS == NULL)
		DCV_VARS=(struct privatebet_vars*)malloc(sizeof(struct privatebet_vars));//	DCV_VARS = calloc(1,sizeof(struct privatebet_vars));
	else
		memset(DCV_VARS,0x00,sizeof(struct privatebet_vars));
	
	BET_dcv->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
	BET_dcv->maxchips = CARDS777_MAXCHIPS;
	BET_dcv->chipsize = CARDS777_CHIPSIZE;
	BET_dcv->numplayers=0;
	BET_betinfo_set(BET_dcv,"demo",range,0,Maxplayers);

	dcv_info.numplayers=0;
	dcv_info.maxplayers=BET_dcv->maxplayers;
	BET_permutation(dcv_info.permis,BET_dcv->range);
        dcv_info.deckid=rand256(0);
	dcv_info.dcv_key.priv=curve25519_keypair(&dcv_info.dcv_key.prod);

		
	invoiceID=0;	
		
	for(int i=0;i<BET_dcv->range;i++) {
		permis_d[i]=dcv_info.permis[i];
	}

	
	dcvKeyInfo=cJSON_CreateObject();
	jaddbits256(dcvKeyInfo,"deckid",dcv_info.deckid);
	jaddbits256(dcvKeyInfo,"pubkey",dcv_info.dcv_key.prod);
	
	uri=(char*)malloc(sizeof(char)*200);
	memset(uri,0x00,sizeof(uri));
	BET_rest_uri(&uri);
	printf("%s::%d::uri:%s\n",__FUNCTION__,__LINE__,uri);
	
	dcvInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(dcvInfo,"method","dcv");
	cJSON_AddStringToObject(dcvInfo,"dcv",cJSON_Print(dcvKeyInfo));
	cJSON_AddStringToObject(dcvInfo,"uri",uri);
	cJSON_AddNumberToObject(dcvInfo,"balance",BET_rest_listfunds());
	lws_write(wsi,cJSON_Print(dcvInfo),strlen(cJSON_Print(dcvInfo)),0);

	if(uri)
		free(uri);
		
	return 0;
}


int32_t BET_rest_from_dcv(struct lws *wsi, cJSON *argjson)
{
	cJSON *info=NULL;
	info=cJSON_CreateObject();
	info=cJSON_Parse(jstr(argjson,"dcv"));
	printf("\n%s\n",jstr(argjson,"dcv"));
	printf("\n%s",jstr(info,"deckid"));
	printf("\n%s",jstr(info,"pubkey"));
	
	return 0;
}

int32_t BET_rest_from_bvv(struct lws *wsi, cJSON *argjson)
{
	cJSON *info=NULL;
	info=cJSON_CreateObject();
	info=cJSON_Parse(jstr(argjson,"bvv"));
	printf("\n%s\n",cJSON_Print(info));

	return 0;
}

int32_t BET_rest_from_player(struct lws *wsi, cJSON *argjson)
{
	cJSON *info=NULL;
	info=cJSON_CreateObject();
	info=cJSON_Parse(jstr(argjson,"player"));
	printf("\n%s\n",cJSON_Print(info));

	return 0;
}


int32_t BET_rest_default(struct lws *wsi, cJSON *argjson)
{
	cJSON *defaultInfo=NULL;
	defaultInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(defaultInfo,"default","No method found in the back end for the action");
	lws_write(wsi,cJSON_Print(defaultInfo),strlen(cJSON_Print(defaultInfo)),0);
	return 0;
}

int32_t BET_rest_chat(struct lws *wsi, cJSON *argjson)
{
	cJSON *chatInfo=NULL;
	chatInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(chatInfo,"chat",jstr(argjson,"value"));
	lws_write(wsi,cJSON_Print(chatInfo),strlen(cJSON_Print(chatInfo)),0);
	return 0;
}

int32_t BET_rest_seats(struct lws *wsi, cJSON *argjson)
{
	cJSON *tableInfo=NULL,*seatInfo=NULL,*seatsInfo=NULL;
	char *rendered=NULL;
	int32_t retval=0,bytes;
	
	tableInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(tableInfo,"method","seats");
	cJSON_AddItemToObject(tableInfo,"seats",seatsInfo);

	cJSON_AddItemToObject(tableInfo,"seats",seatsInfo=cJSON_CreateArray());


	seatInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(seatInfo,"name","player1");
	cJSON_AddNumberToObject(seatInfo,"seat",0);
	cJSON_AddNumberToObject(seatInfo,"stack",0);
	cJSON_AddNumberToObject(seatInfo,"empty",0);
	cJSON_AddNumberToObject(seatInfo,"playing",1);

	cJSON_AddItemToArray(seatsInfo,seatInfo);

	if(seatInfo)
		memset(seatInfo,0x00,sizeof(seatInfo));
	
	cJSON_AddStringToObject(seatInfo,"name","player2");
	cJSON_AddNumberToObject(seatInfo,"seat",1);
	cJSON_AddNumberToObject(seatInfo,"stack",0);
	cJSON_AddNumberToObject(seatInfo,"empty",0);
	cJSON_AddNumberToObject(seatInfo,"playing",1);

	cJSON_AddItemToArray(seatsInfo,seatInfo);
        
		
	rendered=cJSON_Print(tableInfo);
	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,rendered);
	lws_write(wsi,rendered,strlen(rendered),0);

	

	bytes=nn_send(BET_dcv->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
		retval=-1;

	return 0;
	
}


int32_t BET_rest_game(struct lws *wsi, cJSON *argjson)
{
	char buf[100];
	cJSON *gameInfo=NULL,*gameDetails=NULL,*potInfo=NULL;
	char *rendered=NULL;
	gameDetails=cJSON_CreateObject();
	cJSON_AddNumberToObject(gameDetails,"seats",2);
	
	potInfo=cJSON_CreateArray();
	cJSON_AddItemToArray(potInfo,cJSON_CreateNumber(0));

	cJSON_AddItemToObject(gameDetails,"pot",potInfo);
	sprintf(buf,"Texas Holdem Poker:%d/%d",small_blind_amount,big_blind_amount);
		
	cJSON_AddStringToObject(gameDetails,"gametype",buf);

	gameInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(gameInfo,"method","game");
	cJSON_AddItemToObject(gameInfo,"game",gameDetails);
	rendered=cJSON_Print(gameInfo);
	lws_write(wsi,rendered,strlen(rendered),0);
	return 0;
		
}


	
int32_t BET_rest_dcv_default(struct lws *wsi, cJSON *argjson)
{
	cJSON *defaultInfo=NULL;
	defaultInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(defaultInfo,"default","Unable to process this command");

	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(defaultInfo));
	lws_write(wsi,cJSON_Print(defaultInfo),strlen(cJSON_Print(defaultInfo)),0);
	return 0;
}

int32_t BET_rest_client_join_req(struct lws *wsi, cJSON *argjson)
{
	cJSON *playerInfo=NULL;
	char *uri=NULL;
	char *rendered=NULL;

	int32_t bytes;

	
	BET_dcv->numplayers=++players_joined;
	dcv_info.peerpubkeys[players_joined-1]=jbits256(argjson,"pubkey");
	strcpy(dcv_info.uri[players_joined-1],jstr(argjson,"uri"));
	
	
	
	playerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerInfo,"method","join_res");
	cJSON_AddNumberToObject(playerInfo,"peerid",BET_dcv->numplayers-1); //players numbering starts from 0(zero)
	jaddbits256(playerInfo,"pubkey",jbits256(argjson,"pubkey"));

	
	uri=(char*)malloc(sizeof(char)*200);
	memset(uri,0x00,sizeof(uri));
	BET_rest_uri(&uri);
	printf("%s::%d::uri::%s\n",__FUNCTION__,__LINE__,uri);
	
	cJSON_AddStringToObject(playerInfo,"uri",uri);

	
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(playerInfo));

	lws_write(wsi,cJSON_Print(playerInfo),strlen(cJSON_Print(playerInfo)),0);


		
	rendered=cJSON_Print(playerInfo);
	bytes=nn_send(BET_dcv->pubsock,rendered,strlen(rendered),0);
	if(bytes < 0)
	{
		printf("\n%s:%d::sending failed\n",__FUNCTION__,__LINE__);
	}

	if(uri)
		free(uri);

	return 0;
}

int32_t BET_rest_broadcast_table_info(struct lws *wsi)
{
	cJSON *tableInfo=NULL,*playersInfo=NULL;
	char str[65];
	int32_t retval=1;;
	
	tableInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(tableInfo,"method","TableInfo");
	cJSON_AddItemToObject(tableInfo,"playersInfo",playersInfo=cJSON_CreateArray());
	for(int32_t i=0;i<BET_dcv->maxplayers;i++)
	{
		cJSON_AddItemToArray(playersInfo,cJSON_CreateString(bits256_str(str,dcv_info.peerpubkeys[i])));
	}
	printf("\nTable Info:%s",cJSON_Print(tableInfo));

	lws_write(wsi,cJSON_Print(tableInfo),strlen(cJSON_Print(tableInfo)),0);
	
	return retval;
}

int32_t BET_rest_check_BVV_Ready(struct lws *wsi)
{
	int32_t retval=-1;
	cJSON *bvvReady=NULL,*uriInfo=NULL;
	bvvReady=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvReady,"method","check_bvv_ready");
	cJSON_AddItemToObject(bvvReady,"uri_info",uriInfo=cJSON_CreateArray());
	for(int i=0;i<BET_dcv->maxplayers;i++)
	{
		jaddistr(uriInfo,dcv_info.uri[i]);
	}

	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(bvvReady));
	lws_write(wsi,cJSON_Print(bvvReady),strlen(cJSON_Print(bvvReady)),0);
	
	return retval;
}


int32_t BET_rest_dcv_start_init(struct lws *wsi, cJSON *argjson)
{
	cJSON *init=NULL;
	char *rendered=NULL;
	int32_t bytes;
	
	init=cJSON_CreateObject();
	cJSON_AddStringToObject(init,"method","init");

	lws_write(wsi,cJSON_Print(init),strlen(cJSON_Print(init)),0);

	rendered=cJSON_Print(init);
	bytes=nn_send(BET_dcv->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
		printf("\n%s::%d::Failed to send data",__FUNCTION__,__LINE__);

	return 0;
}

int32_t BET_rest_dcv_process_init_p(struct lws *wsi, cJSON *argjson)
{
  	int32_t peerid,retval=1;
  	bits256 cardpubvalues[CARDS777_MAXCARDS];
	cJSON *cardinfo=NULL;

  	peerid=jint(argjson,"peerid");
  	cardinfo=cJSON_GetObjectItem(argjson,"cardinfo");
	for(int i=0;i<cJSON_GetArraySize(cardinfo);i++)
	{
			cardpubvalues[i]=jbits256i(cardinfo,i);
	} 	
	
	retval=sg777_deckgen_vendor(peerid,dcv_info.cardprods[peerid],dcv_info.dcvblindcards[peerid],BET_dcv->range,cardpubvalues,dcv_info.deckid);
	dcv_info.numplayers=dcv_info.numplayers+1;
	return retval;
}


int32_t BET_rest_dcv_deck_init_info(struct lws *wsi, cJSON *argjson)
{
      cJSON *deck_init_info,*cjsoncardprods,*cjsondcvblindcards,*cjsong_hash,*cjsonpeerpubkeys;
	  char str[65];
	  int32_t retval=1;
	  
	  deck_init_info=cJSON_CreateObject();
	  cJSON_AddStringToObject(deck_init_info,"method","init_d");
	  jaddbits256(deck_init_info,"deckid",dcv_info.deckid);
	  cJSON_AddItemToObject(deck_init_info,"cardprods",cjsoncardprods=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<BET_dcv->range;j++)
		{
			cJSON_AddItemToArray(cjsoncardprods,cJSON_CreateString(bits256_str(str,dcv_info.cardprods[i][j])));
		}
	  }
	  cJSON_AddItemToObject(deck_init_info,"dcvblindcards",cjsondcvblindcards=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<BET_dcv->range;j++)
		{
			cJSON_AddItemToArray(cjsondcvblindcards,cJSON_CreateString(bits256_str(str,dcv_info.dcvblindcards[i][j])));
		}
	  }

	  cJSON_AddItemToObject(deck_init_info,"g_hash",cjsong_hash=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<BET_dcv->range;j++)
		{
			cJSON_AddItemToArray(cjsong_hash,cJSON_CreateString(bits256_str(str,g_hash[i][j])));
		}
	  }
	  cJSON_AddItemToObject(deck_init_info,"peerpubkeys",cjsonpeerpubkeys=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
      {
      	cJSON_AddItemToArray(cjsonpeerpubkeys,cJSON_CreateString(bits256_str(str,dcv_info.peerpubkeys[i])));
	  }

	  //printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(deck_init_info));	
	  lws_write(wsi,cJSON_Print(deck_init_info),strlen(cJSON_Print(deck_init_info)),0);
	  return retval;
}

int32_t BET_rest_check_player_ready(struct lws *wsi, cJSON *argjson)
{
	int flag=1;
	player_ready[jint(argjson,"playerID")]=1;
	for(int i=0;i<BET_dcv->maxplayers;i++)
	{
		if(player_ready[i]==0)
		{
			flag=0;
			break;
		}
	}
	return flag;
}

int32_t BET_rest_send_turn_info(struct lws *wsi,int32_t playerid,int32_t cardid,int32_t card_type)
{
	cJSON *turninfo=NULL;
	int retval=1;

	turninfo=cJSON_CreateObject();
	cJSON_AddStringToObject(turninfo,"method","turn");
	cJSON_AddNumberToObject(turninfo,"playerid",playerid);
	cJSON_AddNumberToObject(turninfo,"cardid",cardid);
	cJSON_AddNumberToObject(turninfo,"card_type",card_type);

	lws_write(wsi,cJSON_Print(turninfo),strlen(cJSON_Print(turninfo)),0);

	return retval;
}



int32_t BET_rest_dcv_turn(struct lws *wsi)
{
	int32_t retval=1;

	if(hole_cards_drawn == 0)
	{
		for(int i=0;i<no_of_hole_cards;i++)
		{
			for(int j=0;j<BET_dcv->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_rest_send_turn_info(wsi,j,(i*(BET_dcv->maxplayers))+j,hole_card);
					goto end;
		
				}
			}
		}	
	}
	else if(flop_cards_drawn==0)
	{
		for(int i=no_of_hole_cards;i<no_of_hole_cards+no_of_flop_cards;i++)
		{
			for(int j=0;j<BET_dcv->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					if((i-(no_of_hole_cards)) == 0)
					{
						retval=BET_rest_send_turn_info(wsi,j,(no_of_hole_cards*(BET_dcv->maxplayers))+(i-no_of_hole_cards)+1,flop_card_1);	
					}
					else if((i-(no_of_hole_cards)) == 1)
					{
						retval=BET_rest_send_turn_info(wsi,j,(no_of_hole_cards*(BET_dcv->maxplayers))+(i-no_of_hole_cards)+1,flop_card_2);	
					}
					else if((i-(no_of_hole_cards)) == 2)
					{
						retval=BET_rest_send_turn_info(wsi,j,(no_of_hole_cards*(BET_dcv->maxplayers))+(i-no_of_hole_cards)+1,flop_card_3);	
					}
					goto end;
					
				}
			}
		}
	}
	else if(turn_card_drawn==0)
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		
		for(int i=no_of_hole_cards+no_of_flop_cards;i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card;i++)
		{
			for(int j=0;j<2/*BET_dcv->maxplayers*/;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_rest_send_turn_info(wsi,j,(no_of_hole_cards*2/*BET_dcv->maxplayers*/)+(i-no_of_hole_cards)+2,turn_card);
					goto end;
				}
			}
		}
		
	}
	else if(river_card_drawn==0)
	{
		for(int i=no_of_hole_cards+no_of_flop_cards+no_of_turn_card;i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card+no_of_river_card;i++)
		{
			for(int j=0;j<2/*BET_dcv->maxplayers*/;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_rest_send_turn_info(wsi,j,(no_of_hole_cards*2/*BET_dcv->maxplayers*/)+(i-no_of_hole_cards)+3,river_card);
					goto end;
				}
			}
		}
	}
	else
		retval=2;
	end:	
		return retval;
}

int32_t BET_rest_receive_card(struct lws *wsi, cJSON *playerCardInfo)
{
	int retval=1,playerid,cardid,card_type,flag;
	
	playerid=jint(playerCardInfo,"playerid");
	cardid=jint(playerCardInfo,"cardid");
	card_type=jint(playerCardInfo,"card_type");

	eval_game_p[no_of_cards]=playerid;
	eval_game_c[no_of_cards]=cardid;
	no_of_cards++;

	if(card_type == hole_card)
	{
		card_matrix[(cardid%BET_dcv->maxplayers)][(cardid/BET_dcv->maxplayers)]=1;
		card_values[(cardid%BET_dcv->maxplayers)][(cardid/BET_dcv->maxplayers)]=jint(playerCardInfo,"decoded_card");	
	}
	else if(card_type == flop_card_1)
	{
		card_matrix[playerid][no_of_hole_cards]=1;
		card_values[playerid][no_of_hole_cards]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == flop_card_2)
	{
		card_matrix[playerid][no_of_hole_cards+1]=1;
		card_values[playerid][no_of_hole_cards+1]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == flop_card_3)
	{
		card_matrix[playerid][no_of_hole_cards+2]=1;
		card_values[playerid][no_of_hole_cards+2]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == turn_card)
	{
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == river_card)
	{
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=jint(playerCardInfo,"decoded_card");
	}
	if(hole_cards_drawn == 0)
	{
		flag=1;
		for(int i=0;((i<no_of_hole_cards) && (flag));i++)
		{
			for(int j=0;((j<BET_dcv->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			hole_cards_drawn=1;
				
	}
	else if(flop_cards_drawn == 0)
	{
		flag=1;
		for(int i=no_of_hole_cards;((i<no_of_hole_cards+no_of_flop_cards) && (flag));i++)
		{
			for(int j=0;((j<BET_dcv->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			flop_cards_drawn=1;
		
	}
	else if(turn_card_drawn == 0)
	{
		flag=1;
		
		for(int i=no_of_hole_cards+no_of_flop_cards;((i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card) && (flag));i++)
		{
			for(int j=0;((j<2/*BET_dcv->maxplayers*/) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			turn_card_drawn=1;
			
		
	}
	else if(river_card_drawn == 0)
	{
		flag=1;
		for(int i=no_of_hole_cards+no_of_flop_cards+no_of_turn_card;((i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card+no_of_river_card) && (flag));i++)
		{
			for(int j=0;((j<2/*BET_dcv->maxplayers*/) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			river_card_drawn=1;
		
	}

		if(flag)
		{
			if(DCV_VARS->round == 0)
			{
				retval=BET_rest_DCV_small_blind(wsi);
				
			}
			else
			{
				retval=BET_rest_DCV_round_betting(wsi);
			}
		}
		else
		{
			retval=BET_rest_dcv_turn(wsi);
		}
		
		return retval;
	
}


void BET_rest_DCV_reset(struct lws *wsi)
{
	
	players_joined=0;
	turn=0;no_of_cards=0;no_of_rounds=0;no_of_bets=0;
	hole_cards_drawn=0;community_cards_drawn=0;flop_cards_drawn=0;turn_card_drawn=0;river_card_drawn=0;
	invoiceID=0;	

	for(int i=0;i<dcv_info.numplayers;i++)
			player_ready[i]=0;	
		
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<dcv_info.numplayers;j++)
		{
			card_matrix[j][i]=0;
			card_values[j][i]=-1;
		}
	}
				
	BET_rest_dcv_init(wsi,NULL);
}


int32_t BET_rest_evaluate_hand(struct lws *wsi)
{
	int retval=1,max_score=0,no_of_winners=0;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS],players_left=0,only_winner=-1;
	int32_t dcv_maxplayers=2;

	printf("\n****************************%s:%d::********************\n",__FUNCTION__,__LINE__);
	for(int i=0;i<dcv_maxplayers;i++)
	{
			p[i]=DCV_VARS->bet_actions[i][(DCV_VARS->round-1)];
			
			if((DCV_VARS->bet_actions[i][DCV_VARS->round]==fold)|| (DCV_VARS->bet_actions[i][DCV_VARS->round]==allin)) 
				players_left++;
			else
				only_winner=i;
	}
	players_left=dcv_maxplayers-players_left;
	if(players_left<2)
	{
		if(only_winner != -1)
		{
			printf("\nWinning player is :%d, winning amount:%d",only_winner,DCV_VARS->pot);
			goto end;
		}
	}
		
	printf("\nEach player got the below cards:\n");
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if(p[i]==fold)
			scores[i]=0;
		else
		{
			printf("\n For Player id: %d, cards: ",i);
			for(int j=0;j<hand_size;j++)
			{
				int temp=card_values[i][j];
				printf("%s-->%s \t",suit[temp/13],face[temp%13]);
				h[j]=(unsigned char)card_values[i][j];
			
			}
				scores[i]=SevenCardDrawScore(h);
			}
	}
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if(max_score<scores[i])
			max_score=scores[i];
	}
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if(scores[i]==max_score)
		{
			winners[i]=1;
			no_of_winners++;
		}
		else
			winners[i]=0;
	}
	
	printf("\nWinning Amount:%d",(DCV_VARS->pot/no_of_winners));
	printf("\nWinning Players Are:");
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if(winners[i]==1)
		{
			BET_rest_DCV_create_invoice_request(wsi,(DCV_VARS->pot/no_of_winners),i);
		}
	}
	printf("\n");

	end:	
		return retval;
}

int32_t BET_rest_relay(struct lws *wsi, cJSON *argjson)
{
	int32_t retval=0;
	lws_write(wsi,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
	
	return retval;
}

int32_t BET_rest_dcv_LN_check()
{
	char channel_id[100],channel_state;
	int argc,retval=1;
	char **argv=NULL,uri[100];
	cJSON *fundChannelInfo=NULL;
	cJSON *temp=NULL;
	argc=6;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);
	strcpy(uri,dcv_info.bvv_uri);
	strcpy(channel_id,strtok(uri, "@"));
	channel_state=LN_get_channel_status(channel_id);
	if((channel_state != 2) && (channel_state != 3))
	{
		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"connect");
		strcpy(argv[2],dcv_info.bvv_uri);
		argv[3]=NULL;
		argc=3;
	
		//ln_bet(argc,argv,buf);
		temp=cJSON_CreateObject();
		make_command(argc,argv,&temp);

		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argv[4]=NULL;
		argc=4;

		
		//ln_bet(argc,argv,buf);
		

		fundChannelInfo=cJSON_CreateObject();
		make_command(argc,argv,&fundChannelInfo);

		if(jint(fundChannelInfo,"code") == -1)
		{
			retval=-1;
			printf("\n%s:%d: Message: %s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
			goto end;
		}
	}

	while((channel_state=LN_get_channel_status(channel_id)) != 3)
	{
		if(channel_state == 2)
		{
			printf("\nCHANNELD AWAITING LOCKIN");
			sleep(5);
		}
		else
		{
			retval=-1;
			printf("\n%s:%d: DCV is failed to establish the channel with BVV",__FUNCTION__,__LINE__);
			goto end;
		}
	}	
	printf("\nDCV-->BVV channel ready");

	for(int i=0;i<BET_dcv->maxplayers;i++)
	{
		strcpy(uri,dcv_info.uri[i]);
		strcpy(channel_id,strtok(uri, "@"));

		while((channel_state=LN_get_channel_status(channel_id)) != 3)
		{
			if(channel_state == 2)
			{
				printf("\nCHANNELD AWAITING LOCKIN");
				sleep(5);
			}
			else
			{
				retval=-1;
				printf("\n%s:%d: Player: %d is failed to establish the channel with DCV",__FUNCTION__,__LINE__,i);
				goto end;
			}
		}
		
		printf("\nPlayer %d --> DCV channel ready",i);	
	}

	end:
	if(argv)
	{
		for(int i=0;i<6;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
				
	}

	return retval;
}


int32_t BET_rest_bvv_join(struct lws *wsi, cJSON *argjson)
{
	char uri[100],channel_id[100];
	strcpy(uri,jstr(argjson,"uri"));
	strcpy(dcv_info.bvv_uri,uri);
	if((LN_get_channel_status(strtok(jstr(argjson,"uri"), "@")) != 3)) // 3 means channel is already established with the peer
	{
		if(BET_rest_connect(uri))
		{
			strcpy(channel_id,strtok(jstr(argjson,"uri"), "@"));
			BET_rest_fundChannel(channel_id);
		}
	}
	return 0;
}



int32_t BET_rest_create_invoice(struct lws *wsi,cJSON *argjson)
{
	int argc,retval=1;
	char **argv;
	char hexstr [65];
	cJSON *invoiceInfo=NULL,*invoice=NULL;
	argc=6;
	argv =(char**)malloc(argc*sizeof(char*));
	invoiceID++;
	for(int i=0;i<argc;i++)
	{
			argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	dcv_info.betamount+=jint(argjson,"betAmount");

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"invoice");
	sprintf(argv[2],"%d",jint(argjson,"betAmount"));
	sprintf(argv[3],"%s_%d_%d_%d_%d",bits256_str(hexstr,dcv_info.deckid),invoiceID,jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	sprintf(argv[4],"Invoice_details_playerID:%d,round:%d,betting_Amount:%d",jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	argv[5]=NULL;
	argc=5;

	invoice=cJSON_CreateObject();
	make_command(argc,argv,&invoice);

	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(invoice));
	if(jint(invoice,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(invoice,"message"));
		goto end;
	}
	else
	{
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","invoice");
		cJSON_AddNumberToObject(invoiceInfo,"playerID",jint(argjson,"playerID"));
		cJSON_AddNumberToObject(invoiceInfo,"round",jint(argjson,"round"));
		cJSON_AddNumberToObject(invoiceInfo,"betAmount",jint(argjson,"betAmount"));
		
		if(strcmp(jstr(cJSON_GetObjectItem(argjson,"payment_params"),"action"),"round_betting")==0)
		{
			cJSON_AddNumberToObject(invoiceInfo,"option",jint(argjson,"option"));
		}
		
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));
		cJSON_AddItemToObject(invoiceInfo,"payment_params",cJSON_GetObjectItem(argjson,"payment_params"));
		lws_write(wsi,cJSON_Print(invoiceInfo),strlen(cJSON_Print(invoiceInfo)),0);
					
	}
	
	end:
		
	if(argv)
	{
		for(int i=0;i<6;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return retval;
}




int32_t BET_rest_DCV_create_invoice(struct lws *wsi,cJSON *argjson)
{
	int argc,retval=1;
	char **argv=NULL;
	char hexstr [65];
	cJSON *invoiceInfo=NULL,*invoice=NULL;

	argc=6;
	argv =(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
			argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"invoice");
	sprintf(argv[2],"%d",jint(argjson,"winningAmount"));
	sprintf(argv[3],"%d_%d_%s",jint(argjson,"playerID"),jint(argjson,"winningAmount"),bits256_str(hexstr,BET_get_deckid(jint(argjson,"playerID"))));
	sprintf(argv[4],"\"Invoice details playerID:%d,winning Amount:%d\"",jint(argjson,"playerID"),jint(argjson,"winningAmount"));
	argv[5]=NULL;
	argc=5;

	
	invoice=cJSON_CreateObject();
	make_command(argc,argv,&invoice);

	if(jint(invoice,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(invoice,"message"));
		goto end;
	}
	else
	{
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","winningClaim");
		cJSON_AddNumberToObject(invoiceInfo,"playerID",jint(argjson,"playerID"));
		cJSON_AddNumberToObject(invoiceInfo,"winningAmount",jint(argjson,"winningAmount"));
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));
	
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(invoiceInfo));
		lws_write(wsi,cJSON_Print(invoiceInfo),strlen(cJSON_Print(invoiceInfo)),0);
					
	}
	
	end:
		if(argv)
		{
			for(int i=0;i<6;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		
		
		return retval;
}

int32_t BET_rest_DCV_winningClaim(struct lws *wsi,cJSON *argjson)
{
	int32_t retval=1;
	char *invoice=NULL;
	cJSON *invoiceInfo=NULL;
	
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);

	retval=BET_rest_pay(jstr(invoiceInfo,"bolt11"));
	if(retval)
		printf("\n%d Satoshis paid to the player:%d\n",jint(argjson,"winningAmount"),jint(argjson,"playerID"));
	
	return retval;
}

int32_t BET_dcv_frontend(struct lws *wsi, cJSON *argjson)
{

	int retval=1;
	char *rendered=NULL,*method=NULL;
	int32_t bytes=0;
	method=jstr(argjson,"method");
	if(strcmp(method,"game") == 0)	
	{
		retval=BET_rest_game(wsi,argjson);
	}
	else if(strcmp(method,"seats") == 0)
	{
		printf("\n%s:%d::maxplayers:%d",__FUNCTION__,__LINE__,BET_dcv->maxplayers);
		retval=BET_rest_seats(wsi,argjson);
		
	}
	else if(strcmp(method,"chat") == 0)
	{
		retval=	BET_rest_chat(wsi,argjson);
	}
	else if(strcmp(method,"reset") == 0)
	{
		BET_DCV_force_reset(BET_dcv,DCV_VARS);
		rendered=cJSON_Print(argjson);
		bytes=nn_send(BET_dcv->pubsock,rendered,strlen(rendered),0);
		if(bytes<0)
			retval=-1;
	}
	else
	{		
		printf("%s::%d::Method::%s is not known to the system\n",__FUNCTION__,__LINE__,method);

	}

	return retval;
}

char lws_buf[65536];
int32_t lws_buf_length=0;

int lws_callback_http_dummy(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
	    cJSON *argjson=NULL;
		switch(reason)
	    {
	        case LWS_CALLBACK_RECEIVE:
				memcpy(lws_buf+lws_buf_length,in,len);
				lws_buf_length+=len;
				if (!lws_is_final_fragment(wsi))
						break;
				argjson=cJSON_Parse(lws_buf);
				if( BET_dcv_frontend(wsi,argjson) != 0 )
				{
					printf("\n%s:%d:Failed to process the host command",__FUNCTION__,__LINE__);
				}
				memset(lws_buf,0x00,sizeof(lws_buf));
				lws_buf_length=0;
	            break;
			case LWS_CALLBACK_ESTABLISHED:
				wsi_global_host = wsi;
				printf("%s::%d::LWS_CALLBACK_ESTABLISHED\n",__FUNCTION__,__LINE__);
				break;
			case LWS_CALLBACK_SERVER_WRITEABLE:
				if(dcvDataExists)
				{
					lws_write(wsi,cJSON_Print(dcvDataToWrite),strlen(cJSON_Print(dcvDataToWrite)),0);
					dcvDataExists=0;
				}	
				break;
				
	    }
	    return 0;
}


static struct lws_protocols protocols[] = {
	{ "http", lws_callback_http_dummy, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted;

static const struct lws_http_mount mount = {
	/* .mount_next */		NULL,		/* linked-list "next" */
	/* .mountpoint */		"/",		/* mountpoint URL */
	/* .origin */			"./mount-origin", /* serve from dir */
	/* .def */			"index.html",	/* default filename */
	/* .protocol */			NULL,
	/* .cgienv */			NULL,
	/* .extra_mimetypes */		NULL,
	/* .interpret */		NULL,
	/* .cgi_timeout */		0,
	/* .cache_max_age */		0,
	/* .auth_mask */		0,
	/* .cache_reusable */		0,
	/* .cache_revalidate */		0,
	/* .cache_intermediaries */	0,
	/* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
	/* .mountpoint_len */		1,		/* char count */
	/* .basic_auth_login_file */	NULL,
};

void sigint_handler(int sig)
{
	interrupted = 1;
}

#endif

void BET_push_host(cJSON *argjson)
{
	if(argjson)
	{
		//lws_write(wsi_global_host,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
		dcv_lws_write(argjson);
	}	
}

int32_t BET_p2p_host_deck_init_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
      cJSON *deck_init_info=NULL,*cjsoncardprods=NULL,*cjsondcvblindcards=NULL,*cjsong_hash=NULL,*cjsonpeerpubkeys=NULL;
	  char str[65],*rendered;
	  int32_t bytes,retval=1;
	  
	  deck_init_info=cJSON_CreateObject();
	  cJSON_AddStringToObject(deck_init_info,"method","init_d");
	  jaddbits256(deck_init_info,"deckid",dcv_info.deckid);
	  cJSON_AddItemToObject(deck_init_info,"cardprods",cjsoncardprods=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<bet->range;j++)
		{
			cJSON_AddItemToArray(cjsoncardprods,cJSON_CreateString(bits256_str(str,dcv_info.cardprods[i][j])));
		}
	  }
	  
	  cJSON_AddItemToObject(deck_init_info,"dcvblindcards",cjsondcvblindcards=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<bet->range;j++)
		{
			cJSON_AddItemToArray(cjsondcvblindcards,cJSON_CreateString(bits256_str(str,dcv_info.dcvblindcards[i][j])));
		}
	  }
	  	
	  cJSON_AddItemToObject(deck_init_info,"g_hash",cjsong_hash=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
	  {
		for(int j=0;j<bet->range;j++)
		{
			cJSON_AddItemToArray(cjsong_hash,cJSON_CreateString(bits256_str(str,g_hash[i][j])));
		}
	  }
	  
	  cJSON_AddItemToObject(deck_init_info,"peerpubkeys",cjsonpeerpubkeys=cJSON_CreateArray());
	  for(int i=0;i<dcv_info.numplayers;i++)
      {
      	cJSON_AddItemToArray(cjsonpeerpubkeys,cJSON_CreateString(bits256_str(str,dcv_info.peerpubkeys[i])));
	  }
	  	
	  
	 // printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(deck_init_info));	
	  
	  rendered=cJSON_Print(deck_init_info);
	  bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	  	
	  if(bytes<0)
	  	retval=-1;

	  return retval;
	 
}




int32_t BET_p2p_host_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
  	int32_t peerid,retval=1;
  	bits256 cardpubvalues[CARDS777_MAXCARDS];
	cJSON *cardinfo=NULL;

  	peerid=jint(argjson,"peerid");
  	cardinfo=cJSON_GetObjectItem(argjson,"cardinfo");
	for(int i=0;i<cJSON_GetArraySize(cardinfo);i++)
	{
			cardpubvalues[i]=jbits256i(cardinfo,i);
	} 	
	
	retval=sg777_deckgen_vendor(peerid,dcv_info.cardprods[peerid],dcv_info.dcvblindcards[peerid],bet->range,cardpubvalues,dcv_info.deckid);
	dcv_info.numplayers=dcv_info.numplayers+1;

	if((peerid+1)<bet->maxplayers)
	{
		retval=BET_p2p_host_start_init(bet,peerid+1);
	}
	
	return retval;
}

int32_t BET_p2p_bvv_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,retval=1,state,buf_size=100;
	char **argv=NULL,uri[100],buf[100];
	cJSON *connectInfo=NULL,*fundChannelInfo=NULL;
	strcpy(uri,jstr(argjson,"uri"));
	strcpy(dcv_info.bvv_uri,uri);
	if((LN_get_channel_status(strtok(jstr(argjson,"uri"), "@")) != 3)) // 3 means channel is already established with the peer
		{
						
			argc=5;
            argv=(char**)malloc(argc*sizeof(char*));
			if(argv ==NULL)
			{
				printf("%s::%d::Memory allocation failed\n",__FUNCTION__,__LINE__);
				goto end;
			}	
			memset(argv,0x00,(argc*sizeof(char*)));
            for(int i=0;i<argc;i++)
            {
	             argv[i]=(char*)malloc(buf_size*sizeof(char));
				 if(argv[i] == NULL)
			 	 {
			 	 	printf("%s::%d::Memory allocation failed\n",__FUNCTION__,__LINE__);
					goto end;
			 	 }
	        }
			argc=3;
			strcpy(argv[0],"lightning-cli");
			strcpy(argv[1],"connect");
			strcpy(argv[2],uri);
			connectInfo=cJSON_CreateObject();
			make_command(argc,argv,&connectInfo);
			cJSON_Print(connectInfo);

			if(jint(connectInfo,"code") != 0)
			{
				retval=-1;
				printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(connectInfo,"message"));
				goto end;
			}
		
			argc=5;
			for(int i=0;i<argc;i++)
			{
				memset(argv[i],0x00,buf_size);
			}
			strcpy(argv[0],"lightning-cli");
			strcpy(argv[1],"fundchannel");
			strcpy(argv[2],jstr(connectInfo,"id"));
			sprintf(buf,"%d",channel_fund_satoshis);
			strcpy(argv[3],buf);
			argc=4;
			fundChannelInfo=cJSON_CreateObject();
			make_command(argc,argv,&fundChannelInfo);

			cJSON_Print(fundChannelInfo);
			if(jint(fundChannelInfo,"code") != 0)
			{
				retval=-1;
				printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
				goto end;
			}
			while((state=LN_get_channel_status(jstr(connectInfo,"id"))) != 3)
			{
				if(state == 2)
				{
				      printf("CHANNELD_AWAITING_LOCKIN\r");
					  fflush(stdout);
				}
				else if(state == 8)
				{
				       printf("\nONCHAIN");
				}
				else
				       printf("\n%s:%d:channel-state:%d\n",__FUNCTION__,__LINE__,state);

				sleep(2);
			}
			printf("DCV-->BVV LN Channel established\n");
			
		}
		
	end:
		if(argv)
		{
			for(int i=0;i<5;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		return retval;
}


void BET_p2p_host_blinds_info(struct lws *wsi)
{
	cJSON *blindsInfo=NULL;
	char *rendered=NULL;
	
	blindsInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(blindsInfo,"method","blindsInfo");
	cJSON_AddNumberToObject(blindsInfo,"small_blind",small_blind_amount);
	cJSON_AddNumberToObject(blindsInfo,"big_blind",big_blind_amount);
	printf("%s::%d::lws::%s\n",__FUNCTION__,__LINE__,jstr(blindsInfo,"method"));

	rendered=cJSON_Print(blindsInfo);
	lws_write(wsi,rendered,strlen(rendered),0);
}
int32_t BET_p2p_host_start_init(struct privatebet_info *bet,int32_t peerid)
{
	int32_t bytes,retval=1;
	cJSON *init=NULL;
	char *rendered=NULL;
	
	init=cJSON_CreateObject();
	cJSON_AddStringToObject(init,"method","init");
	cJSON_AddNumberToObject(init,"peerid",peerid);

	rendered=cJSON_Print(init);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
		retval=-1;

	return retval;
}
int32_t BET_p2p_client_join_req(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *playerinfo=NULL,*getInfo=NULL,*addresses=NULL,*address=NULL;
    uint32_t bytes,retval=1;
	char *rendered=NULL,*uri=NULL;
	int argc;
	char **argv=NULL;

	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));

	//strcpy(player_chips_address[jint(argjson,"gui_playerID")],jstr(argjson,"txID"));
	
    bet->numplayers=++players_joined;
	dcv_info.peerpubkeys[jint(argjson,"gui_playerID")]=jbits256(argjson,"pubkey");
	strcpy(dcv_info.uri[jint(argjson,"gui_playerID")],jstr(argjson,"uri"));
	
	argv=(char**)malloc(4*sizeof(char*));
	argc=3;
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));		
	
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"getinfo");
	argv[2]=NULL;
	getInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&getInfo);
		
	uri=(char*)malloc(100*sizeof(char));
	
	addresses=cJSON_GetObjectItem(getInfo,"address");
	address=cJSON_GetArrayItem(addresses,0);

	strcpy(uri,jstr(getInfo,"id"));
	strcat(uri,"@");
	strcat(uri,jstr(address,"address"));

	playerinfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerinfo,"method","join_res");
	
	cJSON_AddNumberToObject(playerinfo,"peerid",jint(argjson,"gui_playerID"));
	jaddbits256(playerinfo,"pubkey",jbits256(argjson,"pubkey"));
	cJSON_AddStringToObject(playerinfo,"uri",uri);
	cJSON_AddNumberToObject(playerinfo,"dealer",dealerPosition);
	
	rendered=cJSON_Print(playerinfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
	{
		retval=-1;
		printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
		goto end;
	}
	end:
		if(uri)
			free(uri);
		if(argv)
		{
			for(int i=0;i<argc;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		return retval;
 }

int32_t BET_send_turn_info(struct privatebet_info *bet,int32_t playerid,int32_t cardid,int32_t card_type)
{
	cJSON *turninfo=NULL;
	int retval=1,bytes;
	char *rendered=NULL;
	
	turninfo=cJSON_CreateObject();
	cJSON_AddStringToObject(turninfo,"method","turn");
	cJSON_AddNumberToObject(turninfo,"playerid",playerid);
	cJSON_AddNumberToObject(turninfo,"cardid",cardid);
	cJSON_AddNumberToObject(turninfo,"card_type",card_type);
	rendered=cJSON_Print(turninfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
		retval=-1;

	return retval;
}

int32_t BET_p2p_dcv_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1;

	if(hole_cards_drawn == 0)
	{
		for(int i=0;i<no_of_hole_cards;i++)
		{
			for(int j=0;j<bet->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_send_turn_info(bet,j,(i*bet->maxplayers)+j,hole_card);
					goto end;
		
				}
			}
		}	
	}
	else if(flop_cards_drawn==0)
	{
		for(int i=no_of_hole_cards;i<no_of_hole_cards+no_of_flop_cards;i++)
		{
			for(int j=0;j<bet->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					if((i-(no_of_hole_cards)) == 0)
					{
						retval=BET_send_turn_info(bet,j,(no_of_hole_cards*bet->maxplayers)+(i-no_of_hole_cards)+1,flop_card_1);	
					}
					else if((i-(no_of_hole_cards)) == 1)
					{
						retval=BET_send_turn_info(bet,j,(no_of_hole_cards*bet->maxplayers)+(i-no_of_hole_cards)+1,flop_card_2);	
					}
					else if((i-(no_of_hole_cards)) == 2)
					{
						retval=BET_send_turn_info(bet,j,(no_of_hole_cards*bet->maxplayers)+(i-no_of_hole_cards)+1,flop_card_3);	
					}
					goto end;
					
				}
			}
		}
	}
	else if(turn_card_drawn==0)
	{
		for(int i=no_of_hole_cards+no_of_flop_cards;i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card;i++)
		{
			for(int j=0;j<bet->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_send_turn_info(bet,j,(no_of_hole_cards*bet->maxplayers)+(i-no_of_hole_cards)+2,turn_card);
					goto end;
				}
			}
		}
	}
	else if(river_card_drawn==0)
	{
		for(int i=no_of_hole_cards+no_of_flop_cards+no_of_turn_card;i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card+no_of_river_card;i++)
		{
			for(int j=0;j<bet->maxplayers;j++)
			{
				if(card_matrix[j][i] == 0)
				{
					retval=BET_send_turn_info(bet,j,(no_of_hole_cards*bet->maxplayers)+(i-no_of_hole_cards)+3,river_card);
					goto end;
				}
			}
		}
	}
	else
		retval=2;
	end:	
		return retval;
}

int32_t BET_p2p_highest_card(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	for(int i=0;i<1;i++)
	{
		BET_p2p_dcv_turn(argjson,bet,vars);
	}
}


int32_t BET_relay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,bytes;
	char *rendered=NULL;

	rendered=cJSON_Print(argjson);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	
	if(bytes<0)
	{
		retval=-1;
		printf("\n%s :%d Failed to send data",__FUNCTION__,__LINE__);
		goto end;
	}
	end:
		return retval;
}

int32_t BET_broadcast_table_info(struct privatebet_info *bet)
{
	cJSON *tableInfo=NULL,*playersInfo=NULL;
	char str[65],*rendered=NULL;
	int32_t bytes,retval=1;;
	
	tableInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(tableInfo,"method","TableInfo");
	cJSON_AddItemToObject(tableInfo,"playersInfo",playersInfo=cJSON_CreateArray());
	for(int32_t i=0;i<bet->maxplayers;i++)
	{
		cJSON_AddItemToArray(playersInfo,cJSON_CreateString(bits256_str(str,dcv_info.peerpubkeys[i])));
	}
	rendered=cJSON_Print(tableInfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	
	printf("\nTable Info:%s",cJSON_Print(tableInfo));

	if(bytes<0)
		retval=-1;
	return retval;
}

int32_t BET_check_BVV_Ready(struct privatebet_info *bet)
{
	int32_t bytes,retval=-1;
	char *rendered=NULL;
	cJSON *bvvReady=NULL,*uriInfo=NULL;
	bvvReady=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvReady,"method","check_bvv_ready");
	cJSON_AddItemToObject(bvvReady,"uri_info",uriInfo=cJSON_CreateArray());
	for(int i=0;i<bet->maxplayers;i++)
	{
		jaddistr(uriInfo,dcv_info.uri[i]);
	}
	rendered=cJSON_Print(bvvReady);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
			retval=-1;

	return retval;
}

int32_t BET_create_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,bytes,retval=1;
	char **argv=NULL,*rendered=NULL;
	char hexstr [65];
	cJSON *invoiceInfo=NULL,*invoice=NULL;
	argc=6;
	argv =(char**)malloc(argc*sizeof(char*));
	invoiceID++;
	for(int i=0;i<argc;i++)
	{
			argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	dcv_info.betamount+=jint(argjson,"betAmount");

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"invoice");
	sprintf(argv[2],"%ld",(long int)jint(argjson,"betAmount")*mchips_msatoshichips);
	sprintf(argv[3],"%s_%d_%d_%d_%d",bits256_str(hexstr,dcv_info.deckid),invoiceID,jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	sprintf(argv[4],"\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"",jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	argv[5]=NULL;
	argc=5;
	
	invoice=cJSON_CreateObject();
	make_command(argc,argv,&invoice);
	
	if(jint(invoice,"code") != 0)
		retval=-1;
	else
	{
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","invoice");
		cJSON_AddNumberToObject(invoiceInfo,"playerID",jint(argjson,"playerID"));
		cJSON_AddNumberToObject(invoiceInfo,"round",jint(argjson,"round"));
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));

		rendered=cJSON_Print(invoiceInfo);
		bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
		
		if(bytes<0)
			retval=-1;
	}
	
	if(argv)
	{
		for(int i=0;i<6;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return retval;
}



int32_t BET_create_betting_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,bytes,retval=1;
	char **argv=NULL,*rendered=NULL;
	char hexstr [65];
	cJSON *invoiceInfo=NULL,*invoice=NULL;
	argc=6;
	argv =(char**)malloc(argc*sizeof(char*));
	invoiceID++;
	for(int i=0;i<argc;i++)
	{
			argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	dcv_info.betamount+=jint(argjson,"betAmount");

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"invoice");
	sprintf(argv[2],"%ld",(long int)jint(argjson,"invoice_amount")*mchips_msatoshichips);
	sprintf(argv[3],"%s_%d_%d_%d_%d",bits256_str(hexstr,dcv_info.deckid),invoiceID,jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"invoice_amount"));
	sprintf(argv[4],"\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"",jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"invoice_amount"));
	argv[5]=NULL;
	argc=5;
	
	invoice=cJSON_CreateObject();
	make_command(argc,argv,&invoice);
	
	if(jint(invoice,"code") != 0)
			retval=-1;
	else 
	{
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","bettingInvoice");
		cJSON_AddNumberToObject(invoiceInfo,"playerID",jint(argjson,"playerID"));
		cJSON_AddNumberToObject(invoiceInfo,"round",jint(argjson,"round"));
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));
		cJSON_AddItemToObject(invoiceInfo,"actionResponse",cJSON_GetObjectItem(argjson,"actionResponse"));
		
		rendered=cJSON_Print(invoiceInfo);
		bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
		
		if(bytes<0)
			retval=-1;
	}

	if(argv)
	{
		for(int i=0;i<6;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}

	
int32_t BET_settle_game(cJSON *payInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t playerid,max=-1,retval=1;
	cJSON *invoicesInfo=NULL,*invoiceInfo=NULL,*invoice=NULL,*winnerInfo=NULL;
	char *label=NULL;
	int32_t argc,bytes;
	char **argv=NULL,*rendered=NULL;

	argc=4;
	argv=(char**)malloc(sizeof(char*)*argc);
	for(int32_t i=0;i<=argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	
	
	label=jstr(payInfo,"label");
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"listinvoices");
	strcpy(argv[2],label);
	argv[3]=NULL;

	invoicesInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&invoicesInfo);
	
	//ln_bet(argc-1,argv,buf);
	//invoicesInfo=cJSON_Parse(buf);

	if(jint(invoicesInfo,"code") != 0 )
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(invoicesInfo,"message"));
		goto end;
	}
	else
	{
		
		//cJSON_Parse(invoicesInfo);
		
		invoiceInfo=cJSON_CreateObject();
		invoiceInfo=cJSON_GetObjectItem(invoicesInfo,"invoices");
		
		invoice=cJSON_CreateObject();
		invoice=cJSON_GetArrayItem(invoiceInfo,0);
		if(strcmp(jstr(invoice,"status"),"paid")==0)
		{
			dcv_info.paidamount+=jint(invoice,"msatoshi_received");
			printf("\nAmount paid: %d",jint(invoice,"msatoshi_received"));
		}
		no_of_bets++;
		if(no_of_cards == no_of_bets)
		{	
			for(int i=0;i<no_of_cards;i++)
			{
				if(eval_game_c[i]>max)
				{
					max=eval_game_c[i];
					playerid=i;
				}
			}
			winnerInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(winnerInfo,"method","winner");
			cJSON_AddNumberToObject(winnerInfo,"playerid",playerid);
			cJSON_AddNumberToObject(winnerInfo,"cardid",max);
			cJSON_AddNumberToObject(winnerInfo,"winning_amount",dcv_info.betamount);
			rendered=cJSON_Print(winnerInfo);
			bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
			if(bytes < 0)
			{
				retval=-1;
				printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
				goto end;
			}
			
			printf("\nThe winner of the game is player :%d, it got the card:%d\n",playerid,max);
			
		}
	}
	
	end:
		if(argv)
		{
			for(int i=0;i<argc;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		return retval;
		
}

int32_t BET_p2p_check_player_ready(cJSON *playerReady,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int flag=1;
	player_ready[jint(playerReady,"playerid")]=1;
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(player_ready[i]==0)
		{
			flag=0;
			break;
		}
	}
	return flag;
}
int32_t BET_receive_card(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,cardid,card_type,flag=1;
	
	playerid=jint(playerCardInfo,"playerid");
	cardid=jint(playerCardInfo,"cardid");
	card_type=jint(playerCardInfo,"card_type");

	eval_game_p[no_of_cards]=playerid;
	eval_game_c[no_of_cards]=cardid;
	no_of_cards++;

	if(card_type == hole_card)
	{
		card_matrix[(cardid%bet->maxplayers)][(cardid/bet->maxplayers)]=1;
		card_values[(cardid%bet->maxplayers)][(cardid/bet->maxplayers)]=jint(playerCardInfo,"decoded_card");	
	}
	else if(card_type == flop_card_1)
	{
		card_matrix[playerid][no_of_hole_cards]=1;
		card_values[playerid][no_of_hole_cards]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == flop_card_2)
	{
		card_matrix[playerid][no_of_hole_cards+1]=1;
		card_values[playerid][no_of_hole_cards+1]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == flop_card_3)
	{
		card_matrix[playerid][no_of_hole_cards+2]=1;
		card_values[playerid][no_of_hole_cards+2]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == turn_card)
	{
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == river_card)
	{
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=jint(playerCardInfo,"decoded_card");
	}

	#if 0
	printf("\nCard Matrix:\n");
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			printf("%d\t",card_matrix[j][i]);
		}
		printf("\n");
	}
	#endif
	
	if(hole_cards_drawn == 0)
	{
		flag=1;
		for(int i=0;((i<no_of_hole_cards) && (flag));i++)
		{
			for(int j=0;((j<bet->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			hole_cards_drawn=1;
				
	}
	else if(flop_cards_drawn == 0)
	{
		flag=1;
		for(int i=no_of_hole_cards;((i<no_of_hole_cards+no_of_flop_cards) && (flag));i++)
		{
			for(int j=0;((j<bet->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			flop_cards_drawn=1;
		
	}
	else if(turn_card_drawn == 0)
	{
		flag=1;
		for(int i=no_of_hole_cards+no_of_flop_cards;((i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card) && (flag));i++)
		{
			for(int j=0;((j<bet->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			turn_card_drawn=1;
		
	}
	else if(river_card_drawn == 0)
	{
		flag=1;
		for(int i=no_of_hole_cards+no_of_flop_cards+no_of_turn_card;((i<no_of_hole_cards+no_of_flop_cards+no_of_turn_card+no_of_river_card) && (flag));i++)
		{
			for(int j=0;((j<bet->maxplayers) &&(flag));j++)
			{
				if(card_matrix[j][i] == 0)
				{
					flag=0;
				}
			}
		}
		if(flag)
			river_card_drawn=1;
		
	}

		if(flag)
		{
			if(vars->round == 0)
			{
				retval=BET_DCV_small_blind(NULL,bet,vars);
			}
			else
			{
				retval=BET_DCV_round_betting(NULL,bet,vars);
			}
		}
		else
		{
			retval=BET_p2p_dcv_turn(playerCardInfo,bet,vars);
		}
		
		return retval;
	
}


void BET_DCV_reset(struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *resetInfo=NULL;
	
	players_joined=0;
	turn=0;no_of_cards=0;no_of_rounds=0;no_of_bets=0;
	hole_cards_drawn=0;community_cards_drawn=0;flop_cards_drawn=0;turn_card_drawn=0;river_card_drawn=0;
	invoiceID=0;	
		
	for(int i=0;i<bet->maxplayers;i++)
		player_ready[i]=0;	
	
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			card_matrix[j][i]=0;
			card_values[j][i]=-1;
		}
	}

	
	dcv_info.numplayers=0;
	dcv_info.maxplayers=bet->maxplayers;
	BET_permutation(dcv_info.permis,bet->range);
    dcv_info.deckid=rand256(0);
	dcv_info.dcv_key.priv=curve25519_keypair(&dcv_info.dcv_key.prod);
	for(int i=0;i<bet->range;i++)
	{
		permis_d[i]=dcv_info.permis[i];
	
	}
	
	vars->turni=0;
	vars->round=0;
	vars->pot=0;
	vars->last_turn=0;
	vars->last_raise=0;
	for(int i=0;i<bet->maxplayers;i++)
	{
		vars->funds[i]=0;
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
			vars->betamount[i][j]=0;
		}
	}

	dealerPosition=(dealerPosition+1)%bet->maxplayers;
	vars->dealer=dealerPosition;
	
	bet->numplayers=0;
	bet->cardid=-1;
	bet->turni=-1;
	bet->no_of_turns=0;

	resetInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(resetInfo,"method","reset");
	BET_push_host(resetInfo);
}


void BET_DCV_force_reset(struct privatebet_info *bet,struct privatebet_vars *vars)
{
	
	players_joined=0;
	turn=0;no_of_cards=0;no_of_rounds=0;no_of_bets=0;
	hole_cards_drawn=0;community_cards_drawn=0;flop_cards_drawn=0;turn_card_drawn=0;river_card_drawn=0;
	invoiceID=0;	
		
	for(int i=0;i<bet->maxplayers;i++)
		player_ready[i]=0;	
	
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			card_matrix[j][i]=0;
			card_values[j][i]=-1;
		}
	}

	
	dcv_info.numplayers=0;
	dcv_info.maxplayers=bet->maxplayers;
	BET_permutation(dcv_info.permis,bet->range);
    dcv_info.deckid=rand256(0);
	dcv_info.dcv_key.priv=curve25519_keypair(&dcv_info.dcv_key.prod);
	for(int i=0;i<bet->range;i++)
	{
		permis_d[i]=dcv_info.permis[i];
	
	}
	
	vars->turni=0;
	vars->round=0;
	vars->pot=0;
	vars->last_turn=0;
	vars->last_raise=0;
	for(int i=0;i<bet->maxplayers;i++)
	{
		vars->funds[i]=0;
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
			vars->betamount[i][j]=0;
		}
	}
	
	bet->numplayers=0;
	bet->cardid=-1;
	bet->turni=-1;
	bet->no_of_turns=0;

}

int32_t BET_evaluate_hand_test(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,max_score=0,no_of_winners=0,bytes;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS],players_left=0,only_winner=-1;
	cJSON *resetInfo=NULL;
	char *rendered=NULL;
	int fold_flag=0;

	
	char* cards[52] = {"2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC", 
					   "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD", 
					   "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH", 
					   "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS"};
	
	for(int i=0;i<bet->maxplayers;i++)
	{
			p[i]=vars->bet_actions[i][(vars->round)];
			
			if(vars->bet_actions[i][vars->round]==fold)//|| (vars->bet_actions[i][vars->round]==allin)) 
				players_left++;
			else
				only_winner=i;
	}
	
	players_left=bet->maxplayers-players_left;
	if(players_left<2)
	{
		if(only_winner != -1)
		{
			fold_flag=1;
			no_of_winners=1;
			for(int i=0;i<bet->maxplayers;i++)
			{
				if(vars->bet_actions[i][vars->round]==fold)
					winners[i]=0;
				else
					winners[i]=1;
			}
			retval=BET_DCV_invoice_pay(bet,vars,only_winner,vars->pot);
			printf("Winning player is :%d, winning amount:%d\n",only_winner,vars->pot);
			//goto end;
		}
	}
	else
	{
		printf("\nEach player got the below cards:\n");
		for(int i=0;i<bet->maxplayers;i++)
		{
			if(p[i]==fold)
				scores[i]=0;
			else
			{
				printf("\n For Player id: %d, cards: ",i);
				for(int j=0;j<hand_size;j++)
				{
					int temp=card_values[i][j];
					printf("%s-->%s \t",suit[temp/13],face[temp%13]);
					h[j]=(unsigned char)card_values[i][j];
				
				}
					scores[i]=SevenCardDrawScore(h);
				}
		}
		for(int i=0;i<bet->maxplayers;i++)
		{
			if(max_score<scores[i])
				max_score=scores[i];
		}
		for(int i=0;i<bet->maxplayers;i++)
		{
			if(scores[i]==max_score)
			{
				winners[i]=1;
				no_of_winners++;
			}
			else
				winners[i]=0;
		}
		
		printf("\nWinning Amount:%d",(vars->pot/no_of_winners));
		printf("\nWinning Players Are:");
		for(int i=0;i<bet->maxplayers;i++)
		{
			if(winners[i]==1)
			{
				//printf("%s::%d::Winning Address::%s\n",__FUNCTION__,__LINE__,player_chips_address[i]);
				//BET_transferfunds(0.05,player_chips_address[i]);
				retval=BET_DCV_invoice_pay(bet,vars,i,(vars->pot/no_of_winners));
				printf("%d\t",i);
				if(retval == -1)
					goto end;
			}
		}
		printf("\n");
	}
	cJSON *finalInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(finalInfo,"method","finalInfo");
	cJSON *allHoleCardInfo=cJSON_CreateArray();
	cJSON *boardCardInfo=cJSON_CreateArray();
	cJSON *holeCardInfo=NULL;
	cJSON *showInfo=NULL;
	for(int i=0;i<bet->maxplayers;i++)
	{
		holeCardInfo=cJSON_CreateArray();
		for(int j=0;j<no_of_hole_cards;j++)
		{
			if(fold_flag==1)
			{
				cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
			}
			else
			{
				if(card_values[i][j]!=-1)
					cJSON_AddItemToArray(holeCardInfo,cJSON_CreateString(cards[card_values[i][j]]));
				else
					cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
			}	
		}
		cJSON_AddItemToArray(allHoleCardInfo,holeCardInfo);
	}

	for(int j=no_of_hole_cards;j<hand_size;j++)
	{
		if(fold_flag==1)
		{
			cJSON_AddItemToArray(boardCardInfo,cJSON_CreateNull());
		}
		else
		{
			if(card_values[0][j]!=-1)
				cJSON_AddItemToArray(boardCardInfo,cJSON_CreateString(cards[card_values[0][j]]));
			else
				cJSON_AddItemToArray(boardCardInfo,cJSON_CreateNull());
		}
	}

	showInfo=cJSON_CreateObject();
	cJSON_AddItemToObject(showInfo,"allHoleCardsInfo",allHoleCardInfo);
	cJSON_AddItemToObject(showInfo,"boardCardInfo",boardCardInfo);

    cJSON_AddItemToObject(finalInfo,"showInfo",showInfo);
	cJSON_AddNumberToObject(finalInfo,"win_amount",vars->pot/no_of_winners);

	cJSON *winnersInfo=cJSON_CreateArray();
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(winners[i]==1)
		{
			cJSON_AddItemToArray(winnersInfo,cJSON_CreateNumber(i));
		}
	}
	cJSON_AddItemToObject(finalInfo,"winners",winnersInfo);


	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(finalInfo));
	rendered=cJSON_Print(finalInfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	
	if(bytes<0)
	{
		retval=-1;
		printf("%s::%d::Failed to send data\n",__FUNCTION__,__LINE__);
		goto end;
	}
	sleep(5);
	lws_write(wsi_global_host,cJSON_Print(finalInfo),strlen(cJSON_Print(finalInfo)),0);
	end:	
		if(retval != -1)
		{
			resetInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(resetInfo,"method","reset");
			rendered=cJSON_Print(resetInfo);
			bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
			if(bytes<0)
				retval=-1;
			BET_DCV_reset(bet,vars);
		}
		return retval;
}


int32_t BET_evaluate_hand(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,max_score=0,no_of_winners=0,bytes;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS],players_left=0,only_winner=-1;
	cJSON *resetInfo=NULL;
	char *rendered=NULL;
	
	for(int i=0;i<bet->maxplayers;i++)
	{
			p[i]=vars->bet_actions[i][(vars->round-1)];
			
			if((vars->bet_actions[i][vars->round]==fold)|| (vars->bet_actions[i][vars->round]==allin)) 
				players_left++;
			else
				only_winner=i;
	}
	players_left=bet->maxplayers-players_left;
	if(players_left<2)
	{
		if(only_winner != -1)
		{
			retval=BET_DCV_invoice_pay(bet,vars,only_winner,vars->pot);
			printf("\nWinning player is :%d, winning amount:%d",only_winner,vars->pot);
			goto end;
		}
	}
		
	printf("\nEach player got the below cards:\n");
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(p[i]==fold)
			scores[i]=0;
		else
		{
			printf("\n For Player id: %d, cards: ",i);
			for(int j=0;j<hand_size;j++)
			{
				int temp=card_values[i][j];
				printf("%s-->%s \t",suit[temp/13],face[temp%13]);
				h[j]=(unsigned char)card_values[i][j];
			
			}
				scores[i]=SevenCardDrawScore(h);
			}
	}
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(max_score<scores[i])
			max_score=scores[i];
	}
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(scores[i]==max_score)
		{
			winners[i]=1;
			no_of_winners++;
		}
		else
			winners[i]=0;
	}
	
	printf("\nWinning Amount:%d",(vars->pot/no_of_winners));
	printf("\nWinning Players Are:");
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(winners[i]==1)
		{
			retval=BET_DCV_invoice_pay(bet,vars,i,(vars->pot/no_of_winners));
			printf("%d\t",i);
		}
	}
	printf("\n");

	end:	
		if(retval)
		{
			resetInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(resetInfo,"method","reset");
			rendered=cJSON_Print(resetInfo);
			bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
			if(bytes<0)
				retval=-1;
			BET_DCV_reset(bet,vars);
		}
		return retval;
}

void BET_establish_ln_channels(struct privatebet_info *bet)
{
	int argc;
	char **argv=NULL;
	cJSON *connectInfo=NULL;
	
	argc=4;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(100*sizeof(char));
	}
	argv[3]=NULL;
	argc=3;
	for(int i=0;i<bet->maxplayers;i++)
	{
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"connect");
		strcpy(argv[2],dcv_info.uri[i]);

		connectInfo=cJSON_CreateObject();
		make_command(argc,argv,&connectInfo);
		//ln_bet(argc,argv,buf);
		printf("\n%s:%d:Conncet Response:%s",__FUNCTION__,__LINE__,cJSON_Print(connectInfo));
	}

	if(argv)
	{
		for(int i=0;i<4;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}
}

int BET_LN_check_if_peer_exists(char *channel_id)
{
	int argc,retval=0;
	char **argv=NULL;
	cJSON *peerInfo=NULL;
	
	argc=4;	
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"check-if-peer-exists");
	strcpy(argv[2],channel_id);
	argv[3]=NULL;
	argc=3;

	peerInfo=cJSON_CreateObject();
	make_command(argc,argv,&peerInfo);

	if(strcmp(jstr(peerInfo,"peer-exists"),"true")==0)
		retval=1;

	if(argv)
	{
		for(int i=0;i<4;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
		
}
int32_t BET_LN_check(struct privatebet_info *bet)
{
	char channel_id[100],channel_state;
	int argc,retval=1;
	char **argv=NULL,uri[100];
	cJSON *fundChannelInfo=NULL,*connectInfo=NULL;
	argc=6;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);
	strncpy(uri,dcv_info.bvv_uri,sizeof(uri));
	strncpy(channel_id,strtok(uri, "@"),sizeof(channel_id));
	channel_state=LN_get_channel_status(channel_id);
	if((channel_state != 2) && (channel_state != 3))
	{
		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"connect");
		strcpy(argv[2],dcv_info.bvv_uri);
		argc=3;

		connectInfo=cJSON_CreateObject();
		make_command(argc,argv,&connectInfo);
		
	
		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argc=4;

		fundChannelInfo=cJSON_CreateObject();
		make_command(argc,argv,&fundChannelInfo);
		

		if(jint(fundChannelInfo,"code") == -1)
		{
			retval=-1;
			printf("\n%s:%d: Message: %s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
			goto end;
		}
	}

	while((channel_state=LN_get_channel_status(channel_id)) != 3)
	{
		if(channel_state == 2)
		{
			printf("CHANNELD AWAITING LOCKIN\r");
			fflush(stdout);
			sleep(2);
		}
		else
		{
			retval=-1;
			printf("\n%s:%d: DCV is failed to establish the channel with BVV",__FUNCTION__,__LINE__);
			goto end;
		}
	}	
	printf("DCV-->BVV channel ready\n");

	for(int i=0;i<BET_dcv->maxplayers;i++)
	{
		strcpy(uri,dcv_info.uri[i]);
		strcpy(channel_id,strtok(uri, "@"));

		while((channel_state=LN_get_channel_status(channel_id)) != 3)
		{
			if(channel_state == 2)
			{
				printf("CHANNELD AWAITING LOCKIN\r");
				fflush(stdout);
				sleep(2);
			}
			else if((channel_state!=2)&&(channel_state!=3))
			{
				retval=-1;
				printf("\n%s:%d: Player: %d is failed to establish the channel with DCV, channel_state=%d, JUST WAIT\n",__FUNCTION__,__LINE__,i,channel_state);
				sleep(2);
				//break;
			}
		}
		
		printf("Player %d --> DCV channel ready\n",i);	
	}
	retval=1;
	end:
		if(argv)
		{
			for(int i=0;i<6;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		return retval;
}

void BET_p2p_LN_close(struct privatebet_info *bet)
{
	int argc;
	char **argv=NULL;
	cJSON *closeInfo=NULL;
	argc=4;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"close");
	argv[3]=NULL;
	argc=3;
	closeInfo=cJSON_CreateObject();
	for(int i=0;i<bet->maxplayers;i++)
	{
		memset(argv[2],0x00,sizeof(argv[2]));
		strcpy(argv[2],strtok(dcv_info.uri[i],"@"));
		make_command(argc,argv,&closeInfo);
	}

	if(argv)
	{
		for(int i=0;i<4;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}
}
int32_t BET_award_winner(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,retval=1;
	char **argv=NULL,channel_id[100],*invoice=NULL;
	cJSON *payResponse=NULL,*invoiceInfo=NULL,*fundChannelInfo=NULL;

	
	argc=5;
	argv=(char**)malloc(sizeof(char*)*argc);
	for(int32_t i=0;i<argc;i++)
		argv[i]=(char*)malloc(1000*sizeof(char));
	strcpy(channel_id,strtok(dcv_info.uri[jint(argjson,"playerid")], "@"));
	if(LN_get_channel_status(channel_id)!=3)
	{
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argv[4]=NULL;
		argc=4;
		
		fundChannelInfo=cJSON_CreateObject();
		make_command(argc,argv,&fundChannelInfo);
		//ln_bet(argc,argv,buf);
		//fundChannelInfo=cJSON_Parse(buf);

		if(jint(fundChannelInfo,"code") != 0)
		{
			retval=-1;
			printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
			goto end;
		}

		printf("\nFund channel response:%s\n",cJSON_Print(fundChannelInfo));
		int state;
		while((state=LN_get_channel_status(channel_id)) != 3)
		 {
			 if(state == 2)
			 {
				 printf("\nCHANNELD_AWAITING_LOCKIN");
			 }
			 else if(state == 8)
			 {
				 printf("\nONCHAIN");
			 }
			 else
				 printf("\n%s:%d:channel-state:%d\n",__FUNCTION__,__LINE__,state);

			 printf("%s::%d::%d\n",__FUNCTION__,__LINE__,state);	
			 sleep(10);
		  }
		     printf("%s::%d::%d\n",__FUNCTION__,__LINE__,state);	
	}
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);
	
	for(int32_t i=0;i<argc;i++)
		memset(argv[i],0,sizeof(argv[i]));
	
	argc=3;
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"pay");
	sprintf(argv[2],"%s",jstr(invoiceInfo,"bolt11"));
	argv[3]=NULL;

	
	payResponse=cJSON_CreateObject();
	make_command(argc,argv,&payResponse);
	//ln_bet(argc,argv,buf);
	//payResponse=cJSON_Parse(buf);

	if(jint(payResponse,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(payResponse,"message"));
		goto end;
	}

	if(strcmp(jstr(payResponse,"status"),"complete")==0)
	{
		printf("\nPayment Success\n");
		//BET_p2p_LN_close(bet);
	}
			
	end:
		if(argv)
		{
			for(int i=0;i<5;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		return retval;
}

int32_t BET_dcv_backend(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    char *method; int32_t bytes,retval=1;
	char *rendered=NULL;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
    	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,method);
		if(strcmp(method,"join_req") == 0)
		{
			
			if(bet->numplayers<bet->maxplayers)
			{
				retval=BET_p2p_client_join_req(argjson,bet,vars);
				if(retval<0)
					goto end;
                if(bet->numplayers==bet->maxplayers)
				{
					printf("Table is filled\n");
					retval=BET_LN_check(bet);
					if(retval<0)
					{
						printf("%s::%d::something wrong with BET_LN_check\n",__FUNCTION__,__LINE__);
						goto end;
					}
					BET_check_BVV_Ready(bet);
				}
			}
			
		}
		else if(strcmp(method,"bvv_ready") == 0)
		{
			retval=BET_p2p_host_start_init(bet,0);
			//BET_p2p_host_blinds_info(wsi_global_host);
		}
		else if(strcmp(method,"init_p") == 0)
		{
			retval=BET_p2p_host_init(argjson,bet,vars);
			if(dcv_info.numplayers==dcv_info.maxplayers)
			{
				printf("%s::%d::dcv_info.numplayers::%d\n",__FUNCTION__,__LINE__,dcv_info.numplayers);
				retval=BET_p2p_host_deck_init_info(argjson,bet,vars);
			}
		}
		else if(strcmp(method,"bvv_join") == 0)
		{
			retval=BET_p2p_bvv_join(argjson,bet,vars);
		}
		else if((strcmp(method,"init_b") == 0) || (strcmp(method,"next_turn") == 0))
		{
			if(strcmp(method,"init_b") == 0)
			{
				retval=BET_relay(argjson,bet,vars);
				if(retval<0)
					goto end;
			}
		}
		else if(strcmp(method,"player_ready") == 0)
		{
			if(BET_p2p_check_player_ready(argjson,bet,vars))
			{
				retval=BET_p2p_initiate_statemachine(argjson,bet,vars);
				  
			}				
		}
		else if(strcmp(method, "dealer_ready") == 0)
		{
			retval=BET_p2p_dcv_turn(argjson,bet,vars);
		
		}
		else if(strcmp(method,"playerCardInfo") == 0)
		{
				retval = BET_receive_card(argjson,bet,vars);
		}
		else if(strcmp(method,"invoiceRequest") == 0)
		{
			retval=BET_create_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"bettingInvoiceRequest") == 0)
		{
			retval=BET_create_betting_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"pay") == 0)
		{
			//retval=BET_settle_game(argjson,bet,vars);
		}
		else if(strcmp(method,"claim") == 0)
		{

			retval=BET_award_winner(argjson,bet,vars);
		}
		else if(strcmp(method,"requestShare") == 0 )
		{	
			rendered= cJSON_Print(argjson);
			for(int i=0;i<2;i++)
			{
				bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
				if(bytes<0)
				{
					retval=-1;
					printf("\nMehtod: %s Failed to send data",method);
					goto end;
				}
			}
		}
		else if(strcmp(method,"betting") == 0)
		{
			retval=BET_p2p_betting_statemachine(argjson,bet,vars);
		}
		else if(strcmp(method,"display_current_state") == 0)
		{
			retval=BET_p2p_display_current_state(argjson,bet,vars);
		}
		else if(strcmp(method,"stack") == 0)
		{
			vars->funds[jint(argjson,"playerid")]=jint(argjson,"stack_value");
			retval=BET_relay(argjson,bet,vars);
		}
		else if(strcmp(method,"signedrawtransaction") == 0)
		{
			printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			no_of_signers++;
			if(no_of_signers<max_no_of_signers)
			{
				is_signed[jint(argjson,"playerid")]=1;
				BET_publishmultisigtransaction(jstr(argjson,"tx"));
			}
			else
			{
				cJSON *temp=cJSON_CreateObject();
				cJSON_AddStringToObject(temp,"hex",jstr(argjson,"tx"));
				cJSON *finaltx=BET_sendrawtransaction(temp);
				printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(finaltx));
			}
			
		}
		else
    	{
    		bytes=nn_send(bet->pubsock,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
			if(bytes<0)
			{
				retval=-1;
				printf("\nMehtod: %s Failed to send data",method);
				goto end;
			}
    	}
    }
	end:
    	return retval;
}
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void BET_dcv_backend_loop(void *_ptr)
{
	int32_t recvlen; cJSON *argjson=NULL; 
	void *ptr=NULL; 
	struct privatebet_info *bet = _ptr;

   pthread_mutex_lock(&mutex);
	
	dcv_info.numplayers=0;
	dcv_info.maxplayers=bet->maxplayers;
	BET_permutation(dcv_info.permis,bet->range);
    dcv_info.deckid=rand256(0);
	dcv_info.dcv_key.priv=curve25519_keypair(&dcv_info.dcv_key.prod);

	for(int i=0;i<bet->maxplayers;i++)
		player_ready[i]=0;	
	
	invoiceID=0;	
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			card_matrix[j][i]=0;
			card_values[j][i]=-1;
		}
	}
	
	for(int i=0;i<bet->range;i++)
	{
		permis_d[i]=dcv_info.permis[i];
	
	}

	
	srand(time(0));
	dealerPosition=rand()%bet->maxplayers;
	DCV_VARS->dealer=dealerPosition;
	
    while ( bet->pullsock >= 0 && bet->pubsock >= 0 )
    {
    	ptr=0;
        if ( (recvlen= nn_recv(bet->pullsock,&ptr,NN_MSG,0)) > 0 )
        {
        	char *tmp=clonestr(ptr);
            if ( (argjson= cJSON_Parse(tmp)) != 0 )
            {
                if ( BET_dcv_backend(argjson,bet,DCV_VARS) != 0 ) // usually just relay to players
                {
                	//Do Something
                }
                free_json(argjson);
            }
			if(tmp)
				free(tmp);
			if(ptr)
            	nn_freemsg(ptr);
        }
          
    }
	pthread_mutex_unlock(&mutex);
}


void BET_dcv_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context=NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	printf("\n%s::%d",__FUNCTION__,__LINE__);
	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:1234\n");

	 // for DCV
    memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
    dcv_info.port = 9000;
    dcv_info.mounts = &mount;
    dcv_info.protocols = protocols;
    dcv_info.options =
        LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    dcv_context = lws_create_context(&dcv_info);
    if (!dcv_context) {
        lwsl_err("lws init failed\n");
		printf("%s::%d::lws_context error",__FUNCTION__,__LINE__);
    }   
   
	while (n >= 0 && !interrupted)
	{
        n = lws_service(dcv_context, 1000);
	}
    lws_context_destroy(dcv_context);
		
}





