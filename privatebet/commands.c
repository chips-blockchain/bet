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
#include "commands.h"
#include "bet.h"
#include "client.h"
#include "common.h"
#include "oracle.h"
#include "network.h"
#include "cashier.h"

char *multisigAddress = "bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ";

int32_t chips_iswatchonly(char *address)
{
	int argc, maxsize = 100;
	char **argv = NULL;
	cJSON *is_watch_only = NULL;
	argc = 3;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "validateaddress");
	strcpy(argv[2], address);
	make_command(argc, argv, &is_watch_only);

	cJSON *temp = cJSON_GetObjectItem(is_watch_only, "iswatchonly");
	if (strcmp(cJSON_Print(temp), "true") == 0) {
		return 1;
	} else
		return 0;
}

void chips_spend_multi_sig_address(char *address, double amount)
{
	cJSON *raw_tx = NULL;
	if (chips_iswatchonly(address) == 0) {
		chips_import_address(address);
	}

	raw_tx = chips_create_raw_tx(amount, address);

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(raw_tx));
}

void chips_import_address(char *address)
{
	int argc, maxsize = 100;
	char **argv = NULL;
	cJSON *import_address = NULL;
	argc = 3;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "importaddress");
	strcpy(argv[2], address);

	make_command(argc, argv, &import_address);

	printf("%s::%d\n", __FUNCTION__, __LINE__);
}

char *chips_get_new_address()
{
	int argc, maxsize = 100;
	char **argv = NULL;
	cJSON *new_address = NULL;
	argc = 2;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getnewaddress");

	make_command(argc, argv, &new_address);

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(new_address));

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return cJSON_Print(new_address);
}

int chips_validate_address(char *address)
{
	int argc, maxsize = 1000;
	char **argv = NULL;
	cJSON *address_info = NULL;
	argc = 3;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "validateaddress");
	strcpy(argv[2], address);
	make_command(argc, argv, &address_info);

	cJSON *temp = cJSON_GetObjectItem(address_info, "ismine");
	if (strcmp(cJSON_Print(temp), "true") == 0) {
		return 1;
	} else
		return 0;
}

void chips_list_address_groupings()
{
	int argc, maxsize = 1000;
	char **argv = NULL;
	cJSON *list_address_groupings = NULL;

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	argc = 2;
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "listaddressgroupings");
	make_command(argc, argv, &list_address_groupings);

	for (int i = 0; i < cJSON_GetArraySize(list_address_groupings); i++) {
		cJSON *address_info = cJSON_GetArrayItem(list_address_groupings, i);
		for (int j = 0; j < cJSON_GetArraySize(address_info); j++) {
			cJSON *temp = NULL;
			temp = cJSON_GetArrayItem(address_info, j);
			cJSON *address = cJSON_GetArrayItem(temp, 0);
			if (chips_validate_address(cJSON_Print(address)) == 1) {
				printf("%s::%f\n", cJSON_Print(address),
				       atof(cJSON_Print(cJSON_GetArrayItem(temp, 1))));
			}
		}
	}

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
}
cJSON *chips_transfer_funds(double amount, char *address)
{
	cJSON *tx_info = NULL, *signed_tx = NULL;
	char *raw_tx = cJSON_str(chips_create_raw_tx(amount, address));

	signed_tx = chips_sign_raw_tx_with_wallet(raw_tx);
	tx_info = chips_send_raw_tx(signed_tx);
	return tx_info;
}
cJSON *chips_send_raw_tx(cJSON *signed_tx)
{
	int argc, maxsize = 2048;
	char **argv = NULL;
	cJSON *tx_info = NULL;

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "sendrawtransaction");
	strcpy(argv[2], jstr(signed_tx, "hex"));
	make_command(argc, argv, &tx_info);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return tx_info;
}

cJSON *chips_sign_raw_tx_with_wallet(char *raw_tx)
{
	int argc, maxsize = 2048;
	char **argv = NULL;
	cJSON *signed_tx = NULL;

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(maxsize * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "signrawtransactionwithwallet");
	strcpy(argv[2], raw_tx);
	make_command(argc, argv, &signed_tx);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return signed_tx;
}

int32_t chips_publish_multisig_tx(char *tx)
{
	int32_t flag = 0, bytes, retval = 0;
	cJSON *tx_info = cJSON_CreateObject();
	char *rendered = NULL;
	for (int i = 0; i < bet_dcv->numplayers; i++) {
		if (is_signed[i] == 0) {
			cJSON_AddNumberToObject(tx_info, "playerid", i);
			flag = 1;
			break;
		}
	}
	if (flag) {
		cJSON_AddStringToObject(tx_info, "method", "signrawtransaction");
		cJSON_AddStringToObject(tx_info, "tx", tx);
		rendered = cJSON_Print(tx_info);

		bytes = nn_send(bet_dcv->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0)
			retval = -1;
	}
	return retval;
}

cJSON *chips_create_raw_multi_sig_tx(double amount, char *to_addr, char *from_addr)
{
	char **argv = NULL, *changeAddress = NULL;
	int argc, maxsize = 1024;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, temp_balance = 0, fee = 0.0005;

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	printf("%s::%d::toaddress::%s::fromaddress::%s\n", __FUNCTION__, __LINE__, to_addr, from_addr);

	if ((balance + fee) < amount) {
		printf("%s::%d::Insufficient Funds\n", __FUNCTION__, __LINE__);
	} else {
		cJSON_AddNumberToObject(address_info, to_addr, amount);
		amount += fee;

		argc = 4;
		argv = (char **)malloc(argc * sizeof(char *));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(maxsize * sizeof(char));
		}
		strcpy(argv[0], "chips-cli");
		strcpy(argv[1], "listunspent");
		argc = 2;
		make_command(argc, argv, &listunspent_info);

		for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
			cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
			cJSON *tx_info = cJSON_CreateObject();

			if (strcmp(jstr(temp, "address"), from_addr) == 0) {
				temp_balance += jdouble(temp, "amount");
				if (temp_balance >= amount) {
					changeAddress = jstr(temp, "address");
					change = temp_balance - amount;
					cJSON_AddStringToObject(tx_info, "txid", jstr(temp, "txid"));
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "vout"));
					cJSON_AddItemToArray(tx_list, tx_info);
					break;
				} else {
					cJSON_AddStringToObject(tx_info, "txid", jstr(temp, "txid"));
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "vout"));
					cJSON_AddItemToArray(tx_list, tx_info);
				}
			}
		}
		if (change != 0) {
			cJSON_AddNumberToObject(address_info, changeAddress, change);
		}
		argc = 4;
		for (int i = 0; i < argc; i++)
			memset(argv[i], 0x00, maxsize);

		strcpy(argv[0], "chips-cli");
		strcpy(argv[1], "createrawtransaction");
		sprintf(argv[2], "\'%s\'", cJSON_Print(tx_list));
		sprintf(argv[3], "\'%s\'", cJSON_Print(address_info));
		make_command(argc, argv, &tx);

		if (argv) {
			for (int i = 0; i < argc; i++) {
				if (argv[i])
					free(argv[i]);
			}
			free(argv);
		}

		return tx;
	}
}

cJSON *chips_create_raw_tx(double amount, char *address)
{
	char **argv = NULL, *changeAddress = NULL;
	int argc, maxsize = 1024;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, temp_balance = 0, fee = 0.0005;

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	if (address == NULL) {
		address = (char *)malloc(64 * sizeof(char));
		strcpy(address, multisigAddress);
	}

	printf("%s::%d::address::%s\n", __FUNCTION__, __LINE__, address);

	if ((balance + fee) < amount) {
		printf("%s::%d::Insufficient Funds\n", __FUNCTION__, __LINE__);
	} else {
		cJSON_AddNumberToObject(address_info, address, amount);
		amount += fee;

		argc = 4;
		argv = (char **)malloc(argc * sizeof(char *));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(maxsize * sizeof(char));
		}
		strcpy(argv[0], "chips-cli");
		strcpy(argv[1], "listunspent");
		argc = 2;
		make_command(argc, argv, &listunspent_info);

		for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
			cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
			cJSON *tx_info = cJSON_CreateObject();
			if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")), "true") == 0) {
				temp_balance += jdouble(temp, "amount");
				if (temp_balance >= amount) {
					changeAddress = jstr(temp, "address");
					change = temp_balance - amount;
					cJSON_AddStringToObject(tx_info, "txid", jstr(temp, "txid"));
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "vout"));
					cJSON_AddItemToArray(tx_list, tx_info);
					break;
				} else {
					cJSON_AddStringToObject(tx_info, "txid", jstr(temp, "txid"));
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "vout"));
					cJSON_AddItemToArray(tx_list, tx_info);
				}
			}
		}
		if (change != 0) {
			cJSON_AddNumberToObject(address_info, changeAddress, change);
		}
		argc = 4;
		for (int i = 0; i < argc; i++)
			memset(argv[i], 0x00, maxsize);

		strcpy(argv[0], "chips-cli");
		strcpy(argv[1], "createrawtransaction");
		sprintf(argv[2], "\'%s\'", cJSON_Print(tx_list));
		sprintf(argv[3], "\'%s\'", cJSON_Print(address_info));
		make_command(argc, argv, &tx);

		if (argv) {
			for (int i = 0; i < argc; i++) {
				if (argv[i])
					free(argv[i]);
			}
			free(argv);
		}

		return tx;
	}
}

void chips_list_unspent()
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL;

	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(100 * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "listunspent");
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
		cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);

		if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")), "true") == 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp));
		}
	}

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
}

int32_t chips_get_block_count()
{
	char **argv = NULL, *rendered = NULL;
	int argc, height;
	cJSON *blockHeightInfo = NULL;

	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(100 * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getblockcount");

	make_command(argc, argv, &blockHeightInfo);

	rendered = cJSON_Print(blockHeightInfo);
	height = atoi(rendered);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	if (rendered)
		free(rendered);
	if (blockHeightInfo)
		free(blockHeightInfo);

	return height;
}

int32_t ln_dev_block_height()
{
	char **argv = NULL;
	int argc, block_height;
	cJSON *blockHeightInfo = NULL;

	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(100 * sizeof(char));
	}
	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "dev-blockheight");

	make_command(argc, argv, &blockHeightInfo);
	block_height = jint(blockHeightInfo, "blockheight");

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	if (blockHeightInfo)
		free(blockHeightInfo);

	return block_height;
}

void check_ln_chips_sync()
{
	int32_t chips_bh, ln_bh, flag = 1;
	int32_t threshold_diff = 1000;

	chips_bh = chips_get_block_count();
	ln_bh = ln_dev_block_height();

	while (flag) {
		if ((chips_bh - ln_bh) > threshold_diff) {
			printf("\rln is %d blocks behind chips network", (chips_bh - ln_bh));
			fflush(stdout);
		} else {
			flag = 0;
			printf("ln is in sync with chips\n");
		}

		chips_bh = chips_get_block_count();
		ln_bh = ln_dev_block_height();
	}
}

double chips_get_balance()
{
	char **argv = NULL;
	int argc;
	double balance = 0;
	cJSON *getbalanceInfo = NULL;
	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(100 * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getbalance");
	make_command(argc, argv, &getbalanceInfo);
	balance = atof(cJSON_Print(getbalanceInfo));

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	if (getbalanceInfo)
		free(getbalanceInfo);

	return balance;
}

int32_t chips_lock_transaction(int32_t fund_amount)
{
	int argc, balance;
	char **argv = NULL;
	cJSON *listunspent_info = NULL;
	double fee = 0.0005;

	balance = chips_get_balance();
	if ((fund_amount + fee) >= balance) {
		argc = 2;
		argv = (char **)malloc(argc * sizeof(char *));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(100 * sizeof(char));
		}
		strcpy(argv[0], "chips-cli");
		strcpy(argv[1], "listunspent");
		make_command(argc, argv, &listunspent_info);

		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(listunspent_info));

		for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
			cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);

			if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")), "true") == 0) {
				printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp));
			}
		}
	}
	return 0;
}

cJSON *chips_add_multisig_address()
{
	int argc;
	char **argv = NULL;
	cJSON *addr_list = NULL;
	cJSON *msig_address = NULL;

	argc = 5;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(1000 * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "addmultisigaddress");
	sprintf(argv[2], "%d", threshold_value);
	addr_list = cJSON_CreateArray();

	for (int i = 0; i < no_of_notaries; i++)
		cJSON_AddItemToArray(addr_list, cJSON_CreateString_Length(notary_node_pubkeys[i], 67));

	strcpy(argv[3], cJSON_Print(cJSON_CreateString(cJSON_Print(addr_list))));
	strcpy(argv[4], "-addresstype legacy");

	msig_address = cJSON_CreateObject();
	make_command(argc, argv, &msig_address);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return msig_address;
}

int32_t chips_check_if_tx_unspent(char *input_tx)
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL;
	int32_t spendable = 0;

	printf("%s::%d::%s ", __FUNCTION__, __LINE__, input_tx);
	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(100 * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "listunspent");
	listunspent_info = cJSON_CreateObject();
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) {
		cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
		if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "txid")), input_tx) == 0) {
			if (strcmp(jstr(temp, "address"), legacy_2_of_4_msig_Addr) == 0) {
				spendable = 1;
				break;
			}
		}
	}
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return spendable;
}

char *chips_get_block_hash_from_txid(char *txid)
{
	int argc, arg_size = 1024;
	char **argv = NULL;
	cJSON *raw_tx_info = NULL;
	char *block_hash = NULL;

	argc = 4;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(arg_size * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getrawtransaction");
	strcpy(argv[2], txid);
	strcpy(argv[3], "1");

	raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx_info);
	block_hash = jstr(raw_tx_info, "blockhash");
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return block_hash;
}

int32_t chips_get_block_height_from_block_hash(char *block_hash)
{
	int argc, arg_size = 1024, block_height;
	char **argv = NULL;
	cJSON *block_info = NULL;

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(arg_size * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getblock");
	strcpy(argv[2], block_hash);
	block_info = cJSON_CreateObject();
	make_command(argc, argv, &block_info);
	block_height = jint(block_info, "height");

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return block_height;
}

static int32_t check_if_tx_exists(char *txid, int32_t no_of_txs, char tx_ids[][100])
{
	int32_t exists = 0;
	for (int32_t i = 0; i < no_of_txs; i++) {
		if (strcmp(tx_ids[i], txid) == 0) {
			exists = 1;
			break;
		}
	}
	return exists;
}

cJSON *chips_create_tx_from_tx_list(char *to_addr, int32_t no_of_txs, char tx_ids[][100])
{
	int argc, arg_size = 2048;
	char **argv = NULL;
	cJSON *listunspent_info = NULL;
	double amount = 0;
	cJSON *tx_list = NULL, *to_addr_info = NULL, *tx = NULL;

	to_addr_info = cJSON_CreateObject();
	tx_list = cJSON_CreateArray();
	argc = 4;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(arg_size * sizeof(char));
	}
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "listunspent");
	listunspent_info = cJSON_CreateObject();
	argc = 2;
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
		cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
		if (temp) {
			if (check_if_tx_exists(jstr(temp, "txid"), no_of_txs, tx_ids) == 1) {
				cJSON *tx_info = cJSON_CreateObject();
				amount += jdouble(temp, "amount");
				cJSON_AddStringToObject(tx_info, "txid", jstr(temp, "txid"));
				cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "vout"));
				cJSON_AddItemToArray(tx_list, tx_info);
			}
		}
	}
	cJSON_AddNumberToObject(to_addr_info, to_addr, (amount - chips_tx_fee));
	argc = 4;
	for (int i = 0; i < argc; i++)
		memset(argv[i], 0x00, arg_size);

	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "createrawtransaction");
	sprintf(argv[2], "\'%s\'", cJSON_Print(tx_list));
	sprintf(argv[3], "\'%s\'", cJSON_Print(to_addr_info));
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return tx;
}

cJSON *chips_sign_msig_tx(char *ip, cJSON *raw_tx)
{
	int32_t c_subsock, c_pushsock, bytes, recvlen;
	uint16_t cashier_pubsub_port = 7901, cashier_pushpull_port = 7902;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	void *ptr;
	cJSON *argjson = NULL, *msig_raw_tx = NULL;

	bet_tcp_sock_address(0, bind_sub_addr, ip, cashier_pubsub_port);
	c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

	bet_tcp_sock_address(0, bind_push_addr, ip, cashier_pushpull_port);
	c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", raw_tx);
	bytes = nn_send(c_pushsock, cJSON_Print(msig_raw_tx), strlen(cJSON_Print(msig_raw_tx)), 0);
	if (bytes < 0)
		return NULL;

	while (c_pushsock >= 0 && c_subsock >= 0) {
		ptr = 0;
		if ((recvlen = nn_recv(c_subsock, &ptr, NN_MSG, 0)) > 0) {
			char *tmp = clonestr(ptr);
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
				return argjson;
			}
			if (tmp)
				free(tmp);
			if (ptr)
				nn_freemsg(ptr);
		}
	}
	return NULL;
}

cJSON *chips_spend_msig_txs(char *to_addr, int no_of_txs, char tx_ids[][100])
{
	int signers = 0;
	cJSON *hex = NULL, *tx = NULL;

	bet_check_notary_status();
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			if (signers == 0) {
				cJSON *temp = chips_sign_msig_tx(
					notary_node_ips[i], chips_create_tx_from_tx_list(to_addr, no_of_txs, tx_ids));
				hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "hex");
				signers++;
			} else if (signers == 1) {
				cJSON *temp1 = chips_sign_msig_tx(notary_node_ips[i], hex);
				cJSON *status =
					cJSON_GetObjectItem(cJSON_GetObjectItem(temp1, "signed_tx"), "complete");
				if (strcmp(cJSON_Print(status), "true") == 0) {
					tx = chips_send_raw_tx(cJSON_GetObjectItem(temp1, "signed_tx"));
					signers++;
					break;
				}
			}
		}
	}
	return tx;
}
