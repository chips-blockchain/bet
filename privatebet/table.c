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


int32_t BET_pubkeyfind(struct privatebet_info *bet,bits256 pubkey,char *peerid)
{
    int32_t i;
    //char str[65]; printf("PUBKEY FIND.(%s)\n",bits256_str(str,pubkey));
    for (i=0; i<sizeof(bet->playerpubs)/sizeof(*bet->playerpubs); i++)
        if ( bits256_cmp(pubkey,bet->playerpubs[i]) == 0 )
        {
            if ( strcmp(peerid,bet->peerids[i]) == 0 )
                return(i);
            else return(-1);
        }
    return(-1);
}

int32_t BET_pubkeyadd(struct privatebet_info *bet,bits256 pubkey,char *peerid)
{
    int32_t i,n = 0;
    //char str[65]; printf("PUBKEY ADD.(%s) maxplayers.%d\n",bits256_str(str,pubkey),bet->maxplayers);
    for (i=0; i<bet->maxplayers; i++)
    {
        if ( bits256_cmp(bet->tableid,Mypubkey) == 0 && i >= bet->maxplayers )
            break;
        if ( bits256_nonz(bet->playerpubs[i]) == 0 )
        {
            bet->playerpubs[i] = pubkey;
            safecopy(bet->peerids[i],peerid,sizeof(bet->peerids[i]));
            for (i=0; i<sizeof(bet->playerpubs)/sizeof(*bet->playerpubs); i++)
                if ( bits256_nonz(bet->playerpubs[i]) != 0 )
                {
                    n++;
                    bet->numplayers = n;
                }
            return(n);
        }
        //char str[65]; printf("%s\n",bits256_str(str,bet->playerpubs[i]));
    }
    return(0);
}

void BET_betinfo_set(struct privatebet_info *bet,char *game,int32_t range,int32_t numrounds,int32_t maxplayers)
{
    safecopy(bet->game,game,sizeof(bet->game));
    bet->range = range;
    bet->numrounds = numrounds;
    bet->maxplayers = maxplayers;
}

cJSON *BET_betinfo_json(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    int32_t i,n; cJSON *array,*betjson = cJSON_CreateObject();
    jaddstr(betjson,"method","tablestatus");
    jaddstr(betjson,"game",bet->game);
    jaddbits256(betjson,"tableid",bet->tableid);
    jaddnum(betjson,"timestamp",bet->timestamp);
    jaddnum(betjson,"maxplayers",bet->maxplayers);
    array = cJSON_CreateArray();
    bet->myplayerid = -1;
    for (i=n=0; i<bet->maxplayers; i++)
    {
        if ( bits256_nonz(bet->playerpubs[i]) != 0 )
        {
            n++;
            jaddibits256(array,bet->playerpubs[i]);
            if ( bits256_cmp(bet->playerpubs[i],Mypubkey) == 0 )
                bet->myplayerid = i;
        }
    }
    bet->numplayers = n;
    jadd(betjson,"players",array);
    jaddnum(betjson,"numplayers",n);
    if ( bet->range == 0 )
        bet->range = (52 % CARDS777_MAXCARDS) + 2;
    if ( bet->numrounds == 0 )
        bet->numrounds = Maxrounds;
    jaddnum(betjson,"range",bet->range);
    jaddnum(betjson,"numrounds",bet->numrounds);
    jaddnum(betjson,"gamestart",Gamestart);
    //jaddnum(betjson,"gamestarted",vars->gamestarted);
    if ( Gamestarted != 0 )
    {
        jaddnum(betjson,"round",vars->round);
        jaddnum(betjson,"turni",vars->turni);
    }
    else if ( Gamestart != 0 )
    {
        jaddnum(betjson,"timestamp",time(NULL));
        jaddnum(betjson,"countdown",Gamestart - time(NULL));
    }
    return(betjson);
}

void BET_betvars_parse(struct privatebet_info *bet,struct privatebet_vars *vars,cJSON *argjson)
{
    vars->turni = jint(argjson,"turni");
    vars->round = jint(argjson,"round");
    if ( bits256_cmp(bet->tableid,Mypubkey) != 0 )
    {
        Gamestart = juint(argjson,"gamestart");
        Gamestarted = juint(argjson,"gamestarted");
    }
    //printf("TURNI.(%s)\n",jprint(argjson,0));
}

int32_t BET_betinfo_parse(struct privatebet_info *bet,struct privatebet_vars *vars,cJSON *msgjson)
{
    int32_t i,n; cJSON *players;
    memset(vars,0,sizeof(*vars));
    if ( (players= jarray(&n,msgjson,"players")) != 0 && n > 0 )
    {
        bet->myplayerid = -1;
        for (i=0; i<n; i++)
        {
            bet->playerpubs[i] = jbits256i(players,i);
            if ( bits256_cmp(bet->playerpubs[i],Mypubkey) == 0 )
                bet->myplayerid = i;
        }
    }
    if ( jstr(msgjson,"game") != 0 )
        safecopy(bet->game,jstr(msgjson,"game"),sizeof(bet->game));
    bet->maxplayers = jint(msgjson,"maxplayers");
    if ( (bet->numplayers= jint(msgjson,"numplayers")) != n )
    {
        bet->numplayers = 0;
        printf("Numplayers %d mismatch %d\n",bet->numplayers,n);
    }
    bet->timestamp = juint(msgjson,"timestamp");
    bet->tableid = jbits256(msgjson,"tableid");
    bet->numrounds = jint(msgjson,"numrounds");
    bet->range = jint(msgjson,"range");
    if ( (bet->numrounds= jint(msgjson,"numrounds")) == 0 )
        bet->numrounds = Maxrounds;
    BET_betvars_parse(bet,vars,msgjson);
    return(bet->numplayers);
}

void BET_tablestatus_send(struct privatebet_info *bet,struct privatebet_vars *vars)
{
    cJSON *tablejson;
    tablejson = BET_betinfo_json(bet,vars);
    BET_message_send("BET_tablestatus_send",bet->pubsock>=0?bet->pubsock:bet->pushsock,tablejson,1,bet);
}
