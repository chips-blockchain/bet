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
#include "../picohttpparser/picohttpparser.h"

#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#if 1 //this is for websockets
#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

#define LWS_PLUGIN_STATIC
#include "protocol_lws_minimal.c"

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

#define MAX_THREADS 10

#define MAX_CONNECTION 5
typedef struct pool
{
    int fd;
    pthread_t tid;
    int is_allocated;
}connection_pool_t;
connection_pool_t connections[MAX_CONNECTION] = {};


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
char* suit[NSUITS]= {"hearts","spades","clubs","diamonds"};
char* face[NFACES]= {"ace","two","three","four","five","six","seven","eight","nine",
                     "ten","jack","queen","king"
                    };


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
                    //printf("RELAY.(%s)\n",jprint(argjson,0));
                    // BET_message_send("BET_relay",bet->pubsock,argjson,0,bet);
                    //if ( (sendlen= nn_send(bet->pubsock,ptr,recvlen,0)) != recvlen )
                    //    printf("sendlen.%d != recvlen.%d for %s\n",sendlen,recvlen,jprint(argjson,0));
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
int32_t get_http_body(char *buf,int buflen)
{
	char *method, *path;
	int pret, minor_version;
	struct phr_header headers[100];
	size_t method_len, path_len, num_headers;
	/* parse the request */
	num_headers = sizeof(headers) / sizeof(headers[0]);
	pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
							 &minor_version, headers, &num_headers, 0);
    if (pret > 0)
        return pret;
    else if (pret == -1)
    {
       printf("\nParseError");	
       return -1;
    }
    if (buflen == sizeof(buf))
    {
    	printf("\nRequestIsTooLongError");
        return -1;
    }	
}
void BET_rest_hostcommand(cJSON * inputInfo,struct privatebet_info * bet,struct privatebet_vars * vars,int socketid)
{
	printf("\n%s:%d::%s",__FUNCTION__,__LINE__,cJSON_Print(inputInfo));
	send(socketid, cJSON_Print(inputInfo), strlen(cJSON_Print(inputInfo)) , 0 );
}
void * thread_function(void * arg)
{
    int *index = (int *)arg;
    int err = 0;
    char data[1024] = {0};
    read(connections[*index].fd, data, sizeof(data));
	printf("\n%s:%d::data:%s\n",__FUNCTION__,__LINE__,data);
    send(connections[*index].fd, "received data", strlen("received data"),0);
    close(connections[*index].fd);
    /* time to free t the connection pool index*/
    connections[*index].is_allocated = 0;
    return NULL;
}

void BET_rest_hostloop1(int *fd)
{
	struct privatebet_info * bet;
	struct privatebet_vars * VARS;
	cJSON *inputInfo=NULL;
	//int *index = (int *)_ptr;
    int err = 0;
    char data[1024] = {0};
	
	char buf[4096];
	int pret;
	size_t buflen = 0, prevbuflen = 0;
	ssize_t rret;
	while(1)
	{
		if((rret = read(*fd,buf,sizeof(buf)))>0)
		{
			printf("\n%s:%d::buf:%s\n",__FUNCTION__,__LINE__,buf);
			send(*fd,"{chat : hi}",sizeof("{chat : hi}"),0);		
		}
		else 
			continue;
	}
	//read(*fd,buf,sizeof(buf));
	//send(*fd,buf,sizeof(buf),0);
	//printf("\n%s:%d::buf:%s\n",__FUNCTION__,__LINE__,buf);
	/*
	printf("\n%s:%d\n",__FUNCTION__,__LINE__);
	while (1) {
		buflen=0;
	    if ((rret = read(fd, buf + buflen, sizeof(buf) - buflen)) >0 )
    	{
			buflen += rret;
			printf("\n%s:%d::buf:%s\n",__FUNCTION__,__LINE__,buf);
			pret=get_http_body(buf,buflen);
			if(pret>0)
			{
				inputInfo=cJSON_CreateObject();
				inputInfo=cJSON_Parse(buf+pret);
				BET_rest_hostcommand(inputInfo,bet,VARS,connections[*index].fd);
			}
		
    	}
	}
	
	*/
}
 /* to get the not allocated index from connection*/ 
int get_connection(int **fd , int *index)
{
    int i =0;
    int err = 0;
    for (i =0; i < MAX_CONNECTION; i++)
    {
        if (!connections[i].is_allocated)
        {
            *fd = &connections[i].fd;
            connections[i].is_allocated = 1;
            *index = i;
            return 0;
            
        }
    }
    
    /* it mean all pool has been exhausted*/
    return 1;
}
 /* fucntion to get the total thread at any point of time*/
 int get_total_thread()
 {
	 int i = 0;
	 int count = 0;
	 for (i =0; i < MAX_CONNECTION; i++)
	 {
		 if (connections[i].is_allocated)
		 {
			 count++;
		 }
	 }
	 return count;
 }

void BET_rest_hostloop(void *_ptr)
{
	struct privatebet_info *bet = _ptr; struct privatebet_vars *VARS;
	int server_fd, new_socket;
	struct sockaddr_in addr;
	int opt = 1;
	int addrlen = sizeof(addr);
	cJSON *inputInfo=NULL;
	int err = 0;
	char buf[4096];
	int pret;
	size_t buflen = 0, prevbuflen = 0;
	ssize_t rret;
	int no_of_threads=0;

	pthread_t t[MAX_THREADS];
	
	VARS = calloc(1,sizeof(*VARS));
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
			perror("socket failed");
			exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&addr,sizeof(addr))<0)
	{
			perror("bind failed");
			exit(EXIT_FAILURE);
	}
	if (listen(server_fd, MAX_THREADS) < 0)
	{
			 perror("listen");
			 exit(EXIT_FAILURE);
	}
	
	while(1)
	{
		int fd;
		fd=accept(server_fd,(struct sockaddr *) &addr,(socklen_t *)&addrlen);
		if(fd<0)
		{
			perror("socket accept failed");
			exit(EXIT_FAILURE);
		}
		BET_rest_hostloop1(&fd);
		close(fd);
		
	}
}

void BET_ws_dcvloop(void *_ptr)
{
	struct lws_context_creation_info info;
	struct lws_context *context;
	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	signal(SIGINT, sigint_handler);

	if ((p = lws_cmdline_option(argc, argv, "-d")))
		logs = atoi(p);

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:7681\n");

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = 7681;
	info.mounts = &mount;
	info.protocols = protocols;
	info.options =
		LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	while (n >= 0 && !interrupted)
		n = lws_service(context, 1000);

	lws_context_destroy(context);

		
}




