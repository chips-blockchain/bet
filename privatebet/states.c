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
#include "client.h"
#include "states.h"

char action_str[8][100]={"","small_blind","big_blind","check","raise","call","allin","fold"};



/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/

int32_t BET_p2p_initiate_statemachine(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *dealerInfo=NULL;
	int32_t retval=1,bytes;
	char *rendered=NULL;
	cJSON *temp=NULL;
	vars->turni=0;
	vars->round=0;
	vars->pot=0;
	vars->last_turn=0;
	vars->last_raise=0;
	for(int i=0;i<bet->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
			vars->betamount[i][j]=0;
		}
	}
	dealerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(dealerInfo,"method","dealer");
	cJSON_AddNumberToObject(dealerInfo,"playerid",vars->dealer);

	temp=cJSON_CreateObject();
	temp=cJSON_Parse(cJSON_Print(dealerInfo));

	
	rendered=cJSON_Print(dealerInfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	if(bytes<0)
	{
		retval=-1;
		printf("\n Failed to send data");
		goto end;
	}

	BET_push_host(temp);
	
	end:
		return retval;
}

int32_t BET_DCV_next_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t maxamount=0,retval=-1,players_left=0;

	for(int i=0;i<bet->maxplayers;i++)
	{
		if(vars->bet_actions[i][vars->round]==fold)
			players_left++;
	}
	players_left=bet->maxplayers-players_left;
	if(players_left<2)
		goto end;
	
	for(int i=0;i<bet->maxplayers;i++)
	{
		if(maxamount<vars->betamount[i][vars->round])
			maxamount=vars->betamount[i][vars->round];
	}
	
	for(int i=((vars->turni+1)%bet->maxplayers);(i != vars->turni);i=((i+1)%bet->maxplayers))
	{
		if((vars->bet_actions[i][vars->round] != fold)&&(vars->bet_actions[i][vars->round] != allin)&&(vars->funds[i] != 0))
		{
			if(vars->bet_actions[i][vars->round] == 0)
			{
				retval=i;
				
				for(int j=0;j<bet->maxplayers;j++)
				{
					if((vars->funds[j] == 0)&&(j != retval))
					{
						retval=-1;
						goto end;
					}
				}
			}
			else if(/*(vars->bet_actions[i][vars->round] == 0) ||*/ (vars->bet_actions[i][vars->round] == small_blind) || 
				(vars->bet_actions[i][vars->round] == big_blind) ||	(((vars->bet_actions[i][vars->round] == check) || (vars->bet_actions[i][vars->round] == call) 
										|| (vars->bet_actions[i][vars->round] == raise)) && (maxamount !=vars->betamount[i][vars->round])) )
			{
				retval=i;
				goto end;
								
			}
		}

	}
	end:
		return retval;
		
}
int32_t BET_DCV_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *roundBetting=NULL,*possibilities=NULL,*actions=NULL,*betAmounts=NULL;
	int maxamount=0,bytes,retval=1,players_left=0,toCall=0,toRaise=0,totalBet=0;
	char *rendered=NULL;

		
	if((retval=BET_DCV_next_turn(argjson,bet,vars)) == -1)
	{
		for(int i=0;i<bet->maxplayers;i++)
		{
			if((vars->bet_actions[i][vars->round]==fold))//|| (vars->bet_actions[i][vars->round]==allin)) 
				players_left++;
		}	
		players_left=bet->maxplayers-players_left;

		vars->round+=1;
		vars->turni=vars->dealer;
		vars->last_raise=0;
		//printf("\nRound:%d is completed",vars->round);

		if((vars->round>=CARDS777_MAXROUNDS) || (players_left<2))
		{
			retval=BET_evaluate_hand_test(argjson,bet,vars);
			goto end;
		}
		
		retval=BET_p2p_dcv_turn(argjson,bet,vars);
		goto end;
	}

	printf("%s::%d::This is the fold and allout scenario\n",__FUNCTION__,__LINE__);
	for(int i=0;i<bet->maxplayers;i++)
	{
		printf("%s::%d::player id::%d::funds::%d::vars->round::%d\n",__FUNCTION__,__LINE__,i,vars->funds[i],vars->round);
		for(int j=0;j<=vars->round;j++)
			printf("%d\t",vars->betamount[i][j]);
		printf("\n");
	}
	
	players_left=0;
	for(int i=0;i<bet->maxplayers;i++)
	{
			if((vars->bet_actions[i][vars->round]==fold))//|| (vars->bet_actions[i][vars->round]==allin)) 
				players_left++;
	}
	players_left=bet->maxplayers-players_left;
	if(players_left<2)
	{
		retval=BET_evaluate_hand_test(argjson,bet,vars);
		goto end;
	}
	
	vars->last_turn=vars->turni;
	vars->turni=BET_DCV_next_turn(argjson,bet,vars);

	
	roundBetting=cJSON_CreateObject();
	cJSON_AddStringToObject(roundBetting,"method","betting");
	cJSON_AddStringToObject(roundBetting,"action","round_betting");
	cJSON_AddNumberToObject(roundBetting,"playerid",vars->turni);
	cJSON_AddNumberToObject(roundBetting,"round",vars->round);
	cJSON_AddNumberToObject(roundBetting,"pot",vars->pot);
	/* */
	cJSON_AddItemToObject(roundBetting,"betAmounts",betAmounts=cJSON_CreateArray());
	for(int j=0;j<bet->maxplayers;j++)
	{
		totalBet=0;
		for(int i=0;i<=vars->round;i++)
		{
			totalBet+=vars->betamount[j][i];
		}
		cJSON_AddItemToArray(betAmounts,cJSON_CreateNumber(totalBet));
	}
	
	cJSON_AddItemToObject(roundBetting,"actions",actions=cJSON_CreateArray());
	for(int i=0;i<=vars->round;i++)
	{
		for(int j=0;j<bet->maxplayers;j++)
		{
			if(vars->bet_actions[j][i]>0)
				cJSON_AddItemToArray(actions,cJSON_CreateNumber(vars->bet_actions[j][i]));
		}
	}
	
	cJSON_AddItemToObject(roundBetting,"possibilities",possibilities=cJSON_CreateArray());

	if(vars->betamount[vars->last_turn][vars->round] == vars->betamount[vars->turni][vars->round])
	{
		// check, allin, fold
		if(vars->bet_actions[vars->turni][vars->round]==big_blind)
			toCall=vars->betamount[vars->turni][vars->round];
		
	}
	else
	{
		// raise, call, allin, fold
		toCall=vars->betamount[vars->last_turn][vars->round];
	}

	
	if(vars->last_raise<big_blind_amount)
		toRaise=big_blind_amount;
	else
		toRaise=vars->last_raise;

	toRaise+=toCall;
	
	
	cJSON_AddNumberToObject(roundBetting,"toCall",toCall);
	cJSON_AddNumberToObject(roundBetting,"minRaiseTo",toRaise);
	
	
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
			{
				if(i==allin)
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(call));
				else
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
			}		
			
		}
		else
		{
			int allin_flag=1;
			for(int i=raise;i<=fold;i++)
			{
				if(i==raise)
				{
					for(int j=0;j<bet->maxplayers;j++)
					{
						if((j != vars->turni)&&(vars->funds[i] != 0))
						{
							cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
							break;
						}
					}
				}
				else if(i==call)
				{
					int totatBet=0;
					for(int j=0;j<=vars->round;j++)
						totatBet+=vars->betamount[vars->turni][j];
					if(toCall==(totalBet+vars->funds[vars->turni]))
					{
						allin_flag=0;
					}
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));
				}
				else if(i==allin)
				{
					if(allin_flag==1)
						cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));	
				}
				else 
					cJSON_AddItemToArray(possibilities,cJSON_CreateNumber(i));						
				
			}
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
	int retval=1,playerid,round,bet_amount=0,players_left=0,min_amount=0;
	char *action =NULL;
	cJSON *playerFunds=NULL;
	
	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	bet_amount=jint(argjson,"invoice_amount");
	min_amount=jint(argjson,"min_amount");

	vars->betamount[playerid][round]+=bet_amount;
	vars->pot+=bet_amount;
	vars->funds[playerid]-=bet_amount;
	
	cJSON_AddNumberToObject(argjson,"pot",vars->pot);
	cJSON_AddItemToObject(argjson,"player_funds",playerFunds=cJSON_CreateArray());
	for(int i=0;i<bet->maxplayers;i++)
	{
		cJSON_AddItemToArray(playerFunds,cJSON_CreateNumber(vars->funds[i]));

	}		

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
			vars->last_raise=bet_amount-min_amount;
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

		for(int i=0;i<bet->maxplayers;i++)
		{
			if(vars->bet_actions[i][round]==fold)
				players_left++;
		}
		players_left=bet->maxplayers-players_left;
		if(players_left<2)
		{
			for(int i=0;i<bet->maxplayers;i++)
			{
				for(int j=vars->round;j<CARDS777_MAXROUNDS;j++)
				{
					vars->bet_actions[i][j]=vars->bet_actions[i][round]; //check
				}
			}
		}
	}
	retval=BET_DCV_round_betting(argjson,bet,vars);

	return retval;
}


int32_t BET_DCV_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");

	vars->big_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=big_blind;
	vars->betamount[playerid][round]=vars->big_blind;
	vars->pot+=vars->big_blind;
	vars->funds[playerid]-=vars->big_blind;
	
	retval=BET_DCV_round_betting(argjson,bet,vars);
	
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
	cJSON_AddNumberToObject(big_blind_info,"pot",vars->pot);
	
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
	int32_t retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	

	vars->small_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=small_blind;
	vars->betamount[playerid][round]=vars->small_blind;
	vars->pot+=vars->small_blind;
	vars->funds[playerid]-=vars->small_blind;

	retval=BET_DCV_big_blind(argjson,bet,vars);

	return retval;
}


int32_t BET_DCV_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *smallBlindInfo=NULL;
	int32_t retval=1,bytes;
	char *rendered=NULL;

	vars->last_turn=vars->dealer;
	vars->turni=(vars->dealer)%bet->maxplayers;//vars->dealer+1 is removed since dealer is the one who does small_blind

	smallBlindInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(smallBlindInfo,"method","betting");
	cJSON_AddStringToObject(smallBlindInfo,"action","small_blind");
	cJSON_AddNumberToObject(smallBlindInfo,"playerid",vars->turni);
	cJSON_AddNumberToObject(smallBlindInfo,"round",vars->round);
	cJSON_AddNumberToObject(smallBlindInfo,"pot",vars->pot);
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
	char *rendered=NULL;
	int retval=1,bytes;
		if((action=jstr(argjson,"action")) != 0)
		{
			
			if(strcmp(action,"small_blind") == 0)
			{
				if(jint(argjson,"playerid") == bet->myplayerid)
				{
					display_cards(argjson,bet,vars);
					retval=BET_p2p_small_blind(argjson,bet,vars);	
				}
			}
			else if(strcmp(action,"big_blind") == 0)
			{
				if(jint(argjson,"playerid") == bet->myplayerid)
				{
					display_cards(argjson,bet,vars);
					retval=BET_p2p_big_blind(argjson,bet,vars); 
				}
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
					display_cards(argjson,bet,vars);
					cJSON *player_funds=NULL;
					cJSON_AddItemToObject(argjson,"player_funds",player_funds=cJSON_CreateArray());

					
					for(int i=0;i<bet->maxplayers;i++)
					{
						int totalBet=0;
						for(int j=0;j<=jint(argjson,"round");j++)  //for(int j=0;j<=vars->round;j++)
						{
							totalBet+=vars->betamount[i][j];
						}
						cJSON_AddItemToArray(player_funds,cJSON_CreateNumber(vars->funds[i]-totalBet));
					}	
					player_lws_write(argjson);
				}
				else
				{
					player_lws_write(argjson);
					display_cards(argjson,bet,vars);
				}
			}
			else if((strcmp(action,"check") == 0) || (strcmp(action,"call") == 0) || (strcmp(action,"raise") == 0)
									|| (strcmp(action,"fold") == 0) || (strcmp(action,"allin") == 0))																					
			{
				if(bet->myplayerid == -2)
				{
					rendered=cJSON_Print(argjson);
					bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
					if(bytes<0)
					{
						retval=-1;
						goto end;
					}
					retval=BET_DCV_round_betting_response(argjson,bet,vars);
				}	
				else
				{	
					retval=BET_player_round_betting_response(argjson,bet,vars);
				}	
			}
		}
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

	return retval;
}


/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t BET_player_small_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,playerid,round;

	
	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	
	
	vars->turni=(vars->turni+1)%bet->maxplayers;
	vars->small_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=small_blind;
	vars->betamount[playerid][round]=vars->small_blind;
	vars->pot+=vars->small_blind;

	return retval;
}

int32_t BET_player_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");

	vars->turni=(vars->turni+1)%bet->maxplayers;
	vars->big_blind=jint(argjson,"amount");
	vars->bet_actions[playerid][round]=big_blind;
	vars->betamount[playerid][round]=vars->big_blind;
	vars->pot+=vars->big_blind;

	return retval;
}

int32_t BET_p2p_dealer_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=0,bytes;
	cJSON *dealerReady=NULL;
	char *rendered=NULL;
	
	vars->dealer=jint(argjson,"playerid");
	vars->turni=vars->dealer;
	vars->pot=0;

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
	BET_push_client(argjson);
	end:
		return retval;
}


int32_t BET_p2p_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *small_blind_info=NULL;
	int32_t amount,retval=1,bytes;
	char *rendered=NULL;
	cJSON *temp=NULL;

	small_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(small_blind_info,"method","betting");
	amount=small_blind_amount;
	vars->player_funds-=amount;
	retval=BET_player_invoice_pay(argjson,bet,vars,amount);
	
	if(retval<0)
		goto end;
	
	cJSON_AddStringToObject(small_blind_info,"action","small_blind_bet");
	cJSON_AddNumberToObject(small_blind_info,"amount",amount);
	vars->betamount[bet->myplayerid][vars->round]=vars->betamount[bet->myplayerid][vars->round]+amount;
	cJSON_AddNumberToObject(small_blind_info,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(small_blind_info,"round",jint(argjson,"round"));

	temp=cJSON_CreateObject();
	temp=cJSON_Parse(cJSON_Print(small_blind_info));
		
	
	rendered=cJSON_Print(small_blind_info);
	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
	if(bytes<0)
	{
		retval=-1;
		printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
		goto end;
	}
	player_lws_write(temp);
				
	end:
		return retval;
}


int32_t BET_p2p_big_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *big_blind_info=NULL,*temp=NULL;
	int32_t amount,retval=1,bytes;
	char *rendered=NULL;

	big_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info,"method","betting");
	cJSON_AddStringToObject(big_blind_info,"action","big_blind_bet");

	amount=big_blind_amount;
	vars->player_funds-=amount;

	retval=BET_player_invoice_pay(argjson,bet,vars,amount);
	if(retval<0)
		goto end;
	
	cJSON_AddNumberToObject(big_blind_info,"amount",amount);
	vars->betamount[bet->myplayerid][vars->round]=vars->betamount[bet->myplayerid][vars->round]+amount;
	cJSON_AddNumberToObject(big_blind_info,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(big_blind_info,"round",jint(argjson,"round"));

	temp=cJSON_CreateObject();
	temp=cJSON_Parse(cJSON_Print(big_blind_info));
	rendered=cJSON_Print(big_blind_info);
	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
	if(bytes<0)
	{
			retval=-1;
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
			goto end;
	}
	player_lws_write(temp);
		
	end:
		return retval;
}



int32_t BET_player_round_betting_test(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *possibilities=NULL,*action_response=NULL;
	int bytes,retval=1,playerid,round,min_amount,option,raise_amount=0,invoice_amount=0;
	char *rendered=NULL;
	
	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	min_amount=jint(argjson,"min_amount");
	
	action_response=cJSON_CreateObject();
	cJSON_AddStringToObject(action_response,"method","betting");
	cJSON_AddNumberToObject(action_response,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(action_response,"round",jint(argjson,"round"));
	cJSON_AddNumberToObject(action_response,"min_amount",min_amount);
	possibilities=cJSON_GetObjectItem(argjson,"possibilities");

	
	option=1;
	vars->bet_actions[playerid][round]=jinti(possibilities,(option-1));

	cJSON_AddStringToObject(action_response,"action",action_str[jinti(possibilities,(option-1))]);
	
	
	if(jinti(possibilities,(option-1))== raise)
	{
		raise_amount=jint(argjson,"bet_amount");
		invoice_amount=raise_amount-vars->betamount[playerid][round];
		
		vars->betamount[playerid][round]+=invoice_amount;
		vars->player_funds-=invoice_amount;


		if(vars->player_funds==0)
		{
			cJSON_DetachItemFromObject(action_response,"action");
			cJSON_AddStringToObject(action_response,"action","allin");
			
		}
		cJSON_AddNumberToObject(action_response,"bet_amount",jint(argjson,"bet_amount"));
		
		cJSON_AddNumberToObject(action_response,"invoice_amount",invoice_amount);

		
		retval=BET_player_create_betting_invoice_request(argjson,action_response,bet,invoice_amount);
		if(retval<0)
			goto end;
		
		
	}
	else if(jinti(possibilities,(option-1)) == call)
	{
		vars->betamount[playerid][round]+=min_amount;
		vars->player_funds-=min_amount;

		if(vars->player_funds==0)
		{
			cJSON_DetachItemFromObject(action_response,"action");
			cJSON_AddStringToObject(action_response,"action","allin");
			printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(action_response));
			
		}
		
		cJSON_AddNumberToObject(action_response,"bet_amount",jint(argjson,"bet_amount"));

		
		cJSON_AddNumberToObject(action_response,"invoice_amount",min_amount);
				
		retval=BET_player_create_betting_invoice_request(argjson,action_response,bet,min_amount);
		if(retval<0)
		{
			goto end;
		}	
		

	}
	else if(jinti(possibilities,(option-1)) == allin)
	{
		vars->betamount[playerid][round]+=vars->player_funds;
		cJSON_AddNumberToObject(action_response,"bet_amount",jint(argjson,"bet_amount"));
		
		cJSON_AddNumberToObject(action_response,"invoice_amount",vars->player_funds);
		
		vars->player_funds=0;

				
		retval=BET_player_create_betting_invoice_request(argjson,action_response,bet,vars->player_funds);
		if(retval<0)
			goto end;	
		
	}
	else
	{
		printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(action_response));
		rendered=cJSON_Print(action_response);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

		if(bytes<0)
		{
			retval = -1;
			printf("\nFailed to send data");
			goto end;
		}
	}
	end:
		return retval;
	
}



int32_t BET_player_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *possibilities=NULL,*action_response=NULL;
	int bytes,retval=1,playerid,round,min_amount,option,raise_amount=0;
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
			if(min_amount<big_blind_amount)
				min_amount=big_blind_amount;
			printf("\nEnter the amount > %d:",min_amount);
			scanf("%d",&raise_amount);
						
		}while((raise_amount<min_amount)||(raise_amount<big_blind_amount)||(raise_amount<(vars->last_raise+min_amount) || (raise_amount>vars->player_funds)));
		vars->player_funds-=raise_amount;
		vars->betamount[playerid][round]+=raise_amount;

		retval=BET_player_invoice_pay(argjson,bet,vars,raise_amount);
		if(retval<0)
			goto end;
		
		cJSON_AddNumberToObject(action_response,"bet_amount",raise_amount);
	}
	else if(jinti(possibilities,(option-1)) == call)
	{
		vars->betamount[playerid][round]+=min_amount;
		vars->player_funds-=min_amount;

		retval=BET_player_invoice_pay(argjson,bet,vars,min_amount);
		if(retval<0)
			goto end;
		
		cJSON_AddNumberToObject(action_response,"bet_amount",min_amount);
	}
	else if(jinti(possibilities,(option-1)) == allin)
	{
		vars->betamount[playerid][round]+=vars->player_funds;

		retval=BET_player_invoice_pay(argjson,bet,vars,vars->player_funds);
		if(retval<0)
			goto end;	
		
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
	end:
		return retval;
	
}


int32_t BET_player_round_betting_response(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval=1,playerid,round,invoice_amount=0;
	char *action =NULL;
	cJSON *playerFunds=NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	invoice_amount=jint(argjson,"invoice_amount");
	
	if(bet->myplayerid!=playerid)
		vars->betamount[playerid][round]+=invoice_amount;

	vars->pot+=invoice_amount;

	cJSON_AddNumberToObject(argjson,"pot",vars->pot);
	cJSON_AddItemToObject(argjson,"player_funds",playerFunds=cJSON_CreateArray());
	for(int i=0;i<bet->maxplayers;i++)
	{
		int totalBet=0;
		for(int j=0;j<=vars->round;j++)
		{
			totalBet+=vars->betamount[i][j];
		}
		cJSON_AddItemToArray(playerFunds,cJSON_CreateNumber(vars->funds[i]-totalBet));
	}	
	
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
		else if(strcmp(action,"allin") == 0)
		{
			vars->bet_actions[playerid][round]=allin;
		}
	}
    BET_push_client(argjson);

	return retval;
}

/***************************************************************
Here contains the functions which are specific to REST calls
****************************************************************/
int32_t BET_rest_initiate_statemachine(struct lws *wsi, cJSON *argjson)
{
	cJSON *dealerInfo=NULL;
	int32_t retval=1;

	DCV_VARS->turni=0;
	DCV_VARS->round=0;
	DCV_VARS->pot=0;
	DCV_VARS->last_turn=0;
	DCV_VARS->last_raise=0;
	for(int i=0;i<BET_dcv->maxplayers;i++)
	{
		DCV_VARS->funds[i]=0;// hardcoded max funds to 10000 satoshis
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			DCV_VARS->bet_actions[i][j]=0;
			DCV_VARS->betamount[i][j]=0;
		}
	}
	srand(time(0));
	DCV_VARS->dealer=rand()%BET_dcv->maxplayers;
	dealerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(dealerInfo,"method","dealer");
	cJSON_AddNumberToObject(dealerInfo,"playerid",DCV_VARS->dealer);

	lws_write(wsi,cJSON_Print(dealerInfo),strlen(cJSON_Print(dealerInfo)),0);

	return retval;
}


int32_t BET_rest_bvv_dealer_info(struct lws *wsi, cJSON *argjson)
{
	int retval=1;

	BVV_VARS=calloc(1,sizeof(struct privatebet_vars));
	BVV_VARS->dealer=jint(argjson,"playerid");
	BVV_VARS->turni=BVV_VARS->dealer;
	BVV_VARS->pot=0;
	BVV_VARS->player_funds=10000000; // hardcoded to 10000 satoshis
	for(int i=0;i<BET_bvv->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			BVV_VARS->bet_actions[i][j]=0;
		}
	}
	for(int i=0;i<BET_bvv->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			BVV_VARS->betamount[i][j]=0;
		}
	}
	return retval;
}


int32_t BET_rest_player_dealer_info(struct lws *wsi, cJSON *argjson,int32_t playerID)
{
	int retval=1;
	cJSON *dealerReady=NULL;

	Player_VARS[playerID]=calloc(1,sizeof(struct privatebet_vars));
	
	Player_VARS[playerID]->dealer=jint(argjson,"playerid");
	Player_VARS[playerID]->turni=Player_VARS[playerID]->dealer;
	Player_VARS[playerID]->pot=0;
	Player_VARS[playerID]->player_funds=10000000; // hardcoded to 10000 satoshis
	for(int i=0;i<BET_player[playerID]->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			Player_VARS[playerID]->bet_actions[i][j]=0;
			Player_VARS[playerID]->betamount[i][j]=0;
		}
	}

	
	all_no_of_shares[playerID]=0;
	all_no_of_player_cards[playerID]=0;
	for(int i=0;i<BET_player[playerID]->range;i++)
	{
		for(int j=0;j<BET_player[playerID]->numplayers;j++)
		{
			all_sharesflag[playerID][i][j]=0;
		}
	}
	for(int i=0;i<hand_size;i++)
	{
		all_player_card_matrix[playerID][i]=0;
		all_player_card_values[playerID][i]=-1;
	}
	
	if(Player_VARS[playerID]->dealer == BET_player[playerID]->myplayerid)
	{
		dealerReady=cJSON_CreateObject();
		cJSON_AddStringToObject(dealerReady,"method","dealer_ready");
		lws_write(wsi,cJSON_Print(dealerReady),strlen(cJSON_Print(dealerReady)),0);
	}
	
	return retval;
}


int32_t BET_rest_DCV_next_turn(struct lws *wsi)
{
	int32_t maxamount=0,retval=-1,players_left=0;
	int32_t dcv_maxplayers=2;
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if((DCV_VARS->bet_actions[i][DCV_VARS->round]==fold)) /*|| (vars->bet_actions[i][vars->round]==allin)*/
			players_left++;
	}
	players_left=dcv_maxplayers-players_left;
	if(players_left<2)
		goto end;
	
	for(int i=0;i<dcv_maxplayers;i++)
	{
		if(maxamount<DCV_VARS->betamount[i][DCV_VARS->round])
			maxamount=DCV_VARS->betamount[i][DCV_VARS->round];
	}
	
	for(int i=((DCV_VARS->turni+1)%dcv_maxplayers);(i != DCV_VARS->turni);i=((i+1)%dcv_maxplayers))
	{
		if((DCV_VARS->bet_actions[i][DCV_VARS->round] != fold)&&(DCV_VARS->bet_actions[i][DCV_VARS->round] != allin))
		{

			if((DCV_VARS->bet_actions[i][DCV_VARS->round] == 0) || (DCV_VARS->bet_actions[i][DCV_VARS->round] == small_blind) || 
				(DCV_VARS->bet_actions[i][DCV_VARS->round] == big_blind) ||	(((DCV_VARS->bet_actions[i][DCV_VARS->round] == check) || (DCV_VARS->bet_actions[i][DCV_VARS->round] == call) 
										|| (DCV_VARS->bet_actions[i][DCV_VARS->round] == raise)) && (maxamount !=DCV_VARS->betamount[i][DCV_VARS->round])) )
			{
				retval=i;
				break;
								
			}

			
		}

	}
	end:
		return retval;
		
}


int32_t BET_rest_DCV_small_blind(struct lws *wsi)
{
	cJSON *smallBlindInfo=NULL;
	int32_t retval=1;


	DCV_VARS->last_turn=DCV_VARS->dealer;
	DCV_VARS->turni=(DCV_VARS->dealer+1)%(BET_dcv->maxplayers);

	smallBlindInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(smallBlindInfo,"method","betting");
	cJSON_AddStringToObject(smallBlindInfo,"action","small_blind");
	cJSON_AddNumberToObject(smallBlindInfo,"playerid",DCV_VARS->turni);
	cJSON_AddNumberToObject(smallBlindInfo,"round",DCV_VARS->round);
	cJSON_AddNumberToObject(smallBlindInfo,"pot",DCV_VARS->pot);


	printf("\nsent:%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(smallBlindInfo));
	lws_write(wsi,cJSON_Print(smallBlindInfo),strlen(cJSON_Print(smallBlindInfo)),0);
	
	return retval;
}


int32_t BET_rest_DCV_round_betting(struct lws *wsi)
{
	cJSON *roundBetting=NULL,*possibilities=NULL,*actions=NULL;
	int maxamount=0,retval=1,players_left=0;
	int32_t dcv_maxplayers=2;

	if((retval=BET_rest_DCV_next_turn(wsi)) == -1)
	{
		for(int i=0;i<dcv_maxplayers;i++)
		{
			if((DCV_VARS->bet_actions[i][DCV_VARS->round]==fold)|| (DCV_VARS->bet_actions[i][DCV_VARS->round]==allin)) 
				players_left++;
		}	
		players_left=dcv_maxplayers-players_left;

		DCV_VARS->round+=1;
		DCV_VARS->turni=DCV_VARS->dealer;
		DCV_VARS->last_raise=0;
		//printf("\nRound:%d is completed",vars->round);

		if((DCV_VARS->round>=CARDS777_MAXROUNDS) || (players_left<2))
		{
			retval=BET_rest_evaluate_hand(wsi);
			goto end;
		}
		retval=BET_rest_dcv_turn(wsi);
		goto end;
	}
	DCV_VARS->last_turn=DCV_VARS->turni;
	DCV_VARS->turni=BET_rest_DCV_next_turn(wsi);

	roundBetting=cJSON_CreateObject();
	cJSON_AddStringToObject(roundBetting,"method","betting");
	cJSON_AddStringToObject(roundBetting,"action","round_betting");
	cJSON_AddNumberToObject(roundBetting,"playerid",DCV_VARS->turni);
	cJSON_AddNumberToObject(roundBetting,"round",DCV_VARS->round);
	cJSON_AddNumberToObject(roundBetting,"pot",DCV_VARS->pot);

	cJSON_AddItemToObject(roundBetting,"actions",actions=cJSON_CreateArray());
	for(int i=0;i<=DCV_VARS->round;i++)
	{
		for(int j=0;j<dcv_maxplayers;j++)
		{
			if(DCV_VARS->bet_actions[j][i]>0)
				cJSON_AddItemToArray(actions,cJSON_CreateNumber(DCV_VARS->bet_actions[j][i]));
		}
	}
	
	cJSON_AddItemToObject(roundBetting,"possibilities",possibilities=cJSON_CreateArray());


	printf("\nBET Actions\n");
	for(int i=0;i<dcv_maxplayers;i++)
	{
		printf("\n");
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			printf("%d\t",DCV_VARS->bet_actions[i][j]);
		}
	}

	printf("\nBET Amounts\n");
	for(int i=0;i<dcv_maxplayers;i++)
	{
		
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			printf("%d\t",DCV_VARS->betamount[i][j]);
		}
		printf("\n");
	}
	
	for(int i=0;i<dcv_maxplayers;i++)
	{	
		if(maxamount<DCV_VARS->betamount[i][DCV_VARS->round])
		{
			maxamount=DCV_VARS->betamount[i][DCV_VARS->round];
		}
	}

	if(maxamount>DCV_VARS->betamount[DCV_VARS->turni][DCV_VARS->round])
	{
		if(maxamount>=DCV_VARS->funds[DCV_VARS->turni])
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
	else if(maxamount == DCV_VARS->betamount[DCV_VARS->turni][DCV_VARS->round])
	{
		if(maxamount>=DCV_VARS->funds[DCV_VARS->turni])
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

	cJSON_AddNumberToObject(roundBetting,"min_amount",(maxamount-DCV_VARS->betamount[DCV_VARS->turni][DCV_VARS->round]));

	lws_write(wsi,cJSON_Print(roundBetting),strlen(cJSON_Print(roundBetting)),0);
	
	end:
		return retval;
	
}


int32_t BET_rest_big_blind_update(struct lws *wsi,cJSON *argjson,int32_t amount)
{
	cJSON *big_blind_info=NULL;
	int32_t retval=1;
	int32_t this_playerID;
	
	this_playerID=jint(argjson,"gui_playerID");

	Player_VARS[this_playerID]->player_funds-=amount;
	Player_VARS[this_playerID]->betamount[BET_player[this_playerID]->myplayerid][Player_VARS[this_playerID]->round]=Player_VARS[this_playerID]->betamount[BET_player[this_playerID]->myplayerid][Player_VARS[this_playerID]->round]+amount;

	big_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info,"method","betting");
	cJSON_AddStringToObject(big_blind_info,"action","big_blind_bet");
	cJSON_AddNumberToObject(big_blind_info,"amount",amount);
	cJSON_AddNumberToObject(big_blind_info,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(big_blind_info,"round",jint(argjson,"round"));

	lws_write(wsi,cJSON_Print(big_blind_info),strlen(cJSON_Print(big_blind_info)),0);
		
	return retval;
}



int32_t BET_rest_big_blind(struct lws *wsi,cJSON *argjson)
{
	int32_t amount,retval=1;

	amount=big_blind_amount;
	retval=BET_rest_player_create_invoice_request(wsi,argjson,amount);

	return retval;
}




int32_t BET_rest_small_blind_update(struct lws *wsi,cJSON *argjson,int32_t amount)
{
	cJSON *small_blind_info=NULL;
	int32_t retval=1;
	int32_t this_playerID;

	this_playerID=jint(argjson,"gui_playerID");

	Player_VARS[this_playerID]->player_funds-=amount;
	Player_VARS[this_playerID]->betamount[BET_player[this_playerID]->myplayerid][Player_VARS[this_playerID]->round]=Player_VARS[this_playerID]->betamount[BET_player[this_playerID]->myplayerid][Player_VARS[this_playerID]->round]+amount;
	small_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(small_blind_info,"method","betting");
	cJSON_AddStringToObject(small_blind_info,"action","small_blind_bet");
	cJSON_AddNumberToObject(small_blind_info,"amount",amount);
	cJSON_AddNumberToObject(small_blind_info,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(small_blind_info,"round",jint(argjson,"round"));
	
	lws_write(wsi,cJSON_Print(small_blind_info),strlen(cJSON_Print(small_blind_info)),0);				

	return retval;
}


int32_t BET_rest_small_blind(struct lws *wsi,cJSON *argjson)
{
	int32_t amount,retval=1;

	amount=small_blind_amount;
	retval=BET_rest_player_create_invoice_request(wsi,argjson,amount);
	
	return retval;
}


int32_t BET_rest_DCV_big_blind(struct lws *wsi,cJSON *argjson)
{
	int32_t retval=1;
	cJSON *big_blind_info=NULL;

	DCV_VARS->last_turn=DCV_VARS->turni;
	DCV_VARS->turni=(DCV_VARS->turni+1)%(BET_dcv->maxplayers);
	big_blind_info=cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info,"method","betting");
	cJSON_AddStringToObject(big_blind_info,"action","big_blind");
	cJSON_AddNumberToObject(big_blind_info,"playerid",DCV_VARS->turni);
	cJSON_AddNumberToObject(big_blind_info,"min_amount",(DCV_VARS->small_blind*2));
	cJSON_AddNumberToObject(big_blind_info,"pot",DCV_VARS->pot);

	lws_write(wsi,cJSON_Print(big_blind_info),strlen(cJSON_Print(big_blind_info)),0);
	
	return retval;
		
}

	
int32_t BET_rest_DCV_small_blind_bet(struct lws *wsi,cJSON *argjson)
{
	int32_t retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
		
	DCV_VARS->small_blind=jint(argjson,"amount");
	DCV_VARS->bet_actions[playerid][round]=small_blind;
	DCV_VARS->betamount[playerid][round]=DCV_VARS->small_blind;
	DCV_VARS->pot+=DCV_VARS->small_blind;
	DCV_VARS->funds[playerid]-=DCV_VARS->small_blind;

	retval=BET_rest_DCV_big_blind(wsi,argjson);

	return retval;
}



int32_t BET_rest_DCV_big_blind_bet(struct lws *wsi,cJSON *argjson)
{
	int retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	
	DCV_VARS->big_blind=jint(argjson,"amount");
	DCV_VARS->bet_actions[playerid][round]=big_blind;
	DCV_VARS->betamount[playerid][round]=DCV_VARS->big_blind;
	DCV_VARS->pot+=DCV_VARS->big_blind;
	DCV_VARS->funds[playerid]-=DCV_VARS->big_blind;
	
	retval=BET_rest_DCV_round_betting(wsi);
	return retval;
}

int32_t BET_rest_player_small_blind_bet(struct lws *wsi,cJSON *argjson,int32_t this_playerID)
{
	int32_t retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	Player_VARS[this_playerID]->turni=(Player_VARS[this_playerID]->turni+1)%BET_player[this_playerID]->maxplayers;
	Player_VARS[this_playerID]->small_blind=jint(argjson,"amount");
	Player_VARS[this_playerID]->bet_actions[playerid][round]=small_blind;
	Player_VARS[this_playerID]->betamount[playerid][round]=Player_VARS[this_playerID]->small_blind;
	Player_VARS[this_playerID]->pot+=Player_VARS[this_playerID]->small_blind;

	return retval;
}


int32_t BET_rest_player_big_blind_bet(struct lws *wsi,cJSON *argjson,int32_t this_playerID)
{
	int retval=1,playerid,round;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	
	Player_VARS[this_playerID]->turni=(Player_VARS[this_playerID]->turni+1)%BET_player[this_playerID]->maxplayers;
	Player_VARS[this_playerID]->big_blind=jint(argjson,"amount");
	Player_VARS[this_playerID]->bet_actions[playerid][round]=big_blind;
	Player_VARS[this_playerID]->betamount[playerid][round]=Player_VARS[this_playerID]->big_blind;
	Player_VARS[this_playerID]->pot+=Player_VARS[this_playerID]->big_blind;

	return retval;
}




int32_t BET_rest_player_round_betting_update(struct lws *wsi,cJSON *argjson,int option,int32_t bet_amount)
{
	cJSON *possibilities=NULL,*action_response=NULL;
	int retval=1,playerid,round; 
	int32_t this_playerID;
	

	this_playerID=jint(argjson,"gui_playerID");
	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	
	action_response=cJSON_CreateObject();
	cJSON_AddStringToObject(action_response,"method","betting");

	cJSON_AddNumberToObject(action_response,"playerid",jint(argjson,"playerid"));
	cJSON_AddNumberToObject(action_response,"round",jint(argjson,"round"));

	possibilities=cJSON_CreateObject();
	possibilities=cJSON_GetObjectItem(argjson,"possibilities");
	
	Player_VARS[this_playerID]->bet_actions[playerid][round]=jinti(possibilities,(option-1));

	cJSON_AddStringToObject(action_response,"action",action_str[jinti(possibilities,(option-1))]);

	if((jinti(possibilities,(option-1))== raise) || (jinti(possibilities,(option-1))== call) || (jinti(possibilities,(option-1))== allin))
	{
		Player_VARS[this_playerID]->player_funds-=bet_amount;
		Player_VARS[this_playerID]->betamount[playerid][round]+=bet_amount;
		
		cJSON_AddNumberToObject(action_response,"bet_amount",bet_amount);
	}
	lws_write(wsi,cJSON_Print(action_response),strlen(cJSON_Print(action_response)),0);
	return retval;
	
}


int32_t BET_rest_player_round_betting(struct lws *wsi,cJSON *argjson)
{
	cJSON *possibilities=NULL;
	int retval=1,min_amount,option,bet_amount=0;
	int32_t this_playerID=jint(argjson,"gui_playerID");

	min_amount=jint(argjson,"min_amount");

	possibilities=cJSON_GetObjectItem(argjson,"possibilities");
	option=1;
	if(jinti(possibilities,(option-1))== raise)
	{
		bet_amount=jint(argjson,"bet_amount");
		BET_rest_player_create_invoice_request_round(wsi,argjson,bet_amount,option);
	}
	else if(jinti(possibilities,(option-1)) == call)
	{
		bet_amount=min_amount;
		BET_rest_player_create_invoice_request_round(wsi,argjson,bet_amount,option);
	}
	else if(jinti(possibilities,(option-1)) == allin)
	{
		bet_amount=Player_VARS[this_playerID]->player_funds;
		BET_rest_player_create_invoice_request_round(wsi,argjson,bet_amount,option);
	}
	
	else if((jinti(possibilities,(option-1))== check) || (jinti(possibilities,(option-1))== fold))
	{
		BET_rest_player_round_betting_update(wsi,argjson,option,0);
	}
	return retval;
	
}

int32_t BET_rest_DCV_round_betting_response(struct lws *wsi,cJSON *argjson)
{
	int retval=1,playerid,round,bet_amount=0,players_left=0;
	char *action =NULL;
	int32_t dcv_maxplayers=2;

	
	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	bet_amount=jint(argjson,"bet_amount");

	DCV_VARS->betamount[playerid][round]+=bet_amount;
	DCV_VARS->pot+=bet_amount;
	DCV_VARS->funds[playerid]-=bet_amount;

	if((action=jstr(argjson,"action")) != NULL)
	{
		if(strcmp(action,"check") == 0)
		{
			DCV_VARS->bet_actions[playerid][round]=check;
		}
		else if(strcmp(action,"call") == 0)
		{
			DCV_VARS->bet_actions[playerid][round]=call;
		}
		else if(strcmp(action,"raise") == 0)
		{
			DCV_VARS->bet_actions[playerid][round]=raise;
		}
		else if(strcmp(action,"allin") == 0)
		{
			for(int i=DCV_VARS->round;i<CARDS777_MAXROUNDS;i++)
				DCV_VARS->bet_actions[playerid][i]=allin;
		}
		else if(strcmp(action,"fold") == 0)
		{
			
			for(int i=DCV_VARS->round;i<CARDS777_MAXROUNDS;i++)
				DCV_VARS->bet_actions[playerid][i]=fold;
			
		}

		for(int i=0;i<dcv_maxplayers;i++)
		{
			if((DCV_VARS->bet_actions[i][round]==fold)) /*|| (vars->bet_actions[i][round]==allin)*/
				players_left++;
		}
		players_left=dcv_maxplayers-players_left;
		if(players_left<2)
		{
			for(int i=0;i<dcv_maxplayers;i++)
			{
				for(int j=DCV_VARS->round;j<CARDS777_MAXROUNDS;j++)
				{
					DCV_VARS->bet_actions[i][j]=DCV_VARS->bet_actions[i][round]; //check
				}
			}
		}
	}
	retval=BET_rest_DCV_round_betting(wsi);
	return retval;
}



int32_t BET_rest_player_round_betting_response(struct lws *wsi,cJSON *argjson,int32_t this_playerID)
{
	int retval=1,playerid,round,bet_amount=0;
	char *action =NULL;

	playerid=jint(argjson,"playerid");
	round=jint(argjson,"round");
	bet_amount=jint(argjson,"bet_amount");

	Player_VARS[this_playerID]->betamount[playerid][round]+=bet_amount;
	Player_VARS[this_playerID]->pot+=bet_amount;
	if((action=jstr(argjson,"action")) != NULL)
	{
		if(strcmp(action,"check") == 0)
		{
			Player_VARS[this_playerID]->bet_actions[playerid][round]=check;
		}
		else if(strcmp(action,"call") == 0)
		{
			Player_VARS[this_playerID]->bet_actions[playerid][round]=call;
		}
		else if(strcmp(action,"raise") == 0)
		{
			Player_VARS[this_playerID]->bet_actions[playerid][round]=raise;
		}
		else if(strcmp(action,"fold") == 0)
		{
			Player_VARS[this_playerID]->bet_actions[playerid][round]=fold;
		}
		else if(strcmp(action,"allin") == 0)
		{
			Player_VARS[this_playerID]->bet_actions[playerid][round]=allin;
		}
	}

	return retval;
}
int32_t BET_rest_betting_statemachine(struct lws *wsi,cJSON *argjson)
{
	char *action=NULL;
	int retval=1;
	int32_t this_playerID=jint(argjson,"gui_playerID");
		if((action=jstr(argjson,"action")) != 0)
		{
			
			if(strcmp(action,"small_blind") == 0)
			{
				if(jint(argjson,"playerid") == BET_player[this_playerID]->myplayerid)
				{
					rest_push_cards(wsi,argjson,this_playerID);
					rest_display_cards(argjson,this_playerID);
					retval=BET_rest_small_blind(wsi,argjson);	
				}
			}
			else if(strcmp(action,"big_blind") == 0)
			{
				if(jint(argjson,"playerid") == BET_player[this_playerID]->myplayerid)
				{
					rest_push_cards(wsi,argjson,this_playerID);
					rest_display_cards(argjson,this_playerID);
					retval=BET_rest_big_blind(wsi,argjson); 
				}
			}
			else if(strcmp(action,"small_blind_bet") == 0)
			{
				
				BET_rest_relay(wsi,argjson);
				retval=BET_rest_DCV_small_blind_bet(wsi,argjson);
			}
			else if(strcmp(action,"small_blind_bet_player") == 0)
			{
				retval=BET_rest_player_small_blind_bet(wsi,argjson,jint(argjson,"gui_playerID"));
			}
			else if(strcmp(action,"big_blind_bet") == 0)
			{
				
				BET_rest_relay(wsi,argjson);
				retval=BET_rest_DCV_big_blind_bet(wsi,argjson);
				
			}
			else if(strcmp(action,"big_blind_bet_player") == 0)
			{
				retval=BET_rest_player_big_blind_bet(wsi,argjson,jint(argjson,"gui_playerID"));
			}
			else if(strcmp(action,"round_betting") == 0)
			{
				if(BET_player[this_playerID]->myplayerid == jint(argjson,"playerid"))
				{
					rest_display_cards(argjson,this_playerID);
					retval=BET_rest_player_round_betting(wsi,argjson);
				}
			}
			else if((strcmp(action,"check") == 0) || (strcmp(action,"call") == 0) || (strcmp(action,"raise") == 0)
									|| (strcmp(action,"fold") == 0) || (strcmp(action,"allin") == 0))																					
			{
				BET_rest_relay(wsi,argjson);
				retval=BET_rest_DCV_round_betting_response(wsi,argjson);
		}
			else if((strcmp(action,"check_player") == 0) || (strcmp(action,"call_player") == 0) || (strcmp(action,"raise_player") == 0)
									|| (strcmp(action,"fold_player") == 0) || (strcmp(action,"allin_player") == 0))																					
			{
				retval=BET_rest_player_round_betting_response(wsi,argjson,jint(argjson,"gui_playerID"));
			}
		}
		return retval;
}



