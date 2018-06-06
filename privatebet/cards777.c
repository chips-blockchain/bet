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


bits256 cards777_initcrypt(bits256 data,bits256 privkey,bits256 pubkey,int32_t invert)
{
    bits256 hash;
    hash = curve25519_shared(privkey,pubkey);
    if ( invert != 0 )
        hash = crecip_donna(hash);
    return(fmul_donna(data,hash));
}

bits256 cards777_cardpriv(bits256 playerpriv,bits256 *cardpubs,int32_t numcards,bits256 cipher)
{
    bits256 cardpriv,checkpub; int32_t i;
    for (i=0; i<numcards; i++)
    {
        cardpriv = cards777_initcrypt(cipher,playerpriv,cardpubs[i],1);
        //char str[65]; printf("%s ",bits256_str(str,cardpubs[i]));
        //printf("(%llx %llx) ",(long long)cardpriv.txid,(long long)curve25519_shared(playerpriv,cardpubs[i]).txid);
        checkpub = curve25519(cardpriv,curve25519_basepoint9());
        if ( memcmp(checkpub.bytes,cardpubs[i].bytes,sizeof(bits256)) == 0 )
        {
            //printf("%d ",cardpriv.bytes[1]);
            //printf("decrypted card.%d %llx\n",cardpriv.bytes[1],(long long)cardpriv.txid);
            return(cardpriv);
        }
    }
    //printf("cardpubs\n");
    //printf("\nplayerpriv %llx cipher.%llx\n",(long long)playerpriv.txid,(long long)cipher.txid);
    memset(cardpriv.bytes,0,sizeof(cardpriv));
    return(cardpriv);
}

int32_t cards777_checkcard(bits256 *cardprivp,int32_t cardi,int32_t slot,int32_t destplayer,bits256 playerpriv,bits256 *cardpubs,int32_t numcards,bits256 card)
{
    bits256 cardpriv;
    cardpriv = cards777_cardpriv(playerpriv,cardpubs,numcards,card);
    if ( cardpriv.txid != 0 )
    {
        if ( slot >= 0 && destplayer != slot )
            printf(">>>>>>>>>>>> ERROR ");
        if ( (0) )
            printf("slot.%d B DECODED cardi.%d destplayer.%d cardpriv.[%d]\n",slot,cardi,destplayer,cardpriv.bytes[1]);
        *cardprivp = cardpriv;
        return(cardpriv.bytes[1]);
    }
    memset(cardprivp,0,sizeof(*cardprivp));
    return(-1);
}

int32_t BET_permutation(int32_t *permi,int32_t numcards)
{
    uint32_t x; int32_t i,nonz,n,pos,desti[CARDS777_MAXCARDS]; uint8_t mask[CARDS777_MAXCARDS/8+1];
    memset(desti,0,sizeof(desti));
    for (i=0; i<numcards; i++)
        desti[i] = i;
    n = numcards;
    i = 0;
    while ( n > 0 )
    {
        OS_randombytes((uint8_t *)&x,sizeof(x));
        pos = (x % n);
        //fprintf(stderr,"%d ",pos);
        if ( desti[pos] == -1 )
        {
            printf("n.%d unexpected pos.%d\n",n,pos);
            continue;
        }
        permi[i++] = desti[pos];
        desti[pos] = desti[n - 1];
        desti[n - 1] = -1;
        n--;
    }
    memset(mask,0,sizeof(mask));
    for (i=0; i<numcards; i++)
        SETBIT(mask,permi[i]);
    for (i=nonz=0; i<numcards; i++)
        if ( GETBIT(mask,i) == 0 )
            printf("err.%d ",i), nonz++;
    if ( nonz != 0 )
    {
        for (i=0; i<numcards; i++)
            printf("%d ",desti[i]);
        printf("missing bits.%d\n",nonz);
        return(-nonz);
    }
    else if ( (0) )
    {
        for (i=0; i<numcards; i++)
            printf("%d ",permi[i]);
        printf("PERMI.%d\n",numcards);
    }
    return(0);
}

int32_t BET_permutation_merge(int32_t *permi,int32_t *permiA,int32_t *permiB,int32_t numcards)
{
    int32_t i,nonz; uint8_t mask[CARDS777_MAXCARDS/8+1];
    memset(mask,0,sizeof(mask));
    for (i=nonz=0; i<numcards; i++)
    {
        if ( permiA[i] < 0 || permiA[i] >= numcards || permiB[i] < 0 || permiB[i] >= numcards )
        {
            printf("illegal permi.%d valueA %d or B %d\n",i,permiA[i],permiB[i]);
            return(-1);
        }
        permi[i] = permiB[permiA[i]];
        SETBIT(mask,permi[i]);
    }
    if ( (0) )
    {
        for (i=0; i<numcards; i++)
            printf("%3d ",permiA[i]);
        printf("permiA\n");
        for (i=0; i<numcards; i++)
            printf("%3d ",permiB[i]);
        printf("permiB\n");
        for (i=0; i<numcards; i++)
            printf("%3d ",permi[i]);
        printf("permi\n\n");
    }
    for (i=0; i<numcards; i++)
        if ( GETBIT(mask,i) == 0 )
            printf("err.%d ",i), nonz++;
    if ( nonz != 0 )
    {
        printf("missing combined bits.%d\n",nonz);
        return(-nonz);
    }
    return(0);
}

uint64_t BET_permutation_metric(int32_t *permis,int32_t numcards)
{
    int32_t i; uint64_t hash = 0;
    for (hash=i=0; i<numcards; i++)
        hash ^= (hash << 5) + (hash >> 2) + (uint32_t)permis[i];                     \
    return(hash);
}

int32_t BET_permutation_sort(int32_t *combined,int32_t permis[][CARDS777_MAXCARDS],int32_t numpermis,int32_t numcards)
{
    int32_t i,permi[CARDS777_MAXCARDS]; uint64_t metrics[CARDS777_MAXPLAYERS][2];
    if ( numpermis == 1 )
    {
        memcpy(combined,permis[0],numcards * sizeof(*combined));
        return(0);
    }
    for (i=0; i<numpermis; i++)
    {
        metrics[i][0] = BET_permutation_metric(permis[i],numcards);
        metrics[i][1] = i;
    }
    sort64s(&metrics[0][0],numpermis,sizeof(metrics[0])); // must make it deterministic!
    for (i=0; i<numcards; i++)
        combined[i] = i;
    for (i=0; i<numpermis; i++)
    {
        //printf("%d ",(int32_t)metrics[i][1]);
        if ( BET_permutation_merge(permi,combined,permis[metrics[i][1]],numcards) < 0 )
            return(-1);
        memcpy(combined,permi,sizeof(*permi) * numcards);
    }
    //printf("permisort.%d\n",numpermis);
    /*total_slow += (OS_milliseconds() - startmillis);
    startmillis = OS_milliseconds();
    n = numpermis;
    for (i=j=0; i+1<n; i+=2)
    {
        if ( BET_permutation_merge(permi,permis[metrics[i][1]],permis[metrics[i+1][1]],numcards) < 0 )
            return(-1);
        memcpy(permis[j++],permi,sizeof(*permi) * numcards);
    }
    if ( (n & 1) != 0 )
        memcpy(permis[j++],permis[metrics[n-1][1]],sizeof(*permi) * numcards);
    n = j;
    while ( n > 1 )
    {
        for (i=j=0; i+1<n; i+=2)
        {
            if ( BET_permutation_merge(permi,permis[i],permis[i+1],numcards) < 0 )
                return(-1);
            memcpy(permis[j++],permi,sizeof(*permi) * numcards);
        }
        if ( (n & 1) != 0 )
            memcpy(permis[j++],permis[n-1],sizeof(*permi) * numcards);
        n = j;
    }
    total_fast += (OS_milliseconds() - startmillis);
    if ( memcmp(permis[0],combined,sizeof(*permi) * numcards) != 0 )
    {
        for (i=0; i<numcards; i++)
            printf("%3d ",permis[0][i]);
        printf("merged\n");
        for (i=0; i<numcards; i++)
            printf("%3d ",combined[i]);
        printf("combined\n\n");
    } else printf("matched! %.3f vs %.3f ratio %.2f\n",total_slow,total_fast,total_slow/total_fast);
    */
    return((int32_t)metrics[0][1]);
}

#ifdef oldway
int32_t cards777_validate(bits256 cardpriv,bits256 final,bits256 *cardpubs,int32_t numcards,bits256 *audit,int32_t numplayers,bits256 playerpub)
{
    int32_t i; bits256 val,checkcard,ver;
    val = final;
    for (i=numplayers-1; i>0; i--)
    {
        val = fmul_donna(audit[i],val);
        //if ( memcmp(tmp.bytes,audit[i-1].bytes,sizeof(tmp)) != 0 )
        //    printf("cards777_validate: mismatched audit[%d] %llx vs %llx %llx\n",i-1,(long long)tmp.txid,(long long)audit[i-1].txid,(long long)audit[i].txid);
    }
    checkcard = val;
    if ( memcmp(checkcard.bytes,audit[0].bytes,sizeof(checkcard)) != 0 )
    {
        printf("cards777_validate: checkcard not validated %llx vs %llx numplayers.%d\n",(long long)checkcard.txid,(long long)audit[0].txid,numplayers);
        return(-1);
    }
    ver = cards777_initcrypt(cardpriv,cardpriv,playerpub,0);
    if ( memcmp(checkcard.bytes,ver.bytes,sizeof(checkcard)) != 0 )
    {
        printf("cards777_validate: ver not validated %llx vs %llx\n",(long long)checkcard.txid,(long long)ver.txid);
        return(-1);
    }
    return(cardpriv.bytes[1]);
}

int32_t cards777_calcmofn(uint8_t *allshares,uint8_t *myshares[],uint8_t *sharenrs,int32_t M,bits256 *xoverz,int32_t numcards,int32_t N)
{
    int32_t size,j; uint8_t space[8192];
    size = N * sizeof(bits256) * numcards;
    gfshare_calc_shares(allshares,(void *)xoverz,size,size,M,N,sharenrs,space,sizeof(space)); // PM &allshares[playerj * size] to playerJ
    for (j=0; j<N; j++)
        myshares[j] = &allshares[j * size];
    return(size);
}

uint8_t *cards777_recover(uint8_t *shares[],uint8_t *sharenrs,int32_t M,int32_t numcards,int32_t N)
{
    void *G; int32_t i,size; uint8_t *recover,recovernrs[255],space[8192];
    size = N * sizeof(bits256) * numcards;
    if ( (recover= calloc(1,size)) == 0 )
    {
        printf("cards777_recover: unexpected out of memory error\n");
        return(0);
    }
    memset(recovernrs,0,sizeof(recovernrs));
    for (i=0; i<N; i++)
        if ( shares[i] != 0 )
            recovernrs[i] = sharenrs[i];
    G = gfshare_initdec(recovernrs,N,size,space,sizeof(space));
    for (i=0; i<N; i++)
        if ( shares[i] != 0 )
            gfshare_dec_giveshare(G,i,shares[i]);
    gfshare_dec_newshares(G,recovernrs);
    gfshare_decextract(0,0,G,recover);
    gfshare_free(G);
    return(recover);
}

void cards777_layer(bits256 *layered,bits256 *xoverz,bits256 *incards,int32_t numcards,int32_t N)
{
    int32_t i,k,nonz = 0; bits256 z_x;
    for (i=nonz=0; i<numcards; i++)
    {
        for (k=0; k<N; k++,nonz++)
        {
            xoverz[nonz] = xoverz_donna(rand256(1));
            z_x = crecip_donna(xoverz[nonz]);
            layered[nonz] = fmul_donna(z_x,incards[nonz]);
            //printf("{%llx -> %llx}.%d ",(long long)incards[nonz].txid,(long long)layered[nonz].txid,nonz);
        }
        //printf("card.%d\n",i);
    }
}

int32_t cards777_shuffle(bits256 *shuffled,bits256 *cards,int32_t numcards,int32_t N)
{
    int32_t i,j,nonz,permi[CARDS777_MAXCARDS];
    if ( numcards == 1 )
    {
        shuffled[0] = cards[0];
        return(0);
    }
    BET_permutation(permi,numcards);
    //printf("pos\n");
    for (i=nonz=0; i<numcards; i++)
    {
        if ( (0) )
            printf("%d ",permi[i]);
        for (j=0; j<N; j++,nonz++)
            shuffled[nonz] = cards[permi[i]*N + j];//, printf("%llx ",(long long)shuffled[nonz].txid);
    }
    return(0);
}

uint8_t *cards777_encode(bits256 *encoded,bits256 *xoverz,uint8_t *allshares,uint8_t *myshares[],uint8_t sharenrs[255],int32_t M,bits256 *ciphers,int32_t numcards,int32_t N)
{
    bits256 shuffled[CARDS777_MAXCARDS * CARDS777_MAXPLAYERS];
    cards777_shuffle(shuffled,ciphers,numcards,N);
    cards777_layer(encoded,xoverz,shuffled,numcards,N);
    //memset(sharenrs,0,255);
    //gfshare_init_sharenrs(sharenrs,0,N,N);
    //cards777_calcmofn(allshares,myshares,sharenrs,M,xoverz,numcards,N);
    memcpy(ciphers,shuffled,numcards * N * sizeof(bits256));
    if ( (0) )
    {
        int32_t i,j,m,size; uint8_t *recover,*testshares[CARDS777_MAXPLAYERS],testnrs[255];
        size = N * sizeof(bits256) * numcards;
        for (j=0; j<1; j++)
        {
            memset(testnrs,0,sizeof(testnrs));
            memset(testshares,0,sizeof(testshares));
            m = (rand() % N) + 1;
            if ( m < M )
                m = M;
            if ( gfshare_init_sharenrs(testnrs,sharenrs,m,N) < 0 )
            {
                printf("iter.%d error init_sharenrs(m.%d of n.%d)\n",j,m,N);
                return(0);
            }
            for (i=0; i<N; i++)
                if ( testnrs[i] == sharenrs[i] )
                    testshares[i] = myshares[i];
            if ( (recover= cards777_recover(testshares,sharenrs,M,numcards,N)) != 0 )
            {
                if ( memcmp(xoverz,recover,size) != 0 )
                    fprintf(stderr,"(ERROR m.%d M.%d N.%d)\n",m,M,N);
                else fprintf(stderr,"reconstructed with m.%d M.%d N.%d\n",m,M,N);
                free(recover);
            } else printf("nullptr from cards777_recover\n");
        }
    }
    return(allshares);
}

bits256 cards777_decode(bits256 *seedp,bits256 *xoverz,int32_t destplayer,bits256 cipher,bits256 *outcards,int32_t numcards,int32_t N)
{
    int32_t i,ind;
    memset(seedp->bytes,0,sizeof(*seedp));
    for (i=0; i<numcards; i++)
    {
        ind = i*N + destplayer;
        //printf("[%llx] ",(long long)outcards[ind].txid);
        if ( memcmp(outcards[ind].bytes,cipher.bytes,32) == 0 )
        {
            *seedp = xoverz[ind];
            cipher = fmul_donna(xoverz[ind],cipher);
            //printf("matched %d -> %llx\n",i,(long long)cipher.txid);
            return(cipher);
        }
    }
    if ( i == numcards )
    {
        printf("decryption error %llx: destplayer.%d no match\n",(long long)cipher.txid,destplayer);
        memset(cipher.bytes,0,sizeof(cipher));
        //cipher = cards777_cardpriv(playerpriv,cardpubs,numcards,cipher);
    }
    return(cipher);
}
#endif

bits256 cards777_deckid(bits256 *pubkeys,int32_t numcards,bits256 cmppubkey)
{
    int32_t i; bits256 bp,pubkey,hash,check,prod;
    memset(check.bytes,0,sizeof(check));
    memset(bp.bytes,0,sizeof(bp)), bp.bytes[0] = 9;
    prod = fmul_donna(bp,crecip_donna(bp));
    for (i=0; i<numcards; i++)
    {
        pubkey = pubkeys[i];
        vcalc_sha256(0,hash.bytes,pubkey.bytes,sizeof(pubkey));
        hash.bytes[0] &= 0xf8, hash.bytes[31] &= 0x7f, hash.bytes[31] |= 64;
        prod = fmul_donna(prod,hash);
    }
    check = prod;
    if ( cmppubkey.txid != 0 )
    {
        if ( memcmp(check.bytes,cmppubkey.bytes,sizeof(check)) != 0 )
            printf("cards777_deckid: mismatched pubkeys permicheck.%llx != prod.%llx\n",(long long)check.txid,(long long)pubkey.txid);
        //else printf("pubkeys matched\n");
    }
    return(check);
}

bits256 cards777_initdeck(bits256 *cards,bits256 *cardpubs,int32_t numcards,int32_t N,bits256 *playerpubs,bits256 *playerprivs)
{
    bits256 privkey,pubkey,hash, bp,prod; int32_t i,j,nonz,num = 0; uint8_t mask[CARDS777_MAXCARDS/8+1];
    bp = curve25519_basepoint9();
    prod = crecip_donna(bp);
    prod = fmul_donna(bp,prod);
    //printf("card777_initdeck unit.%llx numcards.%d numplayers.%d\n",(long long)prod.txid,numcards,N);
    nonz = 0;
    memset(mask,0,sizeof(mask));
    while ( num < numcards )
    {
        privkey = curve25519_keypair(&pubkey);
        if ( (i=privkey.bytes[1]) < numcards && GETBIT(mask,i) == 0 )
        {
            SETBIT(mask,i);
            cardpubs[num] = pubkey;
            if ( playerprivs != 0 )
                printf("%llx.",(long long)privkey.txid);
            for (j=0; j<N; j++,nonz++)
            {
                cards[nonz] = cards777_initcrypt(privkey,privkey,playerpubs[j],0);
                //char str[65]; printf("%s ",bits256_str(str,privkey));
                if ( playerprivs != 0 )
                    printf("[%llx * %llx -> %llx] ",(long long)cards[nonz].txid,(long long)curve25519_shared(playerprivs[j],pubkey).txid,(long long)cards777_initcrypt(cards[nonz],playerprivs[j],pubkey,1).txid);
            }
            vcalc_sha256(0,hash.bytes,pubkey.bytes,sizeof(pubkey));
            hash.bytes[0] &= 0xf8, hash.bytes[31] &= 0x7f, hash.bytes[31] |= 64;
            prod = fmul_donna(prod,hash);
            //char str[65]; printf("[%d] cardi.%d %s\n",i,num,bits256_str(str,cardpubs[num]));
            //printf("(%s) num.%d [%llx] %d prod.%llx\n",buf,num,(long long)mask ^ ((1LL << numcards)-1),i,(long long)prod.txid);
            num++;
        }
    }
    if ( playerprivs != 0 )
        printf("\n%llx %llx playerprivs\n",(long long)playerprivs[0].txid,(long long)playerprivs[1].txid);
    if ( (0) )
    {
        //char str[65];
        //for (i=0; i<numcards; i++)
        //    printf("%s ",bits256_str(str,cardpubs[i]));
        for (i=0; i<numcards; i++)
            printf("%d ",cards[i*N].bytes[1]);
        printf("init order %llx (%llx %llx)\n",(long long)prod.txid,(long long)playerpubs[0].txid,(long long)playerpubs[1].txid);
    }
    return(prod);
}

int32_t BET_ciphercreate(bits256 privkey,bits256 destpub,uint8_t *cipher,uint8_t *data,int32_t datalen)
{
    int32_t msglen; uint32_t crc32; uint8_t *nonce,*buffer,*ptr;
    msglen = datalen;
    nonce = cipher;
    OS_randombytes(nonce,crypto_box_NONCEBYTES);
    ptr = &cipher[crypto_box_NONCEBYTES];
    buffer = malloc(datalen+crypto_box_NONCEBYTES+crypto_box_ZEROBYTES+256);
    msglen = _SuperNET_cipher(nonce,ptr,data,msglen,destpub,privkey,buffer);
    crc32 = calc_crc32(0,ptr,msglen);
    msglen += crypto_box_NONCEBYTES;
    if ( (0) )
    {
        char str[65]; fprintf(stderr,"encode into msglen.%d crc32.%u [%llx] %s\n",msglen,crc32,*(long long *)cipher,bits256_str(str,curve25519(privkey,destpub)));
    }
    free(buffer);
    return(msglen);
}

uint8_t *BET_decrypt(uint8_t *decoded,int32_t maxsize,bits256 senderpub,bits256 mypriv,uint8_t *ptr,int32_t *recvlenp)
{
    uint8_t *nonce,*cipher,*dest=0; int32_t recvlen,cipherlen,i;
    recvlen = *recvlenp;
    nonce = ptr;
    cipher = &ptr[crypto_box_NONCEBYTES];
    cipherlen = (recvlen - crypto_box_NONCEBYTES);

    if ( (0) )
    {
        int32_t i; char str[65];
        for (i=0; i<recvlen; i++)
            printf("%02x",ptr[i]);
        printf(" decrypt [%llx] recvlen.%d crc32.%u %s\n",*(long long *)ptr,recvlen,calc_crc32(0,cipher,cipherlen),bits256_str(str,curve25519(mypriv,senderpub)));
    }
    if ( cipherlen > 0 && cipherlen <= maxsize )
    {
        if ( (dest= _SuperNET_decipher(nonce,cipher,decoded,cipherlen,senderpub,mypriv)) != 0 )
        {
            recvlen = (cipherlen - crypto_box_ZEROBYTES);
            if ( (0) )
            {
                int32_t i;
                for (i=0; i<recvlen; i++)
                    printf("%02x",dest[i]);
                printf(" decrypted\n");
            }
        }
    } else printf("cipher.%d too big for %d\n",cipherlen,maxsize);
    *recvlenp = recvlen;
    return(dest);
}

char *BET_createdeck(cJSON *argjson)
{
    int32_t i,j,k,range,numplayers,size,M,msglen; bits256 *allshares,*cardshares,deckid,*cards,*cardpubs,*playerpubs; char *cipherstr; uint8_t sharenrs[256],space[8192],*encoded; cJSON *cardarray,*ciphers,*pubkeys,*retjson = cJSON_CreateObject();
    if ( (range= juint(argjson,"range")) <= 0 )
        range = 52;
    if ( (pubkeys= jarray(&numplayers,argjson,"pubkeys")) != 0 && numplayers > 0 )
    {
        if ( range > CARDS777_MAXCARDS || numplayers > CARDS777_MAXPLAYERS )
            return(clonestr("{\"error\":\"too many cards or players\"}"));
        cards = calloc(sizeof(*cards),range * numplayers); // raw cards + primary blinds
        cardpubs = calloc(sizeof(*cardpubs),range);
        playerpubs = calloc(sizeof(*playerpubs),numplayers);
        for (i=0; i<numplayers; i++)
            playerpubs[i] = jbits256i(pubkeys,i);
        deckid = cards777_initdeck(cards,cardpubs,range,numplayers,playerpubs,Debug_privkeys);
        memset(sharenrs,0,sizeof(sharenrs));
        gfshare_calc_sharenrs(sharenrs,numplayers,deckid.bytes,sizeof(deckid));
        for (i=0; i<numplayers; i++)
            printf("%d ",sharenrs[i]);
        char str[65]; printf("calc_sharenrs deckid.%s\n",bits256_str(str,deckid));
        cardshares = calloc(numplayers,sizeof(bits256));
        allshares = calloc(numplayers,sizeof(bits256)*numplayers*range);
        M = numplayers/2 + 1;
        for (j=0; j<numplayers; j++)
        {
            fprintf(stderr,"%d ",j);
            for (i=0; i<range; i++)
            {
                gfshare_calc_shares(cardshares[0].bytes,cards[i*numplayers + j].bytes,sizeof(bits256),sizeof(bits256),M,numplayers,sharenrs,space,sizeof(space));
                for (k=0; k<numplayers; k++)
                {
                    allshares[k*numplayers*range + (i*numplayers + j)] = cardshares[k];
                    //char str[65]; printf("(c%d p%d %s) ",i,j,bits256_str(str,cardshares[k]));
                }
                if ( (0) )
                {
                    bits256 recover;
                    void *G = gfshare_initdec(sharenrs,numplayers,sizeof(recover),space,sizeof(space));
                    for (k=0; k<numplayers; k++)
                        if ( bits256_nonz(cardshares[k]) != 0 )
                            gfshare_dec_giveshare(G,k,cardshares[k].bytes);
                    gfshare_dec_newshares(G,sharenrs);
                    gfshare_decextract(0,0,G,recover.bytes);
                    gfshare_free(G);
                    recover = cards777_cardpriv(Myprivkey,cardpubs,range,recover);
                    //printf("[j%d i%d %s] ",j,i,bits256_str(str,recover));
                }
            }
        }
        size = sizeof(bits256) * numplayers * range;
        printf("size %d numplayers.%d range.%d, alloc.%d\n",size,numplayers,range,size + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + 2);
        encoded = calloc(1,size + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + 2 + 256);
        cipherstr = calloc(1,2 * (size + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + 2) + 1 + 512);
        ciphers = cJSON_CreateArray();
        for (k=0; k<numplayers; k++)
        {
            //fprintf(stderr,"cipher %d of %d: ",i,numplayers);
            if ( (0) )
            {
                for (i=0; i<numplayers*range*32; i++)
                    printf("%02x",((uint8_t *)allshares[k * numplayers * range].bytes)[i]);
                printf(" encrypt p%d c%d\n",numplayers,range);
            }
            msglen = BET_ciphercreate(GENESIS_PRIVKEY,playerpubs[k],encoded,allshares[k * numplayers * range].bytes,size);
            init_hexbytes_noT(cipherstr,encoded,msglen);
            if ( Debug_privkeys != 0 )
            {
                uint8_t decoded[100000],*ptr; int32_t recvlen; char str[65];
                fprintf(stderr,"size %d -> msglen.%d (%s)\n",size,msglen,cipherstr);
                recvlen = msglen;
                if ( (ptr= BET_decrypt(decoded,sizeof(decoded),GENESIS_PUBKEY,Debug_privkeys[k],encoded,&recvlen)) == 0 )
                    printf("decrypt error j.%d %s\n",k,bits256_str(str,Debug_privkeys[k])), exit(-1);
                else if ( memcmp(ptr,allshares[k * numplayers * range].bytes,size) != 0 )
                    printf("decrypt error for k.%d numplayers.%d range.%d\n",k,numplayers,range);
            }
            jaddistr(ciphers,cipherstr);
        }
        jadd(retjson,"ciphers",ciphers);
        jaddstr(retjson,"result","success");
        jaddbits256(retjson,"deckid",deckid);
        cardarray = cJSON_CreateArray();
        for (i=0; i<range; i++)
            jaddibits256(cardarray,cardpubs[i]);
        jadd(retjson,"cardpubs",cardarray);
        jadd(retjson,"players",jduplicate(pubkeys));
        jaddnum(retjson,"numplayers",numplayers);
        jaddnum(retjson,"numcards",range);
        jaddnum(retjson,"size",size);
        jaddstr(retjson,"method","deckpacket");
        free(encoded);
        free(cipherstr);
        free(allshares);
        free(cardshares);
        free(cards);
        free(cardpubs);
        free(playerpubs);
    } else jaddstr(retjson,"error","nopubkeys");
    //printf("created deck.(%s)\n",jprint(retjson,0));
    return(jprint(retjson,1));
}
