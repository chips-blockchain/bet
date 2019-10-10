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
