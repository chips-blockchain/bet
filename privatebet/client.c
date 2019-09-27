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
#define _POSIX_C_SOURCE 200809L                /* For pclose, popen, strdup */

#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>


#include "../includes/cJSON.h"
#include "../includes/ppapi/c/pp_stdint.h"
#include "common.h"
#include "bet.h"
#include "cards777.h"
#include "gfshare.h"
#include "client.h"
#include "commands.h"
#include "payment.h"
#include "table.h"
#include "network.h"
#include "states.h"
#include "../log/macrologger.h"


#define LWS_PLUGIN_STATIC
#include "protocol_lws_minimal.c"

struct lws *wsi_global_client=NULL;

struct lws *wsi_global_bvv=NULL;


int32_t player_card_matrix[hand_size];
int32_t player_card_values[hand_size];
int32_t number_cards_drawn=0;

int32_t sharesflag[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

cJSON *dataToWrite=NULL;
int32_t data_exists=0;

char guiData[1024];


struct deck_player_info player_info;
struct deck_bvv_info bvv_info;
int32_t no_of_shares=0;
int32_t player_cards[CARDS777_MAXCARDS];
int32_t no_of_player_cards=0;

int32_t player_id=0;
int32_t player_joined=0;

bits256 all_v_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXCARDS];
bits256 all_g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
int32_t all_sharesflag[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t all_player_card_matrix[CARDS777_MAXPLAYERS][hand_size];
int32_t all_player_card_values[CARDS777_MAXPLAYERS][hand_size];
int32_t all_number_cards_drawn[CARDS777_MAXPLAYERS];
int32_t all_no_of_shares[CARDS777_MAXPLAYERS];
int32_t all_player_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
int32_t all_no_of_player_cards[CARDS777_MAXPLAYERS];
bits256 all_playershares[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];



struct enc_share *all_g_shares[CARDS777_MAXPLAYERS];


char *LN_db="../../.chipsln/lightningd1.sqlite3";


struct privatebet_info *BET_Player1;
struct privatebet_vars *Player1_VARS;
struct deck_player_info player1_info;


struct privatebet_info *BET_Player2;
struct privatebet_vars *Player2_VARS;
struct deck_player_info player2_info;



struct privatebet_info *BET_bvv;
struct privatebet_vars *BVV_VARS;

struct privatebet_info *BET_player[CARDS777_MAXPLAYERS];
struct privatebet_vars *Player_VARS[CARDS777_MAXPLAYERS];
struct deck_player_info all_players_info[CARDS777_MAXPLAYERS];


void player_lws_write(cJSON *data)
{
	if(data_exists==1)
	{
		printf("%s::%d::There is more data\n",__FUNCTION__,__LINE__);
		while(data_exists==1)
			sleep(1);
	}
	memset(guiData,0,sizeof(guiData));
	strncpy(guiData,cJSON_Print(data),strlen(cJSON_Print(data)));
	data_exists=1;
	lws_callback_on_writable(wsi_global_client);
	
}


void make_command(int argc, char **argv,cJSON **argjson)
{
	char command[1024];
	FILE *fp=NULL;
	char data[65536],temp[65536];
	char *buf=NULL;
    memset(command,0x00,sizeof(command));
	memset(data,0x00,sizeof(data));
	for(int i=0;i<argc;i++)
	{
		strcat(command,argv[i]);
		strcat(command," ");
	}	
	 /* Open the command for reading. */
	 fp = popen(command, "r");
	 if (fp == NULL) 
	 {
		   printf("Failed to run command\n" );
		   exit(1);
	 }
	 buf=(char*)malloc(200);
	 if(!buf)
 	 {
 	 	printf("%s::%d::Memory allocation failed\n",__FUNCTION__,__LINE__);
		goto end;
 	 }
	 while(fgets(buf, 200, fp) != NULL)
     {
     	strcat(data,buf);
		memset(buf,0x00,200);
	 }
	if(strncmp("error", data, strlen("error")) == 0) 
	{
		memset(temp,0x00,sizeof(temp));
		strncpy(temp,data+strlen("error"),(strlen(data)-strlen("error")));
		*argjson=cJSON_Parse(temp);
				
	}
	else
	{
		*argjson=cJSON_Parse(data);
		cJSON_AddNumberToObject(*argjson,"code",0);
	}
    end:
	 if(buf)
	 	free(buf);
     pclose(fp);
}

int32_t BET_client_onechip(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    printf("client onechop.(%s)\n",jprint(argjson,0));
    return(0);
}

int32_t BET_client_gameeval(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t i,M,consensus = 0; uint32_t crc32 = 0;
    crc32 = juint(argjson,"crc32");
    vars->evalcrcs[senderid] = crc32;
    //printf("EVAL.(%u).p%d\n",crc32,senderid);
    M = (bet->numplayers >> 1) + 1;
    for (i=0; i<bet->numplayers; i++)
    {
        if ( vars->evalcrcs[i] != 0 && vars->evalcrcs[i] == crc32 )
            consensus++;
    }
    if ( consensus > vars->numconsensus )
        vars->numconsensus = consensus;
    if ( vars->consensus == 0 && consensus >= M )
    {
        vars->consensus = crc32;
        for (i=0; i<bet->numplayers; i++)
            printf("%u ",vars->evalcrcs[i]);
        printf("CONSENSUS.%d\n",consensus);
        BET_statemachine_consensus(bet,vars);
    }
    return(0);
}

int32_t BET_client_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    cJSON *array,*pubkeys,*retjson,*channels,*item; int32_t i,n,flag,already_connected=0,len,err=0; bits256 hash; char *idstr,*source,*dest,*short_id,*rstr;
    printf("BET_client_join\n");
    if ( jstr(argjson,"hostid") != 0 )
    {
        safecopy(Host_peerid,jstr(argjson,"hostid"),sizeof(Host_peerid));
        printf("BET_client_join %s\n",Host_peerid);
        if ( BET_peer_state(Host_peerid,"CHANNELD_NORMAL") == 0 )
        {
            already_connected = 1;
            printf("already connected\n");
        }
        else if ( (retjson= chipsln_connect(Host_ipaddr,LN_port,Host_peerid)) != 0 )
        {
            printf("(%s:%u %s) CONNECTLN.(%s)\n",Host_ipaddr,LN_port,Host_peerid,jprint(retjson,0));
            if ( (idstr= jstr(retjson,"id")) != 0 && strcmp(idstr,Host_peerid) == 0 )
                already_connected = 1;
            free_json(retjson);
        } else printf("null return from chipsln_connect\n");
        if ( already_connected != 0 )
        {
            BET_channels_parse();
            printf("Host_channel.(%s)\n",Host_channel);
            if ( Host_channel[0] == 0 || (int32_t)BET_peer_chipsavail(Host_peerid,bet->chipsize) < 2 )
            {
                if ( (retjson= chipsln_fundchannel(Host_peerid,CARDS777_MAXCHIPS*bet->chipsize)) != 0 )
                {
                    rstr = jprint(retjson,0);
                    if ( strcmp(LN_FUNDINGERROR,rstr) == 0 )
                    {
                        err = 1;
                        system("./fund");
                    }
                    printf("fundchannel -> (%s) err.%d\n",rstr,err);
                    free(rstr);
                    free_json(retjson);
                    if ( err == 0 && BET_peer_state(Host_peerid,"GOSSIPD") != 0 )
                    {
                        for (i=flag=0; i<10; i++)
                        {
                            if ( BET_peer_state(Host_peerid,"CHANNELD_AWAITING_LOCKIN") == 0 )
                            {
                                printf("waiting for CHANNELD_AWAITING_LOCKIN\n");
                                sleep(10);
                            } else break;
                        }
                        for (i=flag=0; i<10; i++)
                        {
                            if ( BET_peer_state(Host_peerid,"CHANNELD_NORMAL") != 0 )
                                sleep(10);
                            else
                            {
                                printf("channel is normal\n");
                                sleep(10);
                                break;
                            }
                        }
                        BET_channels_parse();
                        printf("new Host_channel.(%s)\n",Host_channel);
                    }
                }
            }
        }
    } else printf("no hostid in (%s)\n",jprint(argjson,0));
    BET_hosthash_extract(argjson,bet->chipsize);
    BET_clientpay(bet->chipsize);
    BET_statemachine_joined_table(bet,vars);
    printf("JOIN broadcast.(%s)\n",jprint(argjson,0));
    return(0);
}

int32_t BET_client_tablestatus(cJSON *msgjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    BET_betinfo_parse(bet,vars,msgjson);
    /*if ( vars->turni == bet->myplayerid )
        BET_client_turnisend(bet,vars);*/
    return(0);
}

int32_t BET_client_gamestart(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    cJSON *cmdjson;
    cmdjson = cJSON_CreateObject();
    jaddstr(cmdjson,"method","started");
    jaddstr(cmdjson,"game",bet->game);
    jaddbits256(cmdjson,"tableid",bet->tableid);
    jaddnum(cmdjson,"numrounds",bet->numrounds);
    jaddnum(cmdjson,"range",bet->range);
    jaddbits256(cmdjson,"pubkey",Mypubkey);
    jadd(cmdjson,"actions",BET_statemachine_gamestart_actions(bet,vars));
    if ( bits256_nonz(vars->myhash) == 0 )
    {
        BET_permutation(vars->mypermi,bet->range);
        vcalc_sha256(0,vars->myhash.bytes,(uint8_t *)vars->mypermi,sizeof(*vars->mypermi) * bet->range);
    }
    jaddbits256(cmdjson,"hash",vars->myhash);
    BET_message_send("BET_gamestarted",bet->pushsock,cmdjson,1,bet);
    BET_statemachine_roundstart(bet,vars);
    return(0);
}

int32_t BET_client_gamestarted(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t i; cJSON *array,*reqjson; char str[65];
    if ( senderid >= 0 && senderid <= bet->numplayers )
    {
        vars->hashes[senderid][0] = jbits256(argjson,"hash");
        for (i=0; i<bet->numplayers; i++)
            if ( bits256_nonz(vars->hashes[i][0]) == 0 )
                break;
        if ( i == bet->numplayers )
        {
            array = cJSON_CreateArray();
            for (i=0; i<bet->range; i++)
                jaddinum(array,vars->mypermi[i]);
            reqjson = cJSON_CreateObject();
            jaddstr(reqjson,"method","perm");
            jadd(reqjson,"perm",array);
            jaddbits256(reqjson,"pubkey",Mypubkey);
            BET_message_send("BET_perm",bet->pubsock>=0?bet->pubsock:bet->pushsock,reqjson,1,bet);
        } //else printf("i.%d != num.%d senderid.%d process gamestarted.(%s) [sender.%d] <- %s\n",i,bet->numplayers,senderid,jprint(argjson,0),senderid,bits256_str(str,vars->hashes[senderid][0]));
    }
    return(0);
}

int32_t BET_client_perm(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t i,n,j; cJSON *array;
    //printf("got perm.(%s) sender.%d\n",jprint(argjson,0),senderid);
    if ( senderid >= 0 && senderid < bet->numplayers )
    {
        if ( (array= jarray(&n,argjson,"perm")) != 0 && n == bet->range )
        {
            for (i=0; i<bet->range; i++)
                vars->permis[senderid][i] = jinti(array,i);
            vcalc_sha256(0,vars->hashes[senderid][1].bytes,(uint8_t *)vars->permis[senderid],sizeof(*vars->permis[senderid]) * bet->range);
        }
        for (i=0; i<bet->numplayers; i++)
        {
            if ( bits256_cmp(vars->hashes[i][0],vars->hashes[i][1]) != 0 )
            {
                //char str[65],str2[65]; printf("%d: %s != %s\n",i,bits256_str(str,vars->hashes[i][0]),bits256_str(str2,vars->hashes[i][1]));
                break;
            }
        }
        if ( i == bet->numplayers )
        {
            j = BET_permutation_sort(vars->permi,vars->permis,bet->numplayers,bet->range);
            for (i=0; i<bet->range; i++)
                printf("%d ",vars->permi[i]);
            printf("validated perms best.%d\n",j);
            vars->roundready = 0;
            vars->turni = 0;
            vars->validperms = 1;
            /*if ( vars->turni == bet->myplayerid && vars->round == 0 )
                BET_client_turnisend(bet,vars);*/
        }
    }
    return(0);
}

int32_t BET_client_endround(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t i;
    if ( senderid >= 0 && senderid < bet->numplayers )
    {
        if ( vars->endround[senderid] == 0 )
            vars->endround[senderid] = (uint32_t)time(NULL);
        for (i=0; i<bet->numplayers; i++)
            if ( vars->endround[i] == 0 || vars->actions[vars->roundready][i] == 0 )
                break;
        if ( i == bet->numplayers )
        {
            vars->roundready++;
            BET_statemachine_roundstart(bet,vars);
            /*if ( vars->turni == bet->myplayerid && vars->round == vars->roundready && vars->round < bet->numrounds )
                BET_client_turnisend(bet,vars);*/
        }
    }
    else
    {
        printf(">>>>>>>> BET_client_endround.%d senderid.%d\n",vars->round,senderid);
        BET_statemachine_roundend(bet,vars);
    }
    return(0);
}

int32_t BET_client_MofN(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t cardi,playerj,recvlen; bits256 shard,*ptr; char *cipherstr; uint8_t decoded[sizeof(bits256) + 1024],encoded[sizeof(bits256) + 1024];
    cardi = jint(argjson,"cardi");
    playerj = jint(argjson,"playerj");
    memset(shard.bytes,0,sizeof(shard));
    if ( cardi >= 0 && cardi < bet->range && playerj >= 0 && playerj < bet->numplayers && senderid >= 0 && senderid < bet->numplayers && playerj == bet->myplayerid )
    {
        if ( (cipherstr= jstr(argjson,"cipher")) != 0 )
        {
            char str[65];
            recvlen = (int32_t)strlen(cipherstr) >> 1;
            decode_hex(encoded,recvlen,cipherstr);
            if ( (ptr= (bits256 *)BET_decrypt(decoded,sizeof(decoded),bet->playerpubs[senderid],Myprivkey,encoded,&recvlen)) == 0 )
                printf("decrypt error cardi.%d playerj.%d %s\n",cardi,playerj,bits256_str(str,Myprivkey));
            else shard = *ptr;
            //printf("cipherstr.(%s) -> %s\n",cipherstr,bits256_str(str,shard));
        } else shard = jbits256(argjson,"shard");
        //char str[65],str2[65]; printf("client MofN: cardi.%d playerj.%d %s shardi.%d bet->deckid.%s\n",cardi,playerj,bits256_str(str,shard),senderid,bits256_str(str2,bet->deckid));
        BET_MofN_item(bet->deckid,cardi,bet->cardpubs,bet->range,playerj,bet->numplayers,shard,senderid);
    }
    return(0);
}

int32_t BET_senderid(cJSON *argjson,struct privatebet_info *bet)
{
    int32_t i; char *peerid = jstr(argjson,"peerid");
    for (i=0; i<bet->numplayers; i++)
        if ( bits256_cmp(jbits256(argjson,"pubkey"),bet->playerpubs[i]) == 0 )
            return(i);
    if ( peerid != 0 && strcmp(Host_peerid,peerid) == 0 )
        return(bet->maxplayers);
    return(-1);
}

int32_t BET_clientupdate(cJSON *argjson,uint8_t *ptr,int32_t recvlen,struct privatebet_info *bet,struct privatebet_vars *vars) // update game state based on host broadcast
{
    static uint8_t *decoded; static int32_t decodedlen;
    char *method; int32_t senderid; bits256 *MofN;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
        senderid = BET_senderid(argjson,bet);
        if ( IAMHOST == 0 )
            BET_hosthash_extract(argjson,bet->chipsize);
 //printf("BET_clientupdate: pushsock.%d subsock.%d method.%s sender.%d\n",bet->pushsock,bet->subsock,method,senderid);
        if ( strcmp(method,"tablestatus") == 0 )
            return(BET_client_tablestatus(argjson,bet,vars));
        else if ( strcmp(method,"turni") == 0 )
            return(BET_client_turni(argjson,bet,vars,senderid));
        else if ( strcmp(method,"onechip") == 0 )
            return(BET_client_onechip(argjson,bet,vars,senderid));
        else if ( strcmp(method,"roundend") == 0 )
            return(BET_client_endround(argjson,bet,vars,senderid));
        else if ( strcmp(method,"start0") == 0 )
        {
            bet->numrounds = jint(argjson,"numrounds");
            bet->numplayers = jint(argjson,"numplayers");
            bet->range = jint(argjson,"range");
            return(0);
        }
        else if ( strcmp(method,"start") == 0 )
            return(BET_client_gamestart(argjson,bet,vars));
        else if ( strcmp(method,"gameeval") == 0 )
            return(BET_client_gameeval(argjson,bet,vars,senderid));
        else if ( strcmp(method,"started") == 0 )
            return(BET_client_gamestarted(argjson,bet,vars,senderid));
        else if ( strcmp(method,"perm") == 0 )
            return(BET_client_perm(argjson,bet,vars,senderid));
        else if ( strcmp(method,"deali") == 0 )
            return(BET_client_deali(argjson,bet,vars,senderid));
        else if ( strcmp(method,"join") == 0 )
            return(BET_client_join(argjson,bet,vars,senderid));
        else if ( strcmp(method,"MofN") == 0 )
            return(BET_client_MofN(argjson,bet,vars,senderid));
        else if ( strcmp(method,"deckpacket") == 0 )
        {
            if ( decodedlen < recvlen )
            {
                decoded = realloc(decoded,recvlen);
                decodedlen = recvlen;
                printf("alloc decoded[%d]\n",recvlen);
            }
            if ( (MofN= BET_process_packet(bet->cardpubs,&bet->deckid,GENESIS_PUBKEY,Myprivkey,decoded,decodedlen,Mypubkey,ptr,recvlen,bet->numplayers,bet->range)) == 0 )
            {
                //printf("error processing packet, most likely not encrypted to us\n");
                return(0);
            }
            memcpy(bet->MofN,MofN,sizeof(*MofN) * bet->numplayers * bet->range);
            //int32_t i; char str[65]; for (i=0; i<bet->numplayers * bet->range; i++)
            //    printf("%s ",bits256_str(str,bet->MofN[i]));
            //printf("MofN.%s\n",bits256_str(str,bet->deckid));
            return(0);
        }
    } else printf("clientupdate unexpected.(%s)\n",jprint(argjson,0));
    return(-1);
}
void BET_clientloop(void *_ptr)
{
    uint32_t lasttime = 0; int32_t nonz,recvlen,lastChips_paid; uint16_t port=7798; char connectaddr[64],hostip[64]; void *ptr; cJSON *msgjson,*reqjson; struct privatebet_vars *VARS; struct privatebet_info *bet = _ptr;
    VARS = calloc(1,sizeof(*VARS));
    VARS->lastround = lastChips_paid = -1;
    strcpy(hostip,"5.9.253.195"); // jl777: get from BET blockchain
    printf("client loop: pushsock.%d subsock.%d\n",bet->pushsock,bet->subsock);
    sleep(5);
    while ( 1 )
    {
        if ( bet->subsock >= 0 && bet->pushsock >= 0 )
        {
            nonz = 0;
            if ( (recvlen= nn_recv(bet->subsock,&ptr,NN_MSG,0)) > 0 )
            {
                nonz++;
                if ( (msgjson= cJSON_Parse(ptr)) != 0 )
                {
                    if ( BET_clientupdate(msgjson,ptr,recvlen,bet,VARS) < 0 )
                        printf("unknown clientupdate msg.(%s)\n",jprint(msgjson,0));
                    if ( Num_hostrhashes > 0 && Chips_paid > lastChips_paid )
                    {
                        lastChips_paid = Chips_paid;
                        BET_clientpay(bet->chipsize);
                    }
                    free_json(msgjson);
                }
                nn_freemsg(ptr);
            }
            if ( nonz == 0 )
            {
                if ( time(NULL) > lasttime+60 )
                {
                    printf("%s round.%d turni.%d myid.%d | valid.%d roundready.%d lastround.%d -> myturn? %d\n",bet->game,VARS->round,VARS->turni,bet->myplayerid,VARS->validperms,VARS->roundready,VARS->lastround,VARS->validperms != 0 && VARS->turni == bet->myplayerid && VARS->roundready == VARS->round && VARS->lastround != VARS->round);
                    lasttime = (uint32_t)time(NULL);
                }
                sleep(5);
                BET_statemachine(bet,VARS);
            }
        }
        else if ( hostip[0] != 0 && port > 0 )
        {
            BET_transportname(0,connectaddr,hostip,port);
            safecopy(Host_ipaddr,hostip,sizeof(Host_ipaddr));
            printf("connect %s\n",connectaddr);
            bet->subsock = BET_nanosock(0,connectaddr,NN_SUB);
            BET_transportname(0,connectaddr,hostip,port+1);
            bet->pushsock = BET_nanosock(0,connectaddr,NN_PUSH);
            reqjson = cJSON_CreateObject();
            jaddbits256(reqjson,"pubkey",Mypubkey);
            jaddstr(reqjson,"method","join");
            jaddstr(reqjson,"peerid",LN_idstr);
            //Clientrhash = chipsln_rhash_create(bet->chipsize,"0");
            BET_message_send("BET_havetable",bet->pushsock,reqjson,1,bet);
        }
        else
        {
            // update list of tables
        }
    }
}

char *enc_share_str(char hexstr[177],struct enc_share x)
{
    int bytes=init_hexbytes_noT(hexstr,x.bytes,sizeof(x));
    return(hexstr);
}

void* BET_request(void* _ptr)
{
	struct enc_share temp;
	bits256 share;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	struct privatebet_share *shareInfo=_ptr;
	int32_t ofCardID,ofPlayerID,forPlayerID;
	cJSON *shareReq=NULL;
	char *buf=NULL,str[65],enc_str[177];
	int bytes;

	for(int i=0;i<shareInfo->range;i++)
	{
		for(int j=0;j<shareInfo->numplayers;j++)
		{
			sharesflag[i][j]=0;
		}
	}

	for(int i=0;i<shareInfo->range;i++)
	{
		for(int j=0;j<shareInfo->numplayers;j++)
		{
			if((j!=shareInfo->myplayerid) && (sharesflag[i][j]==0))
			{				
				shareReq=cJSON_CreateObject();
				cJSON_AddStringToObject(shareReq,"method","request_share");
				cJSON_AddNumberToObject(shareReq,"ofCardID",i);
				cJSON_AddNumberToObject(shareReq,"ofPlayerID",j);
				cJSON_AddNumberToObject(shareReq,"forPlayerID",shareInfo->myplayerid);
				buf=cJSON_Print(shareReq);
				bytes=nn_send(shareInfo->pushsock,buf,strlen(buf),0);
				cJSON_Delete(shareReq);
			}
			else if((j==shareInfo->myplayerid) && (sharesflag[i][j]==0)) {
				ofCardID=jint(_ptr,"ofCardID");
				ofPlayerID=jint(_ptr,"ofPlayerID");
				forPlayerID=jint(_ptr,"forPlayerID");
				//temp=g_shares[ofPlayerID*shareInfo->numplayers*shareInfo->range + (ofCardID*shareInfo->numplayers + forPlayerID)];
				temp=g_shares[shareInfo->myplayerid*shareInfo->numplayers*shareInfo->range + (i*shareInfo->numplayers + shareInfo->myplayerid)];
				recvlen = sizeof(temp);
				if ( (ptr= BET_decrypt(decipher,sizeof(decipher),shareInfo->bvv_public_key,shareInfo->player_key.priv,temp.bytes,&recvlen)) == 0 )
						printf("decrypt error ");
				else
				{
					memcpy(share.bytes,ptr,recvlen);
					playershares[i][j]=share;
					sharesflag[i][j]=1;
				}
					
				
			}
		}
	}

}

void* BET_response(void* _ptr)
{
	
	int32_t ofCardID,ofPlayerID,forPlayerID,subsock,pushsock;
	struct enc_share temp;
	char str[65],enc_str[177],*buf;
	bits256 share;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	struct privatebet_share *share_info=_ptr;
	cJSON *share_res;
	char bindaddr[128]="ipc:///tmp/bet.ipc",bindaddr1[128]="ipc:///tmp/bet1.ipc";
	//subsock=BET_nanosock(0,bindaddr,NN_SUB);
	//pushsock=BET_nanosock(0,bindaddr1,NN_PUSH);
	subsock=share_info->subsock;
	pushsock=share_info->pushsock;
	
	int flag=1;
	while(flag)
	{
		flag=0;
		int bytes = nn_recv (subsock, &buf, NN_MSG, 0);
		if(bytes>0)
		{
			
		    share_res=cJSON_Parse(buf);
			if(0==strcmp(cJSON_str(cJSON_GetObjectItem(share_res,"method")),"request_share"))
			{
				ofCardID=jint(share_res,"ofCardID");
				ofPlayerID=jint(share_res,"ofPlayerID");
				forPlayerID=jint(share_res,"forPlayerID");
				if((ofPlayerID==share_info->myplayerid)&&(forPlayerID!=share_info->myplayerid))
					{
						temp=g_shares[ofPlayerID*share_info->numplayers*share_info->range + (ofCardID*share_info->numplayers + forPlayerID)];
						recvlen = sizeof(temp);
						if ( (ptr= BET_decrypt(decipher,sizeof(decipher),share_info->bvv_public_key,share_info->player_key.priv,temp.bytes,&recvlen)) == 0 )
							printf("decrypt error ");
						else
						{
							memcpy(share.bytes,ptr,recvlen);
							cJSON_Delete(share_res);
							share_res=cJSON_CreateObject();
							cJSON_AddStringToObject(share_res,"method","receive_share");
							cJSON_AddNumberToObject(share_res,"ofCardID",ofCardID);
							cJSON_AddNumberToObject(share_res,"ofPlayerID",ofPlayerID);
							cJSON_AddNumberToObject(share_res,"forPlayerID",forPlayerID);
							jaddbits256(share_res,"share",share);
					   }
						if(pushsock>=0){
							char *buf=NULL;
							buf=cJSON_Print(share_res);
							int bytes=nn_send(pushsock,buf,strlen(buf),0);
						}	
					}
				}
			else if(0==strcmp(cJSON_str(cJSON_GetObjectItem(share_res,"method")),"receive_share"))
			{
				ofCardID=jint(share_res,"ofCardID");
				ofPlayerID=jint(share_res,"ofPlayerID");
				forPlayerID=jint(share_res,"forPlayerID");
				if(share_info->myplayerid==forPlayerID)
				{
					sharesflag[ofCardID][ofPlayerID]=1;
					playershares[ofCardID][ofPlayerID]=jbits256(share_res,"share");
				}
			}
		}
		for(int i=0;i<share_info->range;i++)
		{
			for(int j=0;j<share_info->numplayers;j++)
			{
				if(sharesflag[i][j]==0)
				{
					flag=1;
					break;
				}
			}
			if(flag)
				break;	
		}
		sleep(5);
	}
	nn_shutdown(pushsock,0);
	nn_shutdown(subsock,0);
}


bits256 BET_request_share(int32_t ofCardID,int32_t ofPlayerID,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key)
{
	cJSON *shareInfo=NULL;
	bits256 share;
	char *buf=NULL;
	char str[65];
	int bytes;
	int32_t forPlayerID;
	
	shareInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(shareInfo,"method","request_share");
	cJSON_AddNumberToObject(shareInfo,"ofCardID",ofCardID);
	cJSON_AddNumberToObject(shareInfo,"ofPlayerID",ofPlayerID);
	cJSON_AddNumberToObject(shareInfo,"forPlayerID",bet->myplayerid);
	buf=cJSON_Print(shareInfo);
	bytes=nn_send(bet->pushsock,buf,strlen(buf),0);
	cJSON_Delete(shareInfo);
	shareInfo=cJSON_CreateObject();
	bytes=0;
	buf=NULL;
	while(1)
	{
		bytes=nn_recv(bet->subsock,&buf,NN_MSG,0);
		if(bytes>0)
		{
			shareInfo=cJSON_Parse(buf);
			if(0==strcmp(cJSON_str(cJSON_GetObjectItem(shareInfo,"method")),"response_share"))
			{
				share=jbits256(shareInfo,"share");
				break;
        	}
			else if(0==strcmp(cJSON_str(cJSON_GetObjectItem(shareInfo,"method")),"request_share"))
			{
				forPlayerID=jint(shareInfo,"forPlayerID");
				if(forPlayerID!=bet->myplayerid)
				{
					BET_give_share(shareInfo,bet,bvv_public_key,player_key);
				}
			}
		}
		sleep(5);
	}
	return share;
}

void BET_give_share(cJSON *shareInfo,struct privatebet_info *bet,bits256 bvv_public_key,struct pair256 player_key)
{
	int32_t ofCardID,ofPlayerID,forPlayerID;
	struct enc_share temp;
	char str[65],enc_str[177];
	bits256 share;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	ofCardID=jint(shareInfo,"ofCardID");
	ofPlayerID=jint(shareInfo,"ofPlayerID");
	forPlayerID=jint(shareInfo,"forPlayerID");
	cJSON_Print(shareInfo);

	if((ofPlayerID==bet->myplayerid)&&(forPlayerID!=bet->myplayerid))
	{
        temp=g_shares[ofPlayerID*bet->numplayers*bet->range + (ofCardID*bet->numplayers + forPlayerID)];
	    recvlen = sizeof(temp);
		if ( (ptr= BET_decrypt(decipher,sizeof(decipher),bvv_public_key,player_key.priv,temp.bytes,&recvlen)) == 0 )
            printf("decrypt error ");
        else
        {
        	memcpy(share.bytes,ptr,recvlen);
			cJSON_Delete(shareInfo);
			shareInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(shareInfo,"method","response_share");
			cJSON_AddNumberToObject(shareInfo,"ofCardID",ofCardID);
			cJSON_AddNumberToObject(shareInfo,"ofPlayerID",ofPlayerID);
			cJSON_AddNumberToObject(shareInfo,"forPlayerID",forPlayerID);
			jaddbits256(shareInfo,"share",share);
	   }
		if(bet->pushsock>=0){
			char *buf=NULL;
			buf=cJSON_Print(shareInfo);
			int bytes=nn_send(bet->pushsock,buf,strlen(buf),0);
		}	
	}
}


struct enc_share get_API_enc_share(cJSON *obj)
{
    struct enc_share hash; char *str;
	char hexstr[177];
    memset(hash.bytes,0,sizeof(hash));
   if ( obj != 0 )
    {
        if ( is_cJSON_String(obj) != 0 && (str= obj->valuestring) != 0 && strlen(str) == 176 ){
			
			decode_hex(hash.bytes,sizeof(hash),str);

        }
    }   

    return(hash);
}

void* BET_clientplayer(void * _ptr)
{
		static int32_t decodebad,decodegood,good,bad,errs;
    	int32_t unpermi,playererrs=0,decoded[CARDS777_MAXCARDS];
    	bits256 public_key_b;
		bits256 decoded256,temp,playerprivs[CARDS777_MAXCARDS],playercards[CARDS777_MAXCARDS],blindedcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
		int32_t permis[CARDS777_MAXCARDS],numcards,numplayers;
		struct pair256 key;struct privatebet_info *bet = _ptr;
		char str[65],share_str[177];
		cJSON *playerInfo,*gameInfo,*cjsonplayercards,*cjsonblindedcards,*cjsonshamirshards,*cjsoncardprods,*item,*cjsong_hash;
        cJSON *joinInfo;
		numplayers=bet->numplayers;
		numcards=bet->range;
		
      
		if ( bet->subsock >= 0 && bet->pushsock >= 0 )
		{
			key = deckgen_player(playerprivs,playercards,permis,numcards);
			playerInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(playerInfo,"method","init");
			cJSON_AddNumberToObject(playerInfo,"playerid",bet->myplayerid);
			cJSON_AddNumberToObject(playerInfo,"range",bet->range);
			jaddbits256(playerInfo,"publickey",key.prod);
			cJSON_AddItemToObject(playerInfo,"playercards",cjsonplayercards=cJSON_CreateArray());
			for(int i=0;i<numcards;i++) 
			{
				cJSON_AddItemToArray(cjsonplayercards,cJSON_CreateString(bits256_str(str,playercards[i])));
			}
			
			char *rendered=cJSON_Print(playerInfo);
			int bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
			
			while (1) 
			{
				char *buf = NULL;
				int bytes = nn_recv (bet->subsock, &buf, NN_MSG, 0);
				if(bytes>0)
				{
					
					gameInfo=cJSON_Parse(buf);
					if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"decode"))
					{
						
						public_key_b=jbits256(gameInfo,"public_key_b");
						g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
						cjsonblindedcards=cJSON_GetObjectItem(gameInfo,"blindedcards");
						for(int i=0;i<numplayers;i++)
						{
							for(int j=0;j<numcards;j++)
							{
								blindedcards[i][j]=jbits256i(cjsonblindedcards,i*numcards+j);
							}
						}
						cjsonshamirshards=cJSON_GetObjectItem(gameInfo,"shamirshards");
						cJSON_Print(cjsonshamirshards);	
						
						int k=0;
						for(int playerid=0;playerid<numplayers;playerid++)
						{
							for (int i=0; i<numcards; i++)
					        {
					            for (int j=0; j<numplayers; j++) 
								{
									g_shares[k]=get_API_enc_share(cJSON_GetArrayItem(cjsonshamirshards,k));
									k++;
					            }
					        }
						}
						for(int i=0;i<numcards;i++)
						{
							for(int j=0;j<numcards;j++)
							{
								temp=xoverz_donna(curve25519(key.priv,curve25519(playerprivs[i],cardprods[bet->myplayerid][j])));
								vcalc_sha256(0,v_hash[i][j].bytes,temp.bytes,sizeof(temp));
							}
						}
				
					   for(int i=0;i<numcards;i++)
					   {
        				    decoded256 = t_sg777_player_decode(bet,i,numplayers,key,public_key_b,blindedcards[bet->myplayerid][i],cardprods[bet->myplayerid],playerprivs,numcards);
							
							if ( bits256_nonz(decoded256) == 0 )
                				errs++;
            				else
            				{
            					int k;
				                unpermi=-1;
				                for(k=0;k<numcards;k++)
								{
				                    if(permis[k]==decoded256.bytes[30])
									{
				                        unpermi=k;
				                        break;
				                    }
				                }
				                decoded[i] = k;    	
            				}
        			 }
					decodebad += errs;
    				decodegood+= (numcards - errs);
					printf("\nCards Decoded:%d, errored:%d",decodegood,decodebad);					
     				}
					else if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"init_d"))
					{
						printf("\n%s:%d:%s",__FUNCTION__,__LINE__,buf);
						deckid=jbits256(gameInfo,"deckid");
						cjsoncardprods=cJSON_GetObjectItem(gameInfo,"cardprods");
						
						for(int i=0;i<numplayers;i++)
						{
							for(int j=0;j<numcards;j++)
							{
								cardprods[i][j]=jbits256i(cjsoncardprods,i*numcards+j);
							}
						}

						
						cjsong_hash=cJSON_GetObjectItem(gameInfo,"g_hash");
						
						for(int i=0;i<numplayers;i++)
						{
							for(int j=0;j<numcards;j++)
							{
								g_hash[i][j]=jbits256i(cjsong_hash,i*numcards+j);
							}
						}

					}
					else if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"request_share"))
					{
						struct privatebet_share *share_info=NULL;
						
						share_info=calloc(1,sizeof(struct privatebet_share));
						share_info->bvv_public_key=public_key_b;
						share_info->player_key=key;
						share_info->subsock=bet->subsock;
						share_info->myplayerid=bet->myplayerid;
						share_info->range=bet->range;
						share_info->numplayers=bet->numplayers;
						share_info->pushsock=bet->pushsock;
						BET_response(share_info);
						free(share_info);
						//BET_give_share(gameInfo,bet,public_key_b,key);
					}
				}
			}
			nn_shutdown(bet->pushsock,0);
			nn_shutdown(bet->subsock,0);
		}
		return NULL;
}



void* BET_clientbvv(void * _ptr)
{
	  
	  	bits256 deckid,publickeys[CARDS777_MAXPLAYERS],playercards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],finalcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],blindedcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],blindingvals[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
		int32_t numcards,numplayers,playerID,range,flag=1;
		struct pair256 b_key,keys[CARDS777_MAXPLAYERS];struct privatebet_info *bet = _ptr;
		char str[65],share_str[177];

		cJSON *playerInfo,*gameInfo,*cjsonplayercards,*temp,*item,*cjsonblindedcards,*cjsonfinalcards,*cjsonshamirshards;
		numplayers=0;
		numcards=bet->range;
		blinding_vendor_perm(bet->range);
		b_key.priv=curve25519_keypair(&b_key.prod);
		g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
   		if ( bet->subsock >= 0 && bet->pushsock >= 0 ) 
		{
			while(numplayers!=bet->numplayers) {
			  	char *buf=NULL;
				int bytes=nn_recv(bet->subsock,&buf,NN_MSG,0);
				if(bytes>0)	{
					gameInfo=cJSON_Parse(buf);
					if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"init"))
					{
						numplayers++;
						playerID=jint(gameInfo,"playerid");
						range=jint(gameInfo,"range");
						keys[playerID].prod=jbits256(gameInfo,"publickey");
						playerInfo=cJSON_GetObjectItem(gameInfo,"playercards");
						for(int i=0;i<cJSON_GetArraySize(playerInfo);i++) 
						{
								playercards[playerID][i]=jbits256i(playerInfo,i);
						}
					}
				 }
				}
			while (flag) 
			{
				char *buf = NULL;
				int bytes = nn_recv (bet->subsock, &buf, NN_MSG, 0);
				if(bytes>0)
				{
					gameInfo=cJSON_Parse(buf);
					if(0==strcmp(cJSON_str(cJSON_GetObjectItem(gameInfo,"method")),"init_d")) 
					{
						deckid=jbits256(gameInfo,"deckid");
						cjsonfinalcards=cJSON_GetObjectItem(gameInfo,"finalcards");
						for(int playerID=0;playerID<numplayers;playerID++) 
						{
								for(int i=0;i<numcards;i++) 
								{
									finalcards[playerID][i]=jbits256i(cjsonfinalcards,playerID*numcards+i);
								}
						}
	         		    g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
					    for (int playerid=0; playerid<numplayers; playerid++)
						{
					        sg777_blinding_vendor(keys,b_key,blindingvals[playerid],blindedcards[playerid],finalcards[playerid],numcards,numplayers,playerid,deckid); // over network
    					}
						cJSON_Delete(gameInfo);
						gameInfo=cJSON_CreateObject();
						cJSON_AddStringToObject(gameInfo,"method","decode");
						jaddbits256(gameInfo,"public_key_b",b_key.prod);
						cJSON_AddItemToObject(gameInfo,"blindedcards",cjsonblindedcards=cJSON_CreateArray());
						for(int i=0;i<numplayers;i++) 
						{
							for(int j=0;j<numcards;j++) 
							{
							cJSON_AddItemToArray(cjsonblindedcards,cJSON_CreateString(bits256_str(str,blindedcards[i][j])));
							}
						}
						cJSON_AddItemToObject(gameInfo,"shamirshards",cjsonshamirshards=cJSON_CreateArray());
						int k=0;
						for(int playerid=0;playerid<numplayers;playerid++) 
						{
							for (int i=0; i<numcards; i++)
							{
					            for (int j=0; j<numplayers; j++) 
								{
									cJSON_AddItemToArray(cjsonshamirshards,cJSON_CreateString(enc_share_str(share_str,g_shares[k++])));
					            }
					        }
						}
						char *rendered=cJSON_Print(gameInfo);
						nn_send(bet->pushsock,rendered,strlen(rendered),0);
						flag=0;
					}
				}
			}
			nn_shutdown(bet->pushsock,0);
			nn_shutdown(bet->subsock,0);
		
		}
	  return NULL;
}

/*
Below code is aimed to implement p2p Pangea
*/

int32_t BET_p2p_bvv_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t bytes,retval=1;
	char *rendered,str[65],enc_str[177];
	cJSON *cjsondcvblindcards,*cjsonpeerpubkeys,*bvv_init_info,*cjsonbvvblindcards,*cjsonshamirshards;
	bits256 dcvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],peerpubkeys[CARDS777_MAXPLAYERS];
	bits256 bvvblindingvalues[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	
	bvv_info.numplayers=bet->numplayers;
	bvv_info.maxplayers=bet->maxplayers;
	bvv_info.deckid=jbits256(argjson,"deckid");
	bvv_info.bvv_key.priv=curve25519_keypair(&bvv_info.bvv_key.prod);
	cjsonpeerpubkeys=cJSON_GetObjectItem(argjson,"peerpubkeys");
	cjsondcvblindcards=cJSON_GetObjectItem(argjson,"dcvblindcards");
	
	for(uint32_t playerID=0;playerID<bvv_info.maxplayers;playerID++) 
	{
		peerpubkeys[playerID]=jbits256i(cjsonpeerpubkeys,playerID);
		for(int i=0;i<bet->range;i++) 
		{
			dcvblindcards[playerID][i]=jbits256i(cjsondcvblindcards,playerID*bet->range+i); //bvv_info.maxplayers
		}
	}
    g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));

	
	for (uint32_t playerid=0; playerid<bvv_info.maxplayers; playerid++)
	{
		p2p_bvv_init(peerpubkeys,bvv_info.bvv_key,bvvblindingvalues[playerid],bvvblindcards[playerid],
			dcvblindcards[playerid],bet->range,bvv_info.numplayers,playerid,bvv_info.deckid);
		//sleep(5);

	}
	
	bvv_init_info=cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_init_info,"method","init_b");
	jaddbits256(bvv_init_info,"bvvpubkey",bvv_info.bvv_key.prod);
	cJSON_AddItemToObject(bvv_init_info,"bvvblindcards",cjsonbvvblindcards=cJSON_CreateArray());
	for(uint32_t i=0;i<bvv_info.numplayers;i++)
	{
		for(int j=0;j<bet->range;j++)
		{
			cJSON_AddItemToArray(cjsonbvvblindcards,cJSON_CreateString(bits256_str(str,bvvblindcards[i][j])));
		}
	}
	cJSON_AddItemToObject(bvv_init_info,"shamirshards",cjsonshamirshards=cJSON_CreateArray());
	int k=0;
	for(uint32_t playerid=0;playerid<bvv_info.numplayers;playerid++)
	{
		for(int i=0;i<bet->range;i++)
		{
			for(uint32_t j=0;j<bvv_info.numplayers;j++)
			{
				cJSON_AddItemToArray(cjsonshamirshards,cJSON_CreateString(enc_share_str(enc_str,g_shares[k++])));
			}
		}
	}
	rendered=cJSON_Print(bvv_init_info);
	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

	if(bytes<0)
	{
		retval=-1;
		printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
		goto end;
	}
   end:
		return retval;
	
}

int32_t BET_p2p_bvv_join_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *channelInfo=NULL,*addresses,*address,*bvvResponseInfo=NULL;
	int argc,bytes,retval=1,maxsize=10000;
	char **argv=NULL,*uri=NULL,*rendered=NULL;
	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"getinfo");
	argv[2]=NULL;

	channelInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&channelInfo);
	
	cJSON_Print(channelInfo);
	if(jint(channelInfo,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(channelInfo,"message"));
		goto end;
	}

	uri=(char*)malloc(sizeof(char)*100);
	if(!uri)
	{
		printf("%s::%d::malloc failed\n",__FUNCTION__,__LINE__);
		goto end;
	}	
	strcpy(uri,jstr(channelInfo,"id"));
	strcat(uri,"@");
	addresses=cJSON_GetObjectItem(channelInfo,"address");
	address=cJSON_GetArrayItem(addresses,0);
	strcat(uri,jstr(address,"address"));

	bvvResponseInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvResponseInfo,"method","bvv_join");
	cJSON_AddStringToObject(bvvResponseInfo,"uri",uri);
	printf("%s::%d::uri::%s\n",__FUNCTION__,__LINE__,uri);
	rendered=cJSON_Print(bvvResponseInfo);
    bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

	
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
int32_t BET_p2p_connect(char *uri)
{
	char **argv=NULL;
	int argc=4;
	cJSON *connectInfo=NULL;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"connect");
	strcpy(argv[2],uri);
	argv[3]=NULL;
	connectInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&connectInfo);
	//ln_bet(argc-1,argv,response);
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
	return 1;
}

cJSON* BET_p2p_fundchannel(char *channel_id,char *satohis)
{
	char **argv=NULL;
	int argc=5;
	cJSON *fundChannelInfo=NULL;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"fundchannel");
	strcpy(argv[2],channel_id);
	strcpy(argv[3],satohis);
	argv[4]=NULL;

	fundChannelInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&fundChannelInfo);

	//ln_bet(argc-1,argv,response);
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
	return fundChannelInfo;
}



int32_t BET_p2P_check_bvv_ready(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int retval,channel_state,maxsize=10000,bytes;
	cJSON *uriInfo=NULL,*fundChannelInfo=NULL,*bvvReady=NULL;
	char uri[100],channel_id[100],*rendered=NULL;
	
	uriInfo=cJSON_GetObjectItem(argjson,"uri_info");
	for(int i=0;i<cJSON_GetArraySize(uriInfo);i++)
	{
		
		strcpy(uri,jstri(uriInfo,i));
		strcpy(channel_id,strtok(uri,"@"));
		channel_state=LN_get_channel_status(channel_id);
		if((channel_state != 2) && (channel_state != 3))
		{
			BET_p2p_connect(jstri(uriInfo,i));
			fundChannelInfo=cJSON_CreateObject();
			fundChannelInfo=BET_p2p_fundchannel(channel_id,"50000");
	
			if(jint(fundChannelInfo,"code") == -1)
			{
				retval=-1;
				printf("\n%s:%d: Message: %s",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
				goto end;
			}
		}
	}
	for(int i=0;i<cJSON_GetArraySize(uriInfo);i++)
	{
		strcpy(uri,jstri(uriInfo,i));
		strcpy(channel_id,strtok(uri,"@"));
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
				printf("\n%s:%d: BVV is failed to establish the channel with Player: %d",__FUNCTION__,__LINE__,i);
				break;
			}
		}
		
		printf("BVV  --> Player %d channel ready\n",i);	
	}

	bvvReady=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvReady,"method","bvv_ready");
	
	rendered=cJSON_Print(bvvReady);
    bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
	if(bytes<0)
		retval=-1;
	end:
		return retval;
}


void BET_rest_BVV_reset()
{
	printf("%s::%d\n",__FUNCTION__,__LINE__);
	memset(&bvv_info,0, sizeof bvv_info);
	BET_permutation(bvv_info.permis,BET_bvv->range);
    for(int i=0;i<BET_bvv->range;i++)
	{
		permis_b[i]=bvv_info.permis[i];
	
	}
}


void BET_BVV_reset(struct privatebet_info *bet,struct privatebet_vars *vars)
{

	BET_permutation(bvv_info.permis,bet->range);
    for(int i=0;i<bet->range;i++)
	{
		permis_b[i]=bvv_info.permis[i];
	
	}
	if(g_shares)
		free(g_shares);
}


int32_t BET_p2p_bvvcommand_test(struct lws *wsi, cJSON *argjson)
{
    char *method; int32_t senderid,retval=1;
	struct privatebet_info *bet=NULL;
	struct privatebet_vars *vars=NULL;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
    	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,method);
		if ( strcmp(method,"TableInfo") == 0 )
		{
			BET_p2p_table_info(argjson,BET_bvv,vars);
			
		}
   		else if(strcmp(method,"init_d") == 0)
		{
			 BET_p2p_bvv_init(argjson,BET_bvv,vars);
		}
		else if(strcmp(method,"bvv_join") == 0)
		{
			BET_p2p_bvv_join_init(argjson,BET_bvv,vars);
		}
		else if(strcmp(method,"check_bvv_ready") == 0)
		{
			BET_p2P_check_bvv_ready(argjson,BET_bvv,vars);
		}
		else if(strcmp(method,"dealer") == 0)
		{
			retval=BET_p2p_dealer_info(argjson,BET_bvv,vars);
		}
		else if(strcmp(method,"reset") == 0)
		{
			BET_BVV_reset(bet,vars);
			
		}
		else if(strcmp(method,"seats") == 0)
		{
			printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(argjson));
		}
        else
            retval=-1;
    }
    return retval;
}






void BET_p2p_bvvloop_test(void *_ptr)
{
    uint32_t lasttime = 0; uint8_t r; int32_t nonz,recvlen,sendlen; cJSON *argjson,*timeoutjson; void *ptr; double lastmilli = 0.; struct privatebet_info *bet = _ptr; struct privatebet_vars *VARS;
    VARS = calloc(1,sizeof(*VARS));

	cJSON *bvvJoinInfo=NULL;
	
	BET_permutation(bvv_info.permis,bet->range);
    for(int i=0;i<bet->range;i++)
	{
		permis_b[i]=bvv_info.permis[i];
	
	}
	
	bvvJoinInfo=cJSON_CreateObject();
	
	cJSON_AddStringToObject(bvvJoinInfo,"method","bvv_join");
	if ( BET_p2p_bvvcommand(bvvJoinInfo,bet,VARS) < 0 )
	{
        printf("\n%s:%d:BVV joining the table failed",__FUNCTION__,__LINE__);
	}
   


	
	while ( bet->pushsock>= 0 && bet->subsock>= 0 )
    {
		ptr=0;
        if ( (recvlen= nn_recv(bet->subsock,&ptr,NN_MSG,0)) > 0 )
        {

		  	char *tmp=clonestr(ptr);
            if ( (argjson= cJSON_Parse(tmp)) != 0 )
            {
                if ( BET_p2p_bvvcommand(argjson,bet,BVV_VARS) != 0 ) // usually just relay to players
                {
                	// do soemthing incase any command or logic failures
                }
                free_json(argjson);
            }
			if(tmp)
				free(tmp);
			if(ptr)
            	nn_freemsg(ptr);
        }

    }
}




int32_t BET_p2p_bvvcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    char *method; int32_t senderid,retval=1;
	
    if ( (method= jstr(argjson,"method")) != 0 )
    {
    	
		if ( strcmp(method,"TableInfo") == 0 )
		{
			BET_p2p_table_info(argjson,bet,vars);
			
		}
   		else if(strcmp(method,"init_d") == 0)
		{
			 BET_p2p_bvv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"bvv_join") == 0)
		{
			printf("%s::%d::bvv_join\n",__FUNCTION__,__LINE__);
			BET_p2p_bvv_join_init(argjson,bet,vars);
		}
		else if(strcmp(method,"check_bvv_ready") == 0)
		{
			BET_p2P_check_bvv_ready(argjson,bet,vars);
		}
		else if(strcmp(method,"dealer") == 0)
		{
			//retval=BET_p2p_dealer_info(argjson,bet,vars);
		}
		else if(strcmp(method,"reset") == 0)
		{
			printf("%s::%d::method::%s\n",__FUNCTION__,__LINE__,method);
			BET_BVV_reset(bet,vars);
			BET_p2p_bvv_join_init(argjson,BET_bvv,vars);
		}
		else if(strcmp(method,"seats") == 0)
		{
			printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			BET_p2p_bvv_join_init(argjson,bet,vars);
		}
        else
            retval=-1;
    }
    return retval;
}


void BET_p2p_bvvloop(void *_ptr)
{
    uint32_t lasttime = 0; uint8_t r; int32_t nonz,recvlen,sendlen; cJSON *argjson,*timeoutjson; void *ptr; double lastmilli = 0.; struct privatebet_info *bet = _ptr; struct privatebet_vars *VARS;
    VARS = calloc(1,sizeof(*VARS));

	cJSON *bvvJoinInfo=NULL;
	
	BET_permutation(bvv_info.permis,bet->range);
    for(int i=0;i<bet->range;i++)
	{
		permis_b[i]=bvv_info.permis[i];
	
	}
	
	bvvJoinInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvJoinInfo,"method","bvv_join");
	if ( BET_p2p_bvvcommand(bvvJoinInfo,bet,VARS) < 0 )
	{
        printf("\n%s:%d:BVV joining the table failed",__FUNCTION__,__LINE__);
	}


	
	
	while ( bet->pushsock>= 0 && bet->subsock>= 0 )
    {
        if ( (recvlen= nn_recv(bet->subsock,&ptr,NN_MSG,0)) > 0 )
        {
          
            if ( (argjson= cJSON_Parse(ptr)) != 0 )
            {
                if ( BET_p2p_bvvcommand(argjson,bet,VARS) != 0 ) // usually just relay to players
                {
                	// do soemthing incase any command or logic failures
                }
                free_json(argjson);
            }
            nn_freemsg(ptr);
        }
          
    }
}


bits256 BET_p2p_decode_card(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t cardid)
{
	int32_t retval,numplayers,numcards,M,playerid,flag=0;
	bits256 recover,decoded,refval,tmp,xoverz,hash,fe,basepoint;
	uint8_t **shares;
    uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	char str[65];
	char hexstr [ 65 ];
	numplayers=bet->maxplayers;
	numcards=bet->range;
	shares=calloc(numplayers,sizeof(uint8_t*));
    for(int i=0;i<numplayers;i++)
        shares[i]=calloc(sizeof(bits256),sizeof(uint8_t));
    
  	M=(numplayers/2)+1;
	for(int i=0;i<M;i++) 
	{
		memcpy(shares[i],playershares[cardid][i].bytes,sizeof(bits256));
	}
	gfshare_calc_sharenrs(sharenrs,numplayers,player_info.deckid.bytes,sizeof(player_info.deckid)); // same for all players for this round

	gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);



	gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);
	refval = fmul_donna(player_info.bvvblindcards[bet->myplayerid][cardid],crecip_donna(recover));

	//printf("\nDCV blinded card:%s",bits256_str(str,refval));
	
	
	for(int i=0;i<bet->range;i++)
	{
		for(int j=0;j<bet->range;j++)
		{
			bits256 temp=xoverz_donna(curve25519(player_info.player_key.priv,curve25519(player_info.cardprivkeys[i],player_info.cardprods[bet->myplayerid][j])));
			vcalc_sha256(0,v_hash[i][j].bytes,temp.bytes,sizeof(temp));
		}
	}

	
	basepoint=curve25519_basepoint9();
	for (int i=0; i<bet->range; i++)
    {
        for (int j=0; j<bet->range; j++)
        {
        	if ( bits256_cmp(v_hash[i][j],g_hash[bet->myplayerid][cardid]) == 0 )
			{
				//printf("\nThere is a match\n");
				#if 1
				for(int m=0;m<bet->range;m++)
				{
					for(int n=0;n<bet->range;n++)	
					{
						tmp = curve25519(player_info.player_key.priv,curve25519(player_info.cardprivkeys[m],player_info.cardprods[bet->myplayerid][n]));
			            xoverz = xoverz_donna(tmp);
			            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));

						
			            fe = crecip_donna(curve25519_fieldelement(hash));

						decoded = curve25519(fmul_donna(refval,fe),basepoint);
						for(int k=0;k<bet->range;k++)
						{
				            if ( bits256_cmp(decoded,player_info.cardprods[bet->myplayerid][k]) == 0 )
				            {
				               // printf("\nplayer.%d decoded card %s value %d\n",bet->myplayerid,bits256_str(str,decoded),player_info.cardprivkeys[m].bytes[30]);
								//printf("\n");
								player_cards[no_of_player_cards]=atoi(bits256_str(str,decoded));
								no_of_player_cards++;
				        		tmp=player_info.cardprivkeys[m];
								flag=1;
								goto end;
				            }
						}
					}
				}
				#endif
				#if 0
	            tmp = curve25519(player_info.player_key.priv,curve25519(player_info.cardprivkeys[i],player_info.cardprods[bet->myplayerid][j]));
	            xoverz = xoverz_donna(tmp);
	            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));

	            fe = crecip_donna(curve25519_fieldelement(hash));

				decoded = curve25519(fmul_donna(refval,fe),basepoint);
				
	            if ( bits256_cmp(decoded,player_info.cardprods[bet->myplayerid][cardid]) == 0 )
	            {
	                printf("\nplayer.%d decoded card %s value %d\n",bet->myplayerid,bits256_str(str,decoded),player_info.cardprivkeys[i].bytes[30]);
					printf("\n");
	        		tmp=player_info.cardprivkeys[i];
					flag=1;
					goto end;
	            }
				#endif
        	}
        }
    }
	
	end:
		if(!flag)	
			printf("\nDecoding Failed\n");
			
	return tmp;
}

int32_t BET_p2p_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *invoiceInfo=NULL,*payResponse=NULL;
    char *invoice=NULL;
	int argc,retval=1;
	char **argv=NULL;
	int32_t playerID;


	argv=(char**)malloc(4*sizeof(char*));
	argc=3;
	for(int i=0;i<4;i++)
	{
		argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	playerID=jint(argjson,"playerID");
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);
	if(playerID==bet->myplayerid)
	{
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"pay");
		sprintf(argv[2],"%s",jstr(invoiceInfo,"bolt11"));
		argv[3]=NULL;
		payResponse=cJSON_CreateObject();
		make_command(argc,argv,&payResponse);

		if(jint(payResponse,"code") != 0)
		{
			retval=-1;
			printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(payResponse,"message"));
			goto end;
		}
		
		if(strcmp(jstr(payResponse,"status"),"complete")==0)
			printf("Payment Success\n");
		else
			retval=-1;
	}
	end:
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


int32_t BET_p2p_betting_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *invoiceInfo=NULL,*payResponse=NULL;
    char *invoice=NULL;
	int argc,retval=1;
	char **argv=NULL;
	int32_t playerID,bytes;
	char *rendered=NULL;
	cJSON *actionResponse=NULL;
	actionResponse=cJSON_CreateObject();

	actionResponse=cJSON_GetObjectItem(argjson,"actionResponse");
	
	argv=(char**)malloc(4*sizeof(char*));
	argc=3;
	for(int i=0;i<4;i++)
	{
		argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	playerID=jint(argjson,"playerID");
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);
	if(playerID==bet->myplayerid)
	{
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"pay");
		sprintf(argv[2],"%s",jstr(invoiceInfo,"bolt11"));
		argv[3]=NULL;
		payResponse=cJSON_CreateObject();

		make_command(argc,argv,&payResponse);

		if(jint(payResponse,"code") != 0)
		{
			retval=-1;
			goto end;
		}
		
		if(strcmp(jstr(payResponse,"status"),"complete")==0)
			printf("Payment Success\n");
		else
			retval=-1;

		rendered=cJSON_Print(actionResponse);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		if(bytes<0)
		{
			retval=-1;
			goto end;
		}
	}
	end:
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


int32_t BET_p2p_winner(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int argc,bytes,retval=1;
	char **argv=NULL,hexstr [65],*rendered=NULL;
	cJSON *invoiceInfo=NULL,*winnerInvoiceInfo=NULL;
	if(jint(argjson,"playerid")==bet->myplayerid)
	{
		argv =(char**)malloc(6*sizeof(char*));
		for(int i=0;i<5;i++)
		{
				argv[i]=(char*)malloc(sizeof(char)*1000);
		}
		
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"invoice");
		sprintf(argv[2],"%d",jint(argjson,"winning_amount"));
		sprintf(argv[3],"%s_%d",bits256_str(hexstr,player_info.deckid),jint(argjson,"winning_amount"));
		sprintf(argv[4],"Winning claim");
		argv[5]=NULL;
		argc=5;
		
		winnerInvoiceInfo=cJSON_CreateObject();
		make_command(argc,argv,&winnerInvoiceInfo);
		//ln_bet(argc,argv,buf);
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","claim");
		cJSON_AddNumberToObject(invoiceInfo,"playerid",bet->myplayerid);
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(winnerInvoiceInfo));

		rendered=cJSON_Print(invoiceInfo);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		if(bytes<0)
		{
			retval=-1;
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
			goto end;
		}
		
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

int32_t BET_p2p_bet_round(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t bet_amount,round,bytes,retval=1;
	cJSON *betInfo=NULL;
	char *rendered=NULL;
	round=jint(argjson,"round");
	printf("\nEnter Betting Amount in MilliSatoshis:");
	scanf("%d",&bet_amount);

	betInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(betInfo,"method","invoiceRequest");
	cJSON_AddNumberToObject(betInfo,"round",round);
	cJSON_AddNumberToObject(betInfo,"playerID",bet->myplayerid);
	cJSON_AddNumberToObject(betInfo,"betAmount",bet_amount);

	rendered=cJSON_Print(betInfo);

	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
			

	if(bytes<0)
	{
			retval=-1;
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
			goto end;
	}

	end:
		return retval;
	
}

void display_cards(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{

	char* cards[52] = {"2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC", 
					   "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD", 
					   "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH", 
					   "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS"};
	
	cJSON *initCardInfo=NULL,*holeCardInfo=NULL,*initInfo=NULL,*boardCardInfo=NULL;
	
	initInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(initInfo,"method","deal");
	
	initCardInfo=cJSON_CreateObject();
	cJSON_AddNumberToObject(initCardInfo,"dealer",0);
	
	
	holeCardInfo=cJSON_CreateArray();
	for(int32_t i=0;((i<no_of_hole_cards)&&(i<number_cards_drawn));i++)
	{
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateString(cards[player_card_values[i]]));
	}
	
	cJSON_AddItemToObject(initCardInfo,"holecards",holeCardInfo);

	boardCardInfo=cJSON_CreateArray();
	for(int32_t i=no_of_hole_cards;((i<hand_size)&&(i<number_cards_drawn));i++)
	{
		cJSON_AddItemToArray(boardCardInfo,cJSON_CreateString(cards[player_card_values[i]]));
	}
	
	cJSON_AddItemToObject(initCardInfo,"board",boardCardInfo);
	
	cJSON_AddItemToObject(initInfo,"deal",initCardInfo);
	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(initInfo));
	player_lws_write(initInfo);
	//lws_write(wsi_global_client,cJSON_Print(initInfo),strlen(cJSON_Print(initInfo)),0);
}
int32_t BET_p2p_client_receive_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,bytes,cardid,playerid,errs=0,unpermi,card_type;
	cJSON *playerCardInfo=NULL;
	char *rendered=NULL;
	bits256 share,decoded256;
	
	share=jbits256(argjson,"share");
	cardid=jint(argjson,"cardid");
	playerid=jint(argjson,"playerid");
	card_type=jint(argjson,"card_type");

	if(sharesflag[cardid][playerid] ==0 )
	{
			playershares[cardid][playerid]=share;
			sharesflag[cardid][playerid]=1;
			no_of_shares++;
	}
	
	
	if(no_of_shares == bet->maxplayers)
	{
		no_of_shares=0;
		decoded256 = BET_p2p_decode_card(argjson,bet,vars,cardid);	
		if ( bits256_nonz(decoded256) == 0 )
			errs++;
		else
		{
	        unpermi=-1;
            for(int k=0;k<bet->range;k++)
			{
                if(player_info.permis[k]==decoded256.bytes[30])
				{
                    unpermi=k;
                    break;
                }
            }
		}

		if(unpermi != -1)
		{
			player_card_values[number_cards_drawn++]=decoded256.bytes[30];
			playerCardInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(playerCardInfo,"method","playerCardInfo");
			cJSON_AddNumberToObject(playerCardInfo,"playerid",bet->myplayerid);
			cJSON_AddNumberToObject(playerCardInfo,"cardid",cardid);
			cJSON_AddNumberToObject(playerCardInfo,"card_type",card_type);
			cJSON_AddNumberToObject(playerCardInfo,"decoded_card",decoded256.bytes[30]);
			
			rendered=cJSON_Print(playerCardInfo);
			bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
			
			if(bytes<0)
			{
					retval=-1;
					printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
					goto end;
			}
			
		}

	}
	end:
		return retval;
}


int32_t BET_p2p_client_ask_share(struct privatebet_info *bet,int32_t cardid,int32_t playerid,int32_t card_type)
{
	cJSON *requestInfo=NULL;
	char *rendered=NULL;
	int32_t bytes,retval=1;

	requestInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(requestInfo,"method","requestShare");
	cJSON_AddNumberToObject(requestInfo,"playerid",playerid);
	cJSON_AddNumberToObject(requestInfo,"cardid",cardid);
	cJSON_AddNumberToObject(requestInfo,"card_type",card_type);

	rendered=cJSON_Print(requestInfo);
	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

	if(bytes<0)
		retval=-1;

	return retval;
}

int32_t BET_p2p_client_give_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,bytes,playerid,cardid,recvlen,card_type;
	cJSON *share_info=NULL;		
	char *rendered=NULL;
	struct enc_share temp;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr;
	bits256 share;

	playerid=jint(argjson,"playerid");
	cardid=jint(argjson,"cardid");
	card_type=jint(argjson,"card_type");

	if(playerid==bet->myplayerid)
		goto end;
	
	temp=g_shares[playerid*bet->numplayers*bet->range + (cardid*bet->numplayers + bet->myplayerid)];

    recvlen = sizeof(temp);

	if ( (ptr= BET_decrypt(decipher,sizeof(decipher),player_info.bvvpubkey,player_info.player_key.priv,temp.bytes,&recvlen)) == 0 )
	{
		retval=-1;
		printf("decrypt error \n");
		goto end;
	}	
    else
	{
		memcpy(share.bytes,ptr,recvlen);
		share_info=cJSON_CreateObject();
		cJSON_AddStringToObject(share_info,"method","share_info");
		cJSON_AddNumberToObject(share_info,"playerid",bet->myplayerid);
		cJSON_AddNumberToObject(share_info,"cardid",cardid);
		cJSON_AddNumberToObject(share_info,"card_type",card_type);
		jaddbits256(share_info,"share",share);
		
		rendered=cJSON_Print(share_info);
		bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

		if(bytes<0)
		{
			retval=-1;
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
		}	
		
	}
	end:
		return retval;
}

int32_t BET_p2p_get_own_share(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	struct enc_share temp;
	int32_t cardid,retval=1,playerid,recvlen;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr;
	bits256 share;
	char enc_share[177],str[65];
	playerid=jint(argjson,"playerid");
	cardid=jint(argjson,"cardid");
	
	
	temp=g_shares[bet->myplayerid*bet->numplayers*bet->range + (cardid*bet->numplayers + playerid)];
	recvlen = sizeof(temp);

	if ( (ptr= BET_decrypt(decipher,sizeof(decipher),player_info.bvvpubkey,player_info.player_key.priv,temp.bytes,&recvlen)) == 0 )
	{
		retval=-1;
		printf("decrypt error ");
		goto end;
	}
	else
	{
		memcpy(share.bytes,ptr,recvlen);
		playershares[cardid][bet->myplayerid]=share;
		sharesflag[cardid][bet->myplayerid]=1;
		
	}
	end:
		return retval;

}
/*
void BET_p2p_getshares(void *ptr)
{
	
}
*/
int32_t BET_p2p_client_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,playerid;

   	
	playerid=jint(argjson,"playerid");
	
	if(playerid == bet->myplayerid)
	{
		no_of_shares=1;
		retval=BET_p2p_get_own_share(argjson,bet,vars);
		if(retval == -1)
		{
			printf("Failing to get own share: Decryption Error");
			goto end;
		}

		for(int i=0;i<bet->numplayers;i++)
		{
			if((!sharesflag[jint(argjson,"cardid")][i]) && (i != bet->myplayerid))
			{
				retval=BET_p2p_client_ask_share(bet,jint(argjson,"cardid"),jint(argjson,"playerid"),jint(argjson,"card_type"));	
			}
		}

	}
	end:	
		return retval;
}


int32_t BET_p2p_client_player_ready(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *playerReady=NULL;
	char *rendered=NULL;
	int bytes,retval=1;
	
	playerReady=cJSON_CreateObject();
	cJSON_AddStringToObject(playerReady,"method","player_ready");
	cJSON_AddNumberToObject(playerReady,"playerid",bet->myplayerid);
	rendered=cJSON_Print(playerReady);
	bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);

	if(bytes<0)
		retval=-1;

	return retval;
}


int32_t BET_p2p_client_bvv_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)

{
		int32_t retval=1;
		bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
		cJSON *cjsonbvvblindcards,*cjsonshamirshards;
		bits256 temp,playerprivs[CARDS777_MAXCARDS];
		
		
		player_info.bvvpubkey=jbits256(argjson,"bvvpubkey");
		g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
		cjsonbvvblindcards=cJSON_GetObjectItem(argjson,"bvvblindcards");
		
		for(int i=0;i<bet->numplayers;i++)
		{
			for(int j=0;j<bet->range;j++)
			{
				player_info.bvvblindcards[i][j]=jbits256i(cjsonbvvblindcards,i*bet->range+j);
			}
		}


		cjsonshamirshards=cJSON_GetObjectItem(argjson,"shamirshards");
		int k=0;
		for(int playerid=0;playerid<bet->numplayers;playerid++)
		{
			for (int i=0; i<bet->range; i++)
	        {
	            for (int j=0; j<bet->numplayers; j++) 
				{
					g_shares[k]=get_API_enc_share(cJSON_GetArrayItem(cjsonshamirshards,k));
					k++;
	            }
	        }
		}


		
		for(int i=0;i<bet->range;i++)
		{
			for(int j=0;j<bet->range;j++)
			{
				temp=xoverz_donna(curve25519(player_info.player_key.priv,curve25519(playerprivs[i],player_info.cardprods[bet->myplayerid][j])));
				vcalc_sha256(0,v_hash[i][j].bytes,temp.bytes,sizeof(temp));
			}
		}
	retval=BET_p2p_client_player_ready(argjson,bet,vars);
	return retval;
}

int32_t BET_p2p_client_dcv_init(cJSON *dcv_info,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1;
	cJSON *cjsoncardprods,*cjsong_hash;
	
	player_info.deckid=jbits256(dcv_info,"deckid");
	cjsoncardprods=cJSON_GetObjectItem(dcv_info,"cardprods");
	
	for(int i=0;i<bet->numplayers;i++)
	{
		for(int j=0;j<bet->range;j++)
		{
			player_info.cardprods[i][j]=jbits256i(cjsoncardprods,i*bet->range+j);
		}
	}

	
	cjsong_hash=cJSON_GetObjectItem(dcv_info,"g_hash");
	

	for(int i=0;i<bet->numplayers;i++)
	{
		for(int j=0;j<bet->range;j++)
		{
			g_hash[i][j]=jbits256i(cjsong_hash,i*bet->range+j);
		
		}
	}

	
	
	return retval;
}

int32_t BET_p2p_client_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t bytes,retval=1;
	cJSON *cjsonplayercards,*init_p=NULL;
    char *rendered=NULL,str[65];

	init_p=cJSON_CreateObject();

	cJSON_AddStringToObject(init_p,"method","init_p");
	cJSON_AddNumberToObject(init_p,"peerid",bet->myplayerid);
	jaddbits256(init_p,"pubkey",player_info.player_key.prod);
	cJSON_AddItemToObject(init_p,"cardinfo",cjsonplayercards=cJSON_CreateArray());
	for(int i=0;i<bet->range;i++) 
	{
		cJSON_AddItemToArray(cjsonplayercards,cJSON_CreateString(bits256_str(str,player_info.cardpubkeys[i])));
	}
			
    rendered=cJSON_Print(init_p);
    bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
    if(bytes<0)
    {
    	retval=-1;
		printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
		goto end;
    }
    end:
		return retval;
}

int32_t LN_get_channel_status(char *id)
{
	int argc,channel_state=0;
	char **argv=NULL;
	cJSON *channelStateInfo=NULL,*channelStates=NULL,*channelState=NULL;
	argc=4;
    argv=(char**)malloc(argc*sizeof(char*));
    for(int i=0;i<argc;i++)
    {
     argv[i]=(char*)malloc(100*sizeof(char));
    }
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"peer-channel-state");
	strcpy(argv[2],id);
	argv[3]=NULL;
	argc=3;

	//channelStateInfo=cJSON_CreateObject();
    make_command(argc,argv,&channelStateInfo);

	//channelStates=cJSON_CreateObject();
	channelStates=cJSON_GetObjectItem(channelStateInfo,"channel-states");
	//channelState=cJSON_CreateObject();
	for(int i=0;i<cJSON_GetArraySize(channelStates);i++)
	{
		channelState=cJSON_GetArrayItem(channelStates,i);
		channel_state=jint(channelState,"channel-state");
		if(channel_state <= 3)
		{
				break;
		}
	
	}
	end:
		if(argv)
		{
			for(int i=0;i<4;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}
		if(channelStateInfo)
			cJSON_Delete(channelStateInfo);
		return channel_state;
}
int32_t BET_p2p_client_join_res(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	char uri[100];
	int argc,retval=1,channel_state,buf_size=100;
	char **argv=NULL,channel_id[100];
	cJSON *connectInfo=NULL,*fundChannelInfo=NULL;
	cJSON *initCardInfo=NULL,*holeCardInfo=NULL,*initInfo=NULL;
	if(0 == bits256_cmp(player_info.player_key.prod,jbits256(argjson,"pubkey")))
	{
		BET_player_global->myplayerid=jint(argjson,"peerid");
		bet->myplayerid=jint(argjson,"peerid");
		strcpy(uri,jstr(argjson,"uri"));
		strcpy(channel_id,strtok(jstr(argjson,"uri"), "@"));
		channel_state=LN_get_channel_status(channel_id);
		if((channel_state != 2)&&(channel_state !=3)) // 3 means channel is already established with the peer
		{
						
			argc=5;
            argv=(char**)malloc(argc*sizeof(char*));
            for(int i=0;i<argc;i++)
            {
             argv[i]=(char*)malloc(buf_size*sizeof(char));
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
				printf("%s:%d:Message:%s\n",__FUNCTION__,__LINE__,jstr(connectInfo,"method"));
				goto end;
			}
			
			argc=5;
			for(int i=0;i<argc;i++)
			{
				 memset(argv[i],0x00,buf_size);
			}

			argc=4;
			strcpy(argv[0],"lightning-cli");
			strcpy(argv[1],"fundchannel");
			strcpy(argv[2],jstr(connectInfo,"id"));
			//strcpy(argv[3],"500000");
			strcpy(argv[3],STRINGIZE(channel_fund_satoshis));

			fundChannelInfo=cJSON_CreateObject();
			make_command(argc,argv,&fundChannelInfo);
			
			cJSON_Print(fundChannelInfo);

			if(jint(fundChannelInfo,"code") != 0 )
			{
				retval=-1;
				printf("%s::%d::Message:%s\n",__FUNCTION__,__LINE__,jstr(fundChannelInfo,"message"));
				goto end;
			}
		}
		
		int state;
		while((state=LN_get_channel_status(channel_id)) != 3)
		{
			if(state == 2)
			 {
					  printf("CHANNELD_AWAITING_LOCKIN\r");
					  fflush(stdout);
			 }
			  else
		  	 {
		  		retval=-1;
				printf("\n%s:%d:Channel establishment with DCV is not happening, please check the connectivity with the DCV node\n",__FUNCTION__,__LINE__);
				//goto end;
		  	 }
			sleep(2);
		}
		printf("%s::%d::%d\n",__FUNCTION__,__LINE__,state);

		initCardInfo=cJSON_CreateObject();
		cJSON_AddNumberToObject(initCardInfo,"dealer",jint(argjson,"dealer"));
		holeCardInfo=cJSON_CreateArray();
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
		cJSON_AddItemToObject(initCardInfo,"holecards",holeCardInfo);

		initInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(initInfo,"method","deal");
		cJSON_AddItemToObject(initInfo,"deal",initCardInfo);
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(initInfo));

		player_lws_write(initInfo);
		//lws_write(wsi_global_client,cJSON_Print(initInfo),strlen(cJSON_Print(initInfo)),0);
		
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
cJSON* BET_rest_client_join(cJSON *argjson)
{
	cJSON *joininfo=NULL;
	struct pair256 key;
	char hexstr[65],*rendered=NULL;
	int bytes;
	
	key = deckgen_player(player_info.cardprivkeys,player_info.cardpubkeys,player_info.permis,2);
	printf("\nPublic Key:%s",bits256_str(hexstr,key.prod));
	player_info.player_key=key;
	joininfo=cJSON_CreateObject();
	cJSON_AddStringToObject(joininfo,"method","join_req");
	jaddbits256(joininfo,"pubkey",key.prod);
	return joininfo;
}
int32_t BET_p2p_client_join(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t permis[CARDS777_MAXCARDS],bytes,retval=-1;
	cJSON *joininfo=NULL,*channelInfo=NULL,*addresses=NULL,*address=NULL;
	struct pair256 key;
	char *rendered=NULL,*uri=NULL;
	char hexstr[65];
	int argc;
	char **argv=NULL;

	printf("%s:%d\n",__FUNCTION__,__LINE__);
    if(bet->pushsock>=0)
	{
		key = deckgen_player(player_info.cardprivkeys,player_info.cardpubkeys,player_info.permis,bet->range);
		player_info.player_key=key;
        joininfo=cJSON_CreateObject();
        cJSON_AddStringToObject(joininfo,"method","join_req");
        jaddbits256(joininfo,"pubkey",key.prod);    

		argv=(char**)malloc(4*sizeof(char*));
		argc=3;
		for(int i=0;i<argc;i++)
		{
			argv[i]=(char*)malloc(100*sizeof(char));
		}
		
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"getinfo");
		argv[2]=NULL;
		argc=2;
		channelInfo=cJSON_CreateObject();

		make_command(argc,argv,&channelInfo);

		if(jint(channelInfo,"code") != 0 )
		{
			retval=-1;
			printf("\n%s:%d:Message:%s",__FUNCTION__,__LINE__,jstr(channelInfo,"message"));
			goto end;
		}


		uri=(char*)malloc(sizeof(char)*100);
		
		strcpy(uri,jstr(channelInfo,"id"));
		strcat(uri,"@");
		
		addresses=cJSON_CreateObject();
		addresses=cJSON_GetObjectItem(channelInfo,"address");

		address=cJSON_CreateObject();
		address=cJSON_GetArrayItem(addresses,0);
		
		strcat(uri,jstr(address,"address"));
		cJSON_AddStringToObject(joininfo,"uri",uri);
		cJSON_AddNumberToObject(joininfo,"gui_playerID",jint(argjson,"gui_playerID"));

		rendered=cJSON_Print(joininfo);
        bytes=nn_send(bet->pushsock,rendered,strlen(rendered),0);
		if(bytes<0)
		{
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
			goto end;
		}
        retval=1;
    }
	end:
		if(uri)
			free(uri);
		if(argv)
		{
			for(int i=0;i<argc;i++)
				free(argv[i]);

			free(argv);
		}
		
		if(retval == -1)
			printf("%s::%d::Error\n",__FUNCTION__,__LINE__);
    	return retval;	
}


void BET_p2p_table_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	printf("\nTable Info:%s",cJSON_Print(argjson));	
}

int32_t BET_rest_player_reset(struct lws *wsi,cJSON * argjson)
{
	printf("\n%s:%d\n",__FUNCTION__,__LINE__);
	int32_t this_PlayerID=jint(argjson,"gui_playerID");
	all_no_of_shares[this_PlayerID]=0;
	all_no_of_player_cards[this_PlayerID]=0;

	for(int i=0;i<BET_player[this_PlayerID]->range;i++)
	{
		for(int j=0;j<BET_player[this_PlayerID]->numplayers;j++)
		{
			all_sharesflag[this_PlayerID][i][j]=0;
		}
	}
	all_number_cards_drawn[this_PlayerID]=0;

	for(int i=0;i<hand_size;i++)
	{
		all_player_card_matrix[this_PlayerID][i]=0;
		all_player_card_values[this_PlayerID][i]=-1;
	}

	
		
	
	Player_VARS[this_PlayerID]->pot=0;
	Player_VARS[this_PlayerID]->player_funds=10000000; // hardcoded to 10000 satoshis
	for(int i=0;i<BET_player[this_PlayerID]->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			Player_VARS[this_PlayerID]->bet_actions[i][j]=0;
			Player_VARS[this_PlayerID]->betamount[i][j]=0;
		}
	}
	player_joined=0;
	
	memset(&all_players_info, 0, sizeof all_players_info);
	memset(BET_player, 0, sizeof *BET_player);//(CARDS777_MAXPLAYERS *sizeof (struct privatebet_info))	
	
	//return(BET_rest_player_join(wsi,argjson));
	return 1;
}


int32_t BET_player_reset(struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *resetInfo=NULL;
	printf("\n%s:%d\n",__FUNCTION__,__LINE__);
	no_of_shares=0;
	no_of_player_cards=0;
	for(int i=0;i<bet->range;i++)
	{
		for(int j=0;j<bet->numplayers;j++)
		{
			sharesflag[i][j]=0;
		}
	}
	number_cards_drawn=0;	
	for(int i=0;i<hand_size;i++)
	{
		player_card_matrix[i]=0;
		player_card_values[i]=-1;
	}
	
	vars->pot=0;
	vars->player_funds=10000000; // hardcoded to 10000 satoshis
	for(int i=0;i<bet->maxplayers;i++)
	{
		for(int j=0;j<CARDS777_MAXROUNDS;j++)
		{
			vars->bet_actions[i][j]=0;
			vars->betamount[i][j]=0;
		}
	}
	
	//return(BET_p2p_client_join(NULL,bet,vars));
	resetInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(resetInfo,"method","reset");
	//BET_push_client(resetInfo);
	player_lws_write(resetInfo);
	return 1;
}


int32_t BET_p2p_rest_clientupdate(struct lws *wsi,cJSON *argjson) // update game state based on host broadcast
{
	
	int32_t retval=1;
    char *method; 
	char hexstr[65];
	struct privatebet_vars *vars=NULL;
	struct privatebet_info *bet=NULL;
    if ( (method= jstr(argjson,"method")) != 0 )
    {
    	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
		if ( strcmp(method,"player_join") == 0 )
		{
			retval=BET_p2p_client_join(argjson,BET_player_global,vars);
		}
		else if ( strcmp(method,"join_res") == 0 )
		{
			BET_rest_player_join_res(argjson);
			
		}
		else if ( strcmp(method,"TableInfo") == 0 )
		{
			BET_p2p_table_info(argjson,bet,vars);
			
		}
		else if ( strcmp(method,"init") == 0 )
		{
			
    		BET_rest_player_init(wsi_global_client,argjson);
			
		}
		else if(strcmp(method,"init_d") == 0)
		{
			retval=BET_p2p_client_dcv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"init_b") == 0)
		{
			retval=BET_p2p_client_bvv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"turn") == 0)
		{
			
			retval=BET_p2p_client_turn(argjson,bet,vars);
		}
		else if(strcmp(method,"ask_share") == 0)
		{
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"requestShare") == 0)
		{
		
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"share_info") == 0)
		{
			retval=BET_p2p_client_receive_share(argjson,bet,vars);
		}
		else if(strcmp(method,"bet") == 0)
		{
			retval=BET_p2p_bet_round(argjson,bet,vars);
		}
		else if(strcmp(method,"invoice") == 0)
		{
			retval=BET_p2p_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"winner") == 0)
		{
			retval=BET_p2p_winner(argjson,bet,vars);
		}
		else if(strcmp(method,"betting") == 0)
		{
			retval=BET_player_round_betting_test(argjson,BET_player_global,Player_VARS_global);
		}
		else if(strcmp(method,"display_current_state") == 0)
		{
			retval=BET_p2p_display_current_state(argjson,bet,vars);
		}
		else if(strcmp(method,"dealer") == 0)
		{
			retval=BET_p2p_dealer_info(argjson,bet,vars);
		}
		else if(strcmp(method,"invoiceRequest_player") == 0)
		{
			retval=BET_player_create_invoice(argjson,bet,vars,bits256_str(hexstr,player_info.deckid));
		}
		else if(strcmp(method,"reset") == 0)
		{
			retval=BET_player_reset(bet,vars);
		}
		else if(strcmp(method,"seats") == 0)
		{
			//lws_write(wsi_global_client,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
			player_lws_write(argjson);
		}
	}	
	return retval;
}


#if 1
char lws_buf_1[65536];
int32_t lws_buf_length_1=0;

int lws_callback_http_dummy1(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
        int ret_val,ret_len;
      	pthread_t player_t;
        cJSON *argjson=NULL;
		switch(reason)
        {
            case LWS_CALLBACK_RECEIVE:
				memcpy(lws_buf_1+lws_buf_length_1,in,len);
				lws_buf_length_1+=len;
				if (!lws_is_final_fragment(wsi))
						break;
				argjson=cJSON_Parse(lws_buf_1);
				if( BET_p2p_rest_clientupdate(wsi,argjson) != 1 )
				{
					printf("\n%s:%d:Failed to process the host command",__FUNCTION__,__LINE__);
				}
				memset(lws_buf_1,0x00,sizeof(lws_buf_1));
				lws_buf_length_1=0;
		
                break;
			case LWS_CALLBACK_ESTABLISHED:
				wsi_global_client=wsi;		  
				printf("%s:%d::LWS_CALLBACK_ESTABLISHED\n",__FUNCTION__,__LINE__);
				break;
			case LWS_CALLBACK_SERVER_WRITEABLE:
				printf("%s::%d::LWS_CALLBACK_SERVER_WRITEABLE\n",__FUNCTION__,__LINE__);
				if(data_exists)
				{
						//char *rendered=cJSON_Print(dataToWrite);
						//printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(dataToWrite));
						//lws_write(wsi,cJSON_Print(dataToWrite),strlen(cJSON_Print(dataToWrite)),0);
						if(guiData)
						{
							printf("%s::%d::%s\n",__FUNCTION__,__LINE__,guiData);
							lws_write(wsi,guiData,strlen(guiData),0);
							data_exists=0;
							
						}					
				}	
				break;
			default:
				printf("%s::%d::reason::%d\n",__FUNCTION__,__LINE__,reason);
        }
        return 0;
}



static struct lws_protocols protocols1[] = {
	{ "http", lws_callback_http_dummy1, 0, 0 },
	LWS_PLUGIN_PROTOCOL_MINIMAL,
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted1;



static const struct lws_http_mount mount1 = {
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

void player_sigint_handler(int sig)
{
	interrupted1 = 1;
}

void BET_test_function(void* _ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context=NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	//signal(SIGINT,player_sigint_handler);
	lws_set_log_level(logs, NULL);
	
	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
    dcv_info.port = 9000;
    dcv_info.mounts = &mount1;
    dcv_info.protocols = protocols1;
    dcv_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    dcv_context = lws_create_context(&dcv_info);
    if (!dcv_context) {
        printf("lws init failed\n");
        
    }   
   while (n >= 0 && !interrupted1)
	{
        n = lws_service(dcv_context, 1000);
	}
   if(dcv_context)
   		lws_context_destroy(dcv_context);
}
#endif


#if 1
char lws_buf_bvv[2000];
int32_t lws_buf_length_bvv=0;

int lws_callback_http_dummy_bvv(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
        cJSON *argjson=NULL;
		switch(reason)
        {
            case LWS_CALLBACK_RECEIVE:
				memcpy(lws_buf_bvv+lws_buf_length_bvv,in,len);
				lws_buf_length_bvv+=len;
				if (!lws_is_final_fragment(wsi))
						break;
				argjson=cJSON_Parse(lws_buf_bvv);
				printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
				if( BET_p2p_bvvcommand_test(wsi,argjson) != 1 )
				{
					printf("\n%s:%d:Failed to process the host command",__FUNCTION__,__LINE__);
				}
				memset(lws_buf_bvv,0x00,sizeof(lws_buf_bvv));
				lws_buf_length_bvv=0;
		        break;
			case LWS_CALLBACK_ESTABLISHED:
				wsi_global_bvv=wsi;
				printf("\n%s:%d::LWS_CALLBACK_ESTABLISHED\n",__FUNCTION__,__LINE__);
				break;
        }
        return 0;
}



static struct lws_protocols protocols_bvv[] = {
	{ "http", lws_callback_http_dummy_bvv, 0, 0 },
	LWS_PLUGIN_PROTOCOL_MINIMAL,
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted_bvv;



static const struct lws_http_mount mount_bvv = {
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

void BET_test_function_bvv(void* _ptr)
{
	struct lws_context_creation_info dcv_info,bvv_info;
	struct lws_context *dcv_context;
	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	printf("\n%s::%d",__FUNCTION__,__LINE__);
	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:7681\n");
	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
    dcv_info.port = 9000;
    dcv_info.mounts = &mount_bvv;
    dcv_info.protocols = protocols_bvv;
    dcv_info.options =
    LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    dcv_context = lws_create_context(&dcv_info);
    if (!dcv_context) {
        printf("lws init failed\n");
        
    }   
   while (n >= 0 && !interrupted_bvv)
	{
        n = lws_service(dcv_context,1000);
	}
    lws_context_destroy(dcv_context);
}
#endif


int32_t BET_p2p_clientupdate(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars) // update game state based on host broadcast
{
	
    static uint8_t *decoded; static int32_t decodedlen,retval=1;
    char *method; int32_t senderid; bits256 *MofN;
	char hexstr[65];
    if ( (method= jstr(argjson,"method")) != 0 )
    {
	      
    	if ( strcmp(method,"join") == 0 )
		{
			retval=BET_p2p_client_join(argjson,bet,vars);

		}
		else if ( strcmp(method,"join_res") == 0 )
		{
			//retval=BET_p2p_client_join_res(argjson,bet,vars);
			printf("\n%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			BET_rest_player_join_res(argjson);
			
		}
		else if ( strcmp(method,"TableInfo") == 0 )
		{
			BET_p2p_table_info(argjson,bet,vars);
			
		}
		else if ( strcmp(method,"init") == 0 )
		{
			
            //retval=BET_p2p_client_init(argjson,bet,vars);
   			printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			BET_rest_player_init(wsi_global_client,argjson);
			
		}
		else if(strcmp(method,"init_d") == 0)
		{
			retval=BET_p2p_client_dcv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"init_b") == 0)
		{
			retval=BET_p2p_client_bvv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"turn") == 0)
		{
			
			retval=BET_p2p_client_turn(argjson,bet,vars);
		}
		else if(strcmp(method,"ask_share") == 0)
		{
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"requestShare") == 0)
		{
		
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"share_info") == 0)
		{
			retval=BET_p2p_client_receive_share(argjson,bet,vars);
		}
		else if(strcmp(method,"bet") == 0)
		{
			retval=BET_p2p_bet_round(argjson,bet,vars);
		}
		else if(strcmp(method,"invoice") == 0)
		{
			retval=BET_p2p_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"winner") == 0)
		{
			retval=BET_p2p_winner(argjson,bet,vars);
		}
		else if(strcmp(method,"betting") == 0)
		{
			retval=BET_p2p_betting_statemachine(argjson,bet,vars);
		}
		else if(strcmp(method,"display_current_state") == 0)
		{
			retval=BET_p2p_display_current_state(argjson,bet,vars);
		}
		else if(strcmp(method,"dealer") == 0)
		{
			retval=BET_p2p_dealer_info(argjson,bet,vars);
		}
		else if(strcmp(method,"invoiceRequest_player") == 0)
		{
			retval=BET_player_create_invoice(argjson,bet,vars,bits256_str(hexstr,player_info.deckid));
		}
		else if(strcmp(method,"reset") == 0)
		{
			retval=BET_player_reset(bet,vars);
		}
		else if(strcmp(method,"seats") == 0)
		{
			printf("\n%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			//lws_write(wsi_global_client,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
			player_lws_write(argjson);
		}
	}	
	return retval;
}

void BET_push_client(cJSON *argjson)
{
	char *rendered=NULL;
	printf("%s::%d\n",__FUNCTION__,__LINE__);
	player_lws_write(argjson);
	/*
	if(argjson)
	{
		rendered=cJSON_Print(argjson);
		lws_write(wsi_global_client,(unsigned char*)rendered,strlen(rendered),0);
	}
	*/
}

void BET_push_client_blindInfo(cJSON *blindInfo)
{
	char *rendered=NULL;
	player_lws_write(blindInfo);
	/*
	if(blindInfo)
	{
		rendered=cJSON_Print(blindInfo);
		lws_write(wsi_global_client,(unsigned char*)rendered,strlen(rendered),0);
	}
	*/
}

void BET_p2p_player_blinds_info()
{
	cJSON *blindsInfo=NULL;
	blindsInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(blindsInfo,"method","blindsInfo");
	cJSON_AddNumberToObject(blindsInfo,"small_blind",small_blind_amount);
	cJSON_AddNumberToObject(blindsInfo,"big_blind",big_blind_amount);
	printf("%s::%d::lws::%s\n",__FUNCTION__,__LINE__,jstr(blindsInfo,"method"));

	player_lws_write(blindsInfo);
	
	//rendered=cJSON_Print(blindsInfo);
	//lws_write(wsi,rendered,strlen(rendered),0);
}

int32_t BET_p2p_clientupdate_test(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars) // update game state based on host broadcast
{
	
    int32_t retval=1;
    char *method=NULL; 
	char hexstr[65];
	char *rendered=NULL;
	
    if ( (method= jstr(argjson,"method")) != 0 )
    {
    	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,method);
	      
    	if ( strcmp(method,"join") == 0 )
		{
			
			retval=BET_p2p_client_join(argjson,bet,vars);

		}
		else if ( strcmp(method,"join_res") == 0 )
		{
			retval=BET_p2p_client_join_res(argjson,bet,vars);
			
			//retval=BET_rest_player_join_res(wsi_global_client,argjson);
			
		}
		else if ( strcmp(method,"TableInfo") == 0 )
		{
			BET_p2p_table_info(argjson,bet,vars);
			
		}
		else if ( strcmp(method,"init") == 0 )
		{
			if(jint(argjson,"peerid")==bet->myplayerid)
			{
				printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
				//BET_p2p_host_blinds_info(wsi_global_client);
				BET_p2p_player_blinds_info();
				retval=BET_p2p_client_init(argjson,bet,vars);
			}
   		}
		else if(strcmp(method,"init_d") == 0)
		{
			retval=BET_p2p_client_dcv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"init_b") == 0)
		{
			retval=BET_p2p_client_bvv_init(argjson,bet,vars);
		}
		else if(strcmp(method,"turn") == 0)
		{
			
			retval=BET_p2p_client_turn(argjson,bet,vars);
		}
		else if(strcmp(method,"ask_share") == 0)
		{
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"requestShare") == 0)
		{
		
			retval=BET_p2p_client_give_share(argjson,bet,vars);
		}
		else if(strcmp(method,"share_info") == 0)
		{
			retval=BET_p2p_client_receive_share(argjson,bet,vars);
		}
		else if(strcmp(method,"bet") == 0)
		{
			retval=BET_p2p_bet_round(argjson,bet,vars);
		}
		else if(strcmp(method,"invoice") == 0)
		{
			retval=BET_p2p_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"bettingInvoice") == 0)
		{
			retval=BET_p2p_betting_invoice(argjson,bet,vars);
		}
		else if(strcmp(method,"winner") == 0)
		{
			retval=BET_p2p_winner(argjson,bet,vars);
		}
		else if(strcmp(method,"betting") == 0)
		{
			retval=BET_p2p_betting_statemachine(argjson,bet,vars);
		}
		else if(strcmp(method,"display_current_state") == 0)
		{
			retval=BET_p2p_display_current_state(argjson,bet,vars);
		}
		else if(strcmp(method,"dealer") == 0)
		{
			retval=BET_p2p_dealer_info(argjson,bet,vars);
		}
		else if(strcmp(method,"invoiceRequest_player") == 0)
		{
			retval=BET_player_create_invoice(argjson,bet,vars,bits256_str(hexstr,player_info.deckid));
		}
		else if(strcmp(method,"reset") == 0)
		{
			retval=BET_player_reset(bet,vars);
		}
		else if(strcmp(method,"seats") == 0)
		{
			cJSON_AddNumberToObject(argjson,"playerFunds",BET_rest_listfunds());
			player_lws_write(argjson);
			//rendered=cJSON_Print(argjson);
			//lws_write(wsi_global_client,rendered,strlen(rendered),0);
		}
		else if(strcmp(method,"finalInfo") == 0)
		{
			printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
			//lws_write(wsi_global_client,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
			player_lws_write(argjson);			
		}
	}	
	return retval;
}


void BET_p2p_clientloop_test(void * _ptr)
{
	int32_t recvlen=0; 
	void *ptr=NULL; 
	cJSON *msgjson=NULL; struct privatebet_info *bet = _ptr;
    uint8_t flag=1;

	
    while ( flag )
    {
        
        if ( bet->subsock >= 0 && bet->pushsock >= 0 )
        {
        		ptr=0;
				char *tmp=NULL;
	        	recvlen= nn_recv (bet->subsock, &ptr, NN_MSG, 0);
				if(recvlen>0)
					tmp=clonestr(ptr);
                if ((recvlen>0) && ((msgjson= cJSON_Parse(tmp)) != 0 ))
                {
                    if ( BET_p2p_clientupdate_test(msgjson,bet,Player_VARS_global) < 0 )
                    {
                    	printf("\nFAILURE\n");
                    	// do something here, possibly this could be because unknown commnad or because of encountering a special case which state machine fails to handle
                    }           
                    if(tmp)
						free(tmp);
					if(ptr)
						nn_freemsg(ptr);
					
                }
                
        }
        
    }
#if 0
	
	while ( bet->subsock>= 0 && bet->pushsock>= 0 )
	 {
		 if ( (recvlen= nn_recv(bet->subsock,&ptr,NN_MSG,0)) > 0 )
		 {
			 if ( (msgjson= cJSON_Parse(ptr)) != 0 )
			 {
				 if ( BET_p2p_clientupdate_test(msgjson,bet,Player_VARS_global) != 0 ) 
				 {
					 // Do something
				 }
				 
			 }
			 if(ptr)
				 nn_freemsg(ptr);
		 }
		   
	 }
#endif
}



void BET_p2p_clientloop(void * _ptr)
{
    uint32_t lasttime = 0; int32_t nonz,recvlen,lastChips_paid; uint16_t port=7798; char connectaddr[64],hostip[64]; void *ptr; cJSON *msgjson,*reqjson; struct privatebet_vars *VARS; struct privatebet_info *bet = _ptr;
    VARS = calloc(1,sizeof(*VARS));
    uint8_t flag=1;

	msgjson=cJSON_CreateObject();
	/*
	cJSON_AddStringToObject(msgjson,"method","join");
	if ( BET_p2p_clientupdate(msgjson,bet,VARS) < 0 )
	{
        flag=0;
		printf("\n%s:%d:Player joining the table failed",__FUNCTION__,__LINE__);
	}
	*/
    while ( flag )
    {
        
        if ( bet->subsock >= 0 && bet->pushsock >= 0 )
        {
	        	recvlen= nn_recv (bet->subsock, &ptr, NN_MSG, 0);
                if (( (msgjson= cJSON_Parse(ptr)) != 0 ) && (recvlen>0))
                {
                    if ( BET_p2p_clientupdate(msgjson,bet,VARS) < 0 )
                    {
                    	printf("\nFAILURE\n");
                    	// do something here, possibly this could be because unknown commnad or because of encountering a special case which state machine fails to handle
                    }           
                   
                    free_json(msgjson);
                }
                
        }
        
    }
}


/*
The below API's are relate to BVV and Player functionalities
*/
int32_t BET_rest_listfunds()
{
	cJSON *listFunds=NULL,*outputs=NULL;
	int argc,bytes,retval=1;
	char **argv=NULL,*uri=NULL;
	int32_t value=0;
	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"listfunds");
	argv[2]=NULL;

	listFunds=cJSON_CreateObject();
	make_command(argc-1,argv,&listFunds);

	/*
	ln_bet(argc-1,argv,buf);
	listFunds=cJSON_Parse(buf);
	*/
	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(listFunds));

	if(jint(listFunds,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(listFunds,"message"));
		goto end;
	}

	outputs=cJSON_GetObjectItem(listFunds,"outputs");
	for(int32_t i=0;i<cJSON_GetArraySize(outputs);i++)
	{
		value+=jint(cJSON_GetArrayItem(outputs,i),"value");
		
	}

	end:
		
		if(argv)
		{
			for(int i=0;i<3;i++)
			{
				if(argv[i])
					free(argv[i]);
			}
			free(argv);
		}

		free_json(listFunds);
		
		return value;
}


int32_t BET_rest_uri(char **uri)
{
	cJSON *channelInfo=NULL,*addresses,*address,*bvvResponseInfo=NULL;
	int argc,bytes,retval=1;
	char **argv=NULL;
	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
		argv[i]=(char*)malloc(100*sizeof(char));
	

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"getinfo");
	argv[2]=NULL;
	channelInfo=cJSON_CreateObject();
	make_command(argc-1,argv,&channelInfo);
	/*
	ln_bet(argc-1,argv,buf);
	channelInfo=cJSON_Parse(buf);
	*/
	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(channelInfo));
	if(jint(channelInfo,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(channelInfo,"message"));
		goto end;
	}
	
	strcpy(*uri,jstr(channelInfo,"id"));
	strcat(*uri,"@");
	addresses=cJSON_GetObjectItem(channelInfo,"address");
	address=cJSON_GetArrayItem(addresses,0);
	strcat(*uri,jstr(address,"address"));

   end:

	if(argv)
	{
		
		for(int i=0;i<3;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
	
}


int32_t BET_rest_bvv_init(struct lws *wsi, cJSON *argjson)
{
	int32_t numplayers=2,range=52;
	cJSON *bvvJoinInfo=NULL;
    int32_t Maxplayers=2;
	char *uri=NULL;
	BET_bvv=calloc(1,sizeof(struct privatebet_info));
    //BET_bvv->subsock = subsock/*BET_nanosock(0,bindaddr,NN_SUB)*/;
    //BET_bvv->pushsock = pushsock/*BET_nanosock(0,bindaddr1,NN_PUSH)*/;
    BET_bvv->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
    BET_bvv->maxchips = CARDS777_MAXCHIPS;
    BET_bvv->chipsize = CARDS777_CHIPSIZE;
	BET_bvv->numplayers=numplayers;
	BET_bvv->myplayerid=-1;
    BET_betinfo_set(BET_bvv,"demo",range,0,Maxplayers);

			
    BVV_VARS = calloc(1,sizeof(*BVV_VARS));

	BET_permutation(bvv_info.permis,BET_bvv->range);
    for(int i=0;i<BET_bvv->range;i++) {
		permis_b[i]=bvv_info.permis[i];
	}
	
	BET_rest_uri(&uri);
	bvvJoinInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvJoinInfo,"method","bvv_join");
	cJSON_AddStringToObject(bvvJoinInfo,"uri",uri);
	cJSON_AddNumberToObject(bvvJoinInfo,"balance",BET_rest_listfunds());
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(bvvJoinInfo));
	lws_write(wsi,cJSON_Print(bvvJoinInfo),strlen(cJSON_Print(bvvJoinInfo)),0);
	
	return 0;
}


int32_t BET_rest_bvv(struct lws *wsi, cJSON *argjson)
{
	cJSON *bvvInfo=NULL,*bvvKeyInfo=NULL;

	bvv_info.bvv_key.priv=curve25519_keypair(&bvv_info.bvv_key.prod);

	bvvKeyInfo=cJSON_CreateObject();
	jaddbits256(bvvKeyInfo,"pubkey",bvv_info.bvv_key.prod);

	bvvInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvInfo,"bvv",cJSON_Print(bvvKeyInfo));

	printf("\n%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(bvvInfo));
	
	lws_write(wsi,cJSON_Print(bvvInfo),strlen(cJSON_Print(bvvInfo)),0);
	return 0;
}

int32_t BET_rest_player(struct lws *wsi, cJSON *argjson)
{
	cJSON *playerInfo=NULL,*playerKeyInfo=NULL;
	
	player_info.player_key.priv=curve25519_keypair(&player_info.player_key.prod);

	playerKeyInfo=cJSON_CreateObject();
	jaddbits256(playerKeyInfo,"pubkey",player_info.player_key.prod);

	playerInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerInfo,"player",cJSON_Print(playerKeyInfo));

	printf("\n%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(playerInfo));
	
	lws_write(wsi,cJSON_Print(playerInfo),strlen(cJSON_Print(playerInfo)),0);
	return 0;
}


int32_t BET_rest_player_join(struct lws *wsi, cJSON *argjson)
{
	
	bits256 playerprivs[CARDS777_MAXCARDS],playercards[CARDS777_MAXCARDS];
	int32_t permis[CARDS777_MAXCARDS];
	cJSON *joinInfo=NULL;
	char *uri=NULL;
	char *rendered=NULL;
	struct pair256 key;
	int32_t Maxplayers=2,numplayers=2,range=52;
	int32_t bytes=0;

	if(player_joined)
		return 0; //this logic is to avoid the double clicks
	else
		player_joined=1;
	
	for(int32_t i=0;i<Maxplayers;i++)
		BET_player[i]=calloc(1,sizeof(struct privatebet_info));	
	
	//BET_player[player_id]=calloc(1,sizeof(struct privatebet_info));

	player_id=jint(argjson,"gui_playerID");
	BET_player[player_id]->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
	BET_player[player_id]->maxchips = CARDS777_MAXCHIPS;
	BET_player[player_id]->chipsize = CARDS777_CHIPSIZE;
	BET_player[player_id]->numplayers=numplayers;
	BET_betinfo_set(BET_player[player_id],"demo",range,0,Maxplayers);
	BET_player[player_id]->myplayerid=player_id;
		
	key = deckgen_player(all_players_info[player_id].cardprivkeys,all_players_info[player_id].cardpubkeys,all_players_info[player_id].permis,BET_player[player_id]->range);
	all_players_info[player_id].player_key=key;

    joinInfo=cJSON_CreateObject();
    cJSON_AddStringToObject(joinInfo,"method","join_req");
	jaddnum(joinInfo,"player_id",player_id);
	jaddbits256(joinInfo,"pubkey",key.prod);

	
	uri=(char*)malloc(sizeof(char)*200);
	memset(uri,0x00,sizeof(uri));
	BET_rest_uri(&uri);
	printf("%s::%d::uri::%s\n",__FUNCTION__,__LINE__,uri);
	
	cJSON_AddStringToObject(joinInfo,"uri",uri);
	cJSON_AddNumberToObject(joinInfo,"balance",BET_rest_listfunds());
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(joinInfo));
	lws_write(wsi,cJSON_Print(joinInfo),strlen(cJSON_Print(joinInfo)),0);
	player_id++;	

	rendered=cJSON_Print(joinInfo);
    bytes=nn_send(BET_player_global->pushsock,rendered,strlen(rendered),0);
	
	if(bytes<0)
		printf("\n%s:%d::Failed to push the data to DCV\n",__FUNCTION__,__LINE__);
	
	end:
	if(uri)
		free(uri);
	return 0;
}


int32_t BET_rest_player_init(struct lws *wsi, cJSON *argjson)
{
	cJSON *cjsonplayercards,*init_p=NULL;
    char str[65];
	int32_t playerID;

	playerID=jint(argjson,"gui_playerID");
	init_p=cJSON_CreateObject();

	cJSON_AddStringToObject(init_p,"method","init_p");
	cJSON_AddNumberToObject(init_p,"peerid",playerID);
	jaddbits256(init_p,"pubkey",all_players_info[playerID].player_key.prod);
	cJSON_AddItemToObject(init_p,"cardinfo",cjsonplayercards=cJSON_CreateArray());
	for(int i=0;i<BET_player[playerID]->range;i++) 
	{
		cJSON_AddItemToArray(cjsonplayercards,cJSON_CreateString(bits256_str(str,all_players_info[playerID].cardpubkeys[i])));
	}
	printf("\nsent:::%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(init_p));
	//lws_write(wsi,cJSON_Print(init_p),strlen(cJSON_Print(init_p)),0);
	player_lws_write(init_p);
	
	return 0;
}

bits256 BET_get_deckid(int32_t playerID)
{
	return all_players_info[playerID].deckid;
}

int32_t BET_rest_player_process_init_d(struct lws *wsi, cJSON *argjson,int32_t playerID)
{
	int32_t retval=1;
	cJSON *cjsoncardprods,*cjsong_hash;
	char hexstr [ 65 ];

	printf("%s:%d::playerID:%d\n",__FUNCTION__,__LINE__,playerID);
	all_players_info[playerID].deckid=jbits256(argjson,"deckid");
	cjsoncardprods=cJSON_GetObjectItem(argjson,"cardprods");
	
	for(int i=0;i<BET_player[playerID]->numplayers;i++)
	{
		for(int j=0;j<BET_player[playerID]->range;j++)
		{
			all_players_info[playerID].cardprods[i][j]=jbits256i(cjsoncardprods,i*BET_player[playerID]->range+j);
		}
	}

	
	cjsong_hash=cJSON_GetObjectItem(argjson,"g_hash");
	

	for(int i=0;i<BET_player[playerID]->numplayers;i++)
	{
		for(int j=0;j<BET_player[playerID]->range;j++)
		{
			all_g_hash[playerID][i][j]=jbits256i(cjsong_hash,i*BET_player[playerID]->range+j);
		
		}
	}

	
	
	return 0;
}


int32_t BET_rest_bvv_check_bvv_ready(struct lws *wsi, cJSON *argjson)
{
	cJSON *bvvReady=NULL;	

	bvvReady=cJSON_CreateObject();
	cJSON_AddStringToObject(bvvReady,"method","bvv_ready");
	lws_write(wsi,cJSON_Print(bvvReady),strlen(cJSON_Print(bvvReady)),0);
	
	return 0;
}

int32_t BET_rest_bvv_compute_init_b(struct lws *wsi, cJSON *argjson)
{
	int32_t bytes,retval=1;
	char *rendered,str[65],enc_str[177];
	cJSON *cjsondcvblindcards,*cjsonpeerpubkeys,*bvv_init_info,*cjsonbvvblindcards,*cjsonshamirshards;
	bits256 dcvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],peerpubkeys[CARDS777_MAXPLAYERS];
	bits256 bvvblindingvalues[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	
	bvv_info.numplayers=BET_bvv->numplayers;
	bvv_info.maxplayers=BET_bvv->maxplayers;
	bvv_info.deckid=jbits256(argjson,"deckid");
	bvv_info.bvv_key.priv=curve25519_keypair(&bvv_info.bvv_key.prod);
	cjsonpeerpubkeys=cJSON_GetObjectItem(argjson,"peerpubkeys");
	cjsondcvblindcards=cJSON_GetObjectItem(argjson,"dcvblindcards");
	
	for(uint32_t playerID=0;playerID<bvv_info.maxplayers;playerID++) 
	{
		peerpubkeys[playerID]=jbits256i(cjsonpeerpubkeys,playerID);
		for(int i=0;i<BET_bvv->range;i++) 
		{
			dcvblindcards[playerID][i]=jbits256i(cjsondcvblindcards,playerID*BET_bvv->range+i); //bvv_info.maxplayers
		}
	}
	
    g_shares=(struct enc_share*)malloc(BET_bvv->maxplayers*BET_bvv->maxplayers*BET_bvv->range*sizeof(struct enc_share));
	
	
	for (uint32_t playerid=0; playerid<bvv_info.maxplayers; playerid++)
	{
		p2p_bvv_init(peerpubkeys,bvv_info.bvv_key,bvvblindingvalues[playerid],bvvblindcards[playerid],
			dcvblindcards[playerid],BET_bvv->range,bvv_info.numplayers,playerid,bvv_info.deckid);
		//sleep(5);

	}
	
	bvv_init_info=cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_init_info,"method","init_b");
	jaddbits256(bvv_init_info,"bvvpubkey",bvv_info.bvv_key.prod);
	cJSON_AddItemToObject(bvv_init_info,"bvvblindcards",cjsonbvvblindcards=cJSON_CreateArray());
	for(uint32_t i=0;i<bvv_info.numplayers;i++)
	{
		for(int j=0;j<BET_bvv->range;j++)
		{
			cJSON_AddItemToArray(cjsonbvvblindcards,cJSON_CreateString(bits256_str(str,bvvblindcards[i][j])));
		}
	}
	cJSON_AddItemToObject(bvv_init_info,"shamirshards",cjsonshamirshards=cJSON_CreateArray());
	int k=0;
	for(uint32_t playerid=0;playerid<bvv_info.numplayers;playerid++)
	{
		for(int i=0;i<BET_bvv->range;i++)
		{
			for(uint32_t j=0;j<bvv_info.numplayers;j++)
			{
				cJSON_AddItemToArray(cjsonshamirshards,cJSON_CreateString(enc_share_str(enc_str,g_shares[k++])));
			}
		}
	}

	char *temp_buf=NULL;
	temp_buf=(char*)malloc(strlen(cJSON_Print(bvv_init_info))+LWS_PRE);
	strncpy(temp_buf+LWS_PRE,cJSON_Print(bvv_init_info),strlen(cJSON_Print(bvv_init_info)));
	temp_buf[strlen(cJSON_Print(bvv_init_info))+LWS_PRE]='\0';
	lws_write(wsi,temp_buf+LWS_PRE,strlen(cJSON_Print(bvv_init_info)),0);

	return 0;
	
}

int32_t BET_rest_player_ready(struct lws *wsi,int32_t playerID)
{
	cJSON *playerReady=NULL;
	char *rendered=NULL;
	int bytes,retval=1;
	
	playerReady=cJSON_CreateObject();
	cJSON_AddStringToObject(playerReady,"method","player_ready");
	cJSON_AddNumberToObject(playerReady,"playerid",playerID);

	lws_write(wsi,cJSON_Print(playerReady),strlen(cJSON_Print(playerReady)),0);
	
	return 0;
}



int32_t BET_rest_player_process_init_b(struct lws *wsi, cJSON *argjson,int32_t playerID)

{
		static int32_t decodebad,decodegood,good,bad,errs;
		int32_t unpermi,playererrs=0,decoded[CARDS777_MAXCARDS],retval=1;
		bits256 decoded256;
		bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
		cJSON *cjsonbvvblindcards,*cjsonshamirshards;
		bits256 temp,playerprivs[CARDS777_MAXCARDS];

		all_players_info[playerID].bvvpubkey=jbits256(argjson,"bvvpubkey");
		all_g_shares[playerID]=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
		cjsonbvvblindcards=cJSON_GetObjectItem(argjson,"bvvblindcards");
		
		for(int i=0;i<BET_player[playerID]->numplayers;i++)
		{
			for(int j=0;j<BET_player[playerID]->range;j++)
			{
				all_players_info[playerID].bvvblindcards[i][j]=jbits256i(cjsonbvvblindcards,i*BET_player[playerID]->range+j);
			}
		}


		cjsonshamirshards=cJSON_GetObjectItem(argjson,"shamirshards");
		int k=0;
		for(int playerid=0;playerid<BET_player[playerID]->numplayers;playerid++)
		{
			for (int i=0; i<BET_player[playerID]->range; i++)
	        {
	            for (int j=0; j<BET_player[playerID]->numplayers; j++) 
				{
					all_g_shares[playerID][k]=get_API_enc_share(cJSON_GetArrayItem(cjsonshamirshards,k));
					k++;
	            }
	        }
		}


		
		for(int i=0;i<BET_player[playerID]->range;i++)
		{
			for(int j=0;j<BET_player[playerID]->range;j++)
			{
				temp=xoverz_donna(curve25519(all_players_info[playerID].player_key.priv,curve25519(playerprivs[i],all_players_info[playerID].cardprods[BET_player[playerID]->myplayerid][j])));
				vcalc_sha256(0,all_v_hash[playerID][i][j].bytes,temp.bytes,sizeof(temp));
			}
		}
	retval=BET_rest_player_ready(wsi,playerID);
	return retval;
}

int32_t BET_rest_fundChannel(char *channel_id)
{
	int argc,maxsize=10000,retval=1,channel_state;
	char **argv=NULL,*buf=NULL;
	cJSON *fundChannelInfo=NULL;
	argc=5;
	argv=(char**)malloc(argc*sizeof(char*));
	buf=malloc(maxsize);
	for(int i=0;i<argc;i++)
	{
	        argv[i]=(char*)malloc(100*sizeof(char));
	}
	argc=4;
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"fundchannel");
	strcpy(argv[2],channel_id);
	strcpy(argv[3],"500000");
	argv[4]=NULL;

	fundChannelInfo=cJSON_CreateObject();
	make_command(argc,argv,&fundChannelInfo);

	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(fundChannelInfo));

	if(jint(fundChannelInfo,"code") != 0 )
	{
		retval=-1;
		printf("Message:%s",jstr(fundChannelInfo,"message"));
		goto end;
	}

	int state;
	while((state=LN_get_channel_status(channel_id)) != 3)
	{
		if(state == 2)
		 {
				  printf("\nCHANNELD_AWAITING_LOCKIN");
		 }
		  else
	  	 {
	  		retval=-1;
			printf("\n%s:%d:Channel establishment with DCV is not happening, please check the connectivity with the DCV node\n",__FUNCTION__,__LINE__);
			goto end;
	  	 }
		printf("%s::%d::%d\n",__FUNCTION__,__LINE__,state);	
		sleep(10);
	}
	printf("%s::%d::%d\n",__FUNCTION__,__LINE__,state);
end:
	if(buf)
		free(buf);
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

int32_t BET_rest_connect(char *uri)
{
	int argc,maxsize=10000,retval=1,channel_state;
	char **argv=NULL,channel_id[100];
	cJSON *connectInfo=NULL;
	char temp[200];
	strncpy(temp,uri,strlen(uri));
	strcpy(channel_id,strtok(temp, "@"));
	
	channel_state=LN_get_channel_status(channel_id);
	if((channel_state != 2)&&(channel_state !=3)) // 3 means channel is already established with the peer
	{					
		argc=3;
        argv=(char**)malloc(argc*sizeof(char*));
        for(int i=0;i<argc;i++)
        {
         argv[i]=(char*)malloc(100*sizeof(char));
        }
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"connect");
		strcpy(argv[2],uri);
	
		connectInfo=cJSON_CreateObject();
		make_command(argc,argv,&connectInfo);
		/*
		ln_bet(argc,argv,buf);
		connectInfo=cJSON_Parse(buf);
		*/
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(connectInfo));
		
		if(jint(connectInfo,"code") != 0)
		{
			retval=-1;
			printf("\n%s:%d:Message:%s",__FUNCTION__,__LINE__,jstr(connectInfo,"method"));
			goto end;
		}
	}
	end:
	if(argv)
	{
		for(int i=0;i<3;i++)
		{
			if(argv[i])
				free(argv[i]);
		}
		free(argv);
	}	
	return retval;
	
}

bits256 pedersen_commitments(uint32_t amount,bits256 *blind)
{
	struct pair256 key;
	char hexstr[65];
	bits320 bp,x,z;
	bits256 commit;
    
    key.priv=curve25519_keypair(&key.prod);
	blind=&key.priv;
	printf("\nPrivate Key:%s",bits256_str(hexstr,key.priv));
	printf("\nPublic key:%s",bits256_str(hexstr,key.prod));

	bp = fexpand(curve25519_basepoint9());
	printf("\namount:%d,secret:%s",amount,bits256_str(hexstr,bits256_from_compact(amount)));
	commit=curve25519(bits256_from_compact(amount),curve25519_basepoint9());

   printf("\nCommit is :%s",bits256_str(hexstr,commit));

   return commit;
    
}

void test_pedersen_commitments()
{
	bits256 commit[3],blind[3];
	uint32_t amount[3];
	char hexstr[65];
	amount[0]=100;amount[1]=200;amount[2]=300;
	for(int i=0;i<3;i++)
	{
		commit[i]=pedersen_commitments(amount[i],&blind[i]);
		//printf("%s::%d::%s::%s",__FUNCTION__,__LINE__,bits256_str(hexstr,commit[i]),bits256_str(hexstr,blind[i]));
	}
}

int32_t BET_rest_player_join_res(cJSON *argjson)
{
	int32_t playerID;
	char channel_id[100];
	cJSON *initInfo=NULL,*initCardInfo=NULL,*holeCardInfo=NULL;
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	if(0 == bits256_cmp(all_players_info[jint(argjson,"peerid")].player_key.prod,jbits256(argjson,"pubkey")))
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		if(BET_rest_connect(jstr(argjson,"uri")))
		{
			strcpy(channel_id,strtok(jstr(argjson,"uri"), "@"));
			BET_rest_fundChannel(channel_id);
		}

		initCardInfo=cJSON_CreateObject();
		cJSON_AddNumberToObject(initCardInfo,"dealer",0);
		holeCardInfo=cJSON_CreateArray();
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateNull());
		cJSON_AddItemToObject(initCardInfo,"holecards",holeCardInfo);

		initInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(initInfo,"method","deal");
		cJSON_AddItemToObject(initInfo,"deal",initCardInfo);
		/*
		printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(initInfo));
		if(!dataToWrite)
			dataToWrite=cJSON_CreateObject();

		dataToWrite=initInfo;
		data_exists=1;
		lws_callback_on_writable(wsi_global_client);
		//lws_write(wsi,cJSON_Print(initInfo),strlen(cJSON_Print(initInfo)),0);
		*/
		player_lws_write(initInfo);
	}
	return 0;
}


int32_t BET_rest_player_ask_share(struct lws *wsi,int32_t cardid,int32_t playerid,int32_t card_type,int32_t toPlayer,int32_t fromPlayer)
{
	cJSON *requestInfo=NULL;
	char *rendered=NULL;
	int32_t bytes,retval=1;

	printf("%s:%d\n",__FUNCTION__,__LINE__);
	
	requestInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(requestInfo,"method","requestShare");
	cJSON_AddNumberToObject(requestInfo,"playerid",playerid);
	cJSON_AddNumberToObject(requestInfo,"cardid",cardid);
	cJSON_AddNumberToObject(requestInfo,"card_type",card_type);
	cJSON_AddNumberToObject(requestInfo,"toPlayer",toPlayer);
	cJSON_AddNumberToObject(requestInfo,"fromPlayer",fromPlayer);
	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(requestInfo));
	lws_write(wsi,cJSON_Print(requestInfo),strlen(cJSON_Print(requestInfo)),0);

	return retval;
}


int32_t BET_rest_player_get_own_share(struct lws *wsi,cJSON *argjson,int32_t this_playerID)
{
	struct enc_share temp;
	int32_t cardid,retval=1,playerid,recvlen;
	uint8_t decipher[sizeof(bits256) + 1024],*ptr;
	bits256 share;
	char enc_share[177],str[65];
	playerid=jint(argjson,"playerid");
	cardid=jint(argjson,"cardid");

	printf("%s:%d\n",__FUNCTION__,__LINE__);
	
	temp=all_g_shares[this_playerID][BET_player[this_playerID]->myplayerid*BET_player[this_playerID]->numplayers*BET_player[this_playerID]->range + (cardid*BET_player[this_playerID]->numplayers + playerid)];
	recvlen = sizeof(temp);

	if ( (ptr= BET_decrypt(decipher,sizeof(decipher),all_players_info[this_playerID].bvvpubkey,all_players_info[this_playerID].player_key.priv,temp.bytes,&recvlen)) == 0 )
	{
		retval=-1;
		printf("decrypt error \n");
		goto end;
	}
	else
	{
		
		memcpy(share.bytes,ptr,recvlen);
		all_playershares[this_playerID][cardid][BET_player[this_playerID]->myplayerid]=share;
		all_sharesflag[this_playerID][cardid][BET_player[this_playerID]->myplayerid]=1;
		printf("The share is :%s\n",bits256_str(str,share));
		
	}
	end:
		return retval;

}


int32_t BET_rest_player_turn(struct lws *wsi, cJSON *argjson)
{
	int32_t retval=1,playerid,this_playerID;

	playerid=jint(argjson,"playerid");
	this_playerID=jint(argjson,"gui_playerID");
	printf("BET_player[0]->myplayerid=%d,BET_player[1]->myplayerid=%d\n",BET_player[0]->myplayerid,BET_player[1]->myplayerid);
	if(playerid == BET_player[this_playerID]->myplayerid)
	{
		all_no_of_shares[this_playerID]=1;
		retval=BET_rest_player_get_own_share(wsi,argjson,this_playerID);
		if(retval == -1)
		{
			printf("Failing to get own share: Decryption Error");
			goto end;
		}
		for(int i=0;i<BET_player[this_playerID]->numplayers;i++)
		{
			if((!all_sharesflag[this_playerID][jint(argjson,"cardid")][i]) && (i != BET_player[this_playerID]->myplayerid))
			{
				retval=BET_rest_player_ask_share(wsi,jint(argjson,"cardid"),jint(argjson,"playerid"),jint(argjson,"card_type"),i,BET_player[this_playerID]->myplayerid);	
			}
		}
	
	}
	end:	
		return retval;
}



int32_t BET_rest_player_give_share(struct lws *wsi,cJSON *argjson)
{
	int32_t retval=1,bytes,playerid,cardid,recvlen,card_type,this_playerID;
	cJSON *share_info=NULL;		
	char *rendered=NULL;
	struct enc_share temp;
	char str[65],enc_str[177];
	uint8_t decipher[sizeof(bits256) + 1024],*ptr;
	bits256 share;

	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
	
	this_playerID=jint(argjson,"gui_playerID");
	
	playerid=jint(argjson,"playerid");
	cardid=jint(argjson,"cardid");
	card_type=jint(argjson,"card_type");

	printf("\n%s:%d::playerid=%d,BET_player[this_playerID]->myplayerid=%d,playerID=%d\n",__FUNCTION__,__LINE__,
								playerid,BET_player[this_playerID]->myplayerid,this_playerID);
	
	if(playerid==BET_player[this_playerID]->myplayerid)
		goto end;
	
	temp=all_g_shares[this_playerID][playerid*BET_player[this_playerID]->numplayers*BET_player[this_playerID]->range + (cardid*BET_player[this_playerID]->numplayers + BET_player[this_playerID]->myplayerid)];

    recvlen = sizeof(temp);

	if ( (ptr= BET_decrypt(decipher,sizeof(decipher),all_players_info[this_playerID].bvvpubkey,all_players_info[this_playerID].player_key.priv,temp.bytes,&recvlen)) == 0 )
	{
		retval=-1;
		printf("decrypt error \n");
		goto end;
	}	
    else
	{
		memcpy(share.bytes,ptr,recvlen);
		share_info=cJSON_CreateObject();
		cJSON_AddStringToObject(share_info,"method","share_info");
		cJSON_AddNumberToObject(share_info,"playerid",BET_player[this_playerID]->myplayerid);
		cJSON_AddNumberToObject(share_info,"cardid",cardid);
		cJSON_AddNumberToObject(share_info,"card_type",card_type);
		cJSON_AddNumberToObject(share_info,"toPlayer",jint(argjson,"fromPlayer"));
		cJSON_AddNumberToObject(share_info,"fromPlayer",jint(argjson,"toPlayer"));
		jaddbits256(share_info,"share",share);

		lws_write(wsi,cJSON_Print(share_info),strlen(cJSON_Print(share_info)),0);
		
		
	}
	end:
		return retval;
}




bits256 BET_rest_decode_card(int32_t this_playerID,int32_t cardid)
{
	int32_t retval,numplayers,numcards,M,playerid,flag=0;
	bits256 recover,decoded,refval,tmp,xoverz,hash,fe,basepoint;
	uint8_t **shares;
    uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	char str[65];
	char hexstr [ 65 ];
	numplayers=BET_player[this_playerID]->maxplayers;
	numcards=BET_player[this_playerID]->range;
	shares=calloc(numplayers,sizeof(uint8_t*));
    for(int i=0;i<numplayers;i++)
        shares[i]=calloc(sizeof(bits256),sizeof(uint8_t));
    
  	M=(numplayers/2)+1;
	for(int i=0;i<M;i++) 
	{
		memcpy(shares[i],all_playershares[this_playerID][cardid][i].bytes,sizeof(bits256));
	}
	gfshare_calc_sharenrs(sharenrs,numplayers,all_players_info[this_playerID].deckid.bytes,sizeof(all_players_info[this_playerID].deckid)); // same for all players for this round

	gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);



	gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);
	refval = fmul_donna(all_players_info[this_playerID].bvvblindcards[BET_player[this_playerID]->myplayerid][cardid],crecip_donna(recover));

	//printf("\nDCV blinded card:%s",bits256_str(str,refval));
	
	
	for(int i=0;i<BET_player[this_playerID]->range;i++)
	{
		for(int j=0;j<BET_player[this_playerID]->range;j++)
		{
			bits256 temp=xoverz_donna(curve25519(all_players_info[this_playerID].player_key.priv,curve25519(all_players_info[this_playerID].cardprivkeys[i],all_players_info[this_playerID].cardprods[BET_player[this_playerID]->myplayerid][j])));
			vcalc_sha256(0,all_v_hash[this_playerID][i][j].bytes,temp.bytes,sizeof(temp));
		}
	}

	
	basepoint=curve25519_basepoint9();
	for (int i=0; i<BET_player[this_playerID]->range; i++)
    {
        for (int j=0; j<BET_player[this_playerID]->range; j++)
        {
        	if ( bits256_cmp(all_v_hash[this_playerID][i][j],all_g_hash[this_playerID][BET_player[this_playerID]->myplayerid][cardid]) == 0 )
			{
				//printf("\nThere is a match\n");
				#if 1
				for(int m=0;m<BET_player[this_playerID]->range;m++)
				{
					for(int n=0;n<BET_player[this_playerID]->range;n++)	
					{
						tmp = curve25519(all_players_info[this_playerID].player_key.priv,curve25519(all_players_info[this_playerID].cardprivkeys[m],all_players_info[this_playerID].cardprods[BET_player[this_playerID]->myplayerid][n]));
			            xoverz = xoverz_donna(tmp);
			            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));

						
			            fe = crecip_donna(curve25519_fieldelement(hash));

						decoded = curve25519(fmul_donna(refval,fe),basepoint);
						for(int k=0;k<BET_player[this_playerID]->range;k++)
						{
				            if ( bits256_cmp(decoded,all_players_info[this_playerID].cardprods[BET_player[this_playerID]->myplayerid][k]) == 0 )
				            {
				               // printf("\nplayer.%d decoded card %s value %d\n",bet->myplayerid,bits256_str(str,decoded),player_info.cardprivkeys[m].bytes[30]);
								//printf("\n");
								all_player_cards[this_playerID][no_of_player_cards]=atoi(bits256_str(str,decoded));
								all_no_of_player_cards[this_playerID]+=1;
				        		tmp=all_players_info[this_playerID].cardprivkeys[m];
								flag=1;
								goto end;
				            }
						}
					}
				}
				#endif
        	}
        }
    }
	
	end:
		if(!flag)	
			printf("\nDecoding Failed\n");
			
	return tmp;
}

int32_t BET_rest_player_receive_share(struct lws *wsi,cJSON *argjson)
{
	int32_t retval=1,bytes,cardid,playerid,errs=0,unpermi,card_type,this_playerID;
	cJSON *turn_status=NULL,*playerCardInfo=NULL;
	char *rendered=NULL,str[65];
	bits256 share,decoded256;
	
	share=jbits256(argjson,"share");
	cardid=jint(argjson,"cardid");
	playerid=jint(argjson,"playerid");
	card_type=jint(argjson,"card_type");
	this_playerID=jint(argjson,"gui_playerID");

	//printf("\n%s:%d:no_of_shares:%d,maxplayers:%d",__FUNCTION__,__LINE__,no_of_shares,bet->maxplayers);
	if(all_sharesflag[this_playerID][cardid][playerid] ==0 )
	{
			all_playershares[this_playerID][cardid][playerid]=share;
			all_sharesflag[this_playerID][cardid][playerid]=1;
			all_no_of_shares[this_playerID]+=1;
			//printf("\n%s:%d:no_of_shares:%d,maxplayers:%d",__FUNCTION__,__LINE__,no_of_shares,bet->maxplayers);
	}
	
	
	if(all_no_of_shares[this_playerID] == BET_player[this_playerID]->maxplayers)
	{
		all_no_of_shares[this_playerID]=0;
		/*
		for(int i=0;i<bet->maxplayers;i++)
		{
			printf("\n%s",bits256_str(str,playershares[cardid][i]));
		}*/
		decoded256 = BET_rest_decode_card(this_playerID,cardid);
		if ( bits256_nonz(decoded256) == 0 )
			errs++;
		else
		{
	        unpermi=-1;
            for(int k=0;k<BET_player[this_playerID]->range;k++)
			{
                if(all_players_info[this_playerID].permis[k]==decoded256.bytes[30])
				{
                    unpermi=k;
                    break;
                }
            }
		}

		if(unpermi != -1)
		{
			all_player_card_values[this_playerID][all_number_cards_drawn[this_playerID]++]=decoded256.bytes[30];
			playerCardInfo=cJSON_CreateObject();
			cJSON_AddStringToObject(playerCardInfo,"method","playerCardInfo");
			cJSON_AddNumberToObject(playerCardInfo,"playerid",BET_player[this_playerID]->myplayerid);
			cJSON_AddNumberToObject(playerCardInfo,"cardid",cardid);
			cJSON_AddNumberToObject(playerCardInfo,"card_type",card_type);
			cJSON_AddNumberToObject(playerCardInfo,"decoded_card",decoded256.bytes[30]);

			printf("\n%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(playerCardInfo));
			lws_write(wsi,cJSON_Print(playerCardInfo),strlen(cJSON_Print(playerCardInfo)),0);			
		}

	}
	end:
		return retval;
}


void rest_push_cards(struct lws *wsi,cJSON *argjson,int32_t this_playerID)
{
	
	char* suit[NSUITS]= {"clubs","diamonds","hearts","spades"};
	char* face[NFACES]= {"two","three","four","five","six","seven","eight","nine",
						 "ten","jack","queen","king","ace"};
	char* cards[52] = {"2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC", 
					   "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD", 
					   "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH", 
					   "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS"};
	
	char action_str[8][100]={"","small_blind","big_blind","check","raise","call","allin","fold"};
	cJSON *actions=NULL;
	int flag;
	cJSON *initCardInfo=NULL,*holeCardInfo=NULL,*initInfo=NULL,*boardCardInfo=NULL;

	initInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(initInfo,"method","deal");
	
	initCardInfo=cJSON_CreateObject();
	cJSON_AddNumberToObject(initCardInfo,"dealer",0);
	
	
	holeCardInfo=cJSON_CreateArray();
	for(int32_t i=0;((i<no_of_hole_cards)&&(i<all_number_cards_drawn[this_playerID]));i++)
	{
		cJSON_AddItemToArray(holeCardInfo,cJSON_CreateString(cards[all_player_card_values[this_playerID][i]]));
	}
	
	cJSON_AddItemToObject(initCardInfo,"holecards",holeCardInfo);

	boardCardInfo=cJSON_CreateArray();
	for(int32_t i=no_of_hole_cards;((i<hand_size)&&(i<all_number_cards_drawn[this_playerID]));i++)
	{
		cJSON_AddItemToArray(boardCardInfo,cJSON_CreateString(cards[all_player_card_values[this_playerID][i]]));
	}
	
	cJSON_AddItemToObject(initCardInfo,"board",boardCardInfo);

	
	cJSON_AddItemToObject(initInfo,"deal",initCardInfo);
	printf("\n%s::%d::%s",__FUNCTION__,__LINE__,cJSON_Print(initInfo));
	lws_write(wsi,cJSON_Print(initInfo),strlen(cJSON_Print(initInfo)),0);
	/*		
	printf("\n******************** Betting done so far ********************");
	printf("\nsmall_blind:%d, big_blind:%d",small_blind_amount,big_blind_amount);
	printf("\npot size:%d",jint(argjson,"pot"));
	actions=cJSON_GetObjectItem(argjson,"actions");
	int count=0;
	flag=1;
	for(int i=0;((i<=jint(argjson,"round"))&&(flag));i++)
	{
		printf("\nRound:%d",i);
		for(int j=0;((j<BET_player[this_playerID]->maxplayers)&&(flag));j++)
		{
			if(jinti(actions,((i*BET_player[this_playerID]->maxplayers)+j))>0)
				printf("\nplayed id:%d, action: %s",j,action_str[jinti(actions,((i*BET_player[this_playerID]->maxplayers)+j))]);
			count++;	
			if(count==cJSON_GetArraySize(actions))
					flag=0;
		}
		printf("\n");
	}
	*/
}


void rest_display_cards(cJSON *argjson,int32_t this_playerID)
{
	char* suit[NSUITS]= {"clubs","diamonds","hearts","spades"};
	char* face[NFACES]= {"two","three","four","five","six","seven","eight","nine",
						 "ten","jack","queen","king","ace"};
		
	char action_str[8][100]={"","small_blind","big_blind","check","raise","call","allin","fold"};
	cJSON *actions=NULL;
	int flag;

	printf("\n******************** Player Cards ********************");
	printf("\nHole Cards:\n");
	for(int32_t i=0;((i<no_of_hole_cards)&&(i<all_number_cards_drawn[this_playerID]));i++)
	{
		
		printf("%s-->%s \t",suit[all_player_card_values[this_playerID][i]/13],face[all_player_card_values[this_playerID][i]%13]);
	}
	
	flag=1;
	for(int32_t i=no_of_hole_cards;((i<hand_size)&&(i<all_number_cards_drawn[this_playerID]));i++)
	{
		if(flag)
		{
			printf("\nCommunity Cards:\n");
			flag=0;
		}	
		printf("%s-->%s \t",suit[all_player_card_values[this_playerID][i]/13],face[all_player_card_values[this_playerID][i]%13]);
	}
		
	printf("\n******************** Betting done so far ********************");
	printf("\nsmall_blind:%d, big_blind:%d",small_blind_amount,big_blind_amount);
	printf("\npot size:%d",jint(argjson,"pot"));
	actions=cJSON_GetObjectItem(argjson,"actions");
	int count=0;
	flag=1;
	for(int i=0;((i<=jint(argjson,"round"))&&(flag));i++)
	{
		printf("\nRound:%d",i);
		for(int j=0;((j<BET_player[this_playerID]->maxplayers)&&(flag));j++)
		{
			if(jinti(actions,((i*BET_player[this_playerID]->maxplayers)+j))>0)
				printf("\nplayed id:%d, action: %s",j,action_str[jinti(actions,((i*BET_player[this_playerID]->maxplayers)+j))]);
			count++;	
			if(count==cJSON_GetArraySize(actions))
					flag=0;
		}
		printf("\n");
	}
}

int32_t BET_rest_pay(char *bolt11)
{
	cJSON *payResponse=NULL;
   	int argc,maxsize=10000,retval=1;
	char **argv=NULL,*buf=NULL;
	argv=(char**)malloc(4*sizeof(char*));
	buf=malloc(maxsize);
	argc=3;
	for(int i=0;i<4;i++)
	{
		argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"pay");
	strcpy(argv[2],bolt11);
	argv[3]=NULL;

	payResponse=cJSON_CreateObject();
	make_command(argc,argv,&payResponse);
	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(payResponse));
	/*		
	ln_bet(argc,argv,buf);
	payResponse=cJSON_Parse(buf);
	*/	
	if(jint(payResponse,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(payResponse,"message"));
		goto end;
	}
	
	if(strcmp(jstr(payResponse,"status"),"complete")==0)
		printf("\nPayment Success");
	else
		retval=-1;
	end:
		if(buf)
			free(buf);
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

int32_t BET_rest_player_invoice(struct lws *wsi,cJSON *argjson)
{
	cJSON *invoiceInfo=NULL,*paymentInfo=NULL,*payResponse=NULL,*paymentParams=NULL;
    char *invoice=NULL;
	int32_t playerID,bytes,retval;

	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
	
	playerID=jint(argjson,"playerID");
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);
	if(playerID==BET_player[jint(argjson,"gui_playerID")]->myplayerid)
	{
		retval=BET_rest_pay(jstr(invoiceInfo,"bolt11"));
		if(retval)
		{
			paymentParams=cJSON_CreateObject();
			paymentParams=cJSON_GetObjectItem(argjson,"payment_params");
			if(strcmp(jstr(paymentParams,"action"),"small_blind")==0)
			{
				retval=BET_rest_small_blind_update(wsi,paymentParams,jint(argjson,"betAmount"));
			}
			else if(strcmp(jstr(paymentParams,"action"),"big_blind")==0)
			{
				retval=BET_rest_big_blind_update(wsi,paymentParams,jint(argjson,"betAmount"));
			}
			else if(strcmp(jstr(paymentParams,"action"),"round_betting")==0)
			{
				printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(argjson));
				retval=BET_rest_player_round_betting_update(wsi,paymentParams,jint(argjson,"option"),jint(argjson,"betAmount"));
			}
		}
	}
	end:
		return retval;
}

