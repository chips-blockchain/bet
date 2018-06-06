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


