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
#include "payment.h"
#include "network.h"
#include "commands.h"
bits256 Host_rhashes[256]; 
//bits256 Mypubkey;
bits256 Clientrhash;
//char Host_channel[64];
//char *LN_idstr,Host_peerid[67];
//int32_t Num_hostrhashes,Chips_paid;

bits256 BET_clientrhash()
{
    return(Clientrhash);
}

void BET_chip_recv(char *label,struct privatebet_info *bet)
{
    cJSON *reqjson;
    if ( label != 0 && label[0] != 0 )
    {
        reqjson = cJSON_CreateObject();
        jaddstr(reqjson,"method","onechip");
        jaddstr(reqjson,"label",label);
        BET_message_send("BET_chip_recv",bet->pubsock>=0?bet->pubsock:bet->pushsock,reqjson,1,bet);
    }
}

void BET_hostrhash_update(bits256 rhash)
{
    int32_t i;
    //char str[65]; printf("hostrhash update.(%s)\n",bits256_str(str,rhash));
    if ( Num_hostrhashes < sizeof(Host_rhashes)/sizeof(*Host_rhashes) )
    {
        for (i=0; i<Num_hostrhashes; i++)
            if ( bits256_cmp(rhash,Host_rhashes[i]) == 0 )
                return;
        Host_rhashes[Num_hostrhashes++] = rhash;
    }
}

bits256 BET_hosthash_extract(cJSON *argjson,int32_t chipsize)
{
    cJSON *array; bits256 hash,hostrhashes[CARDS777_MAXPLAYERS+1]; int32_t i,n;
    if ( (array= jarray(&n,argjson,"hostrhash")) != 0 )
    {
        for (i=0; i<n; i++)
            hostrhashes[i] = jbits256i(array,i);
        if ( (array= jarray(&n,argjson,"pubkeys")) != 0 && chipsize == jint(argjson,"chipsize") )
        {
            for (i=0; i<n; i++)
            {
                hash = jbits256i(array,i);
                if ( bits256_cmp(hash,Mypubkey) == 0 && Host_peerid[0] != 0 && Host_channel[0] != 0 )
                {
                    BET_hostrhash_update(hostrhashes[i]);
                    return(hostrhashes[i]);
                }
            }
        }
    }
    memset(hash.bytes,0,sizeof(hash));
    return(hash);
}

// { "label" : "0", "rhash" : "366a0c6add6c09a47f001ca92dcf2635012663c52d0a1e4bd634e5f876d29a5e", "msatoshi" : 1000000000, "complete" : false }
struct privatebet_peerln *BET_invoice_complete(char *nextlabel,cJSON *item,struct privatebet_info *bet)
{
    char *label,peerstr[67]; int32_t ind; cJSON *retjson; uint8_t peerid[33]; struct privatebet_peerln *p = 0; bits256 rhash;
    rhash = jbits256(item,"rhash");
    if ( j64bits(item,"msatoshi") != bet->chipsize*1000 )
    {
        printf("mismatched msatoshi %llu != %d*1000\n",(long long)j64bits(item,"msatoshi"),bet->chipsize);
        return(0);
    }
    if ( (label= jstr(item,"label")) != 0 )
    {
        if ( is_hexstr(label,0) == 66 )
        {
            decode_hex(peerid,sizeof(peerid),label);
            memcpy(peerstr,label,66);
            peerstr[66] = 0;
            if ( (p= BET_peerln_find(peerstr)) != 0 )
            {
                if ( label[66] == '_' )
                {
                    ind = atoi(label+66+1);
                    if ( ind >= 0 )
                    {
                        char str[65],str2[65];
                        if ( bits256_cmp(p->hostrhash,rhash) != 0 )
                            printf("warning rhash mismatch %s != %s\n",bits256_str(str,rhash),bits256_str(str2,p->hostrhash));
                        else
                        {
                            sprintf(nextlabel,"%s_%d",peerstr,ind+1);
                            //p->hostrhash = chipsln_rhash_create(bet->chipsize,nextlabel);
                            printf("updated rhash for %s to %s\n",peerstr,bits256_str(str,p->hostrhash));
                        }
                    }
                }
                BET_chip_recv(label,bet);
                if ( (retjson= chipsln_delpaidinvoice(label)) != 0 )
                {
                    printf("delpaidinvoice.(%s) -> (%s)\n",label,jprint(retjson,0));
                    free_json(retjson);
                }
            }
        }
    }
    return(p);
}

int32_t BET_clientpay(uint64_t chipsize)
{
    bits256 rhash,preimage; cJSON *routejson,*retjson,*array; int32_t avail,n=0,retval = -1;
    printf("BET_clientpay.%llu %.8f -> (%s)\n",(long long)chipsize,dstr(chipsize),Host_peerid);
    if ( Host_peerid[0] != 0 && Host_channel[0] != 0 && (n= Num_hostrhashes) > 0 )
    {
        if ( (avail= (int32_t)BET_peer_chipsavail(Host_peerid,chipsize)) < 2 )
        {
            if ( (retjson= chipsln_close(Host_channel)) != 0 )
            {
                printf("close.(%s) -> (%s)\n",Host_channel,jprint(retjson,0));
                free_json(retjson);
            }
            printf("%s numchips.%d error\n",Host_peerid,avail);
            Host_channel[0] = 0;
            system("./fund"); // addfunds
            return(-2);
        }
        printf("chips avail.%d\n",avail);
        rhash = Host_rhashes[n-1];
        if ( bits256_nonz(rhash) != 0 && BET_peer_state(Host_peerid,"CHANNELD_NORMAL") == 0 )
        {
            if ( BET_channel_status(Host_peerid,Host_channel,"active") == 0 )
            {
                array = routejson = 0;
                if ( 0 )
                {
                    array = cJSON_CreateArray();
                    routejson = cJSON_CreateObject();
                    jaddstr(routejson,"id",Host_peerid);
                    jaddstr(routejson,"channel",Host_channel);
                    jaddnum(routejson,"msatoshi",chipsize*1000);
                    jaddnum(routejson,"delay",100);
                    jaddi(array,routejson);
                }
                else
                {
                    if ( (retjson= chipsln_getroute(Host_peerid,chipsize*1000)) != 0 )
                    {
                        printf("getroute.(%s)\n",jprint(retjson,0));
                        if ( (routejson= jarray(&n,retjson,"route")) != 0 )
                            array = jduplicate(routejson);
                    }
                }
                // route { "id" : "02779b57b66706778aa1c7308a817dc080295f3c2a6af349bb1114b8be328c28dc", "channel" : "27446:1:0", "msatoshi" : 1000000, "delay" : 10 }
                // replace rhash in route
                if ( array != 0 )
                {
                    if ( (retjson= chipsln_sendpay(array,rhash)) != 0 )
                    {
                        char str[65],str2[65];
                        preimage = jbits256(retjson,"preimage");
                        printf("sendpay rhash.(%s) %.8f to %s -> %s preimage.%s\n",bits256_str(str,rhash),dstr(chipsize),jprint(array,0),jprint(retjson,0),bits256_str(str2,preimage));
                        if ( bits256_nonz(preimage) != 0 )
                        {
                            Chips_paid++;
                            // if valid, reduce Host_rhashes[]
                            if ( Num_hostrhashes > 0 )
                            {
                                Num_hostrhashes--;
                                retval = 0;
                            }
                        } else printf("ERROR doing sendpay.%s <- %.8f\n",bits256_str(str,rhash),dstr(chipsize));
                        free_json(retjson);
                    } else printf("sendpay null return?\n");
                } else printf("no route\n");
                free_json(array);
            } else printf("channel not ready\n");
        } else printf("peer not ready\n");
    } else printf("cant pay Host_channel.(%s) numchips.%d\n",Host_channel,n);
    return(retval);
}

void BET_channels_parse()
{
    cJSON *channels,*array,*item,*peers; int32_t i,n,len; char *channel,*peerid,*source,*dest,*short_id;
    if ( (peers= chipsln_getpeers()) != 0 )
    {
        //printf("got.(%s)\n",jprint(channels,0));
        if ( (array= jarray(&n,peers,"peers")) != 0 )
        {
            for (i=0; i<n; i++)
            {
                item = jitem(array,i);
                if ( (peerid= jstr(item,"peerid")) != 0 && strcmp(peerid,Host_peerid) == 0 )
                {
                    if ( (channel= jstr(item,"channel")) != 0 )
                    {
                        safecopy(Host_channel,channel,sizeof(Host_channel));
                        printf("Got Host_channel from channel.(%s)\n",channel);
                        free_json(peers);
                        return;
                    }
                    break;
                }
            }
        }
        free_json(peers);
    }
    if ( (channels= chipsln_getchannels()) != 0 )
    {
        //printf("got.(%s)\n",jprint(channels,0));
        if ( (array= jarray(&n,channels,"channels")) != 0 )
        {
            for (i=0; i<n; i++)
            {
                item = jitem(array,i);
                source = jstr(item,"source");
                dest = jstr(item,"destination");
                short_id = jstr(item,"short_id");
                //printf("source.%s dest.%s myid.%s Host.%s short.%s\n",source,dest,LN_idstr,Host_peerid,short_id);
                if ( source != 0 && dest != 0 && strcmp(source,LN_idstr) == 0 && strcmp(dest,Host_peerid) == 0 && short_id != 0 )
                {
                    len = strlen(short_id);
                    if ( len > 3 && short_id[len-2] == '/' )
                    {
                        strcpy(Host_channel,short_id);
                        Host_channel[len-2] = 0;
                        printf("Host_channel.(%s)\n",Host_channel);
                    }
                }
            }
        }
        free_json(channels);
    }
}

/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/
int32_t BET_DCV_create_invoice_request(struct privatebet_info *bet,int32_t playerid, int32_t amount)
{
	int32_t retval=1,bytes;
	cJSON *betInfo=NULL;
	char *rendered=NULL;

	betInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(betInfo,"method","invoiceRequest_player");
	cJSON_AddNumberToObject(betInfo,"playerID",playerid);
	cJSON_AddNumberToObject(betInfo,"betAmount",amount);

	rendered=cJSON_Print(betInfo);

	bytes=nn_send(bet->pubsock,rendered,strlen(rendered),0);

	if(bytes<0)
	{
			retval=-1;
			printf("\n%s:%d: Failed to send data",__FUNCTION__,__LINE__);
			goto end;
	}

	end:
		return retval;
	
}

int32_t BET_DCV_invoice_pay(struct privatebet_info *bet,struct privatebet_vars *vars,int playerid,int amount)
{
	pthread_t pay_t;
	int32_t retval=1;

	  retval=BET_DCV_create_invoice_request(bet,playerid,amount);
   	  if (OS_thread_create(&pay_t,NULL,(void *)BET_DCV_paymentloop,(void *)bet) != 0 )
	  {
		  exit(-1);
	  }   
	  if(pthread_join(pay_t,NULL))
	  {
		  printf("\nError in joining the main thread for player %d",bet->myplayerid);
		  retval=-1;
	  }

	return retval;
}

int32_t BET_DCV_pay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars)
{
	
	cJSON *invoiceInfo=NULL,*paymentInfo=NULL,*payResponse=NULL;
	int argc,retval=1,bytes;
	char **argv=NULL,*rendered=NULL,*invoice=NULL;

	argv=(char**)malloc(4*sizeof(char*));
	argc=3;
	for(int i=0;i<4;i++)
	{
		argv[i]=(char*)malloc(sizeof(char)*1000);
	}
	invoice=jstr(argjson,"invoice");
	invoiceInfo=cJSON_Parse(invoice);

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"pay");
	sprintf(argv[2],"%s",jstr(invoiceInfo,"bolt11"));
	argv[3]=NULL;
	
	payResponse=cJSON_CreateObject();
	make_command(argc,argv,&payResponse);

	printf("\n%s::%d::payment response\n",__FUNCTION__,__LINE__);
	printf("%s\n",cJSON_Print(payResponse));
	
	if(jint(payResponse,"code") != 0)
	{
		retval=-1;
		printf("\n%s:%d: Message:%s",__FUNCTION__,__LINE__,jstr(payResponse,"message"));
		goto end;
	}
	/*	
	if(strcmp(jstr(payResponse,"status"),"complete")==0)
		printf("\nPayment Success");
	*/	
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

void BET_DCV_paymentloop(void * _ptr)
{	
	int32_t recvlen,retval=1;
    uint8_t flag=1;
	void *ptr;
	cJSON *msgjson=NULL;
	char *method=NULL;
	struct privatebet_info *bet = _ptr;
	while ( flag )
    {
        
        if ( bet->subsock >= 0 && bet->pushsock >= 0 )
        {
	        	recvlen= nn_recv (bet->pullsock, &ptr, NN_MSG, 0);
                if (( (msgjson= cJSON_Parse(ptr)) != 0 ) && (recvlen>0))
                {
                   if ( (method= jstr(msgjson,"method")) != 0 )
    			   {
    			   		if(strcmp(method,"invoice") == 0)
			   			{
			   				retval=BET_DCV_pay(msgjson,bet,NULL);
							flag=0;
			   			}
                   }
                   
                    free_json(msgjson);
                }
                
        }
        
    }   
 
}



/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/

int32_t BET_player_create_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,char* deckid)
{
	int argc,bytes,retval=1;
	char **argv=NULL,hexstr [65],*rendered=NULL;
	cJSON *invoiceInfo=NULL,*invoice=NULL;
	if(jint(argjson,"playerid")==bet->myplayerid)
	{
		argv =(char**)malloc(6*sizeof(char*));
		for(int i=0;i<5;i++)
		{
				argv[i]=(char*)malloc(sizeof(char)*1000);
		}
		
		strcpy(argv[0],"lightning-cli");
		strcpy(argv[1],"invoice");
		sprintf(argv[2],"%d",jint(argjson,"betAmount"));
		sprintf(argv[3],"%s_%d",deckid,jint(argjson,"betAmount"));
		sprintf(argv[4],"Winning claim");
		argv[5]=NULL;
		argc=5;

		invoice=cJSON_CreateObject();
		make_command(argc,argv,&invoice);
		//ln_bet(argc,argv,buf);
		invoiceInfo=cJSON_CreateObject();
		cJSON_AddStringToObject(invoiceInfo,"method","invoice");
		cJSON_AddNumberToObject(invoiceInfo,"playerid",bet->myplayerid);
		cJSON_AddStringToObject(invoiceInfo,"label",argv[3]);
		cJSON_AddStringToObject(invoiceInfo,"invoice",cJSON_Print(invoice));

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

int32_t BET_player_create_invoice_request(cJSON *argjson,struct privatebet_info *bet,int32_t amount)
{
	int32_t retval=1,bytes;
	cJSON *betInfo=NULL;
	char *rendered=NULL;

	betInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(betInfo,"method","invoiceRequest");
	cJSON_AddNumberToObject(betInfo,"round",jint(argjson,"round"));
	cJSON_AddNumberToObject(betInfo,"playerID",bet->myplayerid);
	cJSON_AddNumberToObject(betInfo,"betAmount",amount);

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

int32_t BET_player_invoice_pay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int amount)
{
	pthread_t pay_t;
	int32_t retval=1;

	  retval=BET_player_create_invoice_request(argjson,bet,amount);
   	  if (OS_thread_create(&pay_t,NULL,(void *)BET_player_paymentloop,(void *)bet) != 0 )
	  {
		  exit(-1);
	  }   
	  if(pthread_join(pay_t,NULL))
	  {
		  printf("\nError in joining the main thread for player %d",bet->myplayerid);
		  retval=-1;
	  }

	return retval;
}

void BET_player_paymentloop(void * _ptr)
{	
	int32_t recvlen,retval=1;
    uint8_t flag=1;
	void *ptr;
	cJSON *msgjson=NULL;
	char *method=NULL;
	struct privatebet_info *bet = _ptr;
	while ( flag )
    {
        
        if ( bet->subsock >= 0 && bet->pushsock >= 0 )
        {
	        	recvlen= nn_recv (bet->subsock, &ptr, NN_MSG, 0);
                if (( (msgjson= cJSON_Parse(ptr)) != 0 ) && (recvlen>0))
                {
                   if ( (method= jstr(msgjson,"method")) != 0 )
    			   {
    			   		if(strcmp(method,"invoice") == 0)
			   			{
			   				retval=BET_p2p_invoice(msgjson,bet,NULL);
							flag=0;
			   			}
                   }
                   
                    free_json(msgjson);
                }
                
        }
        
    }   
 
}


/*
Below are the payment related REST API's
*/

int32_t BET_rest_player_create_invoice_request_round(struct lws *wsi,cJSON *argjson,int32_t amount,int32_t option)
{
	int32_t retval=1,bytes;
	cJSON *betInfo=NULL;
	

	betInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(betInfo,"method","invoiceRequest");
	cJSON_AddNumberToObject(betInfo,"round",jint(argjson,"round"));
	cJSON_AddNumberToObject(betInfo,"playerID",BET_player[jint(argjson,"gui_playerID")]->myplayerid);
	cJSON_AddNumberToObject(betInfo,"betAmount",amount);
	cJSON_AddNumberToObject(betInfo,"option",option);
	cJSON_AddItemToObject(betInfo,"payment_params",argjson);
	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(betInfo));
	lws_write(wsi,cJSON_Print(betInfo),strlen(cJSON_Print(betInfo)),0);
	
	end:
		return retval;
	
}



int32_t BET_rest_player_create_invoice_request(struct lws *wsi,cJSON *argjson,int32_t amount)
{
	int32_t retval=1,bytes;
	cJSON *betInfo=NULL;
	

	betInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(betInfo,"method","invoiceRequest");
	cJSON_AddNumberToObject(betInfo,"round",jint(argjson,"round"));
	cJSON_AddNumberToObject(betInfo,"playerID",BET_player[jint(argjson,"gui_playerID")]->myplayerid);
	cJSON_AddNumberToObject(betInfo,"betAmount",amount);
	cJSON_AddItemToObject(betInfo,"payment_params",argjson);
	printf("%s:%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(betInfo));
	lws_write(wsi,cJSON_Print(betInfo),strlen(cJSON_Print(betInfo)),0);
	
	end:
		return retval;
	
}

int32_t BET_rest_DCV_create_invoice_request(struct lws *wsi,int32_t amount,int32_t playerID)
{
	int32_t retval=1;
	
	cJSON *invoiceRequestInfo=NULL;

	invoiceRequestInfo=cJSON_CreateObject();
	cJSON_AddStringToObject(invoiceRequestInfo,"method","winningInvoiceRequest");
	cJSON_AddNumberToObject(invoiceRequestInfo,"playerID",playerID);
	cJSON_AddNumberToObject(invoiceRequestInfo,"winningAmount",amount);

	lws_write(wsi,cJSON_Print(invoiceRequestInfo),strlen(cJSON_Print(invoiceRequestInfo)),0);
	
	return retval;
}


