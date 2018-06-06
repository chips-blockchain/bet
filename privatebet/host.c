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

struct privatebet_rawpeerln Rawpeersln[CARDS777_MAXPLAYERS+1],oldRawpeersln[CARDS777_MAXPLAYERS+1];
struct privatebet_peerln Peersln[CARDS777_MAXPLAYERS+1];
int32_t Num_rawpeersln,oldNum_rawpeersln,Num_peersln,Numgames;
int32_t players_joined=0;
int32_t turn=0,no_of_cards=0;
struct deck_dcv_info dcv_info;

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
    if ( (p= BET_peerln_find(raw->peerid)) == 0 )
    {
        p = &Peersln[Num_peersln++];
        p->raw = *raw;
    }
    if ( IAMHOST != 0 && p != 0 )//&& strcmp(Host_peerid,LN_idstr) != 0 )
    {
        sprintf(label,"%s_%d",raw->peerid,0);
        p->hostrhash = chipsln_rhash_create(chipsize,label);
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
                p->hostrhash = chipsln_rhash_create(bet->chipsize,label);
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

	  printf("\n%s:%d:data:%s",__FUNCTION__,__LINE__,rendered);	

	  if(bytes<0)
	  	retval=-1;

	  return retval;
	 
}




int32_t BET_p2p_host_init(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
  	int32_t peerid,retval=-1;
  	bits256 cardpubvalues[CARDS777_MAXPLAYERS];
	cJSON *cardinfo=NULL;
	char str[65];

	
  	peerid=jint(argjson,"peerid");
  	cardinfo=cJSON_GetObjectItem(argjson,"cardinfo");
	for(int i=0;i<cJSON_GetArraySize(cardinfo);i++)
	{
			cardpubvalues[i]=jbits256i(cardinfo,i);
	} 	
	
	sg777_deckgen_vendor(peerid,dcv_info.cardprods[peerid],dcv_info.dcvblindcards[peerid],bet->range,cardpubvalues,dcv_info.deckid);
	dcv_info.numplayers=dcv_info.numplayers+1;
	/*
	printf("\n%s:%d:DCV blinded cards of peerid:%d\n",__FUNCTION__,__LINE__,peerid);
	for(int i=0;i<bet->range;i++)
	{
		printf("\n%s",bits256_str(str,dcv_info.dcvblindcards[peerid][i]));
	}
 	*/
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

	printf("\n%s:%d:data:%s",__FUNCTION__,__LINE__,rendered);
	
	return retval;
}
int32_t BET_p2p_client_join_req(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	cJSON *playerinfo=NULL;
    uint32_t bytes,retval=1;
	char *rendered=NULL;

    bet->numplayers=++players_joined;
	dcv_info.peerpubkeys[players_joined-1]=jbits256(argjson,"pubkey");
	
	playerinfo=cJSON_CreateObject();
	cJSON_AddStringToObject(playerinfo,"method","join_res");
	cJSON_AddNumberToObject(playerinfo,"peerid",bet->numplayers-1); //players numbering starts from 0(zero)
	jaddbits256(playerinfo,"pubkey",jbits256(argjson,"pubkey"));

	rendered=cJSON_Print(playerinfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);
	

	if(bytes<0)
		return 0;
	else
		return 1;
 }

int32_t BET_p2p_dcv_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval=1,bytes;
	cJSON *turninfo=NULL;
	char *rendered=NULL;

	turninfo=cJSON_CreateObject();
	cJSON_AddStringToObject(turninfo,"method","turn");
	cJSON_AddNumberToObject(turninfo,"playerid",(turn)%bet->maxplayers);
	cJSON_AddNumberToObject(turninfo,"cardid",turn);
	turn++;
	rendered=cJSON_Print(turninfo);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	printf("\n%s:%d:data:%s",__FUNCTION__,__LINE__,rendered);

	if(bytes<0)
		retval=-1;	
	
	return retval;
}

int32_t BET_p2p_dcv_turn_status(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval;

	if(strcmp(jstr(argjson,"status"),"complete") == 0)
	{
		no_of_cards++;
		if(no_of_cards<bet->range)
			retval=BET_p2p_dcv_turn(argjson,bet,vars);

	}
	else
	{
		//some action needs to be taken by DCV incase if the turn is not complete
	}
	
	return retval;
}

int32_t BET_p2p_dcv_start(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	return BET_p2p_dcv_turn(argjson,bet,vars);
}


int32_t BET_relay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	int32_t retval,bytes;
	char *rendered=NULL;

	rendered=cJSON_Print(argjson);
	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
		retval=-1;
	else
		retval=1;
	
	return retval;
}

int32_t BET_p2p_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
    char *method; int32_t bytes,retval=1;
	
    if ( (method= jstr(argjson,"method")) != 0 )
    {
		printf("\n%s:%d:data:%s",__FUNCTION__,__LINE__,cJSON_Print(argjson));
		if(strcmp(method,"join_req") == 0)
		{
			if(bet->numplayers<bet->maxplayers)
			{
				BET_p2p_client_join_req(argjson,bet,vars);
                if(bet->numplayers==bet->maxplayers)
				{
					printf("\nTable is filled");
					BET_p2p_host_start_init(bet);
				}
			}
		}
		else if(strcmp(method,"init_p") == 0)
		{
			BET_p2p_host_init(argjson,bet,vars);
			if(dcv_info.numplayers==dcv_info.maxplayers)
			{
				BET_p2p_host_deck_init_info(argjson,bet,vars);
			}
		}
		else if((strcmp(method,"init_b") == 0) || (strcmp(method,"next_turn") == 0))
		{
			printf("\n%s:%d:Game Start",__FUNCTION__,__LINE__);

			if(strcmp(method,"init_b") == 0)
			{
				BET_relay(argjson,bet,vars);
			}

			
			BET_p2p_dcv_start(argjson,bet,vars);
			
		}
		else if(strcmp(method,"turn_status") == 0)
		{
			printf("\n%s:%d:Game Start",__FUNCTION__,__LINE__);
			BET_p2p_dcv_turn_status(argjson,bet,vars);
		}
        else
    	{
    		bytes=nn_send(bet->pubsock,cJSON_Print(argjson),strlen(cJSON_Print(argjson)),0);
			if(bytes<0)
				retval=-1;
    	}
    }
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

