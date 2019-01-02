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
#include "network.h"
#include "table.h"
#include "host.h"
#include "payment.h"
//bits256 Mypubkey;
//int32_t IAMHOST;
//int32_t Gamestart,Gamestarted,Lastturni;


char action_str[8][100]={"","small_blind","big_blind","check","raise","call","all-in","fold"};

void BET_client_turnisend(struct privatebet_info *bet,struct privatebet_vars *vars,cJSON *actions)
{
    cJSON *cmdjson;
    if ( bet->myplayerid < bet->numplayers && bits256_cmp(bet->playerpubs[vars->turni],Mypubkey) == 0 )
    {
        cmdjson = cJSON_CreateObject();
        jaddstr(cmdjson,"method","turni");
        jaddbits256(cmdjson,"tableid",bet->tableid);
        jaddnum(cmdjson,"round",vars->round);
        jaddnum(cmdjson,"turni",vars->turni);
        jaddbits256(cmdjson,"pubkey",Mypubkey);
        if ( actions != 0 )
            jadd(cmdjson,"actions",actions);
        printf("send TURNI.(%s)\n",jprint(cmdjson,0));
        BET_message_send("BET_client_turnisend",bet->pushsock,cmdjson,1,bet);
    }
}

void BET_statemachine_deali(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t deali,int32_t playerj)
{
    cJSON *reqjson;
    //printf("BET_statemachine_deali cardi.%d -> r%d, t%d, d%d playerj.%d\n",vars->permi[deali],vars->roundready,vars->turni,deali,playerj);
    reqjson = cJSON_CreateObject();
    jaddstr(reqjson,"method","deali");
    jaddnum(reqjson,"playerj",playerj);
    jaddnum(reqjson,"deali",deali);
    jaddnum(reqjson,"cardi",vars->permi[deali]);
    BET_message_send("BET_deali",bet->pubsock>=0?bet->pubsock:bet->pushsock,reqjson,1,bet);
}

void BET_client_turninext(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    cJSON *reqjson;
    //printf("BET_turni_next (%d, %d) numplayers.%d range.%d\n",vars->turni,vars->round,bet->numplayers,bet->range);
    printf("TURNI.(r%d t%d).p%d ",vars->round,vars->turni,bet->myplayerid);
    if ( IAMHOST == 0 && vars->validperms == 0 )
        return;
    if ( bits256_cmp(bet->tableid,Mypubkey) != 0 )
        Lastturni = (uint32_t)time(NULL);
    vars->turni++;
    if ( vars->turni >= bet->numplayers )
    {
        printf("Round.%d completed\n",vars->round);
        BET_statemachine_roundend(bet,vars);
        reqjson = cJSON_CreateObject();
        jaddstr(reqjson,"method","roundend");
        jaddnum(reqjson,"round",vars->round);
        jaddbits256(reqjson,"pubkey",Mypubkey);
        BET_message_send("BET_round",bet->pubsock>=0?bet->pubsock:bet->pushsock,reqjson,1,bet);
        vars->round++;
        if ( vars->round >= bet->numrounds )
        {
            BET_statemachine_gameend(bet,vars);
            BET_tablestatus_send(bet,vars);
            vars->validperms = 0;
            bet->timestamp = 0;
            vars->turni = 0;
            Gamestarted = 0;
            vars->round = 0;
            vars->lastround = -1;
            memset(vars->evalcrcs,0,sizeof(vars->evalcrcs));
            vars->consensus = 0;
            vars->numconsensus = 0;
            Gamestart = (uint32_t)time(NULL) + BET_GAMESTART_DELAY;
            printf("Game completed next start.%u vs %u\n------------------\n\n",Gamestart,(uint32_t)time(NULL));
        }
        vars->turni = 0;
    }
}

int32_t BET_client_turni(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    struct privatebet_vars argvars; int32_t n; cJSON *array = 0;
    //printf("client TURNI.(%s) senderid.%d valid.%d\n",jprint(argjson,0),senderid,vars->validperms);
    if ( (IAMHOST != 0 || vars->validperms != 0) && senderid >= 0 && senderid <= bet->numplayers )
    {
        memset(&argvars,0,sizeof(argvars));
        BET_betvars_parse(bet,&argvars,argjson);
        if ( vars->round < bet->numrounds )
        {
            if ( senderid < bet->numplayers )
            {
                if ( vars->actions[vars->round][senderid] != 0 )
                {
                    free_json(vars->actions[vars->round][senderid]);
                    vars->actions[vars->round][senderid] = 0;
                }
                if ( (array= jarray(&n,argjson,"actions")) != 0 )
                    if ( BET_statemachine_turnivalidate(bet,vars,vars->round,senderid) == 0 )
                        vars->actions[vars->round][senderid] = jduplicate(array);
            }
            //printf("round.%d senderid.%d (%s)\n",vars->round,senderid,jprint(vars->actions[vars->round][senderid],0));
            if ( argvars.turni == vars->turni && argvars.round == vars->round )
            {
                BET_client_turninext(bet,vars);
                //printf("new Turni.%d Round.%d\n",Turni,Round);
            }
        }
    }
    return(0);
}

////////////////////////// Game specific statemachine
void BET_statemachine_joined_table(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    printf("BET_statemachine_joined\n");
}

void BET_statemachine_unjoined_table(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    printf("BET_statemachine_unjoined\n");
}

cJSON *BET_statemachine_gamestart_actions(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    printf("BET_statemachine_gamestart timestamp.%u\n",bet->timestamp);
    return(cJSON_CreateArray());
}

void BET_statemachine_roundstart(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    if ( vars->roundready < bet->numrounds )
    {
        if ( vars->round == 0 )
        {
            memset(vars->evalcrcs,0,sizeof(vars->evalcrcs));
            vars->consensus = vars->numconsensus = 0;
        }
        printf("BET_statemachine_roundstart -> %d lastround.%d\n",vars->roundready,vars->lastround);
        vars->lastround = -1;
    }
}

cJSON *BET_statemachine_turni_actions(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    uint32_t r; cJSON *array = 0;
    if ( vars->round < bet->numrounds-1 )
    {
        array = cJSON_CreateArray();
        OS_randombytes((void *)&r,sizeof(r));
        if ( bet->range < 2 )
            r = 0;
        else r %= bet->range;
        jaddinum(array,r);
        printf("BET_statemachine_turni -> r%d turni.%d r.%d / range.%d\n",vars->round,vars->turni,r,bet->range);
    }
    return(array);
}

int32_t BET_statemachine_turnivalidate(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t round,int32_t senderid)
{
    return(0);
}

int32_t BET_statemachine_outofgame(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t round,int32_t senderid)
{
    return(0);
}

cJSON *BET_statemachine_gameeval(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    int32_t round,playerid; uint32_t crc32; cJSON *item,*retjson; char buf[32786];
    retjson = cJSON_CreateObject();
    buf[0] = 0;
    for (round=0; round<bet->numrounds; round++)
    {
        for (playerid=0; playerid<bet->numplayers; playerid++)
        {
            if ( (item= vars->actions[round][playerid]) != 0 )
                sprintf(buf+strlen(buf),"(%s).p%d ",jprint(item,0),playerid);
        }
        sprintf(buf+strlen(buf),"round.%d ",round);
    }
    crc32 = calc_crc32(0,buf,(int32_t)strlen(buf));
    jaddstr(retjson,"method","gameeval");
    jaddstr(retjson,"eval",buf);
    jaddnum(retjson,"crc32",crc32);
    jaddbits256(retjson,"pubkey",Mypubkey);
    BET_message_send("BET_round",bet->pubsock>=0?bet->pubsock:bet->pushsock,retjson,0,bet);
    return(retjson);
}

void BET_statemachine_gameend(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    if ( vars->consensus == 0 )
        printf("%s\n>>>>>>>>>>>>>> BET_statemachine_endgame -> %d\n",jprint(BET_statemachine_gameeval(bet,vars),1),vars->round);
    else printf("GAME end after consensus.%u/%d?\n",vars->consensus,vars->numconsensus);
}

void BET_statemachine_roundend(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    printf("BET_statemachine_endround -> %d\n",vars->round);
    if ( vars->round == bet->numrounds-1 )
        BET_statemachine_gameend(bet,vars);
}

void BET_statemachine_consensus(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    printf("BET_statemachine_consensus.%u num.%d\n",vars->consensus,vars->numconsensus);
}

void BET_statemachine(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    cJSON *actions;
    if ( vars->validperms != 0 && vars->turni == bet->myplayerid && vars->roundready == vars->round && vars->lastround != vars->round )
    {
        actions = BET_statemachine_turni_actions(bet,vars);
        BET_client_turnisend(bet,vars,actions);
        vars->lastround = vars->round;
    }
}
////////////////////////// end Game statemachine



/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/


int32_t BET_p2p_initiate_statemachine(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *dealerInfo=NULL;
	int32_t retval=1,bytes;
	char *rendered=NULL;
	vars->turni=0;
	vars->round=0;
	vars->pot=0;
	vars->last_turn=0;
	vars->last_raise=0;
	for(int i=0;i<bet->maxplayers;i++)
	{
		vars->funds[i]=10000;
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
			vars->betamount[i][j]=0;
		}
	}
	srand(time(0));
	vars->dealer=rand()%bet->maxplayers;
	dealerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(dealerInfo,"method","dealer");
	cJSON_AddNumberToObject(dealerInfo,"playerid",vars->dealer);

	rendered=cJSON_Print(dealerInfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
	{
		retval=-1;
		printf("\n Failed to send data");
		goto end;
	}
	
	end:
		return retval;
}

int32_t BET_p2p_do_blinds(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *small_blind_info=NULL;
	char *rendered=NULL;
	int32_t bytes,retval=1;
	
	small_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(small_blind_info,"method","betting");
	cJSON_AddStringToObject(small_blind_info,"action","small_blind");
	cJSON_AddNumberToObject(small_blind_info,"playerid",vars->turni);

	rendered=cJSON_Print(small_blind_info);
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

int32_t BET_DCV_next_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t flag=0,maxamount=0,retval=-1;
	
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(maxamount<vars->betamount[i][vars->round])
			maxamount=vars->betamount[i][vars->round];
	}
	
	for(int i=((vars->turni+1)%bet->maxplayers);(i != vars->turni);i=((i+1)%bet->maxplayers))
	{
		if((vars->bet_actions[i][vars->round] != fold)&&(vars->bet_actions[i][vars->round] != allin))
		{

			if((vars->bet_actions[i][vars->round] == 0) || (vars->bet_actions[i][vars->round] == small_blind) || 
				(vars->bet_actions[i][vars->round] == big_blind) ||	(((vars->bet_actions[i][vars->round] == check) || (vars->bet_actions[i][vars->round] == call) 
										|| (vars->bet_actions[i][vars->round] == raise)) && (maxamount !=vars->betamount[i][vars->round])) )
			{
				retval=i;
				break;
								
			}

			
		}

	}
	end:
		return retval;
		
}
int32_t BET_DCV_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *roundBetting=NULL,*possibilities=NULL;
	int flag=0,maxamount=0,bytes,retval=1;
	char *rendered=NULL;

	if((retval=BET_DCV_next_turn(argjson,bet,vars)) == -1)
	{
		
		vars->round+=1;
		vars->turni=vars->dealer;
		vars->last_raise=0;
		printf("\nRound:%d is completed",vars->round);

		if(vars->round>=CARDS777_MAXROUNDS)
		{
			retval=BET_evaluate_hand(argjson,bet,vars);
			goto end;
		}
		retval=BET_p2p_dcv_turn(argjson,bet,vars);
		goto end;
	}
	vars->last_turn=vars->turni;
	vars->turni=BET_DCV_next_turn(argjson,bet,vars);

	//vars->turni=(vars->turni+1)%bet->maxplayers;
	
	roundBetting=cJSON_CreateObject();
	cJSON_AddStringToObject(roundBetting,"method","betting");
	cJSON_AddStringToObject(roundBetting,"action","round_betting");
	cJSON_AddNumberToObject(roundBetting,"playerid",vars->turni);
	cJSON_AddNumberToObject(roundBetting,"round",vars->round);
	cJSON_AddItemToObject(roundBetting,"possibilities",possibilities=cJSON_CreateArray());

	
	for(int i=0;i<bet->maxplayers;i++)
	{	
		if(maxamount<vars->betamount[i][vars->round])
		{
			maxamount=vars->betamount[i][vars->round];
		}
	}

	if(maxamount>vars->betamount[vars->turni][vars->round])
	{
		if(maxamount>=vars->funds[vars->turni])
		{
			for(int i=allin;i<=fold;i++)
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
			
		}
		else
		{
			for(int i=raise;i<=fold;i++)
				cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
		}
		
		//raise or fold or call
	}
	else if(maxamount == vars->betamount[vars->turni][vars->round])
	{
		if(maxamount>=vars->funds[vars->turni])
		{	cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(check));
			for(int i=allin;i<=fold;i++)
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
			
		}
		else
		{
			for(int i=check;i<=fold;i++)
			{
				if(i != call)
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
			}
		}
		//raise or fold or call or check
	}

	cJSON_AddNumberToObject(roundBetting,"min_amount",(maxamount-vars->betamount[vars->turni][vars->round]));
	
	rendered=cJSON_Print(roundBetting);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,rendered);
	if(bytes<0)
	{
		retval =-1;
		goto end;
	}
	
	end:
		return retval;
	
}

int32_t BET_DCV_round_betting_response(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,round,bet_amount=0,players_left=0;
	char *action =NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	bet_amount=jint(argjson,"bet_amount");

	vars->betamount[playerid][round]+=bet_amount;
	vars->pot+=bet_amount;
	vars->funds[playerid]-=bet_amount;
	printf("\n%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
	if((action=jstr(argjson,"action")) != NULL)
	{
		if(strcmp(action,"check") == 0)
		{
			vars->bet_actions[playerid][round]=check;
		}
		else if(strcmp(action,"call") == 0)
		{
			vars->bet_actions[playerid][round]=call;
		}
		else if(strcmp(action,"raise") == 0)
		{
			vars->bet_actions[playerid][round]=raise;
		}
		else if(strcmp(action,"allin") == 0)
		{
			for(int i=vars->round;i<CARDS777_MAXROUNDS;i++)
				vars->bet_actions[playerid][i]=allin;
		}
		else if(strcmp(action,"fold") == 0)
		{
			
			for(int i=vars->round;i<CARDS777_MAXROUNDS;i++)
				vars->bet_actions[playerid][i]=fold;
			
		}
		// The below logic is to check if the number of active players < 2
		for(int i=0;i<bet->maxplayers;i++)
		{
			if((vars->bet_actions[i][round]==fold)|| (vars->bet_actions[i][round]==allin))
				players_left++;
		}
		players_left=bet->maxplayers-players_left;
		if(players_left<2)
		{
			for(int i=0;i<bet->maxplayers;i++)
			{
				for(int j=vars->round;j<CARDS777_MAXROUNDS;j++)
				{
					vars->bet_actions[i][j]=check;
				}
			}
		}
		/*
		printf("\nThe players final states are:\n");

		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			for(int i=0;i<bet->maxplayers;i++)
			{
				printf("%d \t",vars->bet_actions[i][j]);
				
			}
			printf("\n");
					
		}*/
	}
	retval=BET_DCV_round_betting(argjson,bet,vars);
	end:
		return retval;
}


int32_t BET_DCV_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,bytes,amount,playerid,round;
	char *rendered=NULL;
	cJSON *display=NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");

	vars->big_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=big_blind;
	vars->betamount[playerid][round]=vars->big_blind;
	vars->pot+=vars->big_blind;
	vars->funds[playerid]-=vars->big_blind;
	
	retval=BET_DCV_round_betting(argjson,bet,vars);
	
	end:
		return retval;
}

int32_t BET_DCV_big_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	
	char *rendered=NULL;
	int32_t retval=1,bytes;
	cJSON *big_blind_info=NULL;

	
	vars->last_turn=vars->turni;
	vars->turni=(vars->turni+1)%bet->maxplayers;
	big_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info,"method","betting");
	cJSON_AddStringToObject(big_blind_info,"action","big_blind");
	cJSON_AddNumberToObject(big_blind_info,"playerid",vars->turni);
	cJSON_AddNumberToObject(big_blind_info,"min_amount",(vars->small_blind*2));
	
	rendered=cJSON_Print(big_blind_info);
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
	
int32_t BET_DCV_small_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,amount,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	

	vars->small_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=small_blind;
	vars->betamount[playerid][round]=vars->small_blind;
	vars->pot+=vars->small_blind;
	vars->funds[playerid]-=vars->small_blind;

	retval=BET_DCV_big_blind(argjson,bet,vars);

	end:
		return retval;
}


int32_t BET_DCV_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *smallBlindInfo=NULL;
	int32_t amount,retval=1,bytes;
	char *rendered=NULL;

	vars->last_turn=vars->dealer;
	vars->turni=(vars->dealer+1)%bet->maxplayers;

	smallBlindInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(smallBlindInfo,"method","betting");
	cJSON_AddStringToObject(smallBlindInfo,"action","small_blind");
	cJSON_AddNumberToObject(smallBlindInfo,"playerid",vars->turni);
	cJSON_AddNumberToObject(smallBlindInfo,"round",vars->round);

	rendered=cJSON_Print(smallBlindInfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
	{
		retval=-1;
		printf("\nFailed to send data");
		goto end;
	}
	
	end:
		return retval;
}
/***************************************************************
Here contains the functions which are common across all the nodes
****************************************************************/

int32_t BET_p2p_betting_statemachine(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	char *action=NULL;
	int retval=1;
		if((action=jstr(argjson,"action")) != 0)
		{
			
			if(strcmp(action,"small_blind") == 0)
			{
				retval=BET_p2p_small_blind(argjson,bet,vars);	
			}
			else if(strcmp(action,"big_blind") == 0)
			{
				retval=BET_p2p_big_blind(argjson,bet,vars);	
			}
			else if(strcmp(action,"small_blind_bet") == 0)
			{
				if(bet->myplayerid == -2)
				{
					BET_relay(argjson,bet,vars);
					retval=BET_DCV_small_blind_bet(argjson,bet,vars);
				}
				else
				{
					BET_player_small_blind_bet(argjson,bet,vars);
				}
			}
			else if(strcmp(action,"big_blind_bet") == 0)
			{
				if(bet->myplayerid == -2)
				{
					BET_relay(argjson,bet,vars);
					retval=BET_DCV_big_blind_bet(argjson,bet,vars);
				}
				else
				{
					retval=BET_player_big_blind_bet(argjson,bet,vars);
				}
				
			}
			else if(strcmp(action,"round_betting") == 0)
			{
				if(bet->myplayerid == jint(argjson,"playerid"))
				{
					retval=BET_player_round_betting(argjson,bet,vars);
				}
			}
			else if((strcmp(action,"check") == 0) || (strcmp(action,"call") == 0) || (strcmp(action,"raise") == 0)
									|| (strcmp(action,"fold") == 0))																					
			{
				if(bet->myplayerid == -2)
					retval=BET_DCV_round_betting_response(argjson,bet,vars);
				else
					retval=BET_player_round_betting_response(argjson,bet,vars);
			}
		}
		printf("\n");
		end:
			return retval;
}

int32_t BET_p2p_display_current_state(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1;
	
	printf("\nsmall_blind:%d",vars->small_blind);
	printf("\nbig_blind:%d",vars->big_blind);

	printf("\nDisplay Actions:\n");

	for(int j=0;j<CARDS777_MAXROUNDS;j++)
	{
		printf("\nRound:%d\n",j);
		for(int i=0;i<bet->maxplayers;i++)
		{
			if(vars->bet_actions[i][j] == small_blind)
			{
				printf("small blind ");
			}
			else if(vars->bet_actions[i][j] == big_blind)
			{
				printf("big blind ");
			}
			else if(vars->bet_actions[i][j] == check)
			{
				printf("raise ");
			}
			else if(vars->bet_actions[i][j] == raise)
			{
				printf("check ");
			}
			else if(vars->bet_actions[i][j] == call)
			{
				printf("call ");
			}
			else if(vars->bet_actions[i][j] == fold)
			{
				printf("fold ");
			}
			printf("%d ",vars->betamount[i][j]);
		}	
	}

	
	
	printf("\n");
	end:
		return retval;
}


/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t BET_player_small_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *small_blind_info=NULL;
	int32_t amount,retval=1,bytes,playerid,round;
	char *rendered=NULL;


	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	
	
	vars->turni=(vars->turni+1)%bet->maxplayers;
	vars->small_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=small_blind;
	vars->betamount[playerid][round]=vars->small_blind;
	vars->pot+=vars->small_blind;

	end:
		return retval;
}

int32_t BET_player_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,bytes,amount,playerid,round;
	char *rendered=NULL;
	cJSON *display=NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");

	vars->turni=(vars->turni+1)%bet->maxplayers;
	vars->big_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=big_blind;
	vars->betamount[playerid][round]=vars->big_blind;
	vars->pot+=vars->big_blind;
	
	end:
		return retval;
}

int32_t BET_p2p_dealer_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,bytes;
	cJSON *dealerReady=NULL;
	char *rendered=NULL;
	
	vars->dealer=jint(argjson,"playerid");
	vars->turni=vars->dealer;
	vars->pot=0;
	vars->player_funds=10000;
	for(int i=0;i<bet->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
		}
	}
	for(int i=0;i<bet->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->betamount[i][j]=0;
		}
	}
	
	if(vars->dealer == bet->myplayerid)
	{
		printf("\n%s:%d::I AM THE DEALER: %d\n",__FUNCTION__,__LINE__,bet->myplayerid);
		dealerReady=cJSON_CreateObject();
		cJSON_AddStringToObject(dealerReady,"method","dealer_ready");
		rendered=cJSON_Print(dealerReady);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		if(bytes<0)
		{
			retval=-1;
			printf("\n Failed to send data");	
			goto end;
		}
	}
	
	end:
		return retval;
}


int32_t BET_p2p_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *small_blind_info=NULL;
	int32_t amount,retval=1,bytes;
	char *rendered=NULL;

	pthread_t pay_t;


	if(jint(argjson,"playerid") == bet->myplayerid)
	{
		small_blind_info=cJSON_CreateObject();
		cJSON_AddStringToObject(small_blind_info,"method","betting");
		do
		{
			printf("\nEnter small blind:");
			scanf("%d",&amount);
		}while((amount<0)||(amount>vars->player_funds));
		vars->player_funds-=amount;

		 BET_p2p_create_invoice_request(argjson,bet,amount);
		 if (OS_thread_create(&pay_t,NULL,(void *)BET_p2p_paymentloop,(void *)bet) != 0 )
		    {
		        exit(-1);
		    }	
			if(pthread_join(pay_t,NULL))
			{
				printf("\nError in joining the main thread for player %d",bet->myplayerid);
			}


			printf("\nThe thread has joined");




		
		cJSON_AddStringToObject(small_blind_info,"action","small_blind_bet");
		cJSON_AddNumberToObject(small_blind_info,"amount",amount);
		vars->betamount[bet->myplayerid][vars->round]=vars->betamount[bet->myplayerid][vars->round]+amount;
		cJSON_AddNumberToObject(small_blind_info,"playerid",jint(argjson,"playerid"));
		cJSON_AddNumberToObject(small_blind_info,"round",jint(argjson,"round"));

		rendered=cJSON_Print(small_blind_info);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		printf("\n%s:%d:%s",__FUNCTION__,__LINE__,rendered);
		if(bytes<0)
		{
				retval=-1;
				printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
				goto end;
		}
				
	}
	end:
		return retval;
}


int32_t BET_p2p_big_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *big_blind_info=NULL;
	int32_t amount,retval=1,bytes;
	char *rendered=NULL;

	if(jint(argjson,"playerid") == bet->myplayerid)
	{
		big_blind_info=cJSON_CreateObject();
		cJSON_AddStringToObject(big_blind_info,"method","betting");
		cJSON_AddStringToObject(big_blind_info,"action","big_blind_bet");

		do
		{
			printf("\nEnter big blind:");
			scanf("%d",&amount);
		}while((amount!=(2*vars->small_blind))||(amount>vars->player_funds));
		vars->player_funds-=amount;
		
		cJSON_AddNumberToObject(big_blind_info,"amount",amount);
		vars->betamount[bet->myplayerid][vars->round]=vars->betamount[bet->myplayerid][vars->round]+amount;
		cJSON_AddNumberToObject(big_blind_info,"playerid",jint(argjson,"playerid"));
		cJSON_AddNumberToObject(big_blind_info,"round",jint(argjson,"round"));

		rendered=cJSON_Print(big_blind_info);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		if(bytes<0)
		{
				retval=-1;
				printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
				goto end;
		}
				
	}
	end:
		return retval;
}

int32_t BET_player_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *roundBetting=NULL,*possibilities=NULL,*action_response=NULL;
	int maxamount=0,bytes,retval=1,playerid,round,min_amount,option,raise_amount=0;
	char *rendered=NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	min_amount=jint(argjson,"min_amount");
	
	action_response=cJSON_CreateObject();
	cJSON_AddStringToObject(action_response,"method","betting");
	cJSON_AddNumberToObject(action_response,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(action_response,"round",jint(argjson,"round"));
	possibilities=cJSON_GetObjectItem(argjson,"possibilities");

	
	printf("\nHere is the possibilities");
	for(int i=0;i<cJSON_GetArraySize(possibilities);i++)
	{
		printf("\n%d %s ",(i+1),action_str[jinti(possibilities,i)]);
		if(call==jinti(possibilities,i))
			printf("%d",min_amount);
	}
	
	do
	{
		printf("\nEnter your option, to chose one::");
		scanf("%d",&option);	
	}while((option<1)||(option>cJSON_GetArraySize(possibilities)));
	

	vars->bet_actions[playerid][round]=jinti(possibilities,(option-1));

	cJSON_AddStringToObject(action_response,"action",action_str[jinti(possibilities,(option-1))]);
	
	if(jinti(possibilities,(option-1))== raise)
	{
		do
		{
			printf("\nEnter the amount > %d:",min_amount);
			scanf("%d",&raise_amount);
						
		}while((raise_amount<min_amount)||(raise_amount<=0)||(raise_amount<(vars->last_raise+min_amount) || (raise_amount>vars->player_funds)));
		vars->player_funds-=raise_amount;
		vars->betamount[playerid][round]+=raise_amount;
		cJSON_AddNumberToObject(action_response,"bet_amount",raise_amount);
	}
	else if(jinti(possibilities,(option-1)) == call)
	{
		vars->betamount[playerid][round]+=min_amount;
		vars->player_funds-=min_amount;
		cJSON_AddNumberToObject(action_response,"bet_amount",min_amount);
	}
	else if(jinti(possibilities,(option-1)) == allin)
	{
		vars->betamount[playerid][round]+=vars->player_funds;
		cJSON_AddNumberToObject(action_response,"bet_amount",vars->player_funds);
		vars->player_funds=0;
	}
	rendered=cJSON_Print(action_response);

	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

	if(bytes<0)
	{
		retval = -1;
		printf("\nFailed to send data");
		goto end;
	}
	

	printf("\n");

	end:
		return retval;
	
}


int32_t BET_player_round_betting_response(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,round,bet_amount=0,maxamount=0,flag1=0,flag2=0;
	char *action =NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	bet_amount=jint(argjson,"bet_amount");

	vars->betamount[playerid][round]+=bet_amount;
	vars->pot+=bet_amount;
	if((action=jstr(argjson,"action")) != NULL)
	{
		if(strcmp(action,"check") == 0)
		{
			vars->bet_actions[playerid][round]=check;
		}
		else if(strcmp(action,"call") == 0)
		{
			vars->bet_actions[playerid][round]=call;
		}
		else if(strcmp(action,"raise") == 0)
		{
			vars->bet_actions[playerid][round]=raise;
		}
		else if(strcmp(action,"fold") == 0)
		{
			vars->bet_actions[playerid][round]=fold;
		}
	}

	end:
		return retval;
}


