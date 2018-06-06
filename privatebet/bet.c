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

// https://lists.linuxfoundation.org/pipermail/lightning-dev/2016-January/000403.html
// ^ is multisig

// jl777: oracle needs to include other data like deckid also, timestamp! thanks cryptographer
// dealer needs to timestamp and sign
// players need to sign their actions and gameeval
// deterministic sort
// new method for layered dealing, old method for layered shuffle
//libscott [11:08 PM]
//the observer is the chain. the state machine doesnt need to be executed on chain, but the HEAD state of the game should be notarised on a regular basis
//considering the case where the dealer uniquely generates the blinding value for each card and generates the M of N shard of it and distributes it among the players...


//[9:09]
//to get to know the card at any given time the player must know atleast M shards from it's peers..

//[11:08]
//Ie, it's the responsibility of each player to notarise that state after each move is made

// redo unpaid deletes
//  from external: git submodule add https://github.com/ianlancetaylor/libbacktrace.git

#include "bet.h"
char *LN_idstr,Host_ipaddr[64],Host_peerid[67],BET_ORACLEURL[64] = "127.0.0.1:7797";
uint16_t LN_port;
int32_t Gamestart,Gamestarted,Lastturni,Maxrounds = 3,Maxplayers = 10;
uint8_t BET_logs[256],BET_exps[510];
bits256 *Debug_privkeys;
struct BET_shardsinfo *BET_shardsinfos;
portable_mutex_t LP_gcmutex,LP_peermutex,LP_commandmutex,LP_networkmutex,LP_psockmutex,LP_messagemutex,BET_shardmutex;
int32_t LP_canbind,IAMLP,IAMHOST,IAMORACLE,LP_STOP_RECEIVED,DOCKERFLAG;
struct LP_peerinfo  *LP_peerinfos,*LP_mypeer;
bits256 Mypubkey,Myprivkey,Clientrhash,Hostrhashes[CARDS777_MAXPLAYERS+1];
char Host_channel[64];
int32_t permis_d[CARDS777_MAXCARDS],permis_b[CARDS777_MAXCARDS];
bits256 *allshares=NULL;
uint8_t sharenrs[256];
struct rpcrequest_info *LP_garbage_collector;
struct enc_share { uint8_t bytes[sizeof(bits256)+crypto_box_NONCEBYTES+crypto_box_ZEROBYTES]; };
struct enc_share *g_shares=NULL;

bits256 v_hash[CARDS777_MAXCARDS][CARDS777_MAXCARDS];
bits256 g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];

int32_t sharesflag[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];
bits256 playershares[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];
bits256 deckid;
uint32_t LP_rand()
{
    uint32_t retval;
    retval = rand();
    retval = (retval << 7) ^ (retval >> 17) ^ rand();
    retval = (retval << 13) ^ (retval >> 13) ^ rand();
    retval = (retval << 17) ^ (retval >> 7) ^ rand();
    return(retval);
}

/*char *issue_LP_psock(char *destip,uint16_t destport,int32_t ispaired)
 {
 char url[512],*retstr;
 sprintf(url,"http://%s:%u/api/stats/psock?ispaired=%d",destip,destport-1,ispaired);
 //return(LP_issue_curl("psock",destip,destport,url));
 retstr = issue_curlt(url,LP_HTTP_TIMEOUT*3);
 printf("issue_LP_psock got (%s) from %s\n",retstr,destip);
 return(retstr);
 }*/
int32_t LP_numpeers()
{
    printf("this needs to be fixed\n");
    return(9);
}
struct LP_millistats
{
    double lastmilli,millisum,threshold;
    uint32_t count;
    char name[64];
} LP_psockloop_stats,LP_reserved_msgs_stats,utxosQ_loop_stats,command_rpcloop_stats,queue_loop_stats,prices_loop_stats,LP_coinsloop_stats,LP_coinsloopBTC_stats,LP_coinsloopKMD_stats,LP_pubkeysloop_stats,LP_privkeysloop_stats,LP_swapsloop_stats,LP_gcloop_stats;

void LP_millistats_update(struct LP_millistats *mp)
{
    double elapsed,millis;
    if ( mp == 0 )
    {
        if ( IAMLP != 0 )
        {
            mp = &LP_psockloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        }
        mp = &LP_reserved_msgs_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &utxosQ_loop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &command_rpcloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &queue_loop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &prices_loop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_coinsloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_coinsloopBTC_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_coinsloopKMD_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_pubkeysloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_privkeysloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_swapsloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
        mp = &LP_gcloop_stats, printf("%32s lag %10.2f millis, threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,OS_milliseconds() - mp->lastmilli,mp->threshold,mp->millisum/(mp->count > 0 ? mp->count: 1),mp->count);
    }
    else
    {
        if ( mp->lastmilli == 0. )
            mp->lastmilli = OS_milliseconds();
        else
        {
            mp->count++;
            millis = OS_milliseconds();
            elapsed = (millis - mp->lastmilli);
            mp->millisum += elapsed;
            if ( mp->threshold != 0. && elapsed > mp->threshold )
            {
                //if ( IAMLP == 0 )
                printf("%32s elapsed %10.2f millis > threshold %10.2f, ave %10.2f millis, count.%u\n",mp->name,elapsed,mp->threshold,mp->millisum/mp->count,mp->count);
            }
            mp->lastmilli = millis;
        }
    }
}

int64_t LP_outpoint_amount(char *symbol,bits256 txid,int32_t vout)
{
    int64_t amount=0; int32_t numvouts; char coinaddr[64]; cJSON *vouts,*txjson;
    printf("need to fix the missing code to link to before using LP_outpoint_amount in LP_bitcoin.c\n");
    exit(0);
    /*if ( (amount= LP_txvalue(coinaddr,symbol,txid,vout)) != 0 )
        return(amount);
    else
    {
        if ( (txjson= LP_gettx(symbol,txid,1)) != 0 )
        {
            if ( (vouts= jarray(&numvouts,txjson,"vout")) != 0 && vout < numvouts )
                amount = LP_value_extract(jitem(vouts,vout),0);
            free_json(txjson);
        }
    }*/
    return(amount);
}

#include "../../SuperNET/iguana/exchanges/LP_network.c"
#include "../../SuperNET/iguana/exchanges/LP_secp.c"
#include "../../SuperNET/iguana/exchanges/LP_bitcoin.c"

void randombytes_buf(void * const buf, const size_t size)
{
    OS_randombytes((void *)buf,(int32_t)size);
}

#include "gfshare.c"
#include "cards777.c"
#include "network.c"
#include "oracle.c"
#include "commands.c"
#include "table.c"
#include "payment.c"
#include "client.c"
#include "host.c"
#include "states.c"

// original shuffle with player 2 encrypting to destplayer
// autodisconnect
// payments/bets -> separate dealer from pub0
// virtualize games
// privatebet host -> publish to BET chain
// tableid management -> leave, select game, start game


int32_t players_init(int32_t numplayers,int32_t numcards,bits256 deckid);
void sg777_players_init(int32_t numplayers,int32_t numcards,bits256 deckid);
#if 0
int main(int argc,const char *argv[])
{
    uint16_t tmp,rpcport = 7797,port = 7797+1;
    char connectaddr[128],bindaddr[128],smartaddr[64],randphrase[32],*modestr,*hostip,*passphrase=0,*retstr; cJSON *infojson,*argjson,*reqjson,*deckjson; uint64_t randvals; bits256 privkey,pubkey,pubkeys[64],privkeys[64]; uint8_t pubkey33[33],taddr=0,pubtype=60; uint32_t i,n,range,numplayers; int32_t testmode=0,pubsock=-1,subsock=-1,pullsock=-1,pushsock=-1; long fsize; struct privatebet_info *BET,*BET2;
    hostip = "127.0.0.1";
    libgfshare_init();
    OS_init();
    portable_mutex_init(&LP_peermutex);
    portable_mutex_init(&LP_commandmutex);
    portable_mutex_init(&LP_networkmutex);
    portable_mutex_init(&LP_psockmutex);
    portable_mutex_init(&LP_messagemutex);
    portable_mutex_init(&BET_shardmutex);
    sleep(1);
    if ( argc > 1 )
    {
    	
        if ( (infojson= chipsln_getinfo()) != 0 )
        {
            if ( (LN_idstr= clonestr(jstr(infojson,"id"))) == 0 || strlen(LN_idstr) != 66 )
                printf("need 33 byte secp pubkey\n"), exit(-1);
            LN_port = juint(infojson,"port");
            printf("getinfo.(%s)\n",jprint(infojson,1));
        } else printf("need to have CHIPS and lightning running\n"), exit(-1);
        printf("help.(%s)\n",jprint(chipsln_help(),1));
        printf("LN_idstr.(%s)\n",LN_idstr);
        
		if ( (argjson= cJSON_Parse(argv[1])) != 0 )
        {
            
            hostip = jstr(argjson,"hostip");
            if ( (tmp= juint(argjson,"hostport")) != 0 )
                port = tmp;
            if ( (tmp= juint(argjson,"rpcport")) != 0 )
                rpcport = tmp;
            if ( OS_thread_create(malloc(sizeof(pthread_t)),NULL,(void *)stats_rpcloop,(void *)&rpcport) != 0 )
            {
                printf("error launching stats rpcloop for port.%u\n",port);
                exit(-1);
            }
            

            if ( (modestr= jstr(argjson,"mode")) != 0 )
            {
                if ( strcmp(modestr,"host") == 0 )
                {
                 
                    if ( hostip == 0 && system("curl -s4 checkip.amazonaws.com > /tmp/myipaddr") == 0 )
                    {
                        if ( (hostip= OS_filestr(&fsize,"/tmp/myipaddr")) != 0 && hostip[0] != 0 )
                        {
                            n = (int32_t)strlen(hostip);
                            if ( hostip[n-1] == '\n' )
                                hostip[--n] = 0;
                        } else printf("error getting myipaddr\n");
                    }
				
                    BET_transportname(1,bindaddr,hostip,port);
                    pubsock = BET_nanosock(1,bindaddr,NN_PUB);
                    BET_transportname(1,bindaddr,hostip,port+1);
                    pullsock = BET_nanosock(1,bindaddr,NN_PULL);
                    IAMHOST = 1;
                    safecopy(Host_peerid,LN_idstr,sizeof(Host_peerid));
                    safecopy(Host_ipaddr,hostip,sizeof(Host_ipaddr));
                    // publish to BET chain
                }
                else if ( strcmp(modestr,"oracle") == 0 )
                {
                    IAMORACLE = 1;
                    while ( 1 )     // just respond to oracle requests
                        sleep(777);
                }
            }
            printf("BET API running on %s:%u pub.%d sub.%d; pull.%d push.%d\n",hostip,port,pubsock,subsock,pullsock,pushsock);
            BET = calloc(1,sizeof(*BET));
            BET2 = calloc(1,sizeof(*BET2));
            BET->pubsock = pubsock;
            BET->pullsock = pullsock;
            BET->subsock = subsock;
            BET->pushsock = pushsock;
            BET->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
            BET->maxchips = CARDS777_MAXCHIPS;
            BET->chipsize = CARDS777_CHIPSIZE;
            *BET2 = *BET;
            if ( passphrase == 0 || passphrase[0] == 0 )
            {
                FILE *fp;
                if ( (fp= fopen("passphrase","rb")) == 0 )
                {
                    OS_randombytes((void *)&randvals,sizeof(randvals));
                    sprintf(randphrase,"%llu",(long long)randvals);
                    printf("randphrase.(%s)\n",randphrase);
                    if ( (fp= fopen("passphrase","wb")) != 0 )
                    {
                        fwrite(randphrase,1,strlen(randphrase),fp);
                        fclose(fp);
                    }
                    passphrase = randphrase;
                }
                else
                {
                    printf("found passphrase file\n");
                    fread(randphrase,1,sizeof(randphrase),fp);
                    passphrase = randphrase;
                    fclose(fp);
                }
            }
            printf("passphrase.(%s) pushsock.%d subsock.%d hostip.(%s)\n",passphrase,pushsock,subsock,hostip);
            conv_NXTpassword(privkey.bytes,pubkey.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
            bitcoin_priv2pub(bitcoin_ctx(),pubkey33,smartaddr,privkey,taddr,pubtype);
            Mypubkey = pubkey;
            Myprivkey = privkey;
            if ( IAMHOST != 0 )
            {
                BET_betinfo_set(BET,"demo",36,0,Maxplayers);
                if ( OS_thread_create(malloc(sizeof(pthread_t)),NULL,(void *)BET_hostloop,(void *)BET) != 0 )
                {
                    printf("error launching BET_hostloop for pub.%d pull.%d\n",BET->pubsock,BET->pullsock);
                    exit(-1);
                }
            }
            else
            {
                if ( OS_thread_create(malloc(sizeof(pthread_t)),NULL,(void *)BET_clientloop,(void *)BET) != 0 )
                {
                    printf("error launching BET_clientloop for sub.%d\n",BET->subsock);
                    exit(-1);
                }
            }
            while ( 1 )
            {
                sleep(1);
                // update display
            }
            //BET_cmdloop(privkey,smartaddr,pubkey33,pubkey,BET2);
            /*if ( hostip == 0 || hostip[0] == 0 )
             hostip = "127.0.0.1";
             BET_transportname(0,connectaddr,hostip,port);
             printf("connect %s\n",connectaddr);
             subsock = BET_nanosock(0,connectaddr,NN_SUB);
             BET_transportname(0,connectaddr,hostip,port+1);
             pushsock = BET_nanosock(0,connectaddr,NN_PUSH);
             sleep(1);*/
            // printf("connect.(%s)\n",jprint(chipsln_connect(hostip,port,LN_idstr),1));
            //BET_mainloop(pubsock,pullsock,subsock,pushsock,jstr(argjson,"passphrase"));
        }
    }
    else
    {
        
		
		#if 0 //using threads
			cJSON *gameInfo=NULL;
			pthread_t player_t[CARDS777_MAXPLAYERS],dcv_t,bvv_t;
			uint32_t values[CARDS777_MAXPLAYERS];
			OS_randombytes((uint8_t *)&range,sizeof(range));
			OS_randombytes((uint8_t *)&numplayers,sizeof(numplayers));
			range = (range % 52) + 1;
			numplayers = (numplayers % (CARDS777_MAXPLAYERS-1)) + 2;

			gameInfo=cJSON_CreateObject();
			cJSON_AddNumberToObject(gameInfo,"numplayers",numplayers);
			cJSON_AddNumberToObject(gameInfo,"range",range);
			cJSON_AddNumberToObject(gameInfo,"maxnumplayers",CARDS777_MAXPLAYERS);
			cJSON_AddNumberToObject(gameInfo,"maxrange",CARDS777_MAXCARDS);

			char *rendered=cJSON_Print(gameInfo);
			
			printf("\nnumplayers=%d",jint(gameInfo,"numplayers"));
			printf("\n%s",rendered);
			numplayers=1;
			for(i=0;i<numplayers;i++){
				values[i]=i;
				if ( OS_thread_create(&player_t[i],NULL,(void *)BET_player,(void *)rendered) != 0 )
				{
					printf("error launching BET_clientloop\n");
					exit(-1);
				}
			}
					
			if ( OS_thread_create(&dcv_t,NULL,(void *)BET_dcv,(void *)rendered) != 0 )
			{
				printf("error launching BET_clientloop\n");
				exit(-1);
			}
			/*
			if ( OS_thread_create(&bvv_t,NULL,(void *)BET_bvv,(void *)BET) != 0 )
			{
				printf("error launching BET_clientloop\n");
				exit(-1);
			}
			*/
			for(i=0;i<numplayers;i++){
				if(pthread_join(player_t[i],NULL)){
					printf("\nError in joining the main thread for player thread %d",i);
				}
			}
			
			if(pthread_join(dcv_t,NULL)){
				printf("\nError in joining the main thread for dcv");
			}
			/*
			if(pthread_join(bvv_t,NULL)){
				printf("\nError in joining the main thread for bvv");
			}
	
			*/
	
		#endif
		testmode=0;
	
		while ( testmode != 1 )
        {
            testmode=1;
            OS_randombytes((uint8_t *)&range,sizeof(range));
            OS_randombytes((uint8_t *)&numplayers,sizeof(numplayers));
            range = (range % 52) + 1;
            numplayers = (numplayers % (CARDS777_MAXPLAYERS-1)) + 2;
			numplayers=2;
			range=2;
			printf("\nnumplayers=%d, numcards=%d\n",numplayers,range);
			sg777_players_init(numplayers,range,rand256(0));
            continue;
            for (i=0; i<numplayers; i++)
                privkeys[i] = curve25519_keypair(&pubkeys[i]);
            //Debug_privkeys = privkeys;
            deckjson = 0;
            if ( (reqjson= BET_createdeck_request(pubkeys,numplayers,range)) != 0 )
            {
                if ( (retstr= BET_oracle_request("createdeck",reqjson)) != 0 )
                {
                    if ( (deckjson= cJSON_Parse(retstr)) != 0 )
                    {
                        printf("BET_roundstart numcards.%d numplayers.%d\n",range,numplayers);
                        BET_roundstart(-1,deckjson,range,privkeys,pubkeys,numplayers,privkeys[0]);
                        printf("finished BET_roundstart numcards.%d numplayers.%d\n",range,numplayers);
                    }
                    free(retstr);
                }
                free_json(reqjson);
            }
            if ( deckjson != 0 )
                free_json(deckjson);
            {
                int32_t permi[CARDS777_MAXCARDS],permis[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
                memset(permi,0,sizeof(permi));
                memset(permis,0,sizeof(permis));
                for (i=0; i<numplayers; i++)
                    BET_permutation(permis[i],range);
                BET_permutation_sort(permi,permis,numplayers,range);
            }
        }
    }
    sleep(1);
    return 0;
}
#endif
#if 1
int main(int argc,const char *argv[])
{
    uint16_t tmp,rpcport = 7797,port = 7797+1;
    char connectaddr[128],bindaddr[128]="ipc:///tmp/bet.ipc",bindaddr1[128]="ipc:///tmp/bet1.ipc",smartaddr[64],randphrase[32],*modestr,*hostip,*passphrase=0,*retstr; 
	cJSON *infojson,*argjson,*reqjson,*deckjson; 
	uint64_t randvals; bits256 privkey,pubkey,pubkeys[64],privkeys[64]; 
	uint8_t pubkey33[33],taddr=0,pubtype=60; uint32_t i,n,range,numplayers; int32_t testmode=0,pubsock=-1,subsock=-1,pullsock=-1,pushsock=-1; long fsize; 
	struct privatebet_info **BET_players,*BET_dcv,*BET_bvv;
	pthread_t players_t[CARDS777_MAXPLAYERS],dcv_t,bvv_t;
	
	
    OS_init();
	libgfshare_init();
	OS_randombytes((uint8_t *)&range,sizeof(range));
    OS_randombytes((uint8_t *)&numplayers,sizeof(numplayers));

	range = (range % 52) + 1;
	numplayers = (numplayers % (CARDS777_MAXPLAYERS-1)) + 2;
	range=4;
	numplayers=2;
    Maxplayers=2;
	printf("%s:%d, range:%d, numplayers:%d\n",__FUNCTION__,__LINE__,range,numplayers);
	// for dcv
	if((argc==2)&&(strcmp(argv[1],"dcv")==0))
	{
		BET_dcv=calloc(1,sizeof(struct privatebet_info));
	    BET_dcv->pubsock = BET_nanosock(1,bindaddr,NN_PUB);
	    BET_dcv->pullsock = BET_nanosock(1,bindaddr1,NN_PULL);
	    BET_dcv->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
	    BET_dcv->maxchips = CARDS777_MAXCHIPS;
	    BET_dcv->chipsize = CARDS777_CHIPSIZE;
		BET_dcv->numplayers=0;
	    BET_betinfo_set(BET_dcv,"demo",range,0,Maxplayers);
	    if ( OS_thread_create(&dcv_t,NULL,(void *)BET_p2p_hostloop,(void *)BET_dcv) != 0 )
	    {
	        printf("error launching BET_hostloop for pub.%d pull.%d\n",BET_dcv->pubsock,BET_dcv->pullsock);
	        exit(-1);
	    }
       	if(pthread_join(dcv_t,NULL))
		{
			printf("\nError in joining the main thread for dcv");
		}
	}

	// for bvv
	else if((argc==2)&&(strcmp(argv[1],"bvv")==0))
	{
		BET_bvv=calloc(1,sizeof(struct privatebet_info));
	    BET_bvv->subsock = BET_nanosock(0,bindaddr,NN_SUB);
	    BET_bvv->pushsock = BET_nanosock(0,bindaddr1,NN_PUSH);
	    BET_bvv->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
	    BET_bvv->maxchips = CARDS777_MAXCHIPS;
	    BET_bvv->chipsize = CARDS777_CHIPSIZE;
		BET_bvv->numplayers=numplayers;
		BET_bvv->myplayerid=0;
	    BET_betinfo_set(BET_bvv,"demo",range,0,Maxplayers);
	    if ( OS_thread_create(&bvv_t,NULL,(void *)BET_p2p_bvvloop,(void *)BET_bvv) != 0 )
	    {
	        printf("error launching BET_clientloop for sub.%d push.%d\n",BET_bvv->subsock,BET_bvv->pushsock);
	        exit(-1);
	    }
		if(pthread_join(bvv_t,NULL))
		{
			printf("\nError in joining the main thread for bvvv");
		}
	}

	// for players
	else if((argc==2)&&(strcmp(argv[1],"player")==0)) 
	{
		char *ptr;
		i=0;
		BET_players=calloc(numplayers,sizeof(struct privatebet_info*));
		//for(int i=0;i<numplayers;i++)
			BET_players[i]=calloc(1,sizeof(struct privatebet_info));
		
	    
		/*for(int i=0;i<numplayers;i++)
		{*/
			BET_players[i]->subsock = BET_nanosock(0,bindaddr,NN_SUB);
		    BET_players[i]->pushsock = BET_nanosock(0,bindaddr1,NN_PUSH);
		    BET_players[i]->maxplayers = (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
		    BET_players[i]->maxchips = CARDS777_MAXCHIPS;
		    BET_players[i]->chipsize = CARDS777_CHIPSIZE;
			BET_players[i]->numplayers=numplayers;
			//BET_players[i]->myplayerid=atoi(argv[2]);
		    BET_betinfo_set(BET_players[i],"demo",range,0,Maxplayers);
		    if (OS_thread_create(&players_t[i],NULL,(void *)BET_p2p_clientloop,(void *)BET_players[i]) != 0 )
		    {
		        printf("error launching BET_clientloop for sub.%d push.%d\n",BET_players[i]->subsock,BET_players[i]->pushsock);
		        exit(-1);
		    }	
		/*}*/
		/*for(int i=0;i<numplayers;i++)
		{*/
			if(pthread_join(players_t[i],NULL))
			{
				printf("\nError in joining the main thread for player %d",i);
			}
		/*}*/
	}
	else
	{
		printf("\nInvalid Usage");
		printf("\nFor DCV: ./bet dcv");
		printf("\nFor BVV: ./bet bvv");
		printf("\nFor Player: ./bet player player_id");
	}
	#if 0
	if(pthread_join(dcv_t,NULL))
	{
		printf("\nError in joining the main thread for dcv");
	}

	if(pthread_join(bvv_t,NULL))
	{
		printf("\nError in joining the main thread for bvvv");
	}

	for(int i=0;i<numplayers;i++)
	{
		if(pthread_join(players_t[i],NULL))
		{
			printf("\nError in joining the main thread for player %d",i);
		}
	}
	#endif
    return 0;
}

#endif
bits256 curve25519_fieldelement(bits256 hash)
{
    hash.bytes[0] &= 0xf8, hash.bytes[31] &= 0x7f, hash.bytes[31] |= 0x40;
    return(hash);
}

bits256 card_rand256(int32_t privkeyflag,int8_t index)
{
    bits256 randval;
    OS_randombytes(randval.bytes,sizeof(randval));
    if ( privkeyflag != 0 )
        randval.bytes[0] &= 0xf8, randval.bytes[31] &= 0x7f, randval.bytes[31] |= 0x40;
    randval.bytes[30] = index;
    return(randval);
}

struct pair256 deckgen_common(struct pair256 *randcards,int32_t numcards)
{
    int32_t i; struct pair256 key,tmp;
    key.priv=curve25519_keypair(&key.prod);
    for (i=0; i<numcards; i++) {
        tmp.priv = card_rand256(1,i);
        tmp.prod = curve25519(tmp.priv,curve25519_basepoint9());
        randcards[i] = tmp;
    }
    return(key);
}

struct pair256 deckgen_common1(struct pair256 *randcards,int32_t numcards)
{
    int32_t i; struct pair256 key,tmp;
    key.priv=curve25519_keypair(&key.prod);
    for (i=0; i<numcards; i++) {
        tmp.priv = curve25519_keypair(&tmp.prod);
        randcards[i] = tmp;
    }
    return(key);
}

void deckgen_common2(struct pair256 *randcards,int32_t numcards)
{
	 for (int32_t i=0; i<numcards; i++)
		randcards[i].priv=curve25519_keypair(&randcards[i].prod);
   
}


void dekgen_vendor_perm(int numcards)
{
    BET_permutation(permis_d,numcards);
}
void blinding_vendor_perm(int numcards)
{
    BET_permutation(permis_b,numcards);
}
struct pair256 deckgen_player(bits256 *playerprivs,bits256 *playercards,int32_t *permis,int32_t numcards)
{
    int32_t i; struct pair256 key,randcards[256];
    key = deckgen_common(randcards,numcards);
    BET_permutation(permis,numcards);
    for (i=0; i<numcards; i++)
    {
        playerprivs[i] = randcards[i].priv; //permis[i]
        playercards[i]=curve25519(playerprivs[i],key.prod);
    }
    return(key);
}

struct pair256 deckgen_vendor(bits256 *cardprods,bits256 *finalcards,int32_t numcards,bits256 *playercards,bits256 deckid) // given playercards[], returns cardprods[] and finalcards[]
{
    static struct pair256 key,randcards[256]; static bits256 active_deckid;
    int32_t i,j,permis[256]; bits256 hash,xoverz,tmp[256];
    if ( bits256_cmp(deckid,active_deckid) != 0 )
        key=deckgen_common1(randcards,numcards);
        
        for (i=0; i<numcards; i++)
        {
            xoverz = xoverz_donna(curve25519(randcards[i].priv,playercards[i]));
            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));
            tmp[i] = fmul_donna(curve25519_fieldelement(hash),randcards[i].priv);

        }
    
    for (i=0; i<numcards; i++)
    {
        finalcards[i] = tmp[permis_d[i]];
        cardprods[i] = randcards[i].prod; // same cardprods[] returned for each player
        
    }
    
    return key;
}

void blinding_vendor(bits256 *blindings,bits256 *blindedcards,bits256 *finalcards,int32_t numcards,int32_t numplayers,int32_t playerid,bits256 deckid)
{
    int32_t i,j,k,M,permi,permis[256]; uint8_t space[8192]; bits256 *cardshares,*recover;
    recover=calloc(1,sizeof(bits256));
    for (i=0; i<numcards; i++)
    {
        blindings[i] = rand256(1);
        blindedcards[i] = fmul_donna(finalcards[permis_b[i]],blindings[i]);
    }
    M = (numplayers/2) + 1;
    
    gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid)); // same for all players for this round
    cardshares = calloc(numplayers,sizeof(bits256));
    if ( allshares == 0 )
        allshares = calloc(numplayers,sizeof(bits256) * numplayers * numcards);
    for (i=0; i<numcards; i++)
    {
        gfshare_calc_shares(cardshares[0].bytes,blindings[i].bytes,sizeof(bits256),sizeof(bits256),M,numplayers,sharenrs,space,sizeof(space));
        // create combined allshares
        for (j=0; j<numplayers; j++) {
            allshares[j*numplayers*numcards + (i*numplayers + playerid)] = cardshares[j];
        }
    }
	free(recover);
	free(cardshares);
    // when all players have submitted their finalcards, blinding vendor can send encrypted allshares for each player, see cards777.c
}

bits256 player_decode(int32_t playerid,int32_t cardID,int numplayers,struct pair256 key,bits256 blindingval,bits256 blindedcard,bits256 *cardprods,bits256 *playerprivs,int32_t *permis,int32_t numcards)
{
    bits256 tmp,xoverz,hash,fe,decoded,refval,basepoint,*cardshares; int32_t i,j,k,unpermi,M; char str[65];uint8_t space[8192];
    struct gfshare_ctx *G;
    bits256 *recover=NULL;
    
    uint8_t **shares;
    shares=calloc(numplayers,sizeof(uint8_t*));
    for(i=0;i<numplayers;i++)
        shares[i]=calloc(sizeof(bits256),sizeof(uint8_t));
    
    basepoint = curve25519_basepoint9();
    recover=calloc(1,sizeof(bits256));
    cardshares = calloc(numplayers,sizeof(bits256));
    for (j=0; j<numplayers; j++)
    {
        cardshares[j]=allshares[j*numplayers*numcards + (cardID*numplayers + playerid)];
    }
    
    M=(numplayers/2)+1;
    for(i=0;i<M;i++) {
        memcpy(shares[i],cardshares[i].bytes,sizeof(bits256));
    }
    gfshare_recoverdata(shares,sharenrs, M,recover->bytes,sizeof(bits256),M);
    refval = fmul_donna(blindedcard,crecip_donna(*recover));
    
    
    for (i=0; i<numcards; i++)
    {
        for (j=0; j<numcards; j++)
        {
            tmp = curve25519(key.priv,curve25519(playerprivs[i],cardprods[j]));
            xoverz = xoverz_donna(tmp);
            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));
            fe = crecip_donna(curve25519_fieldelement(hash));
            decoded = curve25519(fmul_donna(refval,fe),basepoint);
            if ( bits256_cmp(decoded,cardprods[j]) == 0 )
            {
                printf("player.%d decoded card %s value %d\n",playerid,bits256_str(str,decoded),playerprivs[i].bytes[30]);
                return(playerprivs[i]);
            }
        }
    }
    printf("couldnt decode blindedcard %s\n",bits256_str(str,blindedcard));
    memset(tmp.bytes,0,sizeof(tmp));
    return(tmp);
}

int32_t player_init(uint8_t *decoded,bits256 *playerprivs,bits256 *playercards,int32_t *permis,int32_t playerid,int32_t numplayers,int32_t numcards,bits256 deckid)
{
    int32_t i,j,k,errs,unpermi; struct pair256 key;
    bits256 decoded256,cardprods[CARDS777_MAXCARDS],finalcards[CARDS777_MAXCARDS],blindingvals[CARDS777_MAXCARDS],blindedcards[CARDS777_MAXCARDS];
    key = deckgen_player(playerprivs,playercards,permis,numcards);
    deckgen_vendor(cardprods,finalcards,numcards,playercards,deckid); // over network
    blinding_vendor(blindingvals,blindedcards,finalcards,numcards,numplayers,playerid,deckid); // over network
    memset(decoded,0xff,numcards);
    for (errs=i=0; i<numcards; i++)
    {
        decoded256 = player_decode(playerid,i,numplayers,key,blindingvals[i],blindedcards[i],cardprods,playerprivs,permis,numcards);
        if ( bits256_nonz(decoded256) == 0 )
            errs++;
        else
       	{
            unpermi = -1;
            for (j=0; j<numcards; j++)
            {
                if ( permis[j] == decoded256.bytes[30] )
                {
                    unpermi = j;
                    break;
                }
            }
            decoded[i] = j;
            printf("{%d} ",j);
        }
    }
    printf("ordering by playerid.%d\n",playerid);
    return(errs);
}

int32_t players_init(int32_t numplayers,int32_t numcards,bits256 deckid)
{
    
    static int32_t decodebad,decodegood,good,bad,numgames;
    int32_t i,j,playerid,errs,playererrs,permis[CARDS777_MAXPLAYERS][256]; uint8_t decoded[CARDS777_MAXPLAYERS][256]; bits256 playerprivs[CARDS777_MAXPLAYERS][256],playercards[CARDS777_MAXPLAYERS][256]; char str[65];
    numgames++;
    dekgen_vendor_perm(numcards);
    blinding_vendor_perm(numcards);
    
    allshares = calloc(numplayers,sizeof(bits256) * numplayers * numcards);
    for (playererrs=playerid=0; playerid<numplayers; playerid++)
    {
        if ( (errs= player_init(decoded[playerid],playerprivs[playerid],playercards[playerid],permis[playerid],playerid,numplayers,numcards,deckid)) != 0 )
        {
            printf("playerid.%d got errors %d for deckid.%s\n",playerid,errs,bits256_str(str,deckid));
            playererrs++;
        }
        decodebad += errs;
        decodegood += (numcards - errs);
    }
    for (i=0; i<numplayers-1; i++)
    {
        for (j=i+1; j<numplayers; j++)
        {
            if ( memcmp(decoded[i],decoded[j],numcards) != 0 )
            {
                printf("decoded cards mismatch between player %d and %d\n",i,j);
                bad++;
            } good++;
        }
    }
    printf("numplayers.%d numcards.%d deck %s -> numgames.%d playererrs.%d ordering.(good.%d bad.%d) decode.[good %d, bad %d]\n",numplayers,numcards,bits256_str(str,deckid),numgames,playererrs,good,bad,decodegood,decodebad);
    return(playererrs);
}
int32_t sg777_deckgen_vendor(int32_t playerid, bits256 *cardprods,bits256 *finalcards,int32_t numcards,bits256 *playercards,bits256 deckid) // given playercards[], returns cardprods[] and finalcards[]
{
    static struct pair256 randcards[256]; static bits256 active_deckid,hash_temp[CARDS777_MAXCARDS];
    int32_t retval=1; bits256 hash,xoverz,tmp[256];
	char str[65];
	
	if ( bits256_cmp(deckid,active_deckid) != 0 )
        deckgen_common2(randcards,numcards);
	else
		retval=-1;
	
	for (int32_t i=0; i<numcards; i++)
    {
        xoverz = xoverz_donna(curve25519(randcards[i].priv,playercards[i]));
		vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));
		hash_temp[i]=hash; //optimization
		tmp[i] = fmul_donna(curve25519_fieldelement(hash),randcards[i].priv);
	
    }

    for (int32_t i=0; i<numcards; i++)
    {
        finalcards[i] = tmp[permis_d[i]];
		g_hash[playerid][i]=hash_temp[permis_d[i]];//optimization
		cardprods[i] = randcards[i].prod; // same cardprods[] returned for each player

     }
	return retval;
}
bits256 t_sg777_player_decode(struct privatebet_info *bet,int32_t cardID,int numplayers,struct pair256 key,bits256 public_key_b,bits256 blindedcard,bits256 *cardprods,bits256 *playerprivs,int32_t numcards)
{
    bits256 recover,decoded,tmp,xoverz,hash,fe,refval,basepoint,cardshares[CARDS777_MAXPLAYERS]; int32_t i,j,k,unpermi,M; char str[65];
    struct enc_share temp;
    uint8_t **shares,flag=0;
	uint32_t playerid;
	char share_str[177];
	pthread_t t_req,t_rcv,t_res;
	struct privatebet_share *share_info=NULL;

	share_info=calloc(1,sizeof(struct privatebet_share));
	share_info->bvv_public_key=public_key_b;
	share_info->player_key=key;
	share_info->subsock=bet->subsock;
	share_info->myplayerid=bet->myplayerid;
	share_info->range=bet->range;
	share_info->numplayers=bet->numplayers;
	share_info->pushsock=bet->pushsock;

	
	if ( OS_thread_create(&t_res,NULL,(void *)BET_response,share_info) != 0 )
    {
        printf("error launching BET_response thread");
        exit(-1);
    }
	sleep(5);
	if ( OS_thread_create(&t_req,NULL,(void *)BET_request,share_info) != 0 )
    {
        printf("error launching BET_request thread");
        exit(-1);
    }
	if(pthread_join(t_req,NULL))
	{
		printf("\nError in joining the main thread for t_req");
	}
	if(pthread_join(t_res,NULL))
	{
		printf("\nError in joining the main thread for t_res");
	}

	for(int i=0;i<bet->range;i++)
		{
			printf("\ncard:%d",i);
			for(int j=0;j<bet->numplayers;j++)
				{
					printf("\nshare:%s",bits256_str(str,playershares[i][j]));
				}
		}

	
 	#if 1 
    shares=calloc(numplayers,sizeof(uint8_t*));
    for(i=0;i<numplayers;i++)
        shares[i]=calloc(sizeof(bits256),sizeof(uint8_t));
    
    basepoint = curve25519_basepoint9();
    uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
	
	M=(numplayers/2)+1;
	for(i=0;i<M;i++) 
	{
		memcpy(shares[i],playershares[cardID][i].bytes,sizeof(bits256));
	}
	gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid)); // same for all players for this round
	
	gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);
	refval = fmul_donna(blindedcard,crecip_donna(recover));

	for(i=0;i<numcards;i++)
	{
		for(j=0;j<numcards;j++)
		{
			bits256 temp=xoverz_donna(curve25519(key.priv,curve25519(playerprivs[i],cardprods[j])));
			vcalc_sha256(0,v_hash[i][j].bytes,temp.bytes,sizeof(temp));
		}
	}

	playerid=bet->myplayerid;

	for (i=0; i<numcards; i++)
    {
        for (j=0; j<numcards; j++)
        {
        	if ( bits256_cmp(v_hash[i][j],g_hash[playerid][cardID]) == 0 )
			{
	            tmp = curve25519(key.priv,curve25519(playerprivs[i],cardprods[j]));
	            xoverz = xoverz_donna(tmp);
	            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));
	            fe = crecip_donna(curve25519_fieldelement(hash));
	            decoded = curve25519(fmul_donna(refval,fe),basepoint);
	            if ( bits256_cmp(decoded,cardprods[j]) == 0 )
	            {
	                printf("\nplayer.%d decoded card %s value %d\n",playerid,bits256_str(str,decoded),playerprivs[i].bytes[30]);
	        		tmp=playerprivs[i];
					flag=1;
					goto end;
	            }
        	}
        }
    }
	
	end:
   for(i=0;i<numplayers;i++)
   {
	  free(shares[i]);
   }
	free(shares);
	if(!flag)
	{
		memset(tmp.bytes,0,sizeof(tmp));
		printf("\ncouldnt decode blindedcard %s\n",bits256_str(str,blindedcard));
	}
	#endif
    return(tmp);
}

bits256 sg777_player_decode(int32_t playerid,int32_t cardID,int numplayers,struct pair256 *keys,struct pair256 b_key,bits256 blindingval,bits256 blindedcard,bits256 *cardprods,bits256 *playerprivs,int32_t *permis,int32_t numcards)
{
    bits256 recover,decoded,tmp,xoverz,hash,fe,refval,basepoint,cardshares[CARDS777_MAXPLAYERS]; int32_t i,j,k,unpermi,M; char str[65];
    struct enc_share temp;
    uint8_t **shares,flag=0;
    shares=calloc(numplayers,sizeof(uint8_t*));
    for(i=0;i<numplayers;i++)
        shares[i]=calloc(sizeof(bits256),sizeof(uint8_t));
    
    basepoint = curve25519_basepoint9();
    uint8_t decipher[sizeof(bits256) + 1024],*ptr; int32_t recvlen;
    for (j=0; j<numplayers; j++)
    {
        temp=g_shares[j*numplayers*numcards + (cardID*numplayers + playerid)];
        recvlen = sizeof(temp);
        if ( (ptr= BET_decrypt(decipher,sizeof(decipher),b_key.prod,keys[j].priv,temp.bytes,&recvlen)) == 0 )
            printf("decrypt error ");
        else
            memcpy(cardshares[j].bytes,ptr,recvlen);
    }
    M=(numplayers/2)+1;
    for(i=0;i<M;i++) {
        memcpy(shares[i],cardshares[i].bytes,sizeof(bits256));
    }
    gfshare_recoverdata(shares,sharenrs, M,recover.bytes,sizeof(bits256),M);
    refval = fmul_donna(blindedcard,crecip_donna(recover));
    for (i=0; i<numcards; i++)
    {
        for (j=0; j<numcards; j++)
        {
        	if ( bits256_cmp(v_hash[i][j],g_hash[playerid][cardID]) == 0 ){
	            tmp = curve25519(keys[playerid].priv,curve25519(playerprivs[i],cardprods[j]));
	            xoverz = xoverz_donna(tmp);
	            vcalc_sha256(0,hash.bytes,xoverz.bytes,sizeof(xoverz));
	            fe = crecip_donna(curve25519_fieldelement(hash));
	            decoded = curve25519(fmul_donna(refval,fe),basepoint);
	            if ( bits256_cmp(decoded,cardprods[j]) == 0 )
	            {
	                printf("player.%d decoded card %s value %d\n",playerid,bits256_str(str,decoded),playerprivs[i].bytes[30]);
					tmp=playerprivs[i];
					flag=1;
					goto end;
	            }
        	}
        }
    }
	end:
   for(i=0;i<numplayers;i++){
		free(shares[i]);
	}
	free(shares);
	if(!flag){
	memset(tmp.bytes,0,sizeof(tmp));
	printf("couldnt decode blindedcard %s\n",bits256_str(str,blindedcard));
	}
    return(tmp);
}

struct pair256 p2p_bvv_init(bits256 *keys,struct pair256 b_key,bits256 *blindings,bits256 *blindedcards,bits256 *finalcards,int32_t numcards,int32_t numplayers,int32_t playerid,bits256 deckid)
{
    int32_t i,j,k,M,permi,permis[256]; uint8_t space[8192]; bits256 cardshares[CARDS777_MAXPLAYERS],basepoint,temp_hash[CARDS777_MAXCARDS];
    char str[65],share_str[177];
    struct enc_share temp;
	/*
	for (i=0; i<numcards; i++){
		temp_hash[i]=g_hash[playerid][i];
	}*/
	for (i=0; i<numcards; i++)
    {
        blindings[i] = rand256(1);
		blindedcards[i] = fmul_donna(finalcards[permis_b[i]],blindings[i]);
		//g_hash[playerid][i]=temp_hash[permis_b[i]];//optimization
	}
	printf("\n%s:%d:For Player id:%d",__FUNCTION__,__LINE__,playerid);

	for(i=0;i<numcards;i++)
	{
		printf("\nDCV card:%s",bits256_str(str,finalcards[permis_b[i]]));
		printf("\nBVV card:%s",bits256_str(str,blindedcards[i]));
	}
	
    M = (numplayers/2) + 1;
    
    gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid)); // same for all players for this round
	
        for (i=0; i<numcards; i++)
        {
            gfshare_calc_shares(cardshares[0].bytes,blindings[i].bytes,sizeof(bits256),sizeof(bits256),M,numplayers,sharenrs,space,sizeof(space));
            // create combined allshares
            for (j=0; j<numplayers; j++) {
				BET_ciphercreate(b_key.priv,keys[j],temp.bytes,cardshares[j].bytes,sizeof(cardshares[j]));
				memcpy(g_shares[numplayers*numcards*playerid+ i*numplayers + j].bytes,temp.bytes,sizeof(temp));
			}
        }
    // when all players have submitted their finalcards, blinding vendor can send encrypted allshares for each player, see cards777.c
    return b_key;
}

struct pair256 sg777_blinding_vendor(struct pair256 *keys,struct pair256 b_key,bits256 *blindings,bits256 *blindedcards,bits256 *finalcards,int32_t numcards,int32_t numplayers,int32_t playerid,bits256 deckid)
{
    int32_t i,j,k,M,permi,permis[256]; uint8_t space[8192]; bits256 cardshares[CARDS777_MAXPLAYERS],basepoint,temp_hash[CARDS777_MAXCARDS];
    char str[65],share_str[177];
    struct enc_share temp;
	//libgfshare_init();
	
	for (i=0; i<numcards; i++){
		temp_hash[i]=g_hash[playerid][i];
	}
	for (i=0; i<numcards; i++)
    {
        blindings[i] = rand256(1);
		blindedcards[i] = fmul_donna(finalcards[permis_b[i]],blindings[i]);
		g_hash[playerid][i]=temp_hash[permis_b[i]];//optimization
		}
    M = (numplayers/2) + 1;
    
    gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid)); // same for all players for this round
	
        for (i=0; i<numcards; i++)
        {
            gfshare_calc_shares(cardshares[0].bytes,blindings[i].bytes,sizeof(bits256),sizeof(bits256),M,numplayers,sharenrs,space,sizeof(space));
            // create combined allshares
            for (j=0; j<numplayers; j++) {
				BET_ciphercreate(b_key.priv,keys[j].prod,temp.bytes,cardshares[j].bytes,sizeof(cardshares[j]));
				memcpy(g_shares[j*numplayers*numcards + (i*numplayers + playerid)].bytes,temp.bytes,sizeof(temp));
			}
        }
    // when all players have submitted their finalcards, blinding vendor can send encrypted allshares for each player, see cards777.c
    return b_key;
}


void sg777_players_init(int32_t numplayers,int32_t numcards,bits256 deckid)
{
    static int32_t decodebad,decodegood,good,bad;
    int32_t i,j,k,playerid,errs,unpermi,playererrs=0,decoded[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],permis[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]; bits256 playerprivs[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],playercards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]; char str[65];
    struct pair256 keys[CARDS777_MAXPLAYERS],b_key;
    bits256 temp,decoded256,basepoint,cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],finalcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],blindingvals[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS],blindedcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
    dekgen_vendor_perm(numcards);
    blinding_vendor_perm(numcards);
    for (playerid=0; playerid<numplayers; playerid++) {
        keys[playerid]=deckgen_player(playerprivs[playerid],playercards[playerid],permis[playerid],numcards);
    }
    for (playerid=0; playerid<numplayers; playerid++)
    {
        sg777_deckgen_vendor(playerid,cardprods[playerid],finalcards[playerid],numcards,playercards[playerid],deckid);
    }
    b_key.priv=curve25519_keypair(&b_key.prod);
	g_shares=(struct enc_share*)malloc(CARDS777_MAXPLAYERS*CARDS777_MAXPLAYERS*CARDS777_MAXCARDS*sizeof(struct enc_share));
    for (playerid=0; playerid<numplayers; playerid++)
    {
        sg777_blinding_vendor(keys,b_key,blindingvals[playerid],blindedcards[playerid],finalcards[playerid],numcards,numplayers,playerid,deckid); // over network
    }
    for (playerid=0; playerid<numplayers; playerid++){
        errs=0;
		for(i=0;i<numcards;i++){
			for(j=0;j<numcards;j++){
				temp=xoverz_donna(curve25519(keys[playerid].priv,curve25519(playerprivs[playerid][i],cardprods[playerid][j])));
				vcalc_sha256(0,v_hash[i][j].bytes,temp.bytes,sizeof(temp));
				
	            
			}
		}
        for(i=0;i<numcards;i++){
            decoded256 = sg777_player_decode(playerid,i,numplayers,keys,b_key,blindingvals[playerid][i],blindedcards[playerid][i],cardprods[playerid],playerprivs[playerid],permis[playerid],numcards);
            
            if ( bits256_nonz(decoded256) == 0 )
                errs++;
            else
            {
                unpermi=-1;
                for(k=0;k<numcards;k++){
                    if(permis[playerid][k]==decoded256.bytes[30]){
                        unpermi=k;
                        break;
                    }
                }
                decoded[playerid][i] = k;    	
            }
        }
        decodebad += errs;
        decodegood+= (numcards - errs);
    }
    for (i=0; i<numplayers-1; i++)
    {
        for (j=i+1; j<numplayers; j++)
        {
            if ( memcmp(decoded[i],decoded[j],numcards) != 0 )
            {
                printf("decoded cards mismatch between player %d and %d\n",i,j);
                bad++;
            } good++;
        }
    }
    printf("numplayers.%d numcards.%d deck %s -> playererrs.%d good.%d bad.%d decode.[good %d, bad %d]\n",numplayers,numcards,bits256_str(str,deckid),playererrs,good,bad,decodegood,decodebad);
    free(g_shares);
}
