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
#include "protocol_lws_minimal.c"


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



int32_t invoiceID;

char* suit[NSUITS]= {"clubs","diamonds","hearts","spades"};
char* face[NFACES]= {"two","three","four","five","six","seven","eight","nine",
					 "ten","jack","queen","king","ace"};

struct privatebet_info *BET_dcv;
struct privatebet_vars *DCV_VARS;


/*
Below are the API's which are written to support REST
*/

int32_t BET_rest_dcv_init(struct lws *wsi, cJSON *argjson)
{
	int32_t range=52;
	cJSON *dcvInfo=NULL,*dcvKeyInfo=NULL;
	int32_t Maxplayers=2;
	char *uri=NULL;
	
	DCV_VARS = calloc(1,sizeof(struct privatebet_vars));
	
	BET_dcv=calloc(1,sizeof(struct privatebet_info));
	//BET_dcv->pubsock = pubsock;//BET_nanosock(1,bindaddr,NN_PUB);
	//BET_dcv->pullsock = pullsock;//BET_nanosock(1,bindaddr1,NN_PULL);
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

	BET_rest_uri(&uri);
	dcvInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(dcvInfo,"method","dcv");
	cJSON_AddStringToObject(dcvInfo,"dcv",cJSON_Print(dcvKeyInfo));
	cJSON_AddStringToObject(dcvInfo,"uri",uri);
	cJSON_AddNumberToObject(dcvInfo,"balance",BET_rest_listfunds());
	lws_write(wsi,cJSON_Print(dcvInfo),strlen(cJSON_Print(dcvInfo)),0);
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

	seatInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(seatInfo,"name","player1");
	cJSON_AddNumberToObject(seatInfo,"seat",0);
	cJSON_AddNumberToObject(seatInfo,"stack",0);
	cJSON_AddNumberToObject(seatInfo,"empty",0);
	cJSON_AddNumberToObject(seatInfo,"playing",1);

	seatsInfo=cJSON_CreateArray();
	cJSON_AddItemToArray(seatsInfo,seatInfo);

	
	seatInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(seatInfo,"name","player2");
	cJSON_AddNumberToObject(seatInfo,"seat",1);
	cJSON_AddNumberToObject(seatInfo,"stack",0);
	cJSON_AddNumberToObject(seatInfo,"empty",0);
	cJSON_AddNumberToObject(seatInfo,"playing",1);

	cJSON_AddItemToArray(seatsInfo,seatInfo);
        
	tableInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(tableInfo,"method","seats");
	cJSON_AddItemToObject(tableInfo,"seats",seatsInfo);
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(tableInfo));
	lws_write(wsi,cJSON_Print(tableInfo),strlen(cJSON_Print(tableInfo)),0);
	return 0;
	
}


int32_t BET_rest_game(struct lws *wsi, cJSON *argjson)
{
	cJSON *gameInfo=NULL,*gameDetails=NULL,*potInfo=NULL;
	
	gameDetails=cJSON_CreateObject();
	cJSON_AddNumberToObject(gameDetails,"tocall",0);
	cJSON_AddNumberToObject(gameDetails,"seats",2);
	
	potInfo=cJSON_CreateArray();
	cJSON_AddItemToArray(potInfo,cJSON_CreateNumber(0));

	cJSON_AddItemToObject(gameDetails,"pot",potInfo);
	cJSON_AddStringToObject(gameDetails,"gametype","NL Hold'em<br>Blinds: 3/6");

	gameInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(gameInfo,"method","game");
	cJSON_AddItemToObject(gameInfo,"game",gameDetails);

	lws_write(wsi,cJSON_Print(gameInfo),strlen(cJSON_Print(gameInfo)),0);
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

	BET_dcv->numplayers=++players_joined;
	dcv_info.peerpubkeys[players_joined-1]=jbits256(argjson,"pubkey");
	strcpy(dcv_info.uri[players_joined-1],jstr(argjson,"uri"));
	
	
	
	playerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerInfo,"method","join_res");
	cJSON_AddNumberToObject(playerInfo,"peerid",BET_dcv->numplayers-1); //players numbering starts from 0(zero)
	jaddbits256(playerInfo,"pubkey",jbits256(argjson,"pubkey"));

	BET_rest_uri(&uri);
	cJSON_AddStringToObject(playerInfo,"uri",uri);

	
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(playerInfo));

	lws_write(wsi,cJSON_Print(playerInfo),strlen(cJSON_Print(playerInfo)),0);

	return 0;
}

int32_t BET_rest_broadcast_table_info(struct lws *wsi)
{
	cJSON *tableInfo=NULL,*playersInfo=NULL;
	char str[65];
	int32_t bytes,retval=1;;
	
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
	int32_t bytes,retval=-1;
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

	
	init=cJSON_CreateObject();
	cJSON_AddStringToObject(init,"method","init");

	lws_write(wsi,cJSON_Print(init),strlen(cJSON_Print(init)),0);

	return 0;
}

int32_t BET_rest_dcv_process_init_p(struct lws *wsi, cJSON *argjson)
{
  	int32_t peerid,retval=1;
  	bits256 cardpubvalues[CARDS777_MAXCARDS];
	cJSON *cardinfo=NULL;
	char str[65];

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
	  char str[65],*rendered;
	  int32_t bytes,retval=1;
	  
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
	int retval=1,bytes;
	char *rendered=NULL;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

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
	int32_t retval=1,bytes;
	cJSON *turninfo=NULL;
	char *rendered=NULL;

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
	printf("%s:%d\n",__FUNCTION__,__LINE__);
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
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards]=jint(playerCardInfo,"decoded_card");
	}
	else if(card_type == river_card)
	{
		card_matrix[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=1;
		card_values[playerid][no_of_hole_cards+no_of_flop_cards+no_of_turn_card]=jint(playerCardInfo,"decoded_card");
	}
	/*
	printf("\nCard Matrix:\n");
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			printf("%d\t",card_matrix[j][i]);
		}
		printf("\n");
	}
	*/
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
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		
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

	printf("%s::%d::BET_dcv->numplayers:%d\n",__FUNCTION__,__LINE__,dcv_info.numplayers);
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
				
	//BET_rest_dcv_init(wsi,NULL);
}


int32_t BET_rest_evaluate_hand(struct lws *wsi)
{
	int retval=1,max_score=0,no_of_winners=0,winning_amount=0,bytes;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS],players_left=0,only_winner=-1;
	cJSON *resetInfo=NULL,*gameInfo=NULL;
	char *rendered=NULL;
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
			//retval=BET_DCV_invoice_pay(bet,vars,only_winner,vars->pot);
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
			//retval=BET_DCV_invoice_pay(bet,vars,i,(vars->pot/no_of_winners));
			BET_rest_DCV_create_invoice_request(wsi,(DCV_VARS->pot/no_of_winners),i);
	
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
			lws_write(wsi,cJSON_Print(resetInfo),strlen(cJSON_Print(resetInfo)),0);
			BET_rest_DCV_reset(wsi);
			
		}
		return retval;
}

int32_t BET_rest_relay(struct lws *wsi, cJSON *argjson)
{
	int32_t retval=1,bytes;
	printf("sent %s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
	lws_write(wsi,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
	
	return 0;
}

int32_t BET_rest_dcv_LN_check()
{
	char channel_id[100],channel_state;
	int argc,retval=1;
	char **argv,*buf=NULL,uri[100];
	cJSON *peerInfo=NULL,*fundChannelInfo=NULL;
	argc=6;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);
	buf=(char*)malloc(10000);
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
		memset(buf,0x00,sizeof(buf));

		//ln_bet(argc,argv,buf);
		make_command(argc,argv);

		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argv[4]=NULL;
		argc=4;

		
		memset(buf,0x00,sizeof(buf));
		//ln_bet(argc,argv,buf);
		

		fundChannelInfo=cJSON_CreateObject();
		fundChannelInfo=make_command(argc,argv);

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
	if(buf)
		free(buf);
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
	
	#if 0
	bvvJoinInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvJoinInfo,"method","bvv_join");
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(bvvJoinInfo));
	lws_write(wsi,cJSON_Print(bvvJoinInfo),strlen(cJSON_Print(bvvJoinInfo)),0);
	#endif
	return 0;
}



int32_t BET_rest_create_invoice(struct lws *wsi,cJSON *argjson)
{
	int argc,bytes,retval=1;
	char **argv,*rendered;
	char hexstr [65];
	int32_t maxsize = 10000;
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
	make_command_temp(argc,argv,invoice);
	//ln_bet(argc,argv,buf);
	//invoice=cJSON_Parse(buf);
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
		//cJSON_AddStringToObject(invoiceInfo,"option",jint(argjson,"option"));	
		
		if(strcmp(jstr(cJSON_GetObjectItem(argjson,"payment_params"),"action"),"round_betting")==0)
		{
			printf("%s:%d\n",__FUNCTION__,__LINE__);
			cJSON_AddNumberToObject(invoiceInfo,"option",jint(argjson,"option"));
		}
		
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));
		cJSON_AddItemToObject(invoiceInfo,"payment_params",cJSON_GetObjectItem(argjson,"payment_params"));
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

	if(invoice)
		free_json(invoice);
	if(invoiceInfo)
		free_json(invoiceInfo);
	
	return retval;
}




int32_t BET_rest_DCV_create_invoice(struct lws *wsi,cJSON *argjson)
{
	int argc,bytes,retval=1;
	char **argv=NULL,*rendered=NULL;
	char hexstr [65];
	int32_t maxsize = 10000;
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
	sprintf(argv[3],"%d_%d",jint(argjson,"playerID"),jint(argjson,"winningAmount"));
	sprintf(argv[4],"\"Invoice details playerID:%d,winning Amount:%d\"",jint(argjson,"playerID"),jint(argjson,"winningAmount"));
	argv[5]=NULL;
	argc=5;

	
	invoice=cJSON_CreateObject();
	invoice=make_command(argc,argv);
	/*
	ln_bet(argc,argv,buf);
	invoice=cJSON_Parse(buf);
	*/
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
		
		if(invoice)
			free_json(invoice);
		if(invoiceInfo)
			free_json(invoiceInfo);
		
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

int32_t BET_process_rest_method(struct lws *wsi, cJSON *argjson)
{

	int retval=-1;
	printf("Received %s:%d::%s\n",__FUNCTION__,__LINE__,jstr(argjson,"method"));
	if(strcmp(jstr(argjson,"method"),"game") == 0)	
	{
		retval=BET_rest_game(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"seats") == 0)
	{
		retval=BET_rest_seats(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"chat") == 0)
	{
		retval=	BET_rest_chat(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"action") == 0)	
	{
		retval=BET_rest_default(wsi,argjson);
	}	
	else if(strcmp(jstr(argjson,"method"),"dcv") == 0)
	{
		retval=	BET_rest_dcv_init(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"bvv") == 0)
	{
		retval=	BET_rest_bvv_init(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"player") == 0)
	{
		retval=	BET_rest_player(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"from_dcv") == 0)
	{
		retval=	BET_rest_from_dcv(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"from_bvv") == 0)
	{
		retval=	BET_rest_from_bvv(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"from_player") == 0)
	{
		retval=	BET_rest_from_player(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"bvv_join") == 0)
	{
		retval=	BET_rest_bvv_join(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"check_bvv_ready") == 0)
	{
		retval=	BET_rest_bvv_check_bvv_ready(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"player_join") == 0)
	{
		retval=	BET_rest_player_join(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"init") == 0)
	{
		retval=BET_rest_player_init(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"join_req") == 0)
	{
		//printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
		if(BET_dcv->numplayers<BET_dcv->maxplayers)
		{
			retval=BET_rest_client_join_req(wsi,argjson);
		    if(BET_dcv->numplayers==BET_dcv->maxplayers)
			{
				retval=BET_rest_dcv_LN_check();

				if(retval<0)
					goto end;
				
				printf("Table is filled\n");
				BET_rest_broadcast_table_info(wsi);
				BET_rest_check_BVV_Ready(wsi);
			}
		}
	}
	else if(strcmp(jstr(argjson,"method"),"join_res") == 0)
	{
		retval=BET_rest_player_join_res(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"bvv_ready") == 0)
	{
		retval=BET_rest_dcv_start_init(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"init_p") == 0)
	{
		retval=BET_rest_dcv_process_init_p(wsi,argjson);
		if(dcv_info.numplayers==dcv_info.maxplayers)
		{
		
			retval=BET_rest_dcv_deck_init_info(wsi,argjson);
		}
	}
	else if(strcmp(jstr(argjson,"method"),"init_d_bvv") == 0)
	{
		retval=BET_rest_bvv_compute_init_b(wsi,argjson);
		//retval=BET_rest_player_process_init_d(wsi,argjson,0);
		//retval=BET_rest_player_process_init_d(wsi,argjson,1);
		
	}
	else if(strcmp(jstr(argjson,"method"),"init_d_player") == 0)
	{
		retval=BET_rest_player_process_init_d(wsi,argjson,jint(argjson,"gui_playerID"));
	}
	else if(strcmp(jstr(argjson,"method"),"init_b_player") == 0)
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		retval=BET_rest_player_process_init_b(wsi,argjson,jint(argjson,"gui_playerID"));
		//retval=BET_rest_player_process_init_b(wsi,argjson,1);
	}
	else if(strcmp(jstr(argjson,"method"),"player_ready") == 0)
	{
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,jstr(argjson,"method"));

		if(BET_rest_check_player_ready(wsi,argjson))
		{
			retval=BET_rest_initiate_statemachine(wsi,argjson);
			printf("\n%s::%d::Initiate the state machine\n",__FUNCTION__,__LINE__);
			  
		}
		
	}
	else if(strcmp(jstr(argjson,"method"),"dealer_bvv") == 0)
	{
			retval=BET_rest_bvv_dealer_info(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"dealer_player") == 0)
	{
			retval=BET_rest_player_dealer_info(wsi,argjson,jint(argjson,"gui_playerID"));
			//retval=BET_rest_player_dealer_info(wsi,argjson,1);
	}
	else if(strcmp(jstr(argjson,"method"), "dealer_ready") == 0)
	{
			printf("%s::%d\n",__FUNCTION__,__LINE__);
			retval=BET_rest_dcv_turn(wsi);
		
	}
	else if(strcmp(jstr(argjson,"method"),"turn") == 0)
	{
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));	
		retval=BET_rest_player_turn(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"requestShare") == 0)
	{
		
		retval=BET_rest_player_give_share(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"share_info") == 0)
	{
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
		BET_rest_player_receive_share(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"playerCardInfo") == 0)
	{
			retval = BET_rest_receive_card(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"betting") == 0)
	{
		retval=BET_rest_betting_statemachine(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"invoiceRequest") == 0)
	{
			retval=BET_rest_create_invoice(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"invoice") == 0)
	{
			retval=BET_rest_player_invoice(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"winningInvoiceRequest") == 0)
	{
			retval=BET_rest_DCV_create_invoice(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"winningClaim") == 0)
	{
			retval=BET_rest_DCV_winningClaim(wsi,argjson);
			
	}
	else if(strcmp(jstr(argjson,"method"),"push_cards") == 0)
	{
			rest_push_cards(wsi,NULL,jint(argjson,"playerid"));
			
	}
	else if(strcmp(jstr(argjson,"method"),"replay") == 0)
	{
			rest_push_cards(wsi,NULL,jint(argjson,"playerid"));
			lws_write(wsi,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
				
	}
	else if(strcmp(jstr(argjson,"method"),"player_reset") == 0)
	{
		BET_rest_player_reset(wsi,argjson);
	}
	else if(strcmp(jstr(argjson,"method"),"bvv_reset") == 0)
	{
		BET_rest_BVV_reset();
	}
	else
	{
		
		retval=BET_rest_dcv_default(wsi,argjson);

	}

	end:
		return 0;
}

char lws_buf[65536];
int32_t lws_buf_length=0;
int lws_callback_http_dummy(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
        int ret_val,ret_len;
        char *buf=NULL;
        buf=(char*)malloc(len);
        strncpy(buf,in,len);
		
        cJSON *argjson=NULL,*gameInfo=NULL,*gameDetails=NULL,*potInfo=NULL;
        switch(reason)
        {
            case LWS_CALLBACK_RECEIVE:
				memcpy(lws_buf+lws_buf_length,in,len);
				lws_buf_length+=len;
				if (!lws_is_final_fragment(wsi))
						break;
				argjson=cJSON_CreateObject();
				argjson=cJSON_Parse(lws_buf);
				memset(lws_buf,0x00,sizeof(lws_buf));
				lws_buf_length=0;
				while( BET_process_rest_method(wsi,argjson) != 0 )
				{
					printf("\n%s:%d:Failed to process the host command",__FUNCTION__,__LINE__);
				}
                break;
        }
		if(buf)
			free(buf);
        return 0;
}


static struct lws_protocols protocols[] = {
	{ "http", lws_callback_http_dummy, 0, 0 },
	LWS_PLUGIN_PROTOCOL_MINIMAL,
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

struct privatebet_peerln *BET_peerln_find(char *peerid)
{
    int32_t i;
    if ( peerid != 0 && peerid[0] != 0 )
    {
        for (i=0; i<Num_peersln; i++)
            if ( strcmp(Peersln[i].raw.peerid,peerid) == 0 )
                return(&Peersln[i]);
    }
    return(0);
}

struct privatebet_peerln *BET_peerln_create(struct privatebet_rawpeerln *raw,int32_t maxplayers,int32_t maxchips,int32_t chipsize)
{
    struct privatebet_peerln *p; cJSON *inv; char label[64];
    bits256 temp;
    if ( (p= BET_peerln_find(raw->peerid)) == 0 )
    {
        p = &Peersln[Num_peersln++];
        p->raw = *raw;
    }
    if ( IAMHOST != 0 && p != 0 )//&& strcmp(Host_peerid,LN_idstr) != 0 )
    {
        sprintf(label,"%s_%d",raw->peerid,0);
       // p->hostrhash = chipsln_rhash_create(chipsize,label);

    }
    return(p);
}

int32_t BET_host_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    bits256 pubkey; int32_t n; char *peerid,label[64]; bits256 clientrhash; struct privatebet_peerln *p;
    pubkey = jbits256(argjson,"pubkey");
    if ( bits256_nonz(pubkey) != 0 )
    {
        peerid = jstr(argjson,"peerid");
        clientrhash = jbits256(argjson,"clientrhash");
        printf("JOIN.(%s)\n",jprint(argjson,0));
        if ( bits256_nonz(bet->tableid) == 0 )
            bet->tableid = Mypubkey;
        if ( peerid != 0 && peerid[0] != 0 )
        {
            if ((p= BET_peerln_find(peerid)) == 0 )
            {
                p = &Peersln[Num_peersln++];
                memset(p,0,sizeof(*p));
                safecopy(p->raw.peerid,peerid,sizeof(p->raw.peerid));
            }
            if ( p != 0 )
            {
                sprintf(label,"%s_%d",peerid,0);
                //p->hostrhash = chipsln_rhash_create(bet->chipsize,label);
                p->clientrhash = clientrhash;
                p->clientpubkey = pubkey;
            }
            if ( BET_pubkeyfind(bet,pubkey,peerid) < 0 )
            {
                if ( (n= BET_pubkeyadd(bet,pubkey,peerid)) > 0 )
                {
                    if ( n > 1 )
                    {
                        Gamestart = (uint32_t)time(NULL);
                        if ( n < bet->maxplayers )
                            Gamestart += BET_GAMESTART_DELAY;
                        printf("Gamestart in a %d seconds\n",BET_GAMESTART_DELAY);
                    } else printf("Gamestart after second player joins or we get maxplayers.%d\n",bet->maxplayers);
                }
                return(1);
            }
        }
    }
    return(0);
}

int32_t BET_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    char *method; int32_t senderid;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
        senderid = BET_senderid(argjson,bet);
        if ( strcmp(method,"join") == 0 )
            return(BET_host_join(argjson,bet,vars));
        else if ( strcmp(method,"gameeval") == 0 )
        {
            BET_client_gameeval(argjson,bet,vars,senderid);
            return(1);
        }
        else if ( strcmp(method,"turni") == 0 )
        {
            BET_client_turni(argjson,bet,vars,senderid);
            return(1);
        }
        else if ( strcmp(method,"tablestatus") == 0 )
            return(0);
        else return(1);
    }
    return(-1);
}

void BET_host_gamestart(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    cJSON *deckjson,*reqjson; char *retstr;
    reqjson = cJSON_CreateObject();
    jaddstr(reqjson,"method","start0");
    jaddnum(reqjson,"numplayers",bet->numplayers);
    jaddnum(reqjson,"numrounds",bet->numrounds);
    jaddnum(reqjson,"range",bet->range);
    jaddnum(reqjson,"timestamp",bet->timestamp);
    printf("broadcast.(%s)\n",jprint(reqjson,0));
    BET_message_send("BET_start",bet->pubsock,reqjson,1,bet);
    deckjson = 0;
    if ( (reqjson= BET_createdeck_request(bet->playerpubs,bet->numplayers,bet->range)) != 0 )
    {
        //printf("call oracle\n");
        if ( (retstr= BET_oracle_request("createdeck",reqjson)) != 0 )
        {
            if ( (deckjson= cJSON_Parse(retstr)) != 0 )
            {
                printf("BET_roundstart numcards.%d numplayers.%d\n",bet->range,bet->numplayers);
                BET_roundstart(bet->pubsock,deckjson,bet->range,0,bet->playerpubs,bet->numplayers,Myprivkey);
                printf("finished BET_roundstart numcards.%d numplayers.%d\n",bet->range,bet->numplayers);
            }
            free(retstr);
        }
        free_json(reqjson);
    }
    printf("Gamestart.%u vs %u Numplayers.%d ",Gamestart,(uint32_t)time(NULL),bet->numplayers);
    printf("Range.%d numplayers.%d numrounds.%d\n",bet->range,bet->numplayers,bet->numrounds);
    BET_tablestatus_send(bet,vars);
    Lastturni = (uint32_t)time(NULL);
    vars->turni = 0;
    vars->round = 0;
    reqjson = cJSON_CreateObject();
    jaddstr(reqjson,"method","start");
    jaddnum(reqjson,"numplayers",bet->numplayers);
    jaddnum(reqjson,"numrounds",bet->numrounds);
    jaddnum(reqjson,"range",bet->range);
    jaddnum(reqjson,"timestamp",bet->timestamp);
    BET_message_send("BET_start",bet->pubsock,reqjson,1,bet);
}

/*
 Cashier: get/newaddr, <deposit>, display balance, withdraw
 
 Table info: host addr, chipsize, gameinfo, maxchips, minplayers, maxplayers, numplayers, bankroll, performance bond, history, timeout
 
 Join table/numchips: connect to host, open channel with numchips*chipsize, getchannel to verify, getroute
 
 Host: for each channel create numchips invoices.(playerid/tableid/i) for chipsize -> return json with rhashes[]
 
 Player: send one chip as tip and to verify all is working, get elapsed time
 
 Hostloop:
 {
 if ( newpeer (via getpeers) )
 activate player
 if ( incoming chip )
 {
 if initial tip, update state to ready
 if ( game in progress )
 broadcast bet received
 }
 }
 
 Making a bet:
 player picks host's rhash, generates own rhash, sends to host
 
 HOST: recv:[{rhash0[m]}, {rhash1[m]}, ... ], sends[] <- {rhashi[m]}
 Players[]: recv:[{rhash0[m]}, {rhash1[m]}, ... ], send:{rhashi[m]}
 
 host: verifies rhash is from valid player, broadcasts new rhash
 
 each node with M chips should have MAXCHIPS-M rhashes published
 when node gets chip, M ->M+1, invalidate corresponding rhash
 when node sends chip, M -> M-1, need to create a new rhash
 */

void BETS_players_update(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    int32_t i;
    for (i=0; i<bet->numplayers; i++)
    {
        /*update state: new, initial tip, active lasttime, missing, dead
         if ( dead for more than 5 minutes )
         close channel (settles chips)*/
        /* if ( (0) && time(NULL) > Lastturni+BET_PLAYERTIMEOUT )
         {
         timeoutjson = cJSON_CreateObject();
         jaddstr(timeoutjson,"method","turni");
         jaddnum(timeoutjson,"round",VARS->round);
         jaddnum(timeoutjson,"turni",VARS->turni);
         jaddbits256(timeoutjson,"pubkey",bet->playerpubs[VARS->turni]);
         jadd(timeoutjson,"actions",cJSON_Parse("[\"timeout\"]"));
         BET_message_send("TIMEOUT",bet->pubsock,timeoutjson,1,bet);
         //BET_host_turni_next(bet,&VARS);
         }*/
    }
}

int32_t BET_rawpeerln_parse(struct privatebet_rawpeerln *raw,cJSON *item)
{
    raw->unique_id = juint(item,"unique_id");
    safecopy(raw->netaddr,jstr(item,"netaddr"),sizeof(raw->netaddr));
    safecopy(raw->peerid,jstr(item,"peerid"),sizeof(raw->peerid));
    safecopy(raw->channel,jstr(item,"channel"),sizeof(raw->channel));
    safecopy(raw->state,jstr(item,"state"),sizeof(raw->state));
    raw->msatoshi_to_us = jdouble(item,"msatoshi_to_us");
    raw->msatoshi_total = jdouble(item,"msatoshi_total");
    //{ "peers" : [ { "unique_id" : 0, "state" : "CHANNELD_NORMAL", "netaddr" : "5.9.253.195:9735", "peerid" : "02779b57b66706778aa1c7308a817dc080295f3c2a6af349bb1114b8be328c28dc", "channel" : "2646:1:0", "msatoshi_to_us" : 9999000, "msatoshi_total" : 10000000 } ] }
    return(0);
}

cJSON *BET_hostrhashes(struct privatebet_info *bet)
{
    int32_t i; bits256 rhash; struct privatebet_peerln *p; cJSON *array = cJSON_CreateArray();
    for (i=0; i<bet->numplayers; i++)
    {
        if ( (p= BET_peerln_find(bet->peerids[i])) != 0 )
            rhash = p->hostrhash;
        else memset(rhash.bytes,0,sizeof(rhash));
        jaddibits256(array,rhash);
    }
    return(array);
}

int32_t BET_chipsln_update(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    struct privatebet_rawpeerln raw; char nextlabel[512]; struct privatebet_peerln *p; cJSON *rawpeers,*channels,*invoices,*array,*item; int32_t i,n,isnew = 0,waspaid = 0,retval = 0;
    oldNum_rawpeersln = Num_rawpeersln;
    memcpy(oldRawpeersln,Rawpeersln,sizeof(Rawpeersln));
    memset(Rawpeersln,0,sizeof(Rawpeersln));
    Num_rawpeersln = 0;
    if ( (rawpeers= chipsln_getpeers()) != 0 )
    {
        if ( (array= jarray(&n,rawpeers,"peers")) != 0 )
        {
            for (i=0; i<n&&Num_rawpeersln<CARDS777_MAXPLAYERS; i++)
            {
                item = jitem(array,i);
                if ( BET_rawpeerln_parse(&Rawpeersln[Num_rawpeersln],item) == 0 )
                    Num_rawpeersln++;
            }
            if ( memcmp(Rawpeersln,oldRawpeersln,sizeof(Rawpeersln)) != 0 )
            {
                retval++;
                for (i=0; i<Num_rawpeersln; i++)
                {
                    if ( BET_peerln_find(Rawpeersln[i].peerid) == 0 )
                    {
                        BET_peerln_create(&Rawpeersln[i],bet->maxplayers,bet->maxchips,bet->chipsize);
                    }
                }
            }
            if ( (invoices= chipsln_listinvoice("")) != 0 )
            {
                //printf("listinvoice.(%s)\n",jprint(invoices,0));
                if ( is_cJSON_Array(invoices) != 0 && (n= cJSON_GetArraySize(invoices)) > 0 )
                {
                    for (i=0; i<n; i++)
                    {
                        item = jitem(invoices,i);
                        if ( jobj(item,"complete") != 0 && is_cJSON_True(jobj(item,"complete")) != 0 )
                        {
                            printf("completed! %s\n",jprint(item,0));
                            if ( (p= BET_invoice_complete(nextlabel,item,bet)) != 0 )
                            {
                            }
                        } //else printf("not complete %s\n",jprint(jobj(item,"complete"),0));
                    }
                }
                free_json(invoices);
            }
        }
        free_json(rawpeers);
    }
    return(retval);
}

void BET_hostloop(void *_ptr)
{
    uint32_t lasttime = 0; uint8_t r; int32_t nonz,recvlen,sendlen; cJSON *argjson,*timeoutjson; void *ptr; double lastmilli = 0.; struct privatebet_info *bet = _ptr; struct privatebet_vars *VARS;
    VARS = calloc(1,sizeof(*VARS));
    printf("hostloop pubsock.%d pullsock.%d range.%d\n",bet->pubsock,bet->pullsock,bet->range);
    while ( bet->pullsock >= 0 && bet->pubsock >= 0 )
    {
        nonz = 0;
        if ( (recvlen= nn_recv(bet->pullsock,&ptr,NN_MSG,0)) > 0 )
        {
            nonz++;
            if ( (argjson= cJSON_Parse(ptr)) != 0 )
            {
                if ( BET_hostcommand(argjson,bet,VARS) != 0 ) // usually just relay to players
                {
                    //printf("RELAY.(%s)\n",jprint(argjson,0));
                    BET_message_send("BET_relay",bet->pubsock,argjson,0,bet);
                    //if ( (sendlen= nn_send(bet->pubsock,ptr,recvlen,0)) != recvlen )
                    //    printf("sendlen.%d != recvlen.%d for %s\n",sendlen,recvlen,jprint(argjson,0));
                }
                free_json(argjson);
            }
            nn_freemsg(ptr);
        }
        if ( nonz == 0 )
        {
            if ( OS_milliseconds() > lastmilli+100 && BET_chipsln_update(bet,VARS) == 0 )
            {
                lastmilli = OS_milliseconds();
                //BETS_players_update(bet,VARS);
            }
            if ( time(NULL) > lasttime+60 )
            {
                printf("%s round.%d turni.%d myid.%d\n",bet->game,VARS->round,VARS->turni,bet->     myplayerid);
                lasttime = (uint32_t)time(NULL);
            }
            usleep(10000);
        }
        if ( Gamestarted == 0 )
        {
            //printf(">>>>>>>>> t%u gamestart.%u numplayers.%d max.%d turni.%d round.%d\n",(uint32_t)time(NULL),Gamestart,bet->numplayers,bet->maxplayers,VARS->turni,VARS->round);
            if ( time(NULL) >= Gamestart && bet->numplayers == bet->maxplayers && VARS->turni == 0 && VARS->round == 0 )
            {
                printf("GAME.%d\n",Numgames);
                Numgames++;
                bet->timestamp = Gamestarted = (uint32_t)time(NULL);
                BET_host_gamestart(bet,VARS);
            }
        }
    }
}

/*
The following API's describe the events of DCV in Pangea.
*/
void* BET_hostdcv(void * _ptr)
{
		uint32_t numplayers,range,playerID,bytes;
		char str[65];
		cJSON *gameInfo=NULL,*playerInfo=NULL,*item=NULL,*cjsoncardprods=NULL,*cjsonfinalcards=NULL,*cjsong_hash=NULL;
		bits256 playercards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],finalcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],deckid;
		struct privatebet_info *bet = _ptr;
		range=bet->range;
	
	  numplayers=0;
	  dekgen_vendor_perm(bet->range);
      deckid=rand256(0);
      
     
      if ( bet->pubsock >= 0 && bet->pullsock >= 0 ) 
	  {
		while(numplayers!=bet->numplayers)
		  {
			char *buf=NULL;
			int bytes=nn_recv(bet->pullsock,&buf,NN_MSG,0);
			if(bytes>0)
			{
				printf("\n%s:%d,buf:%s",__FUNCTION__,__LINE__,buf);
				gameInfo=cJSON_Parse(buf);
				if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"init"))
				{
					numplayers++;
					playerID=jint(gameInfo,"playerid");
					range=jint(gameInfo,"range");
					playerInfo=cJSON_GetObjectItem(gameInfo,"playercards");
					for(int i=0;i<cJSON_GetArraySize(playerInfo);i++)
					{
							playercards[playerID][i]=jbits256i(playerInfo,i);
					}
				}
				bytes=nn_send(bet->pubsock,buf,strlen(buf),0);
			 }
	      }
		  cJSON_Delete(gameInfo);
		  printf("\n%s:%d:DCV received all players:%d",__FUNCTION__,__LINE__,numplayers);
		  for (int playerid=0; playerid<numplayers; playerid++)
		  {
        		sg777_deckgen_vendor(playerid,cardprods[playerid],finalcards[playerid],range,playercards[playerid],deckid);
          }
		  gameInfo=cJSON_CreateObject();
		  cJSON_AddStringToObject(gameInfo,"method","init_d");
		  jaddbits256(gameInfo,"deckid",deckid);
		  cJSON_AddItemToObject(gameInfo,"cardprods",cjsoncardprods=cJSON_CreateArray());
		  for(int i=0;i<numplayers;i++)
		  {
			for(int j=0;j<range;j++)
			{
				cJSON_AddItemToArray(cjsoncardprods,cJSON_CreateString(bits256_str(str,cardprods[i][j])));
			}
		  }
		  cJSON_AddItemToObject(gameInfo,"finalcards",cjsonfinalcards=cJSON_CreateArray());
		  for(int i=0;i<numplayers;i++)
		  {
			for(int j=0;j<range;j++)
			{
				cJSON_AddItemToArray(cjsonfinalcards,cJSON_CreateString(bits256_str(str,finalcards[i][j])));
			}
		  }

		  cJSON_AddItemToObject(gameInfo,"g_hash",cjsong_hash=cJSON_CreateArray());
		  for(int i=0;i<numplayers;i++)
		  {
			for(int j=0;j<range;j++)
			{
				cJSON_AddItemToArray(cjsong_hash,cJSON_CreateString(bits256_str(str,g_hash[i][j])));
			}
		  }
		  
		  char *rendered=cJSON_Print(gameInfo);
		  bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
		  while(1)
		  {
	  		  	char *buf=NULL;
	  		  	int bytes=nn_recv(bet->pullsock,&buf,NN_MSG,0);
	  			if(bytes>0)
	  			{
	  				bytes=nn_send(bet->pubsock,buf,strlen(buf),0);
					
	  			}
           }
		  nn_shutdown(bet->pullsock,0);
		  nn_shutdown(bet->pubsock,0);
	  }
	  return NULL;
}

int32_t BET_p2p_host_deck_init_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
      cJSON *deck_init_info,*cjsoncardprods,*cjsondcvblindcards,*cjsong_hash,*cjsonpeerpubkeys;
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
	char str[65];

  	peerid=jint(argjson,"peerid");
  	cardinfo=cJSON_GetObjectItem(argjson,"cardinfo");
	for(int i=0;i<cJSON_GetArraySize(cardinfo);i++)
	{
			cardpubvalues[i]=jbits256i(cardinfo,i);
	} 	
	
	retval=sg777_deckgen_vendor(peerid,dcv_info.cardprods[peerid],dcv_info.dcvblindcards[peerid],bet->range,cardpubvalues,dcv_info.deckid);
	dcv_info.numplayers=dcv_info.numplayers+1;
	return retval;
}

int32_t BET_p2p_bvv_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,maxsize=10000,retval=1,state;
	char **argv,uri[100],*buf;
	cJSON *connectInfo=NULL,*fundChannelInfo=NULL;
	strcpy(uri,jstr(argjson,"uri"));
	strcpy(dcv_info.bvv_uri,uri);
	if((LN_get_channel_status(strtok(jstr(argjson,"uri"), "@")) != 3)) // 3 means channel is already established with the peer
		{
						
			argc=5;
            argv=(char**)malloc(argc*sizeof(char*));
            buf=malloc(maxsize);
            for(int i=0;i<argc;i++)
            {
             argv[i]=(char*)malloc(100*sizeof(char));
	        }
			argc=3;
			strcpy(argv[0],"./bet");
			strcpy(argv[1],"connect");
			strcpy(argv[2],uri);
			argv[3]=NULL;
			ln_bet(argc,argv,buf);
			connectInfo=cJSON_Parse(buf);
			cJSON_Print(connectInfo);
			if(jint(connectInfo,"code") != 0)
			{
				retval=-1;
				printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(connectInfo,"message"));
				goto end;
			}
		
			argc=5;
			argv=(char**)malloc(argc*sizeof(char*));
			buf=malloc(maxsize);
			for(int i=0;i<argc;i++)
			{
			        argv[i]=(char*)malloc(100*sizeof(char));
			}
			strcpy(argv[0],"./bet");
			strcpy(argv[1],"fundchannel");
			strcpy(argv[2],jstr(connectInfo,"id"));
			printf("\n id:%s",argv[2]);
			strcpy(argv[3],"500000");
			argv[4]=NULL;
			argc=4;	
			ln_bet(argc,argv,buf);
			fundChannelInfo=cJSON_Parse(buf);
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
				          printf("\nCHANNELD_AWAITING_LOCKIN");
				  }
				  else if(state == 8)
				  {
				           printf("\nONCHAIN");
				  }
				   else
				           printf("\n%s:%d:channel-state:%d\n",__FUNCTION__,__LINE__,state);
				sleep(10);
			}

			printf("\nDCV-->BVV LN Channel established");
			
		}
		
	end:
		return retval;
}

int32_t BET_p2p_host_start_init(struct privatebet_info *bet)
{
	int32_t bytes,retval=-1;
	cJSON *init=NULL;
	char *rendered=NULL;
	
	init=cJSON_CreateObject();
	cJSON_AddStringToObject(init,"method","init");

	rendered=cJSON_Print(init);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);


	return retval;
}
int32_t BET_p2p_client_join_req(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *playerinfo=NULL,*getInfo=NULL,*addresses=NULL,*address=NULL;
    uint32_t bytes,retval=1;
	char *rendered=NULL,*uri=NULL;
	int argc,maxsize=10000;
	char **argv=NULL,*buf=NULL;


	
    bet->numplayers=++players_joined;
	dcv_info.peerpubkeys[players_joined-1]=jbits256(argjson,"pubkey");
	strcpy(dcv_info.uri[players_joined-1],jstr(argjson,"uri"));
	
	argv=(char**)malloc(4*sizeof(char*));
	buf=malloc(maxsize);
	argc=3;
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));		
	
	strcpy(argv[0],"./bet");
	strcpy(argv[1],"getinfo");
	argv[2]=NULL;
	
	ln_bet(argc-1,argv,buf);
	getInfo=cJSON_Parse(buf);
	uri=(char*)malloc(100*sizeof(char));
	
	addresses=cJSON_GetObjectItem(getInfo,"address");
	address=cJSON_GetArrayItem(addresses,0);

	strcpy(uri,jstr(getInfo,"id"));
	strcat(uri,"@");
	strcat(uri,jstr(address,"address"));

	playerinfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerinfo,"method","join_res");
	cJSON_AddNumberToObject(playerinfo,"peerid",bet->numplayers-1); //players numbering starts from 0(zero)
	jaddbits256(playerinfo,"pubkey",jbits256(argjson,"pubkey"));
	cJSON_AddStringToObject(playerinfo,"uri",uri);
	

	rendered=cJSON_Print(playerinfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
	{
		retval=-1;
		printf("\n%s:%d: Failed to send data");
		goto end;
	}
	end:
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
	int32_t retval=1,bytes;
	cJSON *turninfo=NULL;
	char *rendered=NULL;

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
	end:
		return retval;
}

int32_t BET_create_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,bytes,retval=1;
	char **argv,*buf=NULL,*rendered;
	char hexstr [65];
	int32_t maxsize = 1000000;
	cJSON *invoiceInfo=NULL,*invoice=NULL;
	argc=6;
	argv =(char**)malloc(argc*sizeof(char*));
	buf=(char*)malloc(maxsize*sizeof(char));
	invoiceID++;
	for(int i=0;i<argc;i++)
	{
			argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	dcv_info.betamount+=jint(argjson,"betAmount");

	strcpy(argv[0],"./bet");
	strcpy(argv[1],"invoice");
	sprintf(argv[2],"%d",jint(argjson,"betAmount"));
	sprintf(argv[3],"%s_%d_%d_%d_%d",bits256_str(hexstr,dcv_info.deckid),invoiceID,jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	sprintf(argv[4],"Invoice details playerID:%d,round:%d,betting Amount:%d",jint(argjson,"playerID"),jint(argjson,"round"),jint(argjson,"betAmount"));
	argv[5]=NULL;
	argc=5;

	ln_bet(argc,argv,buf);
	invoice=cJSON_CreateObject();
	invoice=cJSON_Parse(buf);
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
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",buf);
		
		rendered=cJSON_Print(invoiceInfo);
		bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
		
		if(bytes<0)
		{
			retval=-1;
			printf("\n%s :%d Failed to send data",__FUNCTION__,__LINE__);
			goto end;
		}
					
	}
	
	end:
		return retval;
}

	
int32_t BET_settle_game(cJSON *payInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t playerid,max=-1,retval=1;
	cJSON *invoicesInfo=NULL,*invoiceInfo=NULL,*invoice=NULL,*winnerInfo=NULL;
	char *label=NULL;
	int32_t argc,bytes;
	int32_t maxsize = 1000000;
	char **argv=NULL,*buf=NULL,*rendered=NULL;

	argc=4;
	argv=(char**)malloc(sizeof(char*)*argc);
	for(int32_t i=0;i<=argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	buf=(char*)malloc(maxsize*sizeof(char));

	
	label=jstr(payInfo,"label");
	strcpy(argv[0],".\bet");
	strcpy(argv[1],"listinvoices");
	strcpy(argv[2],label);
	argv[3]=NULL;
	
	ln_bet(argc-1,argv,buf);
	invoicesInfo=cJSON_CreateObject();
	invoicesInfo=cJSON_Parse(buf);

	if(jint(invoicesInfo,"code") != 0 )
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(invoicesInfo,"message"));
		goto end;
	}
	else
	{
		
		cJSON_Parse(invoicesInfo);
		
		invoiceInfo=cJSON_CreateObject();
		invoiceInfo=cJSON_GetObjectItem(invoicesInfo,"invoices");
		
		invoice=cJSON_CreateObject();
		invoice=cJSON_GetArrayItem(invoiceInfo,0);
		if(strcmp(jstr(invoice,"status"),"paid")==0)
		{
			dcv_info.paidamount+=jint(invoice,"msatoshi_received");
			printf("\nAmount paid: %d",jint(invoice,"msatoshi_received"));
		}
		printf("\n%s:%d:%d",__FUNCTION__,no_of_bets,no_of_cards);
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
	int retval=1,playerid,cardid,card_type,flag;
	
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
	/*
	printf("\nCard Matrix:\n");
	for(int i=0;i<hand_size;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			printf("%d\t",card_matrix[j][i]);
		}
		printf("\n");
	}
	*/
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
		vars->funds[i]=10000000;// hardcoded max funds to 10000 satoshis
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
int32_t BET_evaluate_hand(cJSON *playerCardInfo,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,max_score=0,no_of_winners=0,winning_amount=0,bytes;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS],players_left=0,only_winner=-1;
	cJSON *resetInfo=NULL,*gameInfo=NULL;
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
	int argc,maxsize=1000;
	char **argv=NULL,*buf=NULL;
	
	buf=(char*)malloc(maxsize*sizeof(char));
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
		strcpy(argv[0],".\bet");
		strcpy(argv[1],"connect");
		strcpy(argv[2],dcv_info.uri[i]);
		ln_bet(argc,argv,buf);
		printf("\n%s:%d:Conncet Response:%s",__FUNCTION__,__LINE__,buf);
	}
}

int BET_LN_check_if_peer_exists(char *channel_id)
{
	int argc,retval=1;
	char **argv,*buf=NULL;
	cJSON *peerInfo=NULL;
	
	argc=4;	
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);
	buf=(char*)malloc(10000);

	strcpy(argv[0],"./bet");
	strcpy(argv[1],"check-if-peer-exists");
	strcpy(argv[2],channel_id);
	argv[3]=NULL;
	argc=3;

	ln_bet(argc,argv,buf);
	
	peerInfo=cJSON_CreateObject();
	peerInfo=cJSON_Parse(buf);

	if(strcmp(jstr(peerInfo,"peer-exists"),"true")==0)
		return 1;
	else 
		return 0;
		
}
int32_t BET_LN_check(struct privatebet_info *bet)
{
	char channel_id[100],channel_state;
	int argc,retval=1;
	char **argv,*buf=NULL,uri[100];
	cJSON *peerInfo=NULL,*fundChannelInfo=NULL;
	argc=6;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100);
	buf=(char*)malloc(10000);
	strcpy(uri,dcv_info.bvv_uri);
	strcpy(channel_id,strtok(uri, "@"));
	channel_state=LN_get_channel_status(channel_id);
	if((channel_state != 2) && (channel_state != 3))
	{
		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"./bet");
		strcpy(argv[1],"connect");
		strcpy(argv[2],dcv_info.bvv_uri);
		argv[3]=NULL;
		argc=3;
		memset(buf,0x00,sizeof(buf));
		ln_bet(argc,argv,buf);

		argc=6;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,sizeof(argv[i]));
		strcpy(argv[0],"./bet");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argv[4]=NULL;
		argc=4;

		
		memset(buf,0x00,sizeof(buf));
		ln_bet(argc,argv,buf);

		fundChannelInfo=cJSON_CreateObject();
		fundChannelInfo=cJSON_Parse(buf);

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

	for(int i=0;i<bet->maxplayers;i++)
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
		return retval;
}

void BET_p2p_LN_close(struct privatebet_info *bet)
{
	int argc,maxsize=10000;
	char **argv,*buf;
	argc=4;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	buf=(char*)malloc(maxsize);
	strcpy(argv[0],"./bet");
	strcpy(argv[1],"close");
	argv[3]=NULL;
	argc=3;
	for(int i=0;i<bet->maxplayers;i++)
	{
		strcpy(argv[2],strtok(dcv_info.uri[i],"@"));
		ln_bet(argc,argv,buf);
		printf("\n%s:%d: %s\n",__FUNCTION__,__LINE__,buf);
	}
	
}
int32_t BET_award_winner(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,maxsize=100000,retval=1;
	char **argv=NULL,*buf=NULL,hexstr[65],channel_id[100],*invoice=NULL;
	cJSON *payResponse=NULL,*invoiceInfo=NULL,*fundChannelInfo=NULL;

	buf=(char*)malloc(maxsize*sizeof(char));

	
	argc=5;
	argv=(char**)malloc(sizeof(char*)*argc);
	for(int32_t i=0;i<argc;i++)
		argv[i]=(char*)malloc(1000*sizeof(char));
	strcpy(channel_id,strtok(dcv_info.uri[jint(argjson,"playerid")], "@"));
	if(LN_get_channel_status(channel_id)!=3)
	{
		strcpy(argv[0],".\bet");
		strcpy(argv[1],"fundchannel");
		strcpy(argv[2],channel_id);
		strcpy(argv[3],"500000");
		argv[4]=NULL;
		argc=4;
		ln_bet(argc,argv,buf);

		fundChannelInfo=cJSON_CreateObject();
		fundChannelInfo=cJSON_Parse(buf);

		if(jint(fundChannelInfo,"code") != 0)
		{
			retval=-1;
			printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
			goto end;
		}

		printf("\nFund channel response:%s\n",buf);
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
			 sleep(10);
		  }
	}
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);
	
	for(int32_t i=0;i<argc;i++)
		memset(argv[i],0,sizeof(argv[i]));
	
	argc=3;
	strcpy(argv[0],"./bet");
	strcpy(argv[1],"pay");
	sprintf(argv[2],"%s",jstr(invoiceInfo,"bolt11"));
	argv[3]=NULL;

	memset(buf,0,sizeof(buf));
	ln_bet(argc,argv,buf);
	payResponse=cJSON_CreateObject();
	payResponse=cJSON_Parse(buf);

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
		return retval;
}

int32_t BET_p2p_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    char *method; int32_t bytes,retval=1;
	char *rendered=NULL;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
		if(strcmp(method,"join_req") == 0)
		{
			if(bet->numplayers<bet->maxplayers)
			{
				retval=BET_p2p_client_join_req(argjson,bet,vars);
				if(retval<0)
					goto end;
                if(bet->numplayers==bet->maxplayers)
				{
					printf("Table is filled");
					retval=BET_LN_check(bet);
					if(retval<0)
						goto end;
					BET_broadcast_table_info(bet);
					BET_check_BVV_Ready(bet);
				}
			}
		}
		else if(strcmp(method,"bvv_ready") == 0)
		{
			retval=BET_p2p_host_start_init(bet);
		}
		else if(strcmp(method,"init_p") == 0)
		{
			retval=BET_p2p_host_init(argjson,bet,vars);
			if(dcv_info.numplayers==dcv_info.maxplayers)
			{
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


void BET_p2p_hostloop(void *_ptr)
{
    uint32_t lasttime = 0; uint8_t r; int32_t nonz,recvlen,sendlen; cJSON *argjson,*timeoutjson; void *ptr; double lastmilli = 0.; struct privatebet_info *bet = _ptr; struct privatebet_vars *VARS;
    VARS = calloc(1,sizeof(*VARS));
    
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
    while ( bet->pullsock >= 0 && bet->pubsock >= 0 )
    {
        if ( (recvlen= nn_recv(bet->pullsock,&ptr,NN_MSG,0)) > 0 )
        {
            if ( (argjson= cJSON_Parse(ptr)) != 0 )
            {
                if ( BET_p2p_hostcommand(argjson,bet,VARS) != 0 ) // usually just relay to players
                {
                	// Do something
                }
                free_json(argjson);
            }
            nn_freemsg(ptr);
        }
          
    }
}
/*
BET API loop
*/

void BET_ws_dcvloop(void *_ptr)
{
	struct lws_context_creation_info info,info_1,dcv_info,bvv_info,player1_info,player2_info;
	struct lws_context *context,*context_1,*dcv_context,*bvv_context,*player1_context,*player2_context;
	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	printf("\n%s::%d",__FUNCTION__,__LINE__);
	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:7681\n");
    #if 1
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
        return 1;
    }   
    //for BVV    
    memset(&bvv_info, 0, sizeof bvv_info); /* otherwise uninitialized garbage */
    bvv_info.port = 9001;
    bvv_info.mounts = &mount;
    bvv_info.protocols = protocols;
    bvv_info.options =
        LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    bvv_context = lws_create_context(&bvv_info);
    if (!bvv_context) {
        lwsl_err("lws init failed\n");
        return 1;
    }
    //for Player1
    memset(&player1_info, 0, sizeof player1_info); /* otherwise uninitialized garbage */
    player1_info.port = 9002;
    player1_info.mounts = &mount;
    player1_info.protocols = protocols;
    player1_info.options =
        LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    player1_context = lws_create_context(&player1_info);
    if (!player1_context) {
        lwsl_err("lws init failed\n");
        return 1;
    }
    //for Player2
    memset(&player2_info, 0, sizeof player2_info); /* otherwise uninitialized garbage */
    player2_info.port = 9003;
    player2_info.mounts = &mount;
    player2_info.protocols = protocols;
    player2_info.options =
        LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    player2_context = lws_create_context(&player2_info);
    if (!player2_context) {
        lwsl_err("lws init failed\n");
        return 1;
    }

    #endif

	while (n >= 0 && !interrupted)
	{
		//n = lws_service(context, 1000);
		//n = lws_service(context_1, 1000);
        n = lws_service(dcv_context, 1000);
        n = lws_service(bvv_context, 1000);
        n = lws_service(player1_context, 1000);
        n = lws_service(player2_context, 1000);
	}
	//lws_context_destroy(context);
	//lws_context_destroy(context_1);
    lws_context_destroy(dcv_context);
    lws_context_destroy(bvv_context);
    lws_context_destroy(player1_context);
    lws_context_destroy(player2_context);

		
}




