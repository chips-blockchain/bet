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
#define _POSIX_C_SOURCE 200809L /* For pclose, popen, strdup */

#include "commands.h"
#include "bet.h"
#include "client.h"
#include "common.h"
#include "oracle.h"
#include "network.h"
#include "cashier.h"

#include <stdarg.h>
#include <stdlib.h>

#define arg_size 8192

char *multisigAddress = "bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ";

static int32_t bet_alloc_args(int argc, char ***argv)
{
	int ret = 1;

	*argv = (char **)malloc(argc * sizeof(char *));
	if (*argv == NULL)
		return 0;
	for (int i = 0; i < argc; i++) {
		(*argv)[i] = (char *)malloc(arg_size * sizeof(char));
		if ((*argv)[i] == NULL)
			return 0;
	}
	return ret;
}

static int32_t bet_dealloc_args(int argc, char ***argv)
{
	if (*argv) {
		for (int i = 0; i < argc; i++) {
			if ((*argv)[i])
				free((*argv)[i]);
		}
		free(*argv);
	}
}

static char **bet_copy_args(int argc, ...)
{
	va_list valist;
	char **argv = NULL;

	bet_alloc_args(argc, &argv);
	va_start(valist, argc);

	for (int i = 0; i < argc; i++) {
		strcpy(argv[i], va_arg(valist, char *));
	}
	return argv;
}

int32_t chips_iswatchonly(char *address)
{
	int argc, retval = 0;
	char **argv = NULL;
	cJSON *is_watch_only = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "validateaddress", address);
	is_watch_only = cJSON_CreateObject();
	make_command(argc, argv, &is_watch_only);

	cJSON *temp = cJSON_GetObjectItem(is_watch_only, "iswatchonly");
	if (strcmp(cJSON_Print(temp), "true") == 0)
		retval = 1;
	bet_dealloc_args(argc, &argv);
	return retval;
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

cJSON *chips_import_address(char *address)
{
	int argc;
	char **argv = NULL;
	cJSON *import_address = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "importaddress", address);
	import_address = cJSON_CreateObject();
	make_command(argc, argv, &import_address);
	bet_dealloc_args(argc, &argv);
	return import_address;
}

char *chips_get_new_address()
{
	int argc;
	char **argv = NULL;
	cJSON *new_address = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getnewaddress");
	new_address = cJSON_CreateObject();
	make_command(argc, argv, &new_address);
	bet_dealloc_args(argc, &argv);
	return cJSON_Print(new_address);
}

int chips_validate_address(char *address)
{
	int argc, retval = 0;
	char **argv = NULL;
	cJSON *address_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "validateaddress", address);
	address_info = cJSON_CreateObject();
	make_command(argc, argv, &address_info);
	cJSON *temp = cJSON_GetObjectItem(address_info, "ismine");
	if (strcmp(cJSON_Print(temp), "true") == 0)
		retval = 1;
	bet_dealloc_args(argc, &argv);
	return retval;
}

void chips_list_address_groupings()
{
	int argc;
	char **argv = NULL;
	cJSON *list_address_groupings = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listaddressgroupings");
	list_address_groupings = cJSON_CreateObject();
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
	bet_dealloc_args(argc, &argv);
}

cJSON *chips_transfer_funds_with_data(double amount, char *address, char *data)
{
	cJSON *tx_info = NULL, *signed_tx = NULL;
	char *raw_tx = cJSON_str(chips_create_raw_tx_with_data(amount, address, data));

	signed_tx = chips_sign_raw_tx_with_wallet(raw_tx);
	tx_info = chips_send_raw_tx(signed_tx);
	return tx_info;
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
	int argc;
	char **argv = NULL;
	cJSON *tx_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "sendrawtransaction", jstr(signed_tx, "hex"));
	tx_info = cJSON_CreateObject();
	make_command(argc, argv, &tx_info);
	bet_dealloc_args(argc, &argv);

	return tx_info;
}

cJSON *chips_sign_raw_tx_with_wallet(char *raw_tx)
{
	int argc;
	char **argv = NULL;
	cJSON *signed_tx = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "signrawtransactionwithwallet", raw_tx);
	signed_tx = cJSON_CreateObject();
	make_command(argc, argv, &signed_tx);
	bet_dealloc_args(argc, &argv);
	return signed_tx;
}

int32_t chips_publish_multisig_tx(char *tx)
{
	int32_t flag = 0, bytes, retval = 0;
	cJSON *tx_info = NULL;
	char *rendered = NULL;

	tx_info = cJSON_CreateObject();
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
	char **argv = NULL, *changeAddress = NULL, params[2][arg_size] = { 0 };
	int argc;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, temp_balance = 0;

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	if ((balance + chips_tx_fee) < amount) {
		return NULL;
	} else {
		cJSON_AddNumberToObject(address_info, to_addr, amount);
		amount += chips_tx_fee;

		argc = 2;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "listunspent");
		make_command(argc, argv, &listunspent_info);
		bet_dealloc_args(argc, &argv);

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
		bet_alloc_args(argc, &argv);
		snprintf(params[0], arg_size, "\'%s\'", cJSON_Print(tx_list));
		snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(address_info));
		argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
		make_command(argc, argv, &tx);
		bet_dealloc_args(argc, &argv);
		return tx;
	}
}

cJSON *chips_create_raw_tx(double amount, char *address)
{
	char **argv = NULL, *changeAddress = NULL, params[2][arg_size] = { 0 };
	int argc;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, temp_balance = 0;

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	if (address == NULL) {
		address = (char *)malloc(64 * sizeof(char));
		strcpy(address, multisigAddress);
	}

	if ((balance + chips_tx_fee) < amount) {
		return NULL;
	} else {
		cJSON_AddNumberToObject(address_info, address, amount);
		amount += chips_tx_fee;

		argc = 2;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "listunspent");
		make_command(argc, argv, &listunspent_info);
		bet_dealloc_args(argc, &argv);

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
		bet_alloc_args(argc, &argv);
		snprintf(params[0], arg_size, "\'%s\'", cJSON_Print(tx_list));
		snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(address_info));
		argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
		make_command(argc, argv, &tx);
		bet_dealloc_args(argc, &argv);
		return tx;
	}
}

cJSON *chips_create_raw_tx_with_data(double amount, char *address, char *data)
{
	char **argv = NULL, *changeAddress = NULL, params[2][arg_size] = { 0 };
	int argc;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, temp_balance = 0;

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	if (address == NULL) {
		address = (char *)malloc(64 * sizeof(char));
		strcpy(address, multisigAddress);
		address[63] = '\0';
	}
	if ((balance + chips_tx_fee) < amount) {
		return NULL;
	} else {
		cJSON_AddNumberToObject(address_info, address, amount);
		amount += chips_tx_fee;

		argc = 2;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "listunspent");
		make_command(argc, argv, &listunspent_info);
		bet_dealloc_args(argc, &argv);

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
		cJSON_AddStringToObject(address_info, "data", data);
		argc = 4;
		bet_alloc_args(argc, &argv);
		snprintf(params[0], arg_size, "\'%s\'", cJSON_Print(tx_list));
		snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(address_info));
		argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
		make_command(argc, argv, &tx);
		bet_dealloc_args(argc, &argv);
		return tx;
	}
}

void chips_list_unspent()
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listunspent");
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
		cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);

		if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")), "true") == 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp));
		}
	}
	bet_dealloc_args(argc, &argv);
}

int32_t chips_get_block_count()
{
	char **argv = NULL, *rendered = NULL;
	int argc, height;
	cJSON *block_height = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getblockcount");
	make_command(argc, argv, &block_height);

	rendered = cJSON_Print(block_height);
	height = atoi(rendered);
	bet_dealloc_args(argc, &argv);
	return height;
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
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getbalance");
	make_command(argc, argv, &getbalanceInfo);
	balance = atof(cJSON_Print(getbalanceInfo));
	bet_dealloc_args(argc, &argv);
	return balance;
}

cJSON *chips_add_multisig_address()
{
	int argc;
	char **argv = NULL, param[arg_size];
	cJSON *addr_list = NULL;
	cJSON *msig_address = NULL;

	argc = 5;
	bet_alloc_args(argc, &argv);
	snprintf(param, arg_size, "%d", threshold_value);

	addr_list = cJSON_CreateArray();
	for (int i = 0; i < no_of_notaries; i++)
		cJSON_AddItemToArray(addr_list, cJSON_CreateString_Length(notary_node_pubkeys[i], 67));

	argv = bet_copy_args(argc, "chips-cli", "addmultisigaddress", param,
			     cJSON_Print(cJSON_CreateString(cJSON_Print(addr_list))), "-addresstype legacy");
	msig_address = cJSON_CreateObject();
	make_command(argc, argv, &msig_address);
	bet_dealloc_args(argc, &argv);
	return msig_address;
}

int32_t chips_check_if_tx_unspent(char *input_tx)
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL;
	int32_t spendable = 0;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listunspent");
	listunspent_info = cJSON_CreateObject();
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
		cJSON *temp = NULL;
		temp = cJSON_GetArrayItem(listunspent_info, i);
		if (temp) {
			if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "txid")), input_tx) == 0) {
				if (strcmp(jstr(temp, "address"), legacy_2_of_4_msig_Addr) == 0) {
					spendable = 1;
					break;
				}
			}
		}
	}
	bet_dealloc_args(argc, &argv);
	return spendable;
}

char *chips_get_block_hash_from_txid(char *txid)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx_info = NULL;
	char *block_hash = NULL;

	argc = 4;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", txid, "1");
	raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx_info);
	if (raw_tx_info)
		block_hash = jstr(cJSON_Parse(unstringify(cJSON_Print(raw_tx_info))), "blockhash");
	bet_dealloc_args(argc, &argv);
	return block_hash;
}

int32_t chips_get_block_height_from_block_hash(char *block_hash)
{
	int32_t argc, block_height;
	char **argv = NULL;
	cJSON *block_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getblock", block_hash);
	block_info = cJSON_CreateObject();
	make_command(argc, argv, &block_info);
	block_height = jint(block_info, "height");
	bet_dealloc_args(argc, &argv);
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
	int argc;
	char **argv = NULL, params[2][arg_size] = { 0 };
	cJSON *listunspent_info = NULL;
	double amount = 0;
	cJSON *tx_list = NULL, *to_addr_info = NULL, *tx = NULL;

	to_addr_info = cJSON_CreateObject();
	tx_list = cJSON_CreateArray();
	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listunspent");
	listunspent_info = cJSON_CreateObject();
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

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
	bet_alloc_args(argc, &argv);
	sprintf(params[0], "\'%s\'", cJSON_Print(tx_list));
	sprintf(params[1], "\'%s\'", cJSON_Print(to_addr_info));
	argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);
	bet_dealloc_args(argc, &argv);
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

cJSON *chips_get_raw_tx(char *tx)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", tx);
	raw_tx = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx);
	bet_dealloc_args(argc, &argv);
	return raw_tx;
}

cJSON *chips_decode_raw_tx(cJSON *raw_tx)
{
	int argc;
	char **argv = NULL;
	cJSON *decoded_raw_tx = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "decoderawtransaction", cJSON_Print(raw_tx));
	decoded_raw_tx = cJSON_CreateObject();
	make_command(argc, argv, &decoded_raw_tx);
	bet_dealloc_args(argc, &argv);
	return decoded_raw_tx;
}

int32_t chips_validate_tx(char *tx)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vin = NULL, *txinwitness = NULL;

	raw_tx = chips_get_raw_tx(tx);
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	vin = cJSON_GetObjectItem(decoded_raw_tx, "vin");

	for (int i = 0; i < cJSON_GetArraySize(vin); i++) {
		cJSON *temp = cJSON_GetArrayItem(vin, i);
		txinwitness = cJSON_GetObjectItem(temp, "txinwitness");
		if (txinwitness) {
			cJSON *pubkey = cJSON_GetArrayItem(txinwitness, 1);
			printf("pubkey::%s\n", cJSON_Print(pubkey));
		}
	}
}

void chips_extract_data(char *tx, char **rand_str)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL, *script_pubkey = NULL;
	double zero = 0.0, value;

	raw_tx = chips_get_raw_tx(tx);
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		value = jdouble(temp, "value");
		if (value == zero) {
			script_pubkey = cJSON_GetObjectItem(temp, "scriptPubKey");
			if (script_pubkey) {
				char *data = jstr(script_pubkey, "hex");
				strcpy((*rand_str), data + 4);
				break;
			}
		}
	}
}

cJSON *chips_deposit_to_ln_wallet(double channel_chips)
{
	int argc;
	char **argv = NULL;
	cJSON *newaddr = NULL;
	cJSON *tx = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "newaddr");
	newaddr = cJSON_CreateObject();
	make_command(argc, argv, &newaddr);
	tx = chips_transfer_funds(channel_chips, jstr(newaddr, "address"));
	bet_dealloc_args(argc, &argv);
	return tx;
}

int32_t make_command(int argc, char **argv, cJSON **argjson)
{
	FILE *fp = NULL;
	char *data = NULL, *command = NULL, *buf = NULL;
	int32_t ret = 1, command_size = 16384, data_size = 262144, buf_size = 1024;

	command = calloc(command_size, sizeof(char));
	if (!command) {
		ret = 0;
		goto end;
	}
	data = calloc(data_size, sizeof(char));
	if (!data) {
		ret = 0;
		goto end;
	}
	buf = calloc(buf_size, sizeof(char));
	if (!buf) {
		ret = 0;
		goto end;
	}

	for (int i = 0; i < argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	/* Open the command for reading. */
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		exit(1);
	}

	if (!buf) {
		printf("%s::%d::Malloc failed\n", __FUNCTION__, __LINE__);
		goto end;
	}
	while (fgets(buf, buf_size, fp) != NULL) {
		strcat(data, buf);
		memset(buf, 0x00, buf_size);
	}
	data[data_size-1] = '\0';
	if ((strcmp(argv[0], "lightning-cli") == 0) && (strncmp("error", data, strlen("error")) == 0)) {
		char temp[1024];
		memset(temp, 0x00, sizeof(temp));
		strncpy(temp, data + strlen("error"), (strlen(data) - strlen("error")));
		*argjson = cJSON_Parse(temp);
		ret = 0;

	} else if (strlen(data) == 0) {
		if (strcmp(argv[1], "importaddress") == 0) {
			cJSON_AddNumberToObject(*argjson, "code", 0);

		} else {
			cJSON_AddStringToObject(*argjson, "error", "command failed");
			cJSON_AddStringToObject(*argjson, "command", command);
		}
	} else {
		if ((strcmp(argv[1], "createrawtransaction") == 0) || (strcmp(argv[1], "sendrawtransaction") == 0) ||
		    (strcmp(argv[1], "getnewaddress") == 0) || (strcmp(argv[1], "getrawtransaction") == 0)) {
			if (data[strlen(data) - 1] == '\n')
				data[strlen(data) - 1] = '\0';
			*argjson = cJSON_CreateString(data);
		} else {
			*argjson = cJSON_Parse(data);
			cJSON_AddNumberToObject(*argjson, "code", 0);
		}
	}
end:
	if (buf)
		free(buf);
	if (data)
		free(data);
	if (command)
		free(command);
	pclose(fp);

	return ret;
}

int32_t ln_dev_block_height()
{
	char **argv = NULL;
	int32_t argc, block_height;
	cJSON *bh_info = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "dev-blockheight");
	bh_info = cJSON_CreateObject();
	make_command(argc, argv, &bh_info);
	block_height = jint(bh_info, "blockheight");
	bet_dealloc_args(argc, &argv);
	return block_height;
}

int32_t ln_listfunds()
{
	cJSON *list_funds = NULL, *outputs = NULL;
	int argc;
	char **argv = NULL;
	int32_t value = 0;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "listfunds");
	list_funds = cJSON_CreateObject();
	make_command(argc, argv, &list_funds);

	if (jint(list_funds, "code") != 0) {
		value = 0;
		printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(list_funds, "message"));
		goto end;
	}

	outputs = cJSON_GetObjectItem(list_funds, "outputs");
	for (int32_t i = 0; i < cJSON_GetArraySize(outputs); i++) {
		value += jint(cJSON_GetArrayItem(outputs, i), "value");
	}

end:
	bet_dealloc_args(argc, &argv);
	free_json(list_funds);

	return value;
}

int32_t ln_get_uri(char **uri)
{
	cJSON *channel_info = NULL, *addresses = NULL, *address = NULL;
	int argc, retval = 1;
	char **argv = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "getinfo");
	channel_info = cJSON_CreateObject();
	make_command(argc, argv, &channel_info);

	if (jint(channel_info, "code") != 0) {
		retval = -1;
		printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(channel_info, "message"));
		goto end;
	}

	strcpy(*uri, jstr(channel_info, "id"));
	strcat(*uri, "@");
	addresses = cJSON_GetObjectItem(channel_info, "address");
	address = cJSON_GetArrayItem(addresses, 0);
	strcat(*uri, jstr(address, "address"));

end:
	bet_dealloc_args(argc, &argv);
	return retval;
}

int32_t ln_connect_uri(char *uri)
{
	int argc, retval = 1, channel_state;
	char **argv = NULL, channel_id[100];
	cJSON *connect_info = NULL;
	char temp[200];

	strncpy(temp, uri, strlen(uri));
	strcpy(channel_id, strtok(temp, "@"));

	channel_state = ln_get_channel_status(channel_id);
	if ((channel_state != CHANNELD_AWAITING_LOCKIN) && (channel_state != CHANNELD_NORMAL)) {
		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "lightning-cli", "connect", uri);
		connect_info = cJSON_CreateObject();
		make_command(argc, argv, &connect_info);

		if (jint(connect_info, "code") != 0) {
			retval = -1;
			printf("\n%s:%d:Message:%s", __FUNCTION__, __LINE__, jstr(connect_info, "method"));
			goto end;
		}
	}
end:
	bet_dealloc_args(argc, &argv);
	return retval;
}

cJSON *ln_fund_channel(char *channel_id, int32_t channel_fund_satoshi)
{
	int argc;
	char **argv = NULL;
	cJSON *fund_channel_info = NULL;
	char channel_satoshis[20];

	argc = 4;
	bet_alloc_args(argc, &argv);
	snprintf(channel_satoshis, 20, "%d", channel_fund_satoshi);
	argv = bet_copy_args(argc, "lightning-cli", "fundchannel", channel_id, channel_satoshis);
	fund_channel_info = cJSON_CreateObject();
	make_command(argc, argv, &fund_channel_info);

	bet_dealloc_args(argc, &argv);
	return fund_channel_info;
}

int32_t ln_pay(char *bolt11)
{
	cJSON *pay_response = NULL;
	int argc, retval = 1;
	char **argv = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "pay", bolt11);
	pay_response = cJSON_CreateObject();
	make_command(argc, argv, &pay_response);

	if (jint(pay_response, "code") != 0) {
		retval = -1;
		printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(pay_response, "message"));
		goto end;
	}

	if (strcmp(jstr(pay_response, "status"), "complete") == 0)
		printf("\nPayment Success");
	else
		retval = -1;
end:
	bet_dealloc_args(argc, &argv);
	return retval;
}

cJSON *ln_connect(char *id)
{
	int argc;
	char **argv = NULL;
	cJSON *connect_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "connect", id);
	make_command(argc, argv, &connect_info);
	bet_dealloc_args(argc, &argv);
	return connect_info;
}

void ln_check_peer_and_connect(char *id)
{
	int argc;
	char **argv = NULL;
	cJSON *list_peers_info = NULL;
	cJSON *peers_info = NULL;
	int32_t connected = 0;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "listpeers");
	make_command(argc, argv, &list_peers_info);

	peers_info = cJSON_GetObjectItem(list_peers_info, "peers");
	for (int i = 0; i < cJSON_GetArraySize(peers_info); i++) {
		cJSON *peer = cJSON_GetArrayItem(peers_info, i);

		if (strcmp(jstr(peer, "id"), id) == 0) {
			cJSON *state = cJSON_GetObjectItem(peer, "connected");
			if (strcmp(cJSON_Print(state), "true") == 0) {
				connected = 1;
				break;
			}
		}
	}
	if (connected == 0) {
		ln_connect(id);
	}
	bet_dealloc_args(argc, &argv);
}

int32_t ln_get_channel_status(char *id)
{
	int argc, channel_state = 0;
	char **argv = NULL;
	cJSON *channel_state_info = NULL, *channel_states = NULL, *channelState = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "peer-channel-state", id);
	make_command(argc, argv, &channel_state_info);

	channel_states = cJSON_GetObjectItem(channel_state_info, "channel-states");

	for (int i = 0; i < cJSON_GetArraySize(channel_states); i++) {
		channelState = cJSON_GetArrayItem(channel_states, i);
		channel_state = jint(channelState, "channel-state");
		if (channel_state <= 3) {
			break;
		}
	}
	bet_dealloc_args(argc, &argv);
	if (channel_state_info)
		cJSON_Delete(channel_state_info);
	return channel_state;
}

int32_t ln_wait_for_tx_block_height(int32_t block_height)
{
	int argc;
	char **argv = NULL;
	cJSON *bh_info = NULL;
	int32_t count = 0, max_attempts = 60;
	int32_t ret = 1;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "dev-blockheight");
	bh_info = cJSON_CreateObject();
	make_command(argc, argv, &bh_info);
	while (jint(bh_info, "blockheight") < block_height) {
		sleep(2);
		memset(bh_info, 0x00, sizeof(struct cJSON));
		make_command(argc, argv, &bh_info);
		if (count++ > max_attempts)
			break;
	}
	if (count > max_attempts)
		ret = 0;

	bet_dealloc_args(argc, &argv);
	return ret;
}

int32_t ln_establish_channel(char *uri)
{
	int32_t retval = 1, state;
	cJSON *connect_info = NULL, *fund_channel_info = NULL;
	double amount;
	char uid[100] = { 0 };

	strcpy(uid, uri);
	if ((ln_get_channel_status(strtok(uid, "@")) != CHANNELD_NORMAL)) {
		connect_info = ln_connect(uri);
		if ((retval = jint(connect_info, "code")) != 0)
			return retval;

		if (ln_listfunds() < (channel_fund_satoshis + (chips_tx_fee * satoshis))) {
			amount = channel_fund_satoshis - ln_listfunds();
			amount = amount / satoshis;

			if (chips_get_balance() >= (amount + (2 * chips_tx_fee))) {
				cJSON *tx_info = chips_deposit_to_ln_wallet(amount + chips_tx_fee);
				while (chips_get_block_hash_from_txid(cJSON_Print(tx_info)) == NULL) {
					sleep(2);
				}
				if (tx_info) {
					retval = ln_wait_for_tx_block_height(chips_get_block_height_from_block_hash(
						chips_get_block_hash_from_txid(cJSON_Print(tx_info))));
					if (retval == 0)
						return retval;
				}
			}
		}
		fund_channel_info = ln_fund_channel(jstr(connect_info, "id"), channel_fund_satoshis);
		if ((retval = jint(fund_channel_info, "code")) != 0) {
			return retval;
		}
		while ((state = ln_get_channel_status(jstr(connect_info, "id"))) != CHANNELD_NORMAL) {
			if (state == CHANNELD_AWAITING_LOCKIN) {
				fflush(stdout);
			} else if (state == 8) {
				printf("\nONCHAIN");
			} else
				printf("\n%s:%d:channel-state:%d\n", __FUNCTION__, __LINE__, state);

			sleep(2);
		}
	}
	return retval;
}
