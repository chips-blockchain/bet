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

char *BET_gameresult(cJSON *argjson)
{
    return(clonestr("{\"error\":\"sportsbetting is not yet\"}"));
}

char *BET_oracle_command(void *ctx,char *method,cJSON *argjson)
{
    //printf("got oracle.(%s)\n",jprint(argjson,0));
    if ( strcmp(method,"createdeck") == 0 )
        return(BET_createdeck(argjson));
    else if ( strcmp(method,"gameresult") == 0 )
        return(BET_gameresult(argjson));
    else return(clonestr("{\"error\":\"unknown oracle request\"}"));
}

cJSON *BET_createdeck_request(bits256 *pubkeys,int32_t numplayers,int32_t range)
{
    int32_t i; cJSON *pubs,*reqjson;
    pubs = cJSON_CreateArray();
    for (i=0; i<numplayers; i++)
        jaddibits256(pubs,pubkeys[i]);
    reqjson = cJSON_CreateObject();
    jaddstr(reqjson,"agent","bet");
    jaddstr(reqjson,"method","createdeck");
    jaddnum(reqjson,"range",range);
    jadd(reqjson,"pubkeys",pubs);
    return(reqjson);
}

char *BET_oracle_request(char *method,cJSON *reqjson)
{
    static int32_t maxlen;
    char *retstr,*params; int32_t n;
    params = jprint(reqjson,0);
    if ( (retstr= bitcoind_passthrut("bet",BET_ORACLEURL,"",method,params,30)) != 0 )
    {
        if ( (n= (int32_t)strlen(retstr)) > 0 && retstr[n-1] == '\n' )
            retstr[n-1] = 0;
        if ( n > maxlen )
            maxlen = n;
        printf("%s %s -> (%d) max.%d\n",method,params,n,maxlen);
    } else printf("null return from %s %s\n",method,jprint(reqjson,0));
    free(params);
    return(retstr);
}

