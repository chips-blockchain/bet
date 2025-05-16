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
#include "err.h"

#include <stdarg.h>
#include <stdlib.h>
#include <inttypes.h>

double epsilon = 0.000000001;

char blockchain_cli[1024] = "verus -chain=chips777";

char *chips_cli = "chips-cli";
char *verus_chips_cli = "verus -chain=chips777";

int32_t bet_alloc_args(int argc, char ***argv)
{
	int ret = OK;

	*argv = calloc(argc, sizeof(char *));
	if (*argv == NULL)
		return ERR_MEMORY_ALLOC;
	for (int i = 0; i < argc; i++) {
		(*argv)[i] = calloc(arg_size, sizeof(char));
		if ((*argv)[i] == NULL)
			return ERR_MEMORY_ALLOC;
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

char **bet_copy_args_with_size(int argc, ...)
{
	char **argv = NULL;
	va_list valist, va_copy;

	if (argc <= 0)
		return NULL;

	va_start(valist, argc);
	va_copy(va_copy, valist);

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		int arg_length = strlen(va_arg(va_copy, char *)) + 1;
		argv[i] = (char *)malloc(arg_length * sizeof(char));
		strcpy(argv[i], va_arg(valist, char *));
	}

	va_end(valist);
	va_end(va_copy);
	return argv;
}

char **bet_copy_args(int argc, ...)
{
	int32_t ret = OK;
	char **argv = NULL;
	va_list valist, va_copy;

	ret = bet_alloc_args(argc, &argv);
	if (ret != OK) {
		dlg_error("%s", bet_err_str(ret));
		return NULL;
	}

	va_start(valist, argc);
	va_copy(va_copy, valist);

	for (int i = 0; i < argc; i++) {
		if (strlen(va_arg(va_copy, char *)) > arg_size) {
			ret = ERR_ARG_SIZE_TOO_LONG;
			dlg_error("Error::%s::%s", bet_err_str(ret), va_arg(valist, char *));
			dlg_error("Error in running the command::%s", argv[1]);
			goto end;
		}
		strcpy(argv[i], va_arg(valist, char *));
	}
end:
	va_end(valist);
	va_end(va_copy);
	if (ret != OK) {
		bet_dealloc_args(argc, &argv);
		return NULL;
	}
	return argv;
}

int32_t chips_ismine(char *address)
{
	int argc, ismine = false;
	char **argv = NULL;
	cJSON *addressInfo = NULL, *obj = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "validateaddress", address);
	addressInfo = cJSON_CreateObject();
	make_command(argc, argv, &addressInfo);

	if ((obj = jobj(addressInfo, "ismine")) && is_cJSON_True(obj)) {
		ismine = true;
	}

	bet_dealloc_args(argc, &argv);
	return ismine;
}

int32_t chips_iswatchonly(char *address)
{
	int argc, retval = 0;
	char **argv = NULL;
	cJSON *addressInfo = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "validateaddress", address);
	addressInfo = cJSON_CreateObject();
	make_command(argc, argv, &addressInfo);

	cJSON *temp = cJSON_GetObjectItem(addressInfo, "iswatchonly");
	if (strcmp(cJSON_Print(temp), "true") == 0)
		retval = 1;
	bet_dealloc_args(argc, &argv);
	return retval;
}

void chips_import_address(char *address)
{
	int argc;
	char **argv = NULL;

	argc = 3;

	argv = bet_copy_args(argc, blockchain_cli, "importaddress", address);
	make_command(argc, argv, NULL);
	bet_dealloc_args(argc, &argv);
}

char *chips_get_new_address()
{
	int argc;
	char **argv = NULL;
	cJSON *new_address = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "getnewaddress");
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
	argv = bet_copy_args(argc, blockchain_cli, "validateaddress", address);
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
	argv = bet_copy_args(argc, blockchain_cli, "listaddressgroupings");
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
				dlg_info("%s::%f\n", cJSON_Print(address),
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
	strcpy(argv[0], blockchain_cli);
	strcpy(argv[1], "getblockhash");
	// https://stackoverflow.com/questions/31534474/format-lld-expects-type-long-long-int-but-argument-4-has-type-int64-t/31534505
	// using PRId64 instead of lld or ld, since on Darwin compiler
	// complains of not having lld, where on Linux it complains of not having ld.
	sprintf(argv[2], "%" PRId64, block_height);
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
	argv = bet_copy_args(argc, blockchain_cli, "getblock", block_hash_info);
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
	int32_t retval = 0;
	cJSON *raw_tx_info = NULL, *decoded_raw_tx_info = NULL, *vin = NULL;

	raw_tx_info = chips_get_raw_tx(unstringify(cJSON_Print(txid)));
	if (raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return retval;
	}
	decoded_raw_tx_info = chips_decode_raw_tx(raw_tx_info);
	if (decoded_raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		return retval;
	}

	vin = cJSON_CreateArray();
	vin = cJSON_GetObjectItem(decoded_raw_tx_info, "vin");

	for (int32_t i = 0; i < cJSON_GetArraySize(vin); i++) {
		cJSON *temp = cJSON_GetArrayItem(vin, i);
		if (strcmp(jstr(temp, "txid"), vin_tx_id) == 0) {
			retval = 1;
			break;
		}
	}
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
	cJSON *wallet_tx_details = NULL, *decoded_tx_details = NULL, *vin = NULL, *vin_tx_id = NULL;

	wallet_tx_details = chips_get_raw_tx(txid);
	if (wallet_tx_details == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		goto end;
	}
	decoded_tx_details = chips_decode_raw_tx(wallet_tx_details);
	if (decoded_tx_details == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		goto end;
	}

	vin = cJSON_CreateArray();
	vin = cJSON_GetObjectItem(decoded_tx_details, "vin");
	if (cJSON_GetArraySize(vin) > 0) {
		vin_tx_id = cJSON_CreateObject();
		vin_tx_id = cJSON_GetArrayItem(vin, 0);
	}
end:
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

cJSON *chips_transfer_funds_with_data1(cJSON *vout_info, char *data)
{
	cJSON *tx_info = NULL, *signed_tx = NULL, *raw_tx_info = NULL;
	char *raw_tx = NULL;

	raw_tx = calloc(arg_size, sizeof(char));

	raw_tx_info = chips_create_raw_tx_with_data1(vout_info, data);
	if (raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_CREATE_RAW_TX));
		return NULL;
	}

	strncpy(raw_tx, cJSON_str(raw_tx_info), arg_size);
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

cJSON *chips_transfer_funds_with_data(double amount, char *address, char *data)
{
	cJSON *tx_info = NULL, *signed_tx = NULL, *raw_tx_info = NULL;
	char *raw_tx = NULL;

	raw_tx = calloc(arg_size, sizeof(char));

	raw_tx_info = chips_create_raw_tx_with_data(amount, address, data);
	if (raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_CREATE_RAW_TX));
		return NULL;
	}

	strncpy(raw_tx, cJSON_str(raw_tx_info), arg_size);
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

	if (address) {
		raw_tx = cJSON_str(chips_create_raw_tx_with_data(amount, address, NULL));
		if (NULL == raw_tx)
			return NULL;
		signed_tx = chips_sign_raw_tx_with_wallet(raw_tx);
		tx_info = chips_send_raw_tx(signed_tx);
	}
	return tx_info;
}

cJSON *chips_send_raw_tx(cJSON *signed_tx)
{
	int argc, ret = OK;
	char **argv = NULL;
	cJSON *tx_info = NULL;

	argc = 3;
	ret = bet_alloc_args(argc, &argv);
	if (ret != OK) {
		dlg_error("%s", bet_err_str(ret));
		return NULL;
	}
	argv = bet_copy_args_with_size(argc, blockchain_cli, "sendrawtransaction", jstr(signed_tx, "hex"));

	tx_info = cJSON_CreateObject();
	ret = make_command(argc, argv, &tx_info);
	bet_dealloc_args(argc, &argv);

	if (ret != OK) {
		dlg_error("%s", bet_err_str(ret));
		return NULL;
	}
	return tx_info;
}

cJSON *chips_sign_raw_tx_with_wallet(char *raw_tx)
{
	int argc;
	char **argv = NULL;
	cJSON *signed_tx = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(
		argc, blockchain_cli, "signrawtransaction",
		raw_tx); //sg777: signrawtransactionwithwallet is replaced with signrawtransaction and these changes are temporary for testing chipstensec chain.
	signed_tx = cJSON_CreateObject();
	make_command(argc, argv, &signed_tx);
	bet_dealloc_args(argc, &argv);
	return signed_tx;
}

int32_t chips_publish_multisig_tx(char *tx)
{
	int32_t flag = 0, retval = OK;
	cJSON *tx_info = NULL;

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
		retval = (nn_send(bet_dcv->pubsock, cJSON_Print(tx_info), strlen(cJSON_Print(tx_info)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	}
	return retval;
}

cJSON *chips_spendable_tx()
{
	char **argv = NULL;
	int argc;
	cJSON *listunspent_info = NULL, *spendable_txs = NULL;
	char *temp_file = "utxo.log";

	argc = 4;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "listunspent", ">", temp_file);
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

	spendable_txs = cJSON_CreateArray();
	for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) {
		cJSON *temp = cJSON_GetArrayItem(listunspent_info, i);
		if (strcmp(cJSON_Print(cJSON_GetObjectItem(temp, "spendable")), "true") == 0) {
			cJSON_AddItemReferenceToArray(spendable_txs, temp);
		}
	}
	delete_file(temp_file);
	return spendable_txs;
}

cJSON *chips_create_raw_tx_with_data1(cJSON *vout_info, char *data)
{
	char **argv = NULL, *changeAddress = NULL, params[2][arg_size] = { 0 };
	int argc;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, amount_in_txs = 0, amount_to_transfer = 0;
	char *utxo_temp_file = "utxo.log";

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	/*	
	if (address == NULL) {
		dlg_error("Address to transfer funds in NULL");
		return NULL;
	}
	*/

	for (int32_t i = 0; i < cJSON_GetArraySize(vout_info); i++) {
		amount_to_transfer += jdouble(cJSON_GetArrayItem(vout_info, i), "amount");
	}

	if ((balance + chips_tx_fee) < amount_to_transfer) {
		dlg_warn("Doesn't have sufficient funds to make the tx");
		return NULL;
	}

	for (int32_t i = 0; i < cJSON_GetArraySize(vout_info); i++) {
		cJSON_AddNumberToObject(address_info, jstr(cJSON_GetArrayItem(vout_info, i), "addr"),
					jdouble(cJSON_GetArrayItem(vout_info, i), "amount"));
	}

	amount_to_transfer += chips_tx_fee;

	argc = 4;
	argv = bet_copy_args(argc, blockchain_cli, "listunspent", ">", utxo_temp_file);
	listunspent_info = cJSON_CreateArray();
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) {
		cJSON *utxo = cJSON_GetArrayItem(listunspent_info, i);
		if ((strcmp(cJSON_Print(jobj(utxo, "spendable")), "true") == 0) &&
		    (jdouble(utxo, "amount") >
		     0.0001)) { // This check was added to avoid mining and dust transactions in creating raw tx.
			cJSON *tx_info = cJSON_CreateObject();
			cJSON_AddStringToObject(tx_info, "txid", jstr(utxo, "txid"));
			cJSON_AddNumberToObject(tx_info, "vout", jint(utxo, "vout"));
			cJSON_AddItemToArray(tx_list, tx_info);

			amount_in_txs += jdouble(utxo, "amount");
			if (amount_in_txs >= amount_to_transfer) {
				changeAddress = jstr(utxo, "address");
				change = amount_in_txs - amount_to_transfer;
				break;
			}
		}
	}
	if (amount_to_transfer > amount_in_txs) {
		dlg_warn(
			"Unable to make tx, this can happen in couple of instances: \n1. If there are too many dust tx's this happens if you might be running the mining node on the same node itself.\n2. Trying to spend the tx which is present in the mempool");
		return NULL;
	}

	if (change > 0) {
		cJSON_AddNumberToObject(address_info, changeAddress, change);
	}
	if (data) {
		cJSON_AddStringToObject(address_info, "data", data);
	}

	argc = 4;
	snprintf(params[0], arg_size, "\'%s\'", cJSON_Print(tx_list));
	snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(address_info));
	argv = bet_copy_args(argc, blockchain_cli, "createrawtransaction", params[0], params[1]);
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);
	bet_dealloc_args(argc, &argv);
	delete_file(utxo_temp_file);

	return tx;
}

cJSON *chips_create_raw_tx_with_data(double amount_to_transfer, char *address, char *data)
{
	char **argv = NULL, *changeAddress = NULL, params[2][arg_size] = { 0 };
	int argc;
	cJSON *listunspent_info = NULL, *address_info = NULL, *tx_list = NULL, *tx = NULL;
	double balance, change, amount_in_txs = 0;
	char *utxo_temp_file = "utxo.log";

	balance = chips_get_balance();
	tx_list = cJSON_CreateArray();
	address_info = cJSON_CreateObject();

	if (address == NULL) {
		dlg_error("Address to transfer funds in NULL");
		return NULL;
	}
	if ((balance + chips_tx_fee) < amount_to_transfer) {
		dlg_warn("Insufficient funds\n Funds available::%f \n Funds needed::%f\n", balance, amount_to_transfer);
		return NULL;
	}
	cJSON_AddNumberToObject(address_info, address, amount_to_transfer);
	amount_to_transfer += chips_tx_fee;

	argc = 4;
	argv = bet_copy_args(argc, blockchain_cli, "listunspent", ">", utxo_temp_file);
	listunspent_info = cJSON_CreateArray();
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

	for (int i = 0; i < cJSON_GetArraySize(listunspent_info); i++) {
		cJSON *utxo = cJSON_GetArrayItem(listunspent_info, i);
		if ((strcmp(cJSON_Print(jobj(utxo, "spendable")), "true") == 0) &&
		    (jdouble(utxo, "amount") >
		     0.0001)) { // This check was added to avoid mining and dust transactions in creating raw tx.
			cJSON *tx_info = cJSON_CreateObject();
			cJSON_AddStringToObject(tx_info, "txid", jstr(utxo, "txid"));
			cJSON_AddNumberToObject(tx_info, "vout", jint(utxo, "vout"));
			cJSON_AddItemToArray(tx_list, tx_info);

			amount_in_txs += jdouble(utxo, "amount");
			if (amount_in_txs >= amount_to_transfer) {
				changeAddress = jstr(utxo, "address");
				change = amount_in_txs - amount_to_transfer;
				break;
			}
		}
	}
	if (amount_to_transfer > amount_in_txs) {
		dlg_warn(
			"Unable to make tx, this can happen in couple of instances: \n1. If there are too many dust tx's this happens if you might be running the mining node on the same node itself.\n2. Trying to spend the tx which is present in the mempool");
		return NULL;
	}

	if (change > 0) {
		cJSON_AddNumberToObject(address_info, changeAddress, change);
	}
	if (data) {
		cJSON_AddStringToObject(address_info, "data", data);
	}

	argc = 4;
	snprintf(params[0], arg_size, "\'%s\'", cJSON_Print(tx_list));
	snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(address_info));
	argv = bet_copy_args(argc, blockchain_cli, "createrawtransaction", params[0], params[1]);
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);
	bet_dealloc_args(argc, &argv);
	delete_file(utxo_temp_file);

	return tx;
}

int32_t chips_get_block_count()
{
	int32_t argc, height;
	char **argv = NULL, *rendered = NULL;
	cJSON *block_height = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "getblockcount");
	make_command(argc, argv, &block_height);

	rendered = cJSON_Print(block_height);
	height = atoi(rendered);
	bet_dealloc_args(argc, &argv);
	return height;
}

void check_ln_chips_sync()
{
	int32_t threshold_diff = 1000, retval = OK, ln_bh, chips_bh;

	chips_bh = chips_get_block_count();
	retval = ln_block_height(&ln_bh);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		exit(-1);
	}
	while (1) {
		if ((chips_bh - ln_bh) > threshold_diff) {
			dlg_info("\rln is %d blocks behind chips network\n", (chips_bh - ln_bh));
			fflush(stdout);
		} else
			break;
		chips_bh = chips_get_block_count();
		retval = ln_block_height(&ln_bh);
	}
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
	int32_t argc, retval = OK;
	double balance = 0;
	char **argv = NULL;
	cJSON *getbalanceInfo = NULL;

	argc = 2;
	argv = bet_copy_args(argc, blockchain_cli, "getbalance");
	retval = make_command(argc, argv, &getbalanceInfo);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	} else {
		balance = atof(cJSON_Print(getbalanceInfo));
	}
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
		dlg_info("Not enough trust exists in the system");
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

	argv = bet_copy_args(argc, blockchain_cli, "addmultisigaddress", param,
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

	argv = bet_copy_args(argc, blockchain_cli, "addmultisigaddress", param,
			     cJSON_Print(cJSON_CreateString(cJSON_Print(addr_list)))); //, "-addresstype legacy"
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
	char *temp_file = "utxo.log";

	argc = 4;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "listunspent", ">", temp_file);

	run_command(argc, argv);
	tx_exists = chips_check_tx_exists(temp_file, input_tx);
	bet_dealloc_args(argc, &argv);
	delete_file(temp_file);
	return tx_exists;
}

char *chips_get_block_hash_from_txid(char *txid)
{
	int argc;
	char **argv = NULL;
	cJSON *raw_tx_info = NULL;
	char *block_hash = NULL;

	argc = 4;
	argv = bet_copy_args(argc, blockchain_cli, "getrawtransaction", txid, "1");
	raw_tx_info = cJSON_CreateObject();
	make_command(argc, argv, &raw_tx_info);

	if (jstr(raw_tx_info, "error code") != 0) {
		dlg_info("%s\n", cJSON_Print(raw_tx_info));
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
	argv = bet_copy_args(argc, blockchain_cli, "getblock", block_hash);
	block_info = cJSON_CreateObject();
	make_command(argc, argv, &block_info);
	block_height = jint(block_info, "height");
	bet_dealloc_args(argc, &argv);
	return block_height;
}

cJSON *chips_create_tx_from_tx_list(char *to_addr, int32_t no_of_txs, char tx_ids[][100])
{
	int argc, retval = OK;
	char **argv = NULL, params[2][arg_size] = { 0 }, *hex_data = NULL, *data = NULL, *msig_addr = NULL;
	cJSON *listunspent_info = NULL, *player_info = NULL;
	double amount = 0, value = 0;
	cJSON *tx_list = NULL, *to_addr_info = NULL, *tx = NULL;
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL;
	char *temp_file = "utxo.log";

	to_addr_info = cJSON_CreateObject();
	tx_list = cJSON_CreateArray();
	argc = 4;
	argv = bet_copy_args(argc, blockchain_cli, "listunspent", ">", temp_file);
	listunspent_info = cJSON_CreateObject();
	make_command(argc, argv, &listunspent_info);
	bet_dealloc_args(argc, &argv);

	if (chips_check_tx_exists_in_unspent(temp_file, tx_ids[0]) == 1) {
		raw_tx = chips_get_raw_tx(tx_ids[0]);
		decoded_raw_tx = chips_decode_raw_tx(raw_tx);
		vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");

		hex_data = calloc(tx_data_size * 2, sizeof(char));
		retval = chips_extract_data(tx_ids[0], &hex_data);

		if ((retval == OK) && (hex_data)) {
			data = calloc(tx_data_size, sizeof(char));
			hexstr_to_str(hex_data, data);
			player_info = cJSON_CreateObject();
			player_info = cJSON_Parse(data);
			msig_addr = jstr(player_info, "msig_addr");
		}
		for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
			cJSON *temp = cJSON_GetArrayItem(vout, i);
			dlg_info("%s", cJSON_Print(temp));
			value = jdouble(temp, "value");
			if (value > 0) {
				cJSON *scriptPubKey = cJSON_GetObjectItem(temp, "scriptPubKey");
				cJSON *addresses = cJSON_GetObjectItem(scriptPubKey, "addresses");
				cJSON *address = cJSON_GetObjectItem(scriptPubKey, "address");
				dlg_info("addresses ::%s", cJSON_Print(addresses));
				dlg_info("address ::%s", cJSON_Print(address));

				if (strcmp(msig_addr, jstri(addresses, 0)) == 0) {
					cJSON *tx_info = cJSON_CreateObject();
					amount += jdouble(temp, "value");
					cJSON_AddStringToObject(tx_info, "txid", tx_ids[0]);
					cJSON_AddNumberToObject(tx_info, "vout", jint(temp, "n"));
					cJSON_AddItemToArray(tx_list, tx_info);
				}
			}
		}
	}
	if ((cJSON_GetArraySize(tx_list) == 0) || (amount < chips_tx_fee)) {
		goto end;
	}
	cJSON_AddNumberToObject(to_addr_info, to_addr, (amount - chips_tx_fee));
	argc = 4;
	sprintf(params[0], "\'%s\'", cJSON_Print(tx_list));
	sprintf(params[1], "\'%s\'", cJSON_Print(to_addr_info));
	argv = bet_copy_args(argc, blockchain_cli, "createrawtransaction", params[0], params[1]);
	dlg_info("%s", params[0]);
	dlg_info("%s", params[1]);
	tx = cJSON_CreateObject();
	make_command(argc, argv, &tx);
	bet_dealloc_args(argc, &argv);

end:
	if (data)
		free(data);
	if (hex_data)
		free(hex_data);
	delete_file(temp_file);
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

	dlg_info("%s\n", cJSON_Print(raw_tx));

	bet_check_cashiers_status();
	hex = raw_tx;
	for (int i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			cJSON *temp = chips_sign_msig_tx(notary_node_ips[i], hex);
			if (temp == NULL) {
				continue;
			}
			if (cJSON_GetObjectItem(temp, "signed_tx") != NULL) {
				hex = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "hex");
				signers++;
			} else {
				dlg_error("%s\n", jstr(temp, "err_str"));
				return NULL;
			}
			if (signers == threshold_value) {
				cJSON *status = cJSON_GetObjectItem(cJSON_GetObjectItem(temp, "signed_tx"), "complete");
				if (strcmp(cJSON_Print(status), "true") == 0) {
					tx = chips_send_raw_tx(cJSON_GetObjectItem(temp, "signed_tx"));
					break;
				}
			}
		}
	}
	if (tx == NULL) {
		dlg_error("Error in making payout tx");
	}
	return tx;
}

cJSON *chips_get_raw_tx(char *tx)
{
	int argc, retval = OK;
	char **argv = NULL;
	cJSON *raw_tx = NULL;

	argc = 3;
	argv = bet_copy_args(argc, blockchain_cli, "getrawtransaction", tx);
	raw_tx = cJSON_CreateObject();
	retval = make_command(argc, argv, &raw_tx);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);

	if (jstr(raw_tx, "error") != 0) {
		dlg_error("%s\n", cJSON_Print(raw_tx));
		return NULL;
	}

	return raw_tx;
}

cJSON *chips_decode_raw_tx(cJSON *raw_tx)
{
	int argc, retval = OK;
	char **argv = NULL;
	cJSON *decoded_raw_tx = NULL;

	argc = 3;
	argv = bet_copy_args_with_size(argc, blockchain_cli, "decoderawtransaction", cJSON_Print(raw_tx));

	if (argv) {
		decoded_raw_tx = cJSON_CreateObject();
		retval = make_command(argc, argv, &decoded_raw_tx);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
	}
	bet_dealloc_args(argc, &argv);
	return decoded_raw_tx;
}

void chips_validate_tx(char *tx)
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
			dlg_info("pubkey::%s\n", cJSON_Print(pubkey));
		}
	}
}

#if 0
int32_t chips_extract_data(char *tx, char **rand_str)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL, *script_pubkey = NULL;
	int32_t retval = OK;

	raw_tx = chips_get_raw_tx(tx);
	if (raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return ERR_CHIPS_GET_RAW_TX;
	}
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	if (decoded_raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		return ERR_CHIPS_DECODE_TX;
	}
	vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		script_pubkey = cJSON_GetObjectItem(temp, "scriptPubKey");
		if (0 == strcmp(jstr(script_pubkey, "type"), "nulldata")) {
			char *data = jstr(script_pubkey, "asm");
			strtok(data, " ");
			strcpy((*rand_str), strtok(NULL, data));
			break;
		}
	}
	return retval;
}
#endif

int32_t chips_extract_data(char *tx, char **rand_str)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL, *script_pubkey = NULL;
	int32_t retval = OK;

	raw_tx = chips_get_raw_tx(tx);
	if (raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return ERR_CHIPS_GET_RAW_TX;
	}
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	if (decoded_raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		return ERR_CHIPS_DECODE_TX;
	}
	vout = cJSON_GetObjectItem(decoded_raw_tx, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		//dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp));
		double value = jdouble(temp, "value");
		script_pubkey = cJSON_GetObjectItem(temp, "scriptPubKey");
		if (value == 0.0) {
			char *data = jstr(script_pubkey, "asm");
			strtok(data, " ");
			strcpy((*rand_str), strtok(NULL, data));
			break;
		}
	}
	//dlg_info("%s::%d::str::%s\n", __FUNCTION__, __LINE__, *rand_str);
	return retval;
}

cJSON *chips_extract_tx_data_in_JSON(char *tx)
{
	char *hex_data = NULL, *data = NULL;
	cJSON *tx_data = NULL;

	hex_data = calloc(tx_data_size * 2, sizeof(char));
	data = calloc(tx_data_size * 2, sizeof(char));
	if (chips_extract_data(tx, &hex_data) == OK) {
		hexstr_to_str(hex_data, data);
		tx_data = cJSON_CreateObject();
		tx_data = cJSON_Parse(data);
		if (!is_cJSON_Object(tx_data)) {
			tx_data = NULL;
		}
	}
	if (hex_data)
		free(hex_data);
	if (data)
		free(data);

	return tx_data;
}

cJSON *chips_deposit_to_ln_wallet(double channel_chips)
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *newaddr = NULL;
	cJSON *tx = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "newaddr", "p2sh-segwit");
	newaddr = cJSON_CreateObject();
	retval = make_command(argc, argv, &newaddr);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	tx = chips_transfer_funds(channel_chips, jstr(newaddr, "p2sh-segwit"));
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

double chips_get_balance_on_address_from_tx(char *address, char *tx)
{
	cJSON *raw_tx = NULL, *decoded_raw_tx = NULL, *vout = NULL;
	double balance = 0;

	raw_tx = chips_get_raw_tx(tx);
	if (raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return ERR_CHIPS_GET_RAW_TX;
	}
	decoded_raw_tx = chips_decode_raw_tx(raw_tx);
	if (decoded_raw_tx == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		return ERR_CHIPS_DECODE_TX;
	}

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
	argv = bet_copy_args(argc, blockchain_cli, "listaddressgroupings");
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
	int retval = -1;
	double value = 0.01;

	raw_tx_info = chips_get_raw_tx(tx_id);
	if (raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return retval;
	}
	decode_raw_tx_info = chips_decode_raw_tx(raw_tx_info);
	if (decode_raw_tx_info == NULL) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_DECODE_TX));
		return retval;
	}

	vout = cJSON_GetObjectItem(decode_raw_tx_info, "vout");
	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		cJSON *temp = cJSON_GetArrayItem(vout, i);
		if (abs(value - jdouble(temp, "value") < epsilon)) {
			retval = jint(temp, "n");
			break;
		}
	}
	return retval;
}

cJSON *chips_create_payout_tx(cJSON *payout_addr, int32_t no_of_txs, char tx_ids[][100], char *data)
{
	double payout_amount = 0, amount_in_txs = 0;
	cJSON *tx_list = NULL, *addr_info = NULL, *tx_details = NULL, *payout_tx_info = NULL, *payout_tx = NULL;
	int argc, vout;
	char **argv = NULL, params[2][arg_size] = { 0 }, *sql_query = NULL;

	dlg_info("%s", cJSON_Print(payout_addr));
	for (int32_t i = 0; i < cJSON_GetArraySize(payout_addr); i++) {
		cJSON *addr_info = cJSON_GetArrayItem(payout_addr, i);
		payout_amount += jdouble(addr_info, "amount");
	}
	for (int32_t i = 0; i < no_of_txs; i++) {
		amount_in_txs += chips_get_balance_on_address_from_tx(legacy_m_of_n_msig_addr, tx_ids[i]);
	}
	if ((amount_in_txs - (payout_amount + chips_tx_fee)) < 0) {
		dlg_error("Mismatch b/w Payout and Payin amounts");
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
	dlg_info("%s\n", cJSON_Print(payout_addr));
	addr_info = cJSON_CreateObject();
	for (int32_t i = 0; i < cJSON_GetArraySize(payout_addr); i++) {
		cJSON *temp = cJSON_GetArrayItem(payout_addr, i);
		cJSON_AddNumberToObject(addr_info, jstr(temp, "address"), jdouble(temp, "amount"));
	}
	if (data)
		cJSON_AddStringToObject(addr_info, "data", data);

	argc = 4;
	bet_alloc_args(argc, &argv);
	sprintf(params[0], "\'%s\'", cJSON_Print(tx_list));
	sprintf(params[1], "\'%s\'", cJSON_Print(addr_info));

	argv = bet_copy_args(argc, blockchain_cli, "createrawtransaction", params[0], params[1]);
	make_command(argc, argv, &tx_details);

	dlg_info("raw_tx::%s\n", cJSON_Print(tx_details));

	payout_tx = chips_spend_msig_tx(tx_details);

	payout_tx_info = cJSON_CreateObject();
	cJSON_AddStringToObject(payout_tx_info, "method", "payout_tx");
	cJSON_AddStringToObject(payout_tx_info, "table_id", table_id);
	cJSON_AddItemToObject(payout_tx_info, "tx_info", payout_tx);

	if (payout_tx) {
		dlg_info("tx::%s\n", cJSON_Print(payout_tx));
		sql_query = calloc(sql_query_size, sizeof(char));
		sprintf(sql_query, "UPDATE dcv_tx_mapping set status = 0 where table_id = \"%s\";", table_id);
		bet_run_query(sql_query);
		for (int32_t i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1) {
				bet_msg_cashier(payout_tx_info, notary_node_ips[i]);
			}
		}
	} else {
		dlg_error("%s::%d::Error occured in processing the payout tx ::%s\n", __FUNCTION__, __LINE__,
			  cJSON_Print(tx_details));
	}
	if (sql_query)
		free(sql_query);
	bet_dealloc_args(argc, &argv);
	return payout_tx_info;
}

static void chips_read_valid_unspent(char *file_name, cJSON **argjson)
{
	FILE *fp = NULL;
	char ch, buf[4196];
	int32_t len = 0;
	cJSON *temp = NULL;

	temp = cJSON_CreateObject();
	*argjson = cJSON_CreateArray();
	fp = fopen(file_name, "r");
	if (fp) {
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
		fclose(fp);
	}
}

int32_t chips_check_tx_exists_in_unspent(char *file_name, char *tx_id)
{
	FILE *fp = NULL;
	char ch, buf[4196];
	int32_t len = 0, tx_exists = 0;
	cJSON *temp = NULL;

	temp = cJSON_CreateObject();
	fp = fopen(file_name, "r");
	if (fp) {
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
							if (strcmp(unstringify(cJSON_Print(
									   cJSON_GetObjectItem(temp, "txid"))),
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
		fclose(fp);
	}
	return tx_exists;
}

int32_t chips_check_tx_exists(char *file_name, char *tx_id)
{
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

int32_t chips_is_mempool_empty()
{
	int32_t argc, is_empty = 1;
	char **argv = NULL;
	cJSON *mempool_info = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, blockchain_cli, "getrawmempool");
	make_command(argc, argv, &mempool_info);

	if ((mempool_info) && (cJSON_GetArraySize(mempool_info) != 0))
		is_empty = 0;

	bet_dealloc_args(argc, &argv);
	return is_empty;
}

struct cJSON *do_split_tx_amount(double amount, int32_t no_of_splits)
{
	cJSON *vout_info = NULL, *tx_id = NULL;
	double tx_amount = 0;
	if ((amount + chips_tx_fee) > chips_get_balance()) {
		dlg_warn("Not enough utxo's of amount ::%f are available", amount);
		return NULL;
	}
	tx_amount = amount / no_of_splits;

	vout_info = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_splits; i++) {
		cJSON *temp = cJSON_CreateObject();
		cJSON_AddStringToObject(temp, "addr", chips_get_new_address());
		cJSON_AddNumberToObject(temp, "amount", tx_amount);
		cJSON_AddItemToArray(vout_info, temp);
	}

	tx_id = chips_transfer_funds_with_data1(vout_info, NULL);

	if (tx_id) {
		dlg_info("The split tx is :: %s", cJSON_Print(tx_id));
	} else {
		dlg_error("Error occured in doing tx_split");
	}
	return tx_id;
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
	fp = popen(command, "r");
	if (fp == NULL) {
		dlg_error("Failed to run command::%s\n", command);
		exit(1);
	}

end:
	if (command)
		free(command);
	pclose(fp);

	return ret;
}

static int32_t process_ln_data(char *data, char *command_name, cJSON **ln_data)
{
	int32_t retval = OK;

	*ln_data = NULL;
	if (strlen(data) == 0)
		return ERR_LN_COMMAND;

	*ln_data = cJSON_Parse(data);
	if (*ln_data == NULL)
		return ERR_LN_COMMAND;
	if (jint(*ln_data, "code") != 0) {
		dlg_error("%s", cJSON_Print(*ln_data));
		if (strcmp(command_name, "pay") == 0) {
			retval = ERR_LN_PAY;
		} else if (strcmp(command_name, "invoice") == 0) {
			retval = ERR_LN_INVOICE_CREATE;
		} else if (strcmp(command_name, "newaddr") == 0) {
			retval = ERR_LN_NEWADDR;
		} else if (strcmp(command_name, "listfunds") == 0) {
			retval = ERR_LN_LISTFUNDS;
		} else if (strcmp(command_name, "fundchannel") == 0) {
			retval = ERR_LN_FUNDCHANNEL;
		} else if (strcmp(command_name, "listpeers") == 0) {
			retval = ERR_LN_LISTPEERS;
		} else if (strcmp(command_name, "peer-channel-state") == 0) {
			retval = ERR_LN_PEERCHANNEL_STATE;
		}
	}
	return retval;
}

int32_t make_command(int argc, char **argv, cJSON **argjson)
{
	FILE *fp = NULL;
	char *data = NULL, *command = NULL, *buf = NULL;
	int32_t retval = OK;
	unsigned long data_size = 262144, buf_size = 1024, cmd_size = 1024;

	if (argv == NULL) {
		return ERR_ARGS_NULL;
	}
	for (int32_t i = 0; i < argc; i++) {
		cmd_size += strlen(argv[i]);
	}

	command = calloc(cmd_size, sizeof(char));
	if (!command) {
		retval = ERR_MEMORY_ALLOC;
		return retval;
	}

	for (int32_t i = 0; i < argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	dlg_info("command :: %s\n", command);
	if (strcmp(argv[0], "lightning-cli") == 0)
		dlg_info("LN command :: %s\n", command);

	//dlg_info("\nchips-pbaas command :: %s\n", command);
	fp = popen(command, "r");
	if (fp == NULL) {
		dlg_error("%s::%d::Fail to open the pipe while running the command::%s\n", __FUNCTION__, __LINE__,
			  command);
		if (strcmp(argv[0], blockchain_cli) == 0)
			retval = ERR_CHIPS_COMMAND;
		if (strcmp(argv[0], "lightning-cli") == 0)
			retval = ERR_LN_COMMAND;
		return retval;
	}

	data = calloc(data_size, sizeof(char));
	if (!data) {
		retval = ERR_MEMORY_ALLOC;
		goto end;
	}
	buf = calloc(buf_size, sizeof(char));
	if (!buf) {
		retval = ERR_MEMORY_ALLOC;
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
	if (strcmp(argv[0], "git") == 0) {
		*argjson = cJSON_CreateString((const char *)data);
	} else if (strcmp(argv[0], "lightning-cli") == 0) {
		retval = process_ln_data(data, argv[1], argjson);
	} else if (strcmp(argv[0], blockchain_cli) == 0) {
		if (strlen(data) == 0) {
			if (strcmp(argv[1], "importaddress") == 0) {
				// Do nothing
			} else if (strcmp(argv[1], "listunspent") == 0) {
				chips_read_valid_unspent(argv[3], argjson);
			} else if (strcmp(argv[1], "updateidentity") == 0) {
				retval = ERR_UPDATEIDENTITY;
				jaddnum(*argjson, "error", retval);
			} else if (strcmp(argv[1], "getidentity") == 0) {
				retval = ERR_ID_NOT_FOUND;
			} else {
				dlg_error("%s::%d::Error in running the command ::%s\n", __FUNCTION__, __LINE__,
					  command);
				retval = ERR_CHIPS_COMMAND;
			}
		} else {
			if (strcmp(argv[1], "addmultisigaddress") == 0) {
				if (strcmp(chips_cli, blockchain_cli) == 0) {
					*argjson = cJSON_Parse(data);
					cJSON_AddNumberToObject(*argjson, "code", 0);
				} else if (strcmp(verus_chips_cli, blockchain_cli) == 0) {
					if (data[strlen(data) - 1] == '\n')
						data[strlen(data) - 1] = '\0';
					*argjson = cJSON_CreateObject();

					cJSON_AddStringToObject(*argjson, "address", data);
					cJSON_AddNumberToObject(*argjson, "code", 0);
				}
				goto end;
			} else if ((strcmp(argv[1], "createrawtransaction") == 0) ||
				   (strcmp(argv[1], "sendrawtransaction") == 0) ||
				   (strcmp(argv[1], "getnewaddress") == 0) || (strcmp(argv[1], "getblockhash") == 0)) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';

				*argjson = cJSON_CreateString(data);
			} else if (strcmp(argv[1], "getrawtransaction") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				if (strstr(data, "error") != NULL) {
					retval = ERR_NO_TX_INFO_AVAILABLE;
				} else {
					*argjson = cJSON_CreateString(data);
				}
			} else if (strcmp(argv[1], "getrawmempool") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				*argjson = cJSON_Parse(data);
			} else if (strcmp(argv[1], "updateidentity") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				if (strstr(data, "error") != NULL) {
					retval = ERR_UPDATEIDENTITY;
				} else {
					jaddstr(*argjson, "tx", data);
				}
				jaddnum(*argjson, "error", retval);
			} else if (strcmp(argv[1], "getidentity") == 0) {
				if (strstr(data, "error") != NULL)
					retval = ERR_ID_NOT_FOUND;
				else
					*argjson = cJSON_Parse(data);
			} else if (strcmp(argv[1], "z_getoperationresult") == 0) {
				*argjson = cJSON_Parse(data);
			} else if (strcmp(argv[1], "sendcurrency") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				cJSON_AddStringToObject(*argjson, "op_id", data);
			} else {
				*argjson = cJSON_Parse(data);
				cJSON_AddNumberToObject(*argjson, "code", 0);
			}
		}
	}
end:
	if (buf)
		free(buf);
	if (data)
		free(data);
	if (command)
		free(command);
	if (fp)
		pclose(fp);

	return retval;
}

char *ln_get_new_address()
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *addr_info = NULL;

	argc = 2;
	argv = bet_copy_args(argc, "lightning-cli", "newaddr");
	addr_info = cJSON_CreateObject();
	retval = make_command(argc, argv, &addr_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);

	return jstr(addr_info, "p2sh-segwit");
}

int32_t ln_block_height(int32_t *block_height)
{
	char **argv = NULL;
	int32_t argc, retval = OK;
	cJSON *ln_info = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "getinfo");
	ln_info = cJSON_CreateObject();
	retval = make_command(argc, argv, &ln_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	*block_height = jint(ln_info, "blockheight");
	bet_dealloc_args(argc, &argv);
	return retval;
}

int32_t ln_listfunds()
{
	cJSON *list_funds = NULL, *outputs = NULL;
	int argc;
	char **argv = NULL;
	int32_t value = 0, retval = OK;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "listfunds");
	list_funds = cJSON_CreateObject();
	retval = make_command(argc, argv, &list_funds);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		value = 0;
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

char *ln_get_uri(char **uri)
{
	cJSON *channel_info = NULL, *addresses = NULL, *address = NULL;
	int32_t argc, port, retval = OK;
	char **argv = NULL, port_str[6], *type = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "getinfo");
	channel_info = cJSON_CreateObject();
	retval = make_command(argc, argv, &channel_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		goto end;
	}
	strcpy(*uri, jstr(channel_info, "id"));
	strcat(*uri, "@");
	addresses = cJSON_GetObjectItem(channel_info, "address");
	address = cJSON_GetArrayItem(addresses, 0);
	strcat(*uri, jstr(address, "address"));
	strcat(*uri, ":");
	port = jint(address, "port");
	sprintf(port_str, "%d", port);
	strcat(*uri, port_str);
	type = jstr(address, "type");

end:
	bet_dealloc_args(argc, &argv);
	return type;
}

int32_t ln_connect_uri(char *uri)
{
	int32_t argc, retval = OK, channel_state;
	char **argv = NULL, channel_id[ln_uri_length];
	cJSON *connect_info = NULL;
	char temp[ln_uri_length];

	strncpy(temp, uri, strlen(uri));
	strcpy(channel_id, strtok(temp, "@"));

	channel_state = ln_get_channel_status(channel_id);
	if ((channel_state != CHANNELD_AWAITING_LOCKIN) && (channel_state != CHANNELD_NORMAL)) {
		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "lightning-cli", "connect", uri);
		connect_info = cJSON_CreateObject();
		retval = make_command(argc, argv, &connect_info);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

cJSON *ln_fund_channel(char *channel_id, int32_t channel_fund_satoshi)
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *fund_channel_info = NULL;
	char channel_satoshis[20];

	argc = 4;
	bet_alloc_args(argc, &argv);
	snprintf(channel_satoshis, 20, "%d", channel_fund_satoshi);
	argv = bet_copy_args(argc, "lightning-cli", "fundchannel", channel_id, channel_satoshis);
	fund_channel_info = cJSON_CreateObject();
	retval = make_command(argc, argv, &fund_channel_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);
	return fund_channel_info;
}

int32_t ln_pay(char *bolt11)
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *pay_response = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "pay", bolt11);
	pay_response = cJSON_CreateObject();
	retval = make_command(argc, argv, &pay_response);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

cJSON *ln_connect(char *id)
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *connect_info = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	connect_info = cJSON_CreateObject();

	argv = bet_copy_args(argc, "lightning-cli", "connect", id);
	retval = make_command(argc, argv, &connect_info);

	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}

	bet_dealloc_args(argc, &argv);
	return connect_info;
}

int32_t ln_check_if_address_isof_type(char *type)
{
	int32_t argc, retval = OK, flag = 0;
	char **argv = NULL;
	cJSON *channel_info = NULL, *addresses = NULL, *binding = NULL;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "getinfo");

	channel_info = cJSON_CreateObject();
	retval = make_command(argc, argv, &channel_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		goto end;
	}
	addresses = cJSON_CreateObject();
	addresses = cJSON_GetObjectItem(channel_info, "address");
	for (int32_t i = 0; i < cJSON_GetArraySize(addresses); i++) {
		if (strcmp(jstr(cJSON_GetArrayItem(addresses, i), "type"), type) == 0) {
			flag = 1;
			break;
		}
	}
	if (flag == 0) {
		binding = cJSON_CreateObject();
		binding = cJSON_GetObjectItem(channel_info, "binding");
		for (int32_t i = 0; i < cJSON_GetArraySize(binding); i++) {
			if (strcmp(jstr(cJSON_GetArrayItem(binding, i), "type"), type) == 0) {
				flag = 1;
				break;
			}
		}
	}
	if (flag == 0) {
		retval = ERR_LN_ADDRESS_TYPE_MISMATCH;
	}

end:
	bet_dealloc_args(argc, &argv);
	return retval;
}

int32_t ln_check_peer_and_connect(char *id)
{
	int32_t argc, retval = OK;
	char **argv = NULL;
	cJSON *list_peers_info = NULL;
	cJSON *peers_info = NULL;
	int32_t connected = 0;

	argc = 2;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "listpeers");
	retval = make_command(argc, argv, &list_peers_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		goto end;
	}

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
end:
	bet_dealloc_args(argc, &argv);
	return retval;
}

int32_t ln_get_channel_status(char *id)
{
	int32_t argc, channel_state = 0, retval = OK;
	char **argv = NULL;
	cJSON *channel_state_info = NULL, *channel_states = NULL, *channelState = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "peer-channel-state", id);
	retval = make_command(argc, argv, &channel_state_info);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
	}

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

int32_t ln_establish_channel(char *uri)
{
	int32_t retval = OK, state, ln_bh, chips_bh;
	cJSON *connect_info = NULL, *fund_channel_info = NULL;
	double amount;
	char uid[ln_uri_length] = { 0 };

	strcpy(uid, uri);
	if ((ln_get_channel_status(strtok(uid, "@")) != CHANNELD_NORMAL)) {
		dlg_info("LN uri::%s", uri);
		connect_info = ln_connect(uri);
		if (jint(connect_info, "code") != 0) {
			dlg_error("%s", jstr(connect_info, "message"));
			retval = ERR_LN;
			goto end;
		}
		if (ln_listfunds() < (channel_fund_satoshis + (chips_tx_fee * satoshis))) {
			amount = channel_fund_satoshis - ln_listfunds();
			amount = amount / satoshis;

			dlg_warn(
				"LN wallet doesn't have sufficient funds, checking to load LN wallet from CHIPS wallet...");
			if (chips_get_balance() >= (amount + (2 * chips_tx_fee))) {
				dlg_info("Loading funds from CHIPS to LN wallet...");
				cJSON *tx_info = chips_deposit_to_ln_wallet(amount + chips_tx_fee);

				if (tx_info) {
					dlg_info(" %f CHIPS transferred from CHIPS to LN wallet, tx_id :: %s", amount,
						 cJSON_Print(tx_info));
					//The below while loop is to wait for the tx to be mined.
					while (chips_get_block_hash_from_txid(cJSON_Print(tx_info)) == NULL) {
						sleep(2);
					}
					chips_bh = chips_get_block_height_from_block_hash(
						chips_get_block_hash_from_txid(cJSON_Print(tx_info)));
					do {
						sleep(2);
						retval = ln_block_height(&ln_bh);
						if (retval != OK) {
							dlg_error("%s", bet_err_str(retval));
							return retval;
						}
					} while (ln_bh < chips_bh);
				} else {
					retval = ERR_CHIPS_TX_FAILED;
					dlg_error("Automatic loading of the LN wallet from the CHIPS wallet is failed");
					goto end;
				}
			} else {
				retval = ERR_LN_INSUFFICIENT_FUNDS;
				dlg_warn(
					"Even CHIPS wallet is short of (%f CHIPS) the funds...try loading either CHIPS or LN wallet manually",
					amount);
				goto end;
			}
		}

		dlg_info("Funding the LN channel :: %s", jstr(connect_info, "id"));
		fund_channel_info = ln_fund_channel(jstr(connect_info, "id"), channel_fund_satoshis);
		if (jint(fund_channel_info, "code") != 0) {
			dlg_error("%s", cJSON_Print(fund_channel_info));
			retval = ERR_LN_FUNDCHANNEL;
			goto end;
		}
		while ((state = ln_get_channel_status(jstr(connect_info, "id"))) != CHANNELD_NORMAL) {
			if (state == CHANNELD_AWAITING_LOCKIN) {
				fflush(stdout);
			} else if (state == 8) {
				dlg_info("ONCHAIN");
			} else
				dlg_info("LN channel state :: %d\n", state);

			sleep(2);
		}
	}
end:
	return retval;
}

char *bet_git_version()
{
	int argc = 2;
	char **argv = NULL;
	cJSON *version = NULL;

	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "git", "describe");
	version = cJSON_CreateObject();
	make_command(argc, argv, &version);

	bet_dealloc_args(argc, &argv);

	return unstringify(cJSON_Print(version));
}

int32_t scan_games_info()
{
	int32_t retval = OK, latest_bh, bh;
	cJSON *block_info = NULL, *tx_info = NULL, *tx_data_info = NULL;

	latest_bh = chips_get_block_count();
	bh = sqlite3_get_highest_bh() + 1;
	if (bh < sc_start_block)
		bh = sc_start_block;

	dlg_info("Blocks scanned till bh :: %d", bh);
	for (; bh <= latest_bh; bh++) {
		printf("Scanning blocks ::%d\r", bh);
		block_info = chips_get_block_from_block_height(bh);
		if (block_info) {
			tx_info = cJSON_GetObjectItem(block_info, "tx");
			if (cJSON_GetArraySize(tx_info) < 2) {
				continue;
			} else {
				for (int32_t i = 0; i < cJSON_GetArraySize(tx_info); i++) {
					tx_data_info = chips_extract_tx_data_in_JSON(jstri(tx_info, i));
					if (tx_data_info) {
						retval = bet_insert_sc_game_info(jstri(tx_info, i),
										 jstr(tx_data_info, "table_id"), bh,
										 jstr(tx_data_info, "tx_type"));
					}
				}
			}
		}
	}
	dlg_info("Scanning the blockchain completed, local DB updated.");
	return retval;
}

void wait_for_a_blocktime()
{
	int32_t bh;
	bh = chips_get_block_count();
	do {
		sleep(1);
	} while (bh == chips_get_block_count());
}

bool check_if_tx_exists(char *tx_id)
{
	int32_t argc = 3, retval = OK;
	char **argv = NULL;
	cJSON *argjson = NULL;

	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getrawtransaction", tx_id);
	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval == OK)
		return true;
	else
		return false;
}
