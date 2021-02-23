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
#include "storage.h"
#include "commands.h"
#include "misc.h"

#include <stdarg.h>
#include <stdlib.h>

char *multisigAddress = "bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ";
double epsilon = 0.000000001;

int32_t bet_alloc_args(int argc, char ***argv)
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

void bet_dealloc_args(int argc, char ***argv)
{
	if (*argv) {
		for (int i = 0; i < argc; i++) {
			if ((*argv)[i])
				free((*argv)[i]);
		}
		free(*argv);
	}
}

static void bet_memset_args(int argc, char ***argv)
{
	if (*argv) {
		for (int i = 0; i < argc; i++) {
			if ((*argv)[i])
				memset((*argv)[i], 0x00, arg_size);
		}
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
	return unstringify(cJSON_Print(new_address));
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

cJSON *chips_list_address_groupings()
{
	int argc;
	char **argv = NULL;
	cJSON *list_address_groupings = NULL;
	cJSON *addr_info = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listaddressgroupings");
	list_address_groupings = cJSON_CreateObject();
	make_command(argc, argv, &list_address_groupings);

	addr_info = cJSON_CreateArray();
	for (int i = 0; i < cJSON_GetArraySize(list_address_groupings); i++) {
		cJSON *address_info = cJSON_GetArrayItem(list_address_groupings, i);
		for (int j = 0; j < cJSON_GetArraySize(address_info); j++) {
			cJSON *temp = NULL;
			temp = cJSON_GetArrayItem(address_info, j);
			cJSON *address = cJSON_GetArrayItem(temp, 0);
			if (chips_validate_address(cJSON_Print(address)) == 1) {
				cJSON_AddItemToArray(addr_info, cJSON_CreateString(unstringify(cJSON_Print(address))));
				printf("%s::%f\n", cJSON_Print(address),
				       atof(cJSON_Print(cJSON_GetArrayItem(temp, 1))));
			}
		}
	}
	bet_dealloc_args(argc, &argv);
	return addr_info;
}

cJSON *chips_get_block_hash_from_height(int64_t block_height)
{
	int argc;
	char **argv = NULL;
	cJSON *block_hash_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "chips-cli");
	strcpy(argv[1], "getblockhash");
	sprintf(argv[2], "%ld", block_height);
	block_hash_info = cJSON_CreateObject();
	make_command(argc, argv, &block_hash_info);
	bet_dealloc_args(argc, &argv);
	return block_hash_info;
}

cJSON *chips_get_block_from_block_hash(char *block_hash_info)
{
	int argc;
	char **argv = NULL;
	cJSON *block_info = NULL;

	argc = 3;
	argv = bet_copy_args(argc, "chips-cli", "getblock", block_hash_info);
	block_info = cJSON_CreateObject();
	make_command(argc, argv, &block_info);
	bet_dealloc_args(argc, &argv);
	return block_info;
}

cJSON *chips_get_block_from_block_height(int64_t block_height)
{
	cJSON *block_hash = NULL;
	cJSON *block_info = NULL;

	block_hash = cJSON_CreateObject();
	block_hash = chips_get_block_hash_from_height(block_height);
	if (block_hash) {
		block_info = cJSON_CreateObject();
		block_info = chips_get_block_from_block_hash(unstringify(cJSON_Print(block_hash)));
	}
	return block_info;
}

int32_t chips_if_tx_vin_of_tx(cJSON *txid, char *vin_tx_id)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx_info = NULL;
	cJSON *decoded_raw_tx_info = NULL;
	cJSON *vin = NULL;
	int32_t retval = 0;

	argc = 3;
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", unstringify(cJSON_Print(txid)));
	raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx_info);

	if (raw_tx_info == NULL)
		goto end;

	bet_memset_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "decoderawtransaction", unstringify(cJSON_Print(raw_tx_info)));
	decoded_raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &decoded_raw_tx_info);

	vin = cJSON_CreateArray();
	vin = cJSON_GetObjectItem(decoded_raw_tx_info, "vin");

	for (int32_t i = 0; i < cJSON_GetArraySize(vin); i++) {
		cJSON *temp = cJSON_GetArrayItem(vin, i);
		if (strcmp(jstr(temp, "txid"), vin_tx_id) == 0) {
			retval = 1;
			break;
		}
	}
end:
	bet_dealloc_args(argc, &argv);
	return retval;
}
cJSON *chips_find_parent_tx(int64_t block_height, char *vin_tx_id)
{
	cJSON *block_info = NULL;
	cJSON *tx = NULL;
	int32_t max_blocks_to_serach = 20;

	block_info = cJSON_CreateObject();
	tx = cJSON_CreateArray();
	for (int32_t i = 1; i < max_blocks_to_serach; i++) {
		while ((block_height + i) > chips_get_block_count()) {
			sleep(1);
		}
		block_info = chips_get_block_from_block_height(block_height + i);
		if (block_info) {
			tx = cJSON_GetObjectItem(block_info, "tx");
			if (cJSON_GetArraySize(tx) > 1) {
				for (int32_t j = 1; j < cJSON_GetArraySize(tx); j++) {
					if (chips_if_tx_vin_of_tx(cJSON_GetArrayItem(tx, j), vin_tx_id) == 1)
						return cJSON_GetArrayItem(tx, j);
				}
			}
		}
	}
	return NULL;
}

cJSON *chips_get_vin_from_tx(char *txid)
{
	int argc;
	char **argv = NULL;
	cJSON *wallet_tx_details = NULL, *decoded_tx_details = NULL, *vin = NULL, *vin_tx_id = NULL;

	argc = 3;
	argv = bet_copy_args(argc, "chips-cli", "gettransaction", txid);
	wallet_tx_details = cJSON_CreateObject();
	make_command(argc, argv, &wallet_tx_details);
	if (jstr(wallet_tx_details, "error message") == NULL) {
		bet_memset_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "decoderawtransaction", jstr(wallet_tx_details, "hex"));
		decoded_tx_details = cJSON_CreateObject();
		make_command(argc, argv, &decoded_tx_details);
		vin = cJSON_CreateArray();
		vin = cJSON_GetObjectItem(decoded_tx_details, "vin");
		if (cJSON_GetArraySize(vin) > 0) {
			vin_tx_id = cJSON_CreateObject();
			vin_tx_id = cJSON_GetArrayItem(vin, 0);
		}
	}
	bet_dealloc_args(argc, &argv);
	return vin_tx_id;
}

cJSON *validate_given_tx(int64_t block_height, char *txid)
{
	cJSON *tx = NULL;
	cJSON *vin_tx_id = NULL;

	while ((block_height + 1) <= chips_get_block_count())
		sleep(1);

	if (chips_get_block_hash_from_txid(txid) == NULL) {
		vin_tx_id = cJSON_CreateObject();
		vin_tx_id = chips_get_vin_from_tx(txid);
		if (vin_tx_id) {
			tx = cJSON_CreateObject();
			tx = chips_find_parent_tx(block_height, jstr(vin_tx_id, "txid"));
		}
	}
	return tx;
}

cJSON *chips_transfer_funds_with_data(double amount, char *address, char *data)
{
	cJSON *tx_info = NULL, *signed_tx = NULL;
	char *raw_tx = NULL;

	raw_tx = calloc(arg_size, sizeof(char));
	strncpy(raw_tx, cJSON_str(chips_create_raw_tx_with_data(amount, address, data)), arg_size);
	signed_tx = cJSON_CreateObject();
	signed_tx = chips_sign_raw_tx_with_wallet(raw_tx);

	if (jstr(signed_tx, "error") == NULL) {
		tx_info = cJSON_CreateObject();
		tx_info = chips_send_raw_tx(signed_tx);
		if (jstr(tx_info, "error") == NULL) {
			return tx_info;
		}
	}
	return NULL;
}

cJSON *chips_transfer_funds(double amount, char *address)
{
	cJSON *tx_info = NULL, *signed_tx = NULL;
	char *raw_tx = NULL;

	raw_tx = cJSON_str(chips_create_raw_tx(amount, address));
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

		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "listunspent", " > listunspent.log");
		make_command(argc, argv, &listunspent_info);
		bet_dealloc_args(argc, &argv);

		for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) { // sg777: removed -1 from here
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

		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "chips-cli", "listunspent", " > listunspent.log");
		listunspent_info = cJSON_CreateArray();
		make_command(argc, argv, &listunspent_info);
		bet_dealloc_args(argc, &argv);

		for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) { //sg777: removed -1 from here
			cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
			cJSON *tx_info = cJSON_CreateObject();
			char *state = cJSON_Print(cJSON_GetObjectItem(temp, "spendable"));
			if (strcmp(state, "true") == 0) {
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
		tx = cJSON_CreateObject();
		make_command(argc, argv, &tx);
		bet_dealloc_args(argc, &argv);
		return tx;
	}
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
	// printf("chips height - %d\n", height);
	bet_dealloc_args(argc, &argv);
	return height;
}

void check_ln_chips_sync()
{
	int32_t chips_bh, ln_bh;
	int32_t threshold_diff = 1000;

	chips_bh = chips_get_block_count();
	ln_bh = ln_dev_block_height();
	while (1) {
		if ((chips_bh - ln_bh) > threshold_diff) {
			printf("\rln is %d blocks behind chips network\n", (chips_bh - ln_bh));
			fflush(stdout);
		} else
			break;
		chips_bh = chips_get_block_count();
		ln_bh = ln_dev_block_height();
	}
	printf("ln is in sync with chips\n");
}

cJSON *bet_get_chips_ln_bal_info()
{
	cJSON *bal_info = NULL;

	bal_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bal_info, "method", "bal_info");
	cJSON_AddNumberToObject(bal_info, "chips_bal", chips_get_balance());
	cJSON_AddNumberToObject(bal_info, "ln_bal", (ln_listfunds() / satoshis));
	return bal_info;
}

cJSON *bet_get_chips_ln_addr_info()
{
	cJSON *addr_info = NULL;

	addr_info = cJSON_CreateObject();
	cJSON_AddStringToObject(addr_info, "method", "addr_info");
	cJSON_AddStringToObject(addr_info, "chips_addr", chips_get_new_address());
	cJSON_AddStringToObject(addr_info, "ln_addr", ln_get_new_address());
	return addr_info;
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

	if (threshold_value > live_notaries) {
		printf("Not enough trust exists in the system\n");
		return NULL;
	}

	argc = 4;
	bet_alloc_args(argc, &argv);
	snprintf(param, arg_size, "%d", threshold_value);

	addr_list = cJSON_CreateArray();
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1)
			cJSON_AddItemToArray(addr_list, cJSON_CreateString_Length(notary_node_pubkeys[i], 67));
	}

	argv = bet_copy_args(argc, "chips-cli", "addmultisigaddress", param,
			     cJSON_Print(cJSON_CreateString(cJSON_Print(addr_list)))); //"-addresstype legacy"
			     
	msig_address = cJSON_CreateObject();
	make_command(argc, argv, &msig_address);
	bet_dealloc_args(argc, &argv);
	return msig_address;
}

cJSON *chips_add_multisig_address_from_list(int32_t threshold_value, cJSON *addr_list)
{
	int argc;
	char **argv = NULL, param[arg_size];
	cJSON *msig_address = NULL;

	argc = 4;
	snprintf(param, arg_size, "%d", threshold_value);

	argv = bet_copy_args(argc, "chips-cli", "addmultisigaddress", param,
			     cJSON_Print(cJSON_CreateString(cJSON_Print(addr_list))));//, "-addresstype legacy"
	msig_address = cJSON_CreateObject();

	make_command(argc, argv, &msig_address);
	bet_dealloc_args(argc, &argv);
	return msig_address;
}

int32_t chips_check_if_tx_unspent(char *input_tx)
{
	char **argv = NULL;
	int argc;
	int32_t tx_exists = 0;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listunspent", " > listunspent.log");

	run_command(argc, argv);
	printf("%s::%s\n", legacy_m_of_n_msig_addr, input_tx);
	tx_exists = chips_check_tx_exists(input_tx);
	bet_dealloc_args(argc, &argv);
	printf("%s::%d::%d\n", __FUNCTION__, __LINE__, tx_exists);
	return tx_exists;
}

#if 0
int32_t chips_check_if_tx_unspent(char *input_tx)
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL;
	int32_t spendable = 0;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listunspent"," > listunspent.log");
	listunspent_info = cJSON_CreateObject();
	
	make_command(argc, argv, &listunspent_info);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info) - 1; i++) {
		cJSON *temp = NULL;
		temp = cJSON_GetArrayItem(listunspent_info, i);
		if (temp) {
			if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "txid")), input_tx) == 0) {
				if (strcmp(jstr(temp, "address"), legacy_m_of_n_msig_addr) == 0) {
					spendable = 1;
					break;
				}
			}
		}
	}
	bet_dealloc_args(argc, &argv);
	return spendable;
}
#endif
char *chips_get_block_hash_from_txid(char *txid)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx_info = NULL;
	char *block_hash = NULL;

	argc = 4;
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", txid, "1");
	raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx_info);

	if (jstr(raw_tx_info, "error code") != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(raw_tx_info));
		return NULL;
	}

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
	int argc, rc;
	char **argv = NULL, params[2][arg_size] = { 0 }, *hex_data = NULL, *data = NULL, *msig_addr = NULL;
	cJSON *listunspent_info = NULL, *player_info = NULL;
	double amount = 0, value = 0;
	cJSON *tx_list = NULL, *to_addr_info = NULL, *tx = NULL;
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL;
	
	for (int32_t i = 0; i < no_of_txs; i++) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, tx_ids[i]);
	}
	to_addr_info = cJSON_CreateObject();
	tx_list = cJSON_CreateArray();
	argc = 3;
	argv = bet_copy_args(argc, "chips-cli", "listunspent", " > listunspent.log");
	listunspent_info = cJSON_CreateObject();
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

	if(chips_check_tx_exists_in_unspent(tx_ids[0]) == 1) {
		raw_tx  = chips_get_raw_tx(tx_ids[0]);
		decoded_raw_tx = chips_decode_raw_tx(raw_tx);
		vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");

		hex_data = calloc(1, tx_data_size * 2);
		rc = chips_extract_data(tx_ids[0], &hex_data);

		if (rc == 1) {
			data = calloc(1, tx_data_size);
			hexstr_to_str(hex_data, data);
			player_info = cJSON_CreateObject();
			player_info = cJSON_Parse(data);
			msig_addr = jstr(player_info,"msig_addr");
		}

		for(int i = 0; i<cJSON_GetArraySize(vout); i++) {
			cJSON *temp = cJSON_GetArrayItem(vout,i);
			
			value = jdouble(temp,"value");
			if(value > 0) {
				cJSON *scriptPubKey = cJSON_GetObjectItem(temp,"scriptPubKey");
				cJSON *addresses = cJSON_GetObjectItem(scriptPubKey,"addresses");
				if(strcmp(msig_addr,jstri(addresses,0)) == 0) {
					cJSON *tx_info = cJSON_CreateObject();
					amount += jdouble(temp,"value");
					cJSON_AddStringToObject(tx_info, "txid", tx_ids[0]);
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "n"));
					cJSON_AddItemToArray(tx_list, tx_info); 		
				}
			}
		}
	}
	if ((cJSON_GetArraySize(tx_list) == 0) || (amount < chips_tx_fee)) {
		return NULL;
	}
	cJSON_AddNumberToObject(to_addr_info, to_addr, (amount - chips_tx_fee));
	argc = 4;
	sprintf(params[0], "\'%s\'", cJSON_Print(tx_list));
	sprintf(params[1], "\'%s\'", cJSON_Print(to_addr_info));
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, params[0]);
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, params[1]);
	argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);
	
	bet_dealloc_args(argc, &argv);
	if(data)
		free(data);
	if(hex_data)
		free(hex_data);
	
	return tx;
}

cJSON *chips_sign_msig_tx_of_table_id(char *ip, cJSON *raw_tx, char *table_id)
{
	cJSON *msig_raw_tx = NULL, *tx = NULL;

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", raw_tx);
	cJSON_AddStringToObject(msig_raw_tx, "table_id", table_id);
	cJSON_AddStringToObject(msig_raw_tx, "id", unique_id);
	tx = bet_msg_cashier_with_response_id(msig_raw_tx, ip, "signed_tx");

	return tx;
}

cJSON *chips_sign_msig_tx(char *ip, cJSON *raw_tx)
{
	cJSON *msig_raw_tx = NULL, *tx = NULL;

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", raw_tx);
	cJSON_AddStringToObject(msig_raw_tx, "table_id", table_id);
	cJSON_AddStringToObject(msig_raw_tx, "id", unique_id);
	tx = bet_msg_cashier_with_response_id(msig_raw_tx, ip, "signed_tx");

	return tx;
}

cJSON *chips_spend_msig_txs(char *to_addr, int no_of_txs, char tx_ids[][100])
{
	int signers = 0;
	cJSON *hex = NULL, *tx = NULL;

	bet_check_cashiers_status();
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			if (signers == 0) {
				cJSON *temp = chips_sign_msig_tx(
					notary_node_ips[i], chips_create_tx_from_tx_list(to_addr, no_of_txs, tx_ids));
				if (temp == NULL) {
					continue;
				}
				hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "hex");
				signers++;
			} else if (signers == 1) {
				cJSON *temp1 = chips_sign_msig_tx(notary_node_ips[i], hex);
				if (temp1 == NULL) {
					continue;
				}
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

static cJSON *chips_spend_msig_tx(cJSON *raw_tx)
{
	int signers = 0;
	cJSON *hex = NULL, *tx = NULL;

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(raw_tx));

	bet_check_cashiers_status();
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			if (signers == 0) {
				cJSON *temp = chips_sign_msig_tx(notary_node_ips[i], raw_tx);
				if (temp == NULL) {
					continue;
				}
				if (cJSON_GetObjectItem(temp, "signed_tx") != NULL) {
					hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "hex");
					signers++;
				} else {
					printf("%s::%d::%s\n", __FUNCTION__, __LINE__, jstr(temp, "err_str"));
					return NULL;
				}
			} else if (signers == 1) {
				cJSON *temp1 = chips_sign_msig_tx(notary_node_ips[i], hex);
				if (temp1 == NULL) {
					continue;
				}
				if (cJSON_GetObjectItem(temp1, "signed_tx") != NULL) {
					cJSON *status = cJSON_GetObjectItem(cJSON_GetObjectItem(temp1, "signed_tx"),
									    "complete");
					if (strcmp(cJSON_Print(status), "true") == 0) {
						tx = chips_send_raw_tx(cJSON_GetObjectItem(temp1, "signed_tx"));
						signers++;
						break;
					}
				} else {
					printf("%s::%d::%s\n", __FUNCTION__, __LINE__, jstr(temp1, "err_str"));
					return NULL;
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
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", tx);
	raw_tx = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx);
	bet_dealloc_args(argc, &argv);

	if (jstr(raw_tx, "error") != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(raw_tx));
		return NULL;
	}

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

int32_t chips_extract_data(char *tx, char **rand_str)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL, *script_pubkey = NULL;
	double zero = 0.0, value;
	int32_t retval = 0;

	raw_tx = chips_get_raw_tx(tx);
	if (raw_tx == NULL) {
		return retval;
	}
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	if (decoded_raw_tx == NULL) {
		return retval;
	}
	vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		value = jdouble(temp, "value");
		if (value == zero) {
			script_pubkey = cJSON_GetObjectItem(temp, "scriptPubKey");
			if (script_pubkey) {
				char *data = jstr(script_pubkey, "hex");
				strcpy((*rand_str),
				       data + 8); // first 4 bytes contains OP_RETURN hex code so we are skipping them
				break;
			}
		}
	}
	if (*rand_str)
		retval = 1;
	return retval;
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

static int32_t find_address_in_addresses(char *address, cJSON *argjson)
{
	cJSON *addresses = NULL, *script_pub_key = NULL;
	int32_t retval = 0;

	script_pub_key = cJSON_GetObjectItem(argjson, "scriptPubKey");
	addresses = cJSON_GetObjectItem(script_pub_key, "addresses");
	for (int i = 0; i < cJSON_GetArraySize(addresses); i++) {
		if (strcmp(unstringify(cJSON_Print(cJSON_GetArrayItem(addresses, i))), address) == 0) {
			retval = 1;
			return retval;
		}
	}
	return retval;
}

static char *get_address_in_addresses(cJSON *argjson)
{
	cJSON *addresses = NULL, *script_pub_key = NULL;

	script_pub_key = cJSON_GetObjectItem(argjson, "scriptPubKey");
	addresses = cJSON_GetObjectItem(script_pub_key, "addresses");
	for (int i = 0; i < cJSON_GetArraySize(addresses); i++) {
		return (unstringify(cJSON_Print(cJSON_GetArrayItem(addresses, i))));
	}
	return NULL;
}

double chips_get_balance_on_address_from_tx(char *address, char *tx)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL;
	double balance = 0;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", tx);
	raw_tx = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx);

	bet_memset_args(argc, &argv);

	argv = bet_copy_args(argc, "chips-cli", "decoderawtransaction", cJSON_Print(raw_tx));
	decoded_raw_tx = cJSON_CreateObject();
	make_command(argc, argv, &decoded_raw_tx);

	vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");

	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		if (find_address_in_addresses(address, cJSON_GetArrayItem(vout, i)) == 1) {
			balance += jdouble(cJSON_GetArrayItem(vout, i), "value");
		}
	}
	return balance;
}

char *chips_get_wallet_address()
{
	int argc;
	char **argv = NULL;
	cJSON *addresses = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "listaddressgroupings");
	make_command(argc, argv, &addresses);

	for (int32_t i = 0; i < cJSON_GetArraySize(addresses); i++) {
		cJSON *temp = cJSON_GetArrayItem(addresses, i);
		for (int32_t j = 0; j < cJSON_GetArraySize(temp); j++) {
			cJSON *temp1 = cJSON_GetArrayItem(temp, j);

			if (chips_validate_address(unstringify(cJSON_Print(cJSON_GetArrayItem(temp1, 0)))))
				return (unstringify(cJSON_Print(cJSON_GetArrayItem(temp1, 0))));
		}
	}
	bet_dealloc_args(argc, &argv);
	return chips_get_new_address();
}

static int32_t chips_get_vout_from_tx(char *tx_id)
{
	cJSON *raw_tx_info = NULL, *decode_raw_tx_info = NULL, *vout = NULL;
	int argc, retval = -1;
	char **argv = NULL;
	double value = 0.01;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "getrawtransaction", tx_id);
	make_command(argc, argv, &raw_tx_info);

	bet_memset_args(argc, &argv);
	argv = bet_copy_args(argc, "chips-cli", "decoderawtransaction", cJSON_Print(raw_tx_info));
	make_command(argc, argv, &decode_raw_tx_info);

	vout = cJSON_GetObjectItem(decode_raw_tx_info, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		if (abs(value - jdouble(temp, "value") < epsilon)) {
			retval = jint(temp, "n");
			break;
		}
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

cJSON *chips_create_payout_tx(cJSON *payout_addr, int32_t no_of_txs, char tx_ids[][100], char *data)
{
	double payout_amount = 0, amount_in_txs = 0;
	cJSON *tx_list = NULL, *addr_info = NULL, *tx_details = NULL, *payout_tx_info = NULL;
	int argc, vout;
	char **argv = NULL, params[2][arg_size] = { 0 }, *sql_query = NULL;

	for (int32_t i = 0; i < cJSON_GetArraySize(payout_addr); i++) {
		cJSON *addr_info = cJSON_GetArrayItem(payout_addr, i);
		payout_amount += jdouble(addr_info, "amount");
	}
	for (int32_t i = 0; i < no_of_txs; i++) {
		amount_in_txs += chips_get_balance_on_address_from_tx(legacy_m_of_n_msig_addr, tx_ids[i]);
	}
	if (abs((payout_amount + chips_tx_fee) - amount_in_txs) < epsilon) {
		printf("%s::%d::%f::%f\n", __FUNCTION__, __LINE__, payout_amount, amount_in_txs);
		for (int32_t i = 0; i < no_of_txs; i++) {
			printf("%s\n", tx_ids[i]);
		}
	} else {
		printf("%s::%d::Amount mismatch between the payout tx and payin tx\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	tx_list = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_txs; i++) {
		if ((vout = chips_get_vout_from_tx(tx_ids[i])) >= 0) {
			cJSON *tx_info = cJSON_CreateObject();
			cJSON_AddStringToObject(tx_info, "txid", tx_ids[i]);
			cJSON_AddNumberToObject(tx_info, "vout", vout);
			cJSON_AddItemToArray(tx_list, tx_info);
		}
	}
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(payout_addr));
	addr_info = cJSON_CreateObject();
	for (int32_t i = 0; i < cJSON_GetArraySize(payout_addr); i++) {
		cJSON *temp = cJSON_GetArrayItem(payout_addr, i);
		cJSON_AddNumberToObject(addr_info, jstr(temp, "address"), jdouble(temp, "amount"));
	}
	if (data)
		cJSON_AddStringToObject(addr_info, "data", data);

	argc = 4;
	bet_alloc_args(argc, &argv);
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(addr_info));
	sprintf(params[0], "\'%s\'", cJSON_Print(tx_list));
	sprintf(params[1], "\'%s\'", cJSON_Print(addr_info));

	argv = bet_copy_args(argc, "chips-cli", "createrawtransaction", params[0], params[1]);
	make_command(argc, argv, &tx_details);

	printf("%s::%d::raw_tx::%s\n", __FUNCTION__, __LINE__, cJSON_Print(tx_details));

	cJSON *tx = chips_spend_msig_tx(tx_details);

	payout_tx_info = cJSON_CreateObject();
	cJSON_AddStringToObject(payout_tx_info, "method", "payout_tx");
	cJSON_AddStringToObject(payout_tx_info, "table_id", table_id);
	cJSON_AddItemToObject(payout_tx_info, "tx_info", tx);

	if (tx) {
		printf("%s::%d::tx::%s\n", __FUNCTION__, __LINE__, cJSON_Print(tx));
		sql_query = calloc(1, 400);
		sprintf(sql_query, "UPDATE dcv_tx_mapping set status = 0 where table_id = \"%s\";", table_id);
		bet_run_query(sql_query);
		for (int32_t i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1) {
				bet_msg_cashier(payout_tx_info, notary_node_ips[i]);
			}
		}
	} else {
		printf("%s::%d::Error occured in processing the payout tx ::%s\n", __FUNCTION__, __LINE__,
		       cJSON_Print(tx_details));
	}
	if (sql_query)
		free(sql_query);
	bet_dealloc_args(argc, &argv);
	return payout_tx_info;
}

static void chips_read_valid_unspent(cJSON **argjson)
{
	char *file_name = "listunspent.log";
	FILE *fp = NULL;
	char ch, buf[4196];
	int32_t len = 0;
	cJSON *temp = NULL;

	temp = cJSON_CreateObject();
	*argjson = cJSON_CreateArray();
	fp = fopen(file_name, "r");
	while ((ch = fgetc(fp)) != EOF) {
		if ((ch != '[') || (ch != ']')) {
			if (ch == '{') {
				buf[len++] = ch;
			} else {
				if (len > 0) {
					if (ch == '}') {
						buf[len++] = ch;
						buf[len] = '\0';
						temp = cJSON_Parse(buf);
						//printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(cJSON_GetObjectItem(temp,"spendable")));
						if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")),
							   "true") == 0) {
							cJSON_AddItemToArray(*argjson, temp);
						}
						memset(buf, 0x00, len);
						len = 0;
					} else {
						buf[len++] = ch;
					}
				}
			}
		}
	}
}



int32_t chips_check_tx_exists_in_unspent(char *tx_id)
{
	char *file_name = "listunspent.log";
	FILE *fp = NULL;
	char ch, buf[4196];
	int32_t len = 0, tx_exists = 0;
	cJSON *temp = NULL;

	temp = cJSON_CreateObject();
	fp = fopen(file_name, "r");
	while ((ch = fgetc(fp)) != EOF) {
		if ((ch != '[') || (ch != ']')) {
			if (ch == '{') {
				buf[len++] = ch;
			} else {
				if (len > 0) {
					if (ch == '}') {
						buf[len++] = ch;
						buf[len] = '\0';
						temp = cJSON_Parse(buf);
						if (strcmp(unstringify(cJSON_Print(cJSON_GetObjectItem(temp, "txid"))),
							   unstringify(tx_id)) == 0) {
								tx_exists = 1;
								break;
						}
						memset(buf, 0x00, len);
						len = 0;
					} else {
						buf[len++] = ch;
					}
				}
			}
		}
	}
	return tx_exists;
}


int32_t chips_check_tx_exists(char *tx_id)
{
	char *file_name = "listunspent.log";
	FILE *fp = NULL;
	char ch, buf[4196];
	int32_t len = 0, tx_exists = 0;
	cJSON *temp = NULL;

	temp = cJSON_CreateObject();
	fp = fopen(file_name, "r");
	while ((ch = fgetc(fp)) != EOF) {
		if ((ch != '[') || (ch != ']')) {
			if (ch == '{') {
				buf[len++] = ch;
			} else {
				if (len > 0) {
					if (ch == '}') {
						buf[len++] = ch;
						buf[len] = '\0';
						temp = cJSON_Parse(buf);
						if (strcmp(unstringify(cJSON_Print(cJSON_GetObjectItem(temp, "txid"))),
							   unstringify(tx_id)) == 0) {
							printf("%s::%d::%s::%s\n", __FUNCTION__, __LINE__,
							       jstr(temp, "address"), legacy_m_of_n_msig_addr);
							if (strcmp(jstr(temp, "address"), legacy_m_of_n_msig_addr) ==
							    0) {
								tx_exists = 1;
								break;
							}
						}
						memset(buf, 0x00, len);
						len = 0;
					} else {
						buf[len++] = ch;
					}
				}
			}
		}
	}
	return tx_exists;
}

int32_t run_command(int argc, char **argv)
{
	char *command = NULL;
	int32_t ret = 1;
	unsigned long command_size = 16384;
	FILE *fp = NULL;

	command = calloc(command_size, sizeof(char));
	if (!command) {
		ret = 0;
		goto end;
	}

	for (int i = 0; i < argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	/* Open the command for reading. */
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, command);
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		exit(1);
	}

end:
	if (command)
		free(command);
	pclose(fp);

	return ret;
}

int32_t make_command(int argc, char **argv, cJSON **argjson)
{
	FILE *fp = NULL;
	char *data = NULL, *command = NULL, *buf = NULL;
	int32_t ret = 1;
	unsigned long command_size = 16384, data_size = 262144, buf_size = 1024;

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
	unsigned long temp_size = 0;
	unsigned long new_size = data_size;
	while (fgets(buf, buf_size, fp) != NULL) {
		temp_size = temp_size + strlen(buf);
		if (temp_size >= new_size) {
			char *temp = calloc(new_size, sizeof(char));
			strncpy(temp, data, strlen(data));
			free(data);
			new_size = new_size * 2;
			data = calloc(new_size, sizeof(char));
			strncpy(data, temp, strlen(temp));
			free(temp);
		}
		strcat(data, buf);
		memset(buf, 0x00, buf_size);
	}
	data[new_size - 1] = '\0';
	if ((strcmp(argv[0], "lightning-cli") == 0) && (strncmp("error", data, strlen("error")) == 0)) {
		char temp[1024];
		memset(temp, 0x00, sizeof(temp));
		strncpy(temp, data + strlen("error"), (strlen(data) - strlen("error")));
		*argjson = cJSON_Parse(temp);
		ret = 0;

	} else if (strlen(data) == 0) {
		*argjson = cJSON_CreateObject();
		if (strcmp(argv[1], "importaddress") == 0) {
			cJSON_AddNumberToObject(*argjson, "code", 0);
		} else if (strcmp(argv[1], "listunspent") == 0) {
			chips_read_valid_unspent(argjson);
		} else {
			cJSON_AddStringToObject(*argjson, "error", "command failed");
			cJSON_AddStringToObject(*argjson, "command", command);
		}
	} else {
		if ((strcmp(argv[1], "createrawtransaction") == 0) || (strcmp(argv[1], "sendrawtransaction") == 0) ||
		    (strcmp(argv[1], "getnewaddress") == 0) || (strcmp(argv[1], "getrawtransaction") == 0) ||
		    (strcmp(argv[1], "getblockhash") == 0)) {
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

char *ln_get_new_address()
{
	int argc;
	char **argv = NULL;
	cJSON *addr_info = NULL;

	argc = 2;
	argv = bet_copy_args(argc, "lightning-cli", "newaddr");
	addr_info = cJSON_CreateObject();
	make_command(argc, argv, &addr_info);
	bet_dealloc_args(argc, &argv);

	return unstringify(cJSON_Print(addr_info));
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
	// printf("LN height - %d\n", block_height);
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
