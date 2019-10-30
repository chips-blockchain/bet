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
#include "commands.h"
#include "oracle.h"
#include "client.h"

char BET_ORACLEURL[64] = "127.0.0.1:7797";
int32_t IAMORACLE;
char *multisigAddress="bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ";

int32_t BET_iswatchonly(char *address)
{
	
	int argc,maxsize=100;
	char **argv=NULL;
	cJSON *isWatchOnly=NULL;
	argc=3;

	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"validateaddress");
	strcpy(argv[2],address);
	make_command(argc,argv,&isWatchOnly);

	
	cJSON *temp=cJSON_GetObjectItem(isWatchOnly,"iswatchonly");
	if(strcmp(cJSON_Print(temp),"true")==0)
	{
		return 1;
	}	
	else
		return 0;
	
}

void BET_spentmultisigaddress(char *address,double amount)
{
	cJSON *rawTxInfo=NULL;
	if(BET_iswatchonly(address) == 0)
	{
		BET_importaddress(address);
	}

	rawTxInfo=BET_createrawtransaction(amount,address);

	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(rawTxInfo));
	
	
	
}

void BET_importaddress(char* address)
{
	int argc,maxsize=100;
	char **argv=NULL;
	cJSON *importAddressInfo=NULL;
	argc=3;

	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"importaddress");
	strcpy(argv[2],address);

	make_command(argc,argv,&importAddressInfo);

	printf("%s::%d\n",__FUNCTION__,__LINE__);
			
}


char* BET_getnewaddress()
{
	int argc,maxsize=100;
	char **argv=NULL;
	cJSON *newAddressInfo=NULL;
	argc=2;

	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"getnewaddress");

	make_command(argc,argv,&newAddressInfo);

	printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(newAddressInfo));

	
	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	return cJSON_Print(newAddressInfo);
}

int BET_validateaddress(char* address)
{
	int argc,maxsize=1000;
	char **argv=NULL;
	cJSON *addressInfo=NULL;
	argc=3;

	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"validateaddress");
	strcpy(argv[2],address);
	make_command(argc,argv,&addressInfo);

	cJSON *temp=cJSON_GetObjectItem(addressInfo,"ismine");
	if(strcmp(cJSON_Print(temp),"true")==0)
	{
		return 1;
	}	
	else
		return 0;
}


void BET_listaddressgroupings()
{
	int argc,maxsize=1000;
	char **argv=NULL;
	cJSON *listaddressgroupingsInfo=NULL;

	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	argc=2;
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"listaddressgroupings");
	make_command(argc,argv,&listaddressgroupingsInfo);

	for(int i=0;i<cJSON_GetArraySize(listaddressgroupingsInfo);i++)
	{
		cJSON *addressInfo=cJSON_GetArrayItem(listaddressgroupingsInfo,i);
		for(int j=0;j<cJSON_GetArraySize(addressInfo);j++)
		{
			cJSON *temp=NULL;
			temp=cJSON_GetArrayItem(addressInfo,j);
			cJSON *address=cJSON_GetArrayItem(temp,0);
			if(BET_validateaddress(cJSON_Print(address))==1)
			{
				
				printf("%s::%f\n",cJSON_Print(address),atof(cJSON_Print(cJSON_GetArrayItem(temp,1))));
			}
		}
	}

	
	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}
		
}
cJSON* BET_transferfunds(double amount,char* address)
{
	cJSON *txInfo=NULL,*signedTx=NULL;
	char *rawTransaction=cJSON_str(BET_createrawtransaction(amount,address));
	
	signedTx=BET_signrawtransactionwithwallet(rawTransaction);
	txInfo=BET_sendrawtransaction(signedTx);
	return txInfo;
}
cJSON* BET_sendrawtransaction(cJSON *signedTransaction)
{
	int argc,maxsize=1000;
	char **argv=NULL;
	cJSON *txInfo=NULL;

	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"sendrawtransaction");
	strcpy(argv[2],jstr(signedTransaction,"hex"));
	make_command(argc,argv,&txInfo);

	
	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	return txInfo;
			
}

cJSON* BET_signrawtransactionwithwallet(char *rawtransaction)
{
	int argc,maxsize=1000;
	char **argv=NULL;
	cJSON *signedTransaction=NULL;
	
	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(maxsize*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"signrawtransactionwithwallet");
	strcpy(argv[2],rawtransaction);
	make_command(argc,argv,&signedTransaction);

	
	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	return signedTransaction;	
}


int32_t BET_publishmultisigtransaction(char* tx)
{
	int32_t flag=0,bytes,retval=0;
	cJSON *txInfo=cJSON_CreateObject();
	char* rendered=NULL;
	for(int i=0;i<BET_dcv->numplayers;i++)
	{
		if(is_signed[i]==0)
		{
			cJSON_AddNumberToObject(txInfo,"playerid",i);
			flag=1;
			break;
		}
	}
	if(flag)
	{
		cJSON_AddStringToObject(txInfo,"method","signrawtransaction");
		cJSON_AddStringToObject(txInfo,"tx",tx);
		rendered=cJSON_Print(txInfo);

		bytes=nn_send(BET_dcv->pubsock,rendered,strlen(rendered),0);
		if(bytes<0)
			retval=-1;
	}
	return retval;
}

cJSON* BET_createrawmultisigtransaction(double amount,char* toaddress, char* fromaddress)
{
	char **argv=NULL,*changeAddress=NULL;
	int argc,maxsize=1024;
	cJSON *listunspentInfo=NULL,*addressInfo=NULL,*txListInfo=NULL,*createTX=NULL;
	double balance,change,temp_balance=0,fee=0.0005;

	balance=BET_getbalance();
	txListInfo=cJSON_CreateArray();
	addressInfo=cJSON_CreateObject();

	printf("%s::%d::toaddress::%s::fromaddress::%s\n",__FUNCTION__,__LINE__,toaddress,fromaddress);
	
	if((balance+fee)<amount)
	{
		printf("%s::%d::Insufficient Funds\n",__FUNCTION__,__LINE__);
	}
	else
	{
		cJSON_AddNumberToObject(addressInfo,toaddress,amount);
		amount+=fee;
		
		argc=4;
		argv=(char**)malloc(argc*sizeof(char*));
		for(int i=0;i<argc;i++)
		{
			argv[i]=(char*)malloc(maxsize*sizeof(char));
		}
		strcpy(argv[0],"chips-cli");
		strcpy(argv[1],"listunspent");
		argc=2;
		make_command(argc,argv,&listunspentInfo);
		
		
		for(int i=0;i<cJSON_GetArraySize(listunspentInfo)-1;i++)
		{
			cJSON *temp=cJSON_GetArrayItem(listunspentInfo,i);
			cJSON *txInfo=cJSON_CreateObject();

			if(strcmp(jstr(temp,"address"),fromaddress) == 0)
			{
				temp_balance+=jdouble(temp,"amount");
				if(temp_balance>=amount)
				{
					changeAddress=jstr(temp,"address");
					change=temp_balance-amount;
					cJSON_AddStringToObject(txInfo,"txid",jstr(temp,"txid"));
					cJSON_AddNumberToObject(txInfo,"vout",jint(temp,"vout"));
					cJSON_AddItemToArray(txListInfo,txInfo);
					break;
				}
				else
				{
					cJSON_AddStringToObject(txInfo,"txid",jstr(temp,"txid"));
					cJSON_AddNumberToObject(txInfo,"vout",jint(temp,"vout"));
					cJSON_AddItemToArray(txListInfo,txInfo);

				}
			}
		}
		if(change != 0)
		{
			cJSON_AddNumberToObject(addressInfo,changeAddress,change);
		}
		argc=4;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,maxsize);

		strcpy(argv[0],"chips-cli");
		strcpy(argv[1],"createrawtransaction");
		sprintf(argv[2],"\'%s\'",cJSON_Print(txListInfo));
		sprintf(argv[3],"\'%s\'",cJSON_Print(addressInfo));
		make_command(argc,argv,&createTX);

		if(argv)
		{
			for(int i=0;i<argc;i++)
			{
				if(argv[i])
					free(argv[i]);
					
			}
			free(argv);
		}

		return createTX;
	}
}


cJSON* BET_createrawtransaction(double amount,char* address)
{
	char **argv=NULL,*changeAddress=NULL;
	int argc,maxsize=1024;
	cJSON *listunspentInfo=NULL,*addressInfo=NULL,*txListInfo=NULL,*createTX=NULL;
	double balance,change,temp_balance=0,fee=0.0005;

	balance=BET_getbalance();
	txListInfo=cJSON_CreateArray();
	addressInfo=cJSON_CreateObject();

	if(address==NULL)
	{
		address=(char*)malloc(64*sizeof(char));
		strcpy(address,multisigAddress);
	}	

	printf("%s::%d::address::%s\n",__FUNCTION__,__LINE__,address);
	
	if((balance+fee)<amount)
	{
		printf("%s::%d::Insufficient Funds\n",__FUNCTION__,__LINE__);
	}
	else
	{
		cJSON_AddNumberToObject(addressInfo,address,amount);
		amount+=fee;
		
		argc=4;
		argv=(char**)malloc(argc*sizeof(char*));
		for(int i=0;i<argc;i++)
		{
			argv[i]=(char*)malloc(maxsize*sizeof(char));
		}
		strcpy(argv[0],"chips-cli");
		strcpy(argv[1],"listunspent");
		argc=2;
		make_command(argc,argv,&listunspentInfo);
		
		
		for(int i=0;i<cJSON_GetArraySize(listunspentInfo)-1;i++)
		{
			cJSON *temp=cJSON_GetArrayItem(listunspentInfo,i);
			cJSON *txInfo=cJSON_CreateObject();
			if(strcmp(cJSON_Print(cJSON_GetObjectItem(temp,"spendable")),"true") == 0)
			{
				temp_balance+=jdouble(temp,"amount");
				if(temp_balance>=amount)
				{
					changeAddress=jstr(temp,"address");
					change=temp_balance-amount;
					cJSON_AddStringToObject(txInfo,"txid",jstr(temp,"txid"));
					cJSON_AddNumberToObject(txInfo,"vout",jint(temp,"vout"));
					cJSON_AddItemToArray(txListInfo,txInfo);
					break;
				}
				else
				{
					cJSON_AddStringToObject(txInfo,"txid",jstr(temp,"txid"));
					cJSON_AddNumberToObject(txInfo,"vout",jint(temp,"vout"));
					cJSON_AddItemToArray(txListInfo,txInfo);

				}
			}
		}
		if(change != 0)
		{
			cJSON_AddNumberToObject(addressInfo,changeAddress,change);
		}
		argc=4;
		for(int i=0;i<argc;i++)
			memset(argv[i],0x00,maxsize);

		strcpy(argv[0],"chips-cli");
		strcpy(argv[1],"createrawtransaction");
		sprintf(argv[2],"\'%s\'",cJSON_Print(txListInfo));
		sprintf(argv[3],"\'%s\'",cJSON_Print(addressInfo));
		make_command(argc,argv,&createTX);

		if(argv)
		{
			for(int i=0;i<argc;i++)
			{
				if(argv[i])
					free(argv[i]);
					
			}
			free(argv);
		}

		return createTX;
	}
}


void BET_listunspent()
{
	char **argv=NULL;
	int argc;
	cJSON *listunspentInfo=NULL;
	
	argc=2;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(100*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"listunspent");
	make_command(argc,argv,&listunspentInfo);

	
	for(int i=0;i<cJSON_GetArraySize(listunspentInfo)-1;i++)
	{
		cJSON *temp=cJSON_GetArrayItem(listunspentInfo,i);
		
		if(strcmp(cJSON_Print(cJSON_GetObjectItem(temp,"spendable")),"true") == 0)
		{
			printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(temp));
		}
	}

	
	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}
		
}

int32_t BET_get_chips_blockheight()
{
	char **argv=NULL,*rendered=NULL;
	int argc,height;
	cJSON *blockHeightInfo=NULL;
	
	argc=2;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(100*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"getblockcount");

	make_command(argc,argv,&blockHeightInfo);

	rendered=cJSON_Print(blockHeightInfo);
	height=atoi(rendered);

	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	if(rendered)
		free(rendered);
	if(blockHeightInfo)
		free(blockHeightInfo);
	
	return height;
}

int32_t BET_get_ln_blockheight()
{
	char **argv=NULL;
	int argc,block_height;
	cJSON *blockHeightInfo=NULL;
	
	argc=2;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(100*sizeof(char));
	}
	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"dev-blockheight");

	make_command(argc,argv,&blockHeightInfo);
	block_height=jint(blockHeightInfo,"blockheight");

	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	if(blockHeightInfo)
		free(blockHeightInfo);
	
	return block_height; 
}


void BET_check_sync()
{
	int32_t chips_bh,ln_bh,flag=1;
	int32_t threshold_diff=1000;
	
	chips_bh=BET_get_chips_blockheight();
	ln_bh=BET_get_ln_blockheight();
	
	while(flag)
	{
		if((chips_bh-ln_bh)>threshold_diff)
		{
			printf("\rln is %d blocks behind chips network",(chips_bh-ln_bh));
			fflush(stdout);
		}
		else
		{
			flag=0;
			printf("ln is in sync with chips\n");
	
		}
		
		chips_bh=BET_get_chips_blockheight();
		ln_bh=BET_get_ln_blockheight();
		
	}
}


double BET_getbalance()
{
	char **argv=NULL;
	int argc;
	double balance=0;
	cJSON *getbalanceInfo=NULL;
	argc=2;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		argv[i]=(char*)malloc(100*sizeof(char));
	}
	strcpy(argv[0],"chips-cli");
	strcpy(argv[1],"getbalance");
	make_command(argc,argv,&getbalanceInfo);
	balance=atof(cJSON_Print(getbalanceInfo));

	if(argv)
	{
		for(int i=0;i<argc;i++)
		{
			if(argv[i])
				free(argv[i]);
				
		}
		free(argv);
	}

	if(getbalanceInfo)
		free(getbalanceInfo);

	return balance;
}

int32_t BET_lock_transaction(int32_t fundAmount)
{
	int argc,balance;
	char **argv=NULL;
	cJSON *listunspentInfo=NULL;
	double fee=0.0005;
	
	balance=BET_getbalance();
	if((fundAmount+fee)>=balance)
	{
		argc=2;
		argv=(char**)malloc(argc*sizeof(char*));
		for(int i=0;i<argc;i++)
		{
			argv[i]=(char*)malloc(100*sizeof(char));
		}
		strcpy(argv[0],"chips-cli");
		strcpy(argv[1],"listunspent");
		make_command(argc,argv,&listunspentInfo);
		
		printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(listunspentInfo));
		
		for(int i=0;i<cJSON_GetArraySize(listunspentInfo)-1;i++)
		{
			cJSON *temp=cJSON_GetArrayItem(listunspentInfo,i);
			
			if(strcmp(cJSON_Print(cJSON_GetObjectItem(temp,"spendable")),"true") == 0)
			{
				printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(temp));
			}
		}
	}
	return 0;
}
