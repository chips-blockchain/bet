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

cJSON *BET_pubkeys(struct privatebet_info *bet)
{
    int32_t i; cJSON *array = cJSON_CreateArray();
    for (i=0; i<bet->numplayers; i++)
        jaddibits256(array,bet->playerpubs[i]);
    return(array);
}

void BET_message_send(char *debugstr,int32_t sock,cJSON *msgjson,int32_t freeflag,struct privatebet_info *bet)
{
    int32_t sendlen,len; char *msg;
    if ( jobj(msgjson,"sender") != 0 )
        jdelete(msgjson,"sender");
    jaddbits256(msgjson,"sender",Mypubkey);
    if ( jobj(msgjson,"peerid") != 0 )
        jdelete(msgjson,"peerid");
    jaddstr(msgjson,"peerid",LN_idstr);
    if ( jobj(msgjson,"hostrhash") != 0 )
        jdelete(msgjson,"hostrhash");
    if ( jobj(msgjson,"pubkeys") != 0 )
        jdelete(msgjson,"pubkeys");
    if ( jobj(msgjson,"clientrhash") != 0 )
        jdelete(msgjson,"clientrhash");
    if ( jobj(msgjson,"chipsize") != 0 )
        jdelete(msgjson,"chipsize");
    if ( jobj(msgjson,"hostip") != 0 )
        jdelete(msgjson,"hostip");
    if ( jobj(msgjson,"hostid") != 0 )
        jdelete(msgjson,"hostid");
    if ( jobj(msgjson,"LN_port") != 0 )
        jdelete(msgjson,"LN_port");
    if ( IAMHOST != 0 )
    {
        jadd(msgjson,"hostrhash",BET_hostrhashes(bet));
        jadd(msgjson,"pubkeys",BET_pubkeys(bet));
        jaddnum(msgjson,"LN_port",LN_port);
        jaddnum(msgjson,"chipsize",bet->chipsize);
        jaddstr(msgjson,"hostip",Host_ipaddr);
        jaddstr(msgjson,"hostid",Host_peerid);
    } else jaddbits256(msgjson,"clientrhash",BET_clientrhash());
    msg = jprint(msgjson,freeflag);
    len = (int32_t)strlen(msg) + 1;
    if ( (sendlen= nn_send(sock,msg,len,0)) != len )
        printf("%s: sendlen.%d != recvlen.%d for %s, pushsock.%d\n",debugstr,sendlen,len,msg,sock);
    //else printf("SEND.[%s]\n",msg);
    free(msg);
}

void BET_shardkey(uint8_t *key,bits256 deckid,int32_t cardi,int32_t playerj)
{
    memcpy(key,deckid.bytes,sizeof(deckid));
    memcpy(&key[sizeof(deckid)],&cardi,sizeof(cardi));
    memcpy(&key[sizeof(deckid)+sizeof(cardi)],&playerj,sizeof(playerj));
}

struct BET_shardsinfo *BET_shardsfind(bits256 deckid,int32_t cardi,int32_t playerj)
{
    uint8_t key[sizeof(bits256) + sizeof(cardi) + sizeof(playerj)]; struct BET_shardsinfo *shards;
    BET_shardkey(key,deckid,cardi,playerj);
    portable_mutex_lock(&BET_shardmutex);
    HASH_FIND(hh,BET_shardsinfos,key,sizeof(key),shards);
    portable_mutex_unlock(&BET_shardmutex);
    return(shards);
}

void BET_MofN_item(bits256 deckid,int32_t cardi,bits256 *cardpubs,int32_t numcards,int32_t playerj,int32_t numplayers,bits256 shard,int32_t shardi)
{
    void *G; struct BET_shardsinfo *shards; int32_t i,m; uint8_t sharenrs[255],recovernrs[255],space[4096];
    if ( (shards= BET_shardsfind(deckid,cardi,playerj)) == 0 )
    {
        //printf("create new (c%d p%d)\n",cardi,playerj);
        shards = calloc(1,sizeof(*shards) + (numplayers * sizeof(bits256)));
        BET_shardkey(shards->key,deckid,cardi,playerj);
        shards->numcards = numcards;
        shards->numplayers = numplayers;
        portable_mutex_lock(&BET_shardmutex);
        HASH_ADD_KEYPTR(hh,BET_shardsinfos,shards->key,sizeof(shards->key),shards);
        portable_mutex_unlock(&BET_shardmutex);
    } //else printf("extend new (c%d p%d)\n",cardi,playerj);
    if ( shards != 0 )
    {
        shards->data[shardi] = shard;
        for (i=m=0; i<shards->numplayers; i++)
            if ( bits256_nonz(shards->data[i]) != 0 )
                m++;
        if ( m >= (shards->numplayers >> 1) + 1 )
        {
            //printf("got m.%d numplayers.%d M.%d\n",m,shards->numplayers,(shards->numplayers >> 1) + 1);
            gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid));
            //for (i=0; i<numplayers; i++)
            //    printf("%d ",sharenrs[i]);
            //char str[65]; printf("recover calc_sharenrs deckid.%s\n",bits256_str(str,deckid));
            memset(recovernrs,0,sizeof(recovernrs));
            for (i=0; i<shards->numplayers; i++)
                if ( bits256_nonz(shards->data[i]) != 0 )
                    recovernrs[i] = sharenrs[i];
            G = gfshare_initdec(recovernrs,shards->numplayers,sizeof(shards->recover),space,sizeof(space));
            for (i=0; i<shards->numplayers; i++)
                if ( bits256_nonz(shards->data[i]) != 0 )
                    gfshare_dec_giveshare(G,i,shards->data[i].bytes);
            gfshare_dec_newshares(G,recovernrs);
            gfshare_decextract(0,0,G,shards->recover.bytes);
            gfshare_free(G);
            shards->recover = cards777_cardpriv(Myprivkey,cardpubs,numcards,shards->recover);
            char str2[65]; printf("recovered (c%d p%d).%d %s [%d] range.%d\n",cardi,playerj,shardi,bits256_str(str2,shards->recover),shards->recover.bytes[1],numcards);
            if ( shards->recover.bytes[1] >= numcards )
                exit(-1);
        }
    }
}

bits256 *BET_process_packet(bits256 *cardpubs,bits256 *deckidp,bits256 senderpub,bits256 mypriv,uint8_t *decoded,int32_t maxsize,bits256 mypub,uint8_t *sendbuf,int32_t size,int32_t checkplayers,int32_t range)
{
    int32_t j,k,i,n,slen,recvlen,numplayers,numcards,myid=-1; uint8_t *recv; bits256 *MofN,deckid,checkpub,playerpubs[CARDS777_MAXPLAYERS+1]; cJSON *deckjson,*array; char str[65],str2[65];
    slen = (int32_t)strlen((char *)sendbuf);
    if ( (deckjson= cJSON_Parse((char *)sendbuf)) == 0 )
    {
        printf("couldnt parse sendbuf\n");
        return(0);
    }
    if ( (numplayers= jint(deckjson,"numplayers")) <= 0 || numplayers != checkplayers )
    {
        printf("no numplayers\n");
        return(0);
    }
    if ( (numcards= jint(deckjson,"numcards")) <= 0 || numcards != range )
    {
        printf("no numcards or numcards.%d != range.%d (%s)\n",numcards,range,jprint(deckjson,0));
        return(0);
    }
    deckid = jbits256(deckjson,"deckid");
    *deckidp = deckid;
    if ( (array= jarray(&n,deckjson,"players")) != 0 && n == numplayers )
    {
        for (i=0; i<numplayers; i++)
        {
            playerpubs[i] = jbits256i(array,i);
            if ( bits256_cmp(mypub,playerpubs[i]) == 0 )
                myid = i;
        }
        if ( myid < 0 )
        {
            printf("mismatched playerpub[%d of %d]\n",i,numplayers);
            return(0);
        }
    }
    if ( (array= jarray(&n,deckjson,"cardpubs")) != 0 && n == numcards )
    {
        for (i=0; i<numcards; i++)
            cardpubs[i] = jbits256i(array,i);
        checkpub = cards777_deckid(cardpubs,numcards,deckid);
        if ( bits256_cmp(checkpub,deckid) != 0 )
        {
            printf("error comparing deckid %s vs %s\n",bits256_str(str,checkpub),bits256_str(str2,deckid));
            return(0);
        } else printf("verified deckid %s\n",bits256_str(str,deckid));
    }
    if ( memcmp(&sendbuf[slen+1],playerpubs[myid].bytes,sizeof(playerpubs[myid])) == 0 )
    {
        recv = &sendbuf[slen+1+sizeof(bits256)];
        recvlen = size - (int32_t)(slen+1+sizeof(bits256));
        if ( (MofN= (bits256 *)BET_decrypt(decoded,maxsize,senderpub,mypriv,recv,&recvlen)) != 0 )
        {
            if ( recvlen/sizeof(bits256) == numplayers*numcards )
            {
                for (k=0; k<numcards; k++)
                {
                    for (j=0; j<numplayers; j++)
                    {
                        //fprintf(stderr,"[c%d p%d %s].%d ",k,j,bits256_str(str,MofN[k*numplayers + j]),myid);
                        BET_MofN_item(deckid,k,cardpubs,numcards,j,numplayers,MofN[k*numplayers + j],myid);
                    }
                }
                //printf("new recvlen.%d -> %d\n",recvlen,(int32_t)(recvlen/sizeof(bits256)));
                return(MofN);
            } else printf("recvlen %d mismatch p%d c%d\n",recvlen,numplayers,numcards);
        } else printf("decryption error\n");
        return(0);
    }
    return(0);
    // packet for different node
    /*for (i=0; i<32; i++)
     printf("%02x",sendbuf[slen+1+i]);
     printf(" sent pubkey\n");
     for (i=0; i<32; i++)
     printf("%02x",playerpubs[myid].bytes[i]);
     printf(" myid.%d pubkey\n",myid);
     printf("memcmp playerpubs error\n");
     return(-1);*/
}

void BET_broadcast(int32_t pubsock,uint8_t *decoded,int32_t maxsize,bits256 *playerprivs,bits256 *playerpubs,int32_t numplayers,int32_t numcards,uint8_t *sendbuf,int32_t size,bits256 deckid)
{
    int32_t i,slen; bits256 checkdeckid,cardpubs[CARDS777_MAXCARDS];
    slen = (int32_t)strlen((char *)sendbuf);
    if ( pubsock >= 0 )
    {
        nn_send(pubsock,sendbuf,size,0);
    }
    else if ( slen+1+sizeof(bits256) < size && playerprivs != 0 )
    {
        for (i=0; i<numplayers; i++)
        {
            if ( BET_process_packet(cardpubs,&checkdeckid,GENESIS_PUBKEY,playerprivs[i],decoded,maxsize,playerpubs[i],sendbuf,size,numplayers,numcards) != 0 )
                break;
        }
    }
}

void BET_roundstart(int32_t pubsock,cJSON *deckjson,int32_t numcards,bits256 *privkeys,bits256 *playerpubs,int32_t numplayers,bits256 privkey0)
{
    static uint8_t *decoded; static int32_t decodedlen;
    int32_t i,n,len=0,slen,size=0; bits256 deckid; uint8_t *sendbuf=0; cJSON *array; char *deckjsonstr,*msg; cJSON *sendjson;
    deckid = jbits256(deckjson,"deckid");
    if ( (array= jarray(&n,deckjson,"ciphers")) != 0 && n == numplayers )
    {
        sendjson = jduplicate(deckjson);
        jdelete(sendjson,"ciphers");
        jdelete(sendjson,"result");
        deckjsonstr = jprint(sendjson,1);
        //printf("deckjsonstr.(%s)\n",deckjsonstr);
        slen = (int32_t)strlen(deckjsonstr);
        for (i=0; i<numplayers; i++)
        {
            fprintf(stderr,"%d ",i);
            msg = jstri(array,i);
            if ( sendbuf == 0 )
            {
                len = (int32_t)strlen(msg) >> 1;
                size = slen + 1 + sizeof(playerpubs[i]) + len;
                sendbuf = malloc(size);
                memcpy(sendbuf,deckjsonstr,slen+1);
            }
            else if ( (strlen(msg) >> 1) != len )
            {
                printf("[%d of %d] unexpected mismatched len.%d vs %d\n",i,numplayers,(int32_t)strlen(msg),len);
                continue;
            }
            memcpy(&sendbuf[slen+1],playerpubs[i].bytes,sizeof(playerpubs[i]));
            decode_hex(&sendbuf[slen+1+sizeof(playerpubs[i])],len,msg);
            if ( decodedlen < size )
            {
                decoded = realloc(decoded,size);
                decodedlen = size;
                printf("alloc decoded[%d]\n",size);
            }
            BET_broadcast(pubsock,decoded,decodedlen,privkeys,playerpubs,numplayers,numcards,sendbuf,size,deckid);
        }
        free(deckjsonstr);
    }
}

char *BET_transportname(int32_t bindflag,char *str,char *ipaddr,uint16_t port)
{
    sprintf(str,"tcp://%s:%u",bindflag == 0 ? ipaddr : "*",port); // ws is worse
    return(str);
}

int32_t BET_nanosock(int32_t bindflag,char *endpoint,int32_t nntype)
{
    int32_t sock,timeout;
    if ( (sock= nn_socket(AF_SP,nntype)) >= 0 )
    {
        if ( bindflag == 0 )
        {
            if ( nn_connect(sock,endpoint) < 0 )
            {
                printf("connect to %s error for %s\n",endpoint,nn_strerror(nn_errno()));
                nn_close(sock);
                return(-1);
            } else printf("nntype.%d connect to %s connectsock.%d\n",nntype,endpoint,sock);
        }
        else
        {
            if ( nn_bind(sock,endpoint) < 0 )
            {
                printf("bind to %s error for %s\n",endpoint,nn_strerror(nn_errno()));
                nn_close(sock);
                return(-1);
            } else printf("(%s) bound\n",endpoint);
        }
        timeout = 1;
        nn_setsockopt(sock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
        timeout = 100;
        nn_setsockopt(sock,NN_SOL_SOCKET,NN_SNDTIMEO,&timeout,sizeof(timeout));
        //maxsize = 2 * 1024 * 1024;
        //nn_setsockopt(sock,NN_SOL_SOCKET,NN_RCVBUF,&maxsize,sizeof(maxsize));
        if ( nntype == NN_SUB )
            nn_setsockopt(sock,NN_SUB,NN_SUB_SUBSCRIBE,"",0);
    }
    return(sock);
}

void BET_mofn_send(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t cardi,int32_t playerj,int32_t encryptflag)
{
    bits256 shard; cJSON *reqjson; int32_t msglen; uint8_t encoded[sizeof(bits256) + 1024]; char cipherstr[sizeof(encoded)+2+1];
    shard = bet->MofN[cardi*bet->numplayers + playerj];
    if ( bits256_nonz(shard) != 0 )
    {
        reqjson = cJSON_CreateObject();
        jaddstr(reqjson,"method","MofN");
        jaddnum(reqjson,"playerj",playerj);
        jaddnum(reqjson,"cardi",cardi);
        if ( encryptflag == 0 )
            jaddbits256(reqjson,"shard",shard);
        else
        {
            msglen = BET_ciphercreate(Myprivkey,bet->playerpubs[playerj],encoded,shard.bytes,sizeof(shard));
            init_hexbytes_noT(cipherstr,encoded,msglen);
            jaddstr(reqjson,"cipher",cipherstr);
            //char str[65]; printf("%s -> cipherstr.(%s)\n",bits256_str(str,shard),cipherstr);
        }
        //char str[65]; fprintf(stderr,"{j%d c%d %s} ",j,cardi,bits256_str(str,shard));
        BET_message_send("BET_mofn_send",bet->pubsock>=0?bet->pubsock:bet->pushsock,reqjson,1,bet);
    }
    else
    {
        int32_t i; char str[65]; for (i=0; i<bet->numplayers * bet->range; i++)
            printf("%s ",bits256_str(str,bet->MofN[i]));
        printf("MofN.%p\n",bet->MofN);
        printf("null shard cardi.%d/%d j.%d/%d MofN.%p\n",cardi,bet->range,playerj,bet->numplayers,bet->MofN);
    }
}

int32_t BET_client_deali(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid)
{
    int32_t deali,cardi,j;
    deali = jint(argjson,"deali");
    cardi = jint(argjson,"cardi");
    if ( (j= jint(argjson,"playerj")) < 0 )
    {
        for (j=0; j<bet->numplayers; j++)
            BET_mofn_send(bet,vars,cardi,j,0);
    } else BET_mofn_send(bet,vars,cardi,j,1);
    //printf("client deali.%d cardi.%d j.%d\n",deali,cardi,j);
    return(0);
}

