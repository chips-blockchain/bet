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
#include "misc.h"
#include "err.h"

#include <stdarg.h>
#include <inttypes.h>
#include <curl/curl.h>
#include "../../includes/math_compat.h"
#include "config.h"

double epsilon = 0.000000001;

/* REST API support structures and functions */
struct rpc_response {
	char *data;
	size_t size;
};

static size_t rpc_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct rpc_response *resp = (struct rpc_response *)userp;

	char *ptr = realloc(resp->data, resp->size + realsize + 1);
	if (!ptr) {
		dlg_error("Not enough memory for RPC response");
		return 0;
	}

	resp->data = ptr;
	memcpy(&(resp->data[resp->size]), contents, realsize);
	resp->size += realsize;
	resp->data[resp->size] = '\0';

	return realsize;
}

/**
 * Internal: Make a JSON-RPC call via HTTP REST API
 */
static int32_t chips_rpc_rest(const char *method, const char *params_str, cJSON **result)
{
	CURL *curl;
	CURLcode res;
	struct rpc_response chunk = { 0 };

	chunk.data = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();
	if (!curl) {
		dlg_error("Failed to initialize curl");
		free(chunk.data);
		return ERR_CHIPS_RPC;
	}

	// Build JSON-RPC request payload - large buffer for deck data
	size_t params_len = params_str ? strlen(params_str) : 2;
	size_t payload_size = params_len + 256;  // Extra space for JSON wrapper
	char *json_payload = malloc(payload_size);
	if (!json_payload) {
		dlg_error("Failed to allocate memory for JSON payload");
		free(chunk.data);
		curl_easy_cleanup(curl);
		return ERR_MEMORY_ALLOC;
	}
	snprintf(json_payload, payload_size,
		"{\"jsonrpc\":\"1.0\",\"id\":\"bet\",\"method\":\"%s\",\"params\":%s}",
		method, params_str ? params_str : "[]");

	// Set up curl options
	curl_easy_setopt(curl, CURLOPT_URL, bet_get_rpc_url());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, rpc_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	// Set authentication
	char userpwd[512];
	snprintf(userpwd, sizeof(userpwd), "%s:%s", bet_get_rpc_user(), bet_get_rpc_password());
	curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);

	// Set HTTP headers
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// Perform the request
	res = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	free(json_payload);  // Free payload after request

	if (res != CURLE_OK) {
		dlg_error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		free(chunk.data);
		return ERR_CHIPS_RPC;
	}

	long response_code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	curl_easy_cleanup(curl);

	if (response_code != 200) {
		// Only log HTTP errors in verbose mode (not for expected "not found" cases)
		// dlg_error("HTTP error: %ld, Response: %s", response_code, chunk.data);
		free(chunk.data);
		return ERR_CHIPS_RPC;
	}

	// Parse response
	cJSON *json = cJSON_Parse(chunk.data);
	free(chunk.data);

	if (!json) {
		dlg_error("Failed to parse RPC response");
		return ERR_JSON_PARSING;
	}

	// Check for error in response
	cJSON *error = cJSON_GetObjectItem(json, "error");
	if (error && error->type != cJSON_NULL) {
		dlg_error("RPC error: %s", cJSON_Print(error));
		cJSON_Delete(json);
		return ERR_CHIPS_RPC;
	}

	// Extract result
	cJSON *res_obj = cJSON_GetObjectItem(json, "result");
	if (res_obj) {
		*result = cJSON_Duplicate(res_obj, 1);
	} else {
		*result = cJSON_Duplicate(json, 1);
	}

	cJSON_Delete(json);
	return OK;
}

/**
 * Internal: Make a JSON-RPC call via CLI command
 */
static int32_t chips_rpc_cli(const char *method, cJSON *params, cJSON **result)
{
	int argc = 2;
	char **argv = NULL;
	int32_t retval = OK;

	// Count params
	int param_count = params ? cJSON_GetArraySize(params) : 0;
	argc += param_count;

	// Allocate args
	retval = bet_alloc_args(argc, &argv);
	if (retval != OK) {
		return retval;
	}

	// Build command: blockchain_cli method [params...]
	strncpy(argv[0], blockchain_cli, arg_size - 1);
	strncpy(argv[1], method, arg_size - 1);

	/* The CLI is dispatched via popen(), which goes through /bin/sh -c, so any
	 * shell metacharacters in string params (e.g. "*" used as the wildcard
	 * wallet address in sendcurrency) get glob-expanded before they reach the
	 * verus CLI. Always single-quote string params so they survive the shell
	 * verbatim, mirroring what we already do for object/array params. */
	for (int i = 0; i < param_count; i++) {
		cJSON *param = cJSON_GetArrayItem(params, i);
		if (is_cJSON_String(param)) {
			snprintf(argv[2 + i], arg_size, "'%s'", param->valuestring);
		} else if (is_cJSON_Number(param)) {
			snprintf(argv[2 + i], arg_size, "%g", param->valuedouble);
		} else {
			char *json_str = cJSON_PrintUnformatted(param);
			if (json_str) {
				snprintf(argv[2 + i], arg_size, "'%s'", json_str);
				free(json_str);
			}
		}
	}

	/* make_command's blockchain-cli branch (e.g. updateidentity, sendcurrency)
	 * mutates *argjson via jaddstr/jaddnum, which silently no-op when the
	 * target is NULL. The REST path (chips_rpc_rest) pre-allocates via
	 * cJSON_Duplicate, but the CLI path historically did not, so the result
	 * came back NULL and update_with_retry kept retrying forever. Allocate
	 * an empty object up front, mirroring the REST contract. */
	if (*result == NULL) {
		*result = cJSON_CreateObject();
	}

	retval = make_command(argc, argv, result);
	bet_dealloc_args(argc, &argv);

	return retval;
}

/**
 * Unified CHIPS RPC interface - automatically uses REST or CLI based on config
 * 
 * @param method RPC method name (e.g., "getbalance", "getblockchaininfo")
 * @param params cJSON array of parameters (can be NULL for no params)
 * @param result Output cJSON object (caller must free with cJSON_Delete)
 * @return OK on success, error code on failure
 */
int32_t chips_rpc(const char *method, cJSON *params, cJSON **result)
{
	if (!method) {
		dlg_error("RPC method cannot be NULL");
		return ERR_ARGS_NULL;
	}

	if (bet_use_rest_api()) {
		// Use REST API
		char *params_str = NULL;
		if (params) {
			params_str = cJSON_PrintUnformatted(params);
		}
		int32_t retval = chips_rpc_rest(method, params_str ? params_str : "[]", result);
		if (params_str) {
			free(params_str);
		}
		return retval;
	} else {
		// Use CLI
		return chips_rpc_cli(method, params, result);
	}
}

char blockchain_cli[1024] = "verus -chain=chips";

char *chips_cli = "chips-cli";
char *verus_chips_cli = "verus -chain=chips";

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
		/* Need room for terminating NUL in arg_size buffer */
		if (strlen(va_arg(va_copy, char *)) >= arg_size) {
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
// Nanomsg removed - no longer used
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
	int32_t height = 0;
	int32_t retval = OK;
	cJSON *result = NULL;

	retval = chips_rpc("getblockcount", NULL, &result);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		return 0;
	}

	if (result != NULL) {
		if (is_cJSON_Number(result)) {
			height = (int32_t)result->valuedouble;
		} else if (is_cJSON_String(result)) {
			height = atoi(result->valuestring);
		} else {
			char *rendered = cJSON_Print(result);
			if (rendered) {
				height = atoi(rendered);
				free(rendered);
			}
		}
		cJSON_Delete(result);
	}
	return height;
}

// Lightning Network code removed - no longer used

double chips_get_balance()
{
	int32_t retval = OK;
	double balance = 0;
	cJSON *result = NULL;

	retval = chips_rpc("getbalance", NULL, &result);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		return 0;
	}

	if (result != NULL) {
		if (is_cJSON_Number(result)) {
			balance = result->valuedouble;
		} else if (is_cJSON_String(result)) {
			balance = atof(result->valuestring);
		} else {
			// For CLI mode which may return stringified number
			char *balance_str = cJSON_Print(result);
			if (balance_str != NULL) {
				char *clean_str = unstringify(balance_str);
				balance = atof(clean_str);
				free(balance_str);
			}
		}
		cJSON_Delete(result);
	}
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

// Lightning Network code removed - chips_deposit_to_ln_wallet() no longer used

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
	cJSON *params = NULL, *decoded_tx = NULL, *vout = NULL;
	double balance = 0;

	/* Use chips_rpc("getrawtransaction", ["<txid>", 1], ...) which maps to
	 * chips_rpc_cli and produces the correct single-quoted, verbose command:
	 *   verus -chain=VRSCTEST getrawtransaction '<txid>' 1
	 * The old bet_copy_args + make_command path silently drops the "1" arg
	 * when bet_copy_args fails, causing a non-verbose response (hex string)
	 * that cJSON_Parse fails to decode, returning balance 0. */
	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, cJSON_CreateString(tx));
	cJSON_AddItemToArray(params, cJSON_CreateNumber(1));

	chips_rpc("getrawtransaction", params, &decoded_tx);
	cJSON_Delete(params);

	if (!decoded_tx) {
		dlg_error("%s", bet_err_str(ERR_CHIPS_GET_RAW_TX));
		return ERR_CHIPS_GET_RAW_TX;
	}

	vout = cJSON_GetObjectItem(decoded_tx, "vout");

	for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
		if (find_address_in_addresses(address, cJSON_GetArrayItem(vout, i)) == 1) {
			balance += jdouble(cJSON_GetArrayItem(vout, i), "value");
		}
	}
	cJSON_Delete(decoded_tx);
	return balance;
}

/*
 * chips_get_wallet_address
 * ------------------------
 * Returns a wallet R-address suitable for putting in the table_info "addr"
 * field, payout transactions, etc.
 *
 * Historically this called `listaddressgroupings` and walked the entire
 * wallet looking for the first address that passed `chips_validate_address`.
 * On a regtest wallet that has accumulated tens of thousands of addresses
 * from mining, that RPC returns >1M lines of JSON, which (combined with
 * cJSON's O(N^2) array indexing and per-address validateaddress subprocess
 * calls) takes minutes to chew through and parks the caller — including the
 * libwebsockets service thread that pushes table_info to the GUI.
 *
 * We now resolve the address from the running node's own Verus identity:
 *   1. Player nodes have player_config.verus_pid populated from <player>.ini.
 *   2. Cashier / dealer nodes have verus_config.cashier_id / .dealer_id
 *      populated from keys.ini.
 *
 * For whichever identity is configured we issue a single
 *   verus -chain=VRSCTEST getidentity <id>
 * and extract identity.primaryaddresses[0]. The result is cached for the
 * process lifetime since the bound address does not change at runtime.
 *
 * The legacy listaddressgroupings/getnewaddress path is kept as a final
 * fallback so the function never silently returns NULL if no identity is
 * configured (e.g. unit tests / one-off CLI subcommands).
 */
char *chips_get_wallet_address()
{
	static char cached_addr[64] = { 0 };
	if (cached_addr[0] != '\0')
		return cached_addr;

	/* Build a fully-qualified Verus ID. player_config.verus_pid stores the
	 * short form ("p1"), which getidentity rejects; verus_config.cashier_id
	 * / .dealer_id are already FQNs (loaded from keys.ini). For the player
	 * case we append the poker parent FQN ("sg777z.VRSCTEST@") the same way
	 * vdxf.c::get_cmm_from_height does. */
	char fqn[256] = { 0 };
	if (player_config.verus_pid[0] != '\0') {
		snprintf(fqn, sizeof(fqn), "%s.%s", player_config.verus_pid, bet_get_poker_id_fqn());
	} else if (verus_config.initialized && verus_config.cashier_id[0] != '\0') {
		strncpy(fqn, verus_config.cashier_id, sizeof(fqn) - 1);
	} else if (verus_config.initialized && verus_config.dealers_id[0] != '\0') {
		strncpy(fqn, verus_config.dealers_id, sizeof(fqn) - 1);
	}

	if (fqn[0] != '\0') {
		int argc = 3;
		char **argv = NULL;
		cJSON *id_resp = NULL;

		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, blockchain_cli, "getidentity", fqn);
		make_command(argc, argv, &id_resp);
		bet_dealloc_args(argc, &argv);

		if (id_resp != NULL) {
			cJSON *identity = cJSON_GetObjectItem(id_resp, "identity");
			if (identity != NULL) {
				cJSON *primary = cJSON_GetObjectItem(identity, "primaryaddresses");
				if (primary != NULL && cJSON_GetArraySize(primary) > 0) {
					cJSON *first = cJSON_GetArrayItem(primary, 0);
					if (first != NULL && first->type == cJSON_String && first->valuestring != NULL) {
						strncpy(cached_addr, first->valuestring, sizeof(cached_addr) - 1);
						cached_addr[sizeof(cached_addr) - 1] = '\0';
					}
				}
			}
			cJSON_Delete(id_resp);
		}

		if (cached_addr[0] != '\0') {
			dlg_info("chips_get_wallet_address: resolved %s -> %s (cached)", fqn, cached_addr);
			return cached_addr;
		}
		dlg_warn("chips_get_wallet_address: getidentity(%s) yielded no primaryaddresses, falling back",
			 fqn);
	}

	/* Legacy fallback - slow, but preserves prior behaviour when no
	 * identity is available. Result is NOT cached so a future call can
	 * retry the identity path once configs are loaded. */
	{
		int argc = 2;
		char **argv = NULL;
		cJSON *addresses = NULL;

		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, blockchain_cli, "listaddressgroupings");
		make_command(argc, argv, &addresses);

		for (int32_t i = 0; i < cJSON_GetArraySize(addresses); i++) {
			cJSON *temp = cJSON_GetArrayItem(addresses, i);
			for (int32_t j = 0; j < cJSON_GetArraySize(temp); j++) {
				cJSON *temp1 = cJSON_GetArrayItem(temp, j);

				if (chips_validate_address(
					    unstringify(cJSON_Print(cJSON_GetArrayItem(temp1, 0))))) {
					char *addr = unstringify(cJSON_Print(cJSON_GetArrayItem(temp1, 0)));
					cJSON_Delete(addresses);
					bet_dealloc_args(argc, &argv);
					return addr;
				}
			}
		}
		if (addresses != NULL)
			cJSON_Delete(addresses);
		bet_dealloc_args(argc, &argv);
	}
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
		/* Compare doubles correctly: fabs(a-b) < epsilon */
		if (fabs(value - jdouble(temp, "value")) < epsilon) {
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
			/* Skip JSON array brackets */
			if ((ch != '[') && (ch != ']')) {
				if (ch == '{') {
					if (len >= (int32_t)sizeof(buf) - 2) {
						dlg_error("Unspent JSON object too large; truncating parse");
						memset(buf, 0x00, sizeof(buf));
						len = 0;
					}
					buf[len++] = ch;
				} else {
					if (len > 0) {
						if (ch == '}') {
							if (len >= (int32_t)sizeof(buf) - 2) {
								dlg_error("Unspent JSON object too large; truncating parse");
								memset(buf, 0x00, sizeof(buf));
								len = 0;
								continue;
							}
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
							if (len >= (int32_t)sizeof(buf) - 2) {
								dlg_error("Unspent JSON object too large; truncating parse");
								memset(buf, 0x00, sizeof(buf));
								len = 0;
								continue;
							}
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
			/* Skip JSON array brackets */
			if ((ch != '[') && (ch != ']')) {
				if (ch == '{') {
					if (len >= (int32_t)sizeof(buf) - 2) {
						dlg_error("Unspent JSON object too large; truncating parse");
						memset(buf, 0x00, sizeof(buf));
						len = 0;
					}
					buf[len++] = ch;
				} else {
					if (len > 0) {
						if (ch == '}') {
							if (len >= (int32_t)sizeof(buf) - 2) {
								dlg_error("Unspent JSON object too large; truncating parse");
								memset(buf, 0x00, sizeof(buf));
								len = 0;
								continue;
							}
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
							if (len >= (int32_t)sizeof(buf) - 2) {
								dlg_error("Unspent JSON object too large; truncating parse");
								memset(buf, 0x00, sizeof(buf));
								len = 0;
								continue;
							}
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
	if (!fp) {
		dlg_error("Failed to open %s", file_name);
		return 0;
	}
	while ((ch = fgetc(fp)) != EOF) {
		/* Skip JSON array brackets */
		if ((ch != '[') && (ch != ']')) {
			if (ch == '{') {
				if (len >= (int32_t)sizeof(buf) - 2) {
					dlg_error("Unspent JSON object too large; truncating parse");
					memset(buf, 0x00, sizeof(buf));
					len = 0;
				}
				buf[len++] = ch;
			} else {
				if (len > 0) {
					if (ch == '}') {
						if (len >= (int32_t)sizeof(buf) - 2) {
							dlg_error("Unspent JSON object too large; truncating parse");
							memset(buf, 0x00, sizeof(buf));
							len = 0;
							continue;
						}
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
						if (len >= (int32_t)sizeof(buf) - 2) {
							dlg_error("Unspent JSON object too large; truncating parse");
							memset(buf, 0x00, sizeof(buf));
							len = 0;
							continue;
						}
						buf[len++] = ch;
					}
				}
			}
		}
	}
	fclose(fp);
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
	if (fp)
		pclose(fp);

	return ret;
}

// Lightning Network code removed - process_ln_data() no longer used

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
	// dlg_info("command :: %s\n", command);
	// Lightning Network support removed - lightning-cli commands no longer supported
	if (strcmp(argv[0], "lightning-cli") == 0) {
		dlg_warn("Lightning Network support removed - lightning-cli command ignored: %s", command);
	}

	//dlg_info("\nchips-pbaas command :: %s\n", command);
	// Redirect stderr to stdout to capture error messages (2>&1)
	char *command_with_stderr = malloc(strlen(command) + 10);
	if (!command_with_stderr) {
		retval = ERR_MEMORY_ALLOC;
		free(command);
		return retval;
	}
	snprintf(command_with_stderr, strlen(command) + 10, "%s 2>&1", command);
	
	fp = popen(command_with_stderr, "r");
	if (fp == NULL) {
		dlg_error("%s::%d::Fail to open the pipe while running the command::%s\n", __FUNCTION__, __LINE__,
			  command);
		if (strcmp(argv[0], blockchain_cli) == 0)
			retval = ERR_CHIPS_COMMAND;
		// Lightning Network support removed
		if (strcmp(argv[0], "lightning-cli") == 0) {
			dlg_warn("Lightning Network support removed - command not executed");
			retval = ERR_LN_COMMAND;
		}
		free(command_with_stderr);
		free(command);
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
	
	// Check for error messages in output (Verus exceptions, etc.)
	// This catches "UNKNOWN EXCEPTION" and other error patterns
	if (strstr(data, "UNKNOWN EXCEPTION") != NULL) {
		dlg_error("Verus RPC command failed with exception: %s", data);
		if (strcmp(argv[0], blockchain_cli) == 0) {
			retval = ERR_CHIPS_COMMAND;
			// For getidentity, return specific error
			if (strcmp(argv[1], "getidentity") == 0) {
				retval = ERR_ID_NOT_FOUND;
			}
		} else if (strcmp(argv[0], "lightning-cli") == 0) {
			// Lightning Network support removed
			dlg_warn("Lightning Network support removed - command not executed");
			retval = ERR_LN_COMMAND;
		} else {
			retval = ERR_COMMAND_FAILED;
		}
		// Don't try to parse error output as JSON
		goto end;
	}
	
	// Close pipe and check exit status
	int exit_status = pclose(fp);
	fp = NULL; // Set to NULL so we don't close it again
	
	// Check exit status - if command failed and we got no data, it's an error
	// Note: pclose returns -1 on error, or the wait status on success
	// WEXITSTATUS would extract exit code, but we check != 0 for simplicity
	if (exit_status == -1) {
		dlg_error("Error closing pipe for command: %s", command);
		if (strcmp(argv[0], blockchain_cli) == 0) {
			retval = ERR_CHIPS_COMMAND;
		} else if (strcmp(argv[0], "lightning-cli") == 0) {
			// Lightning Network support removed
			dlg_warn("Lightning Network support removed - command not executed");
			retval = ERR_LN_COMMAND;
		} else {
			retval = ERR_COMMAND_FAILED;
		}
		goto end;
	}
	
	// If exit status indicates failure and we got no data, it's an error
	if (exit_status != 0 && strlen(data) == 0) {
		dlg_error("Command failed with exit status %d: %s", exit_status, command);
		if (strcmp(argv[0], blockchain_cli) == 0) {
			retval = ERR_CHIPS_COMMAND;
		} else if (strcmp(argv[0], "lightning-cli") == 0) {
			// Lightning Network support removed
			dlg_warn("Lightning Network support removed - command not executed");
			retval = ERR_LN_COMMAND;
		} else {
			retval = ERR_COMMAND_FAILED;
		}
		goto end;
	}
	
	if (strcmp(argv[0], "git") == 0) {
		*argjson = cJSON_CreateString((const char *)data);
	} else if (strcmp(argv[0], "lightning-cli") == 0) {
		// Lightning Network support removed
		dlg_warn("Lightning Network support removed - command not executed");
		retval = ERR_LN_COMMAND;
	} else if (strcmp(argv[0], blockchain_cli) == 0) {
		if (strlen(data) == 0) {
			if (strcmp(argv[1], "importaddress") == 0) {
				// Do nothing
			} else if (strcmp(argv[1], "listunspent") == 0) {
				chips_read_valid_unspent(argv[3], argjson);
			} else if (strcmp(argv[1], "updateidentity") == 0) {
				dlg_error("updateidentity returned empty output, command: %s", command);
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
					/* verbose=1 callers (e.g. get_tx_block_height) need a
					 * parsed object so jint(result,"height") works. With
					 * verbose=0 the daemon returns a hex string, which is
					 * not valid JSON and we just wrap it. */
					cJSON *parsed = cJSON_Parse(data);
					if (parsed) {
						cJSON_Delete(*argjson);
						*argjson = parsed;
					} else {
						*argjson = cJSON_CreateString(data);
					}
				}
			} else if (strcmp(argv[1], "getrawmempool") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				*argjson = cJSON_Parse(data);
			} else if (strcmp(argv[1], "updateidentity") == 0) {
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				if (strstr(data, "error") != NULL) {
					dlg_error("updateidentity failed, command: %s, daemon output: %s", command, data);
					retval = ERR_UPDATEIDENTITY;
					jaddstr(*argjson, "error_msg", data);
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
			} else if (strcmp(argv[1], "getbalance") == 0) {
				// getbalance returns a plain number string, not JSON
				if (data[strlen(data) - 1] == '\n')
					data[strlen(data) - 1] = '\0';
				*argjson = cJSON_CreateString(data);
			} else {
				*argjson = cJSON_Parse(data);
				if (*argjson == NULL) {
					// If parsing fails, create an error object
					*argjson = cJSON_CreateObject();
					cJSON_AddStringToObject(*argjson, "error", "Failed to parse JSON response");
					retval = ERR_JSON_PARSING;
				} else {
					cJSON_AddNumberToObject(*argjson, "code", 0);
				}
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
	if (command_with_stderr)
		free(command_with_stderr);
	if (fp)
		pclose(fp);

	return retval;
}

// Lightning Network functions removed - no longer used:
// ln_get_new_address, ln_block_height, ln_listfunds, ln_get_uri,
// ln_connect_uri, ln_fund_channel, ln_pay, ln_connect,
// ln_check_if_address_isof_type, ln_check_peer_and_connect,
// ln_get_channel_status, ln_establish_channel

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

bool check_if_tx_exists(const char *tx_id)
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

/**
 * Get the identity address from a Verus ID
 * @param verus_id Full Verus ID (e.g., "cashier.sg777z.chips@")
 * @return Identity address string or NULL on failure
 */
char *get_identity_address(const char *verus_id)
{
	int32_t retval = OK;
	cJSON *params = NULL, *result = NULL;
	char *identity_address = NULL;

	if (!verus_id) {
		return NULL;
	}

	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, cJSON_CreateString(verus_id));
	cJSON_AddItemToArray(params, cJSON_CreateNumber(-1));

	retval = chips_rpc("getidentity", params, &result);
	cJSON_Delete(params);

	if (retval != OK || !result) {
		return NULL;
	}

	cJSON *identity = cJSON_GetObjectItem(result, "identity");
	if (identity) {
		const char *addr = jstr(identity, "identityaddress");
		if (addr) {
			identity_address = strdup(addr);
		}
	}

	cJSON_Delete(result);
	return identity_address;
}

/**
 * Get all transaction IDs for an address
 * @param address Identity address to query
 * @return cJSON array of txids or NULL on failure
 */
cJSON *get_address_txids(const char *address)
{
	return get_address_txids_range(address, 0, 0);
}

/**
 * Get transaction IDs for an address within a block range
 * @param address Identity address to query
 * @param start_block Starting block height (0 for no limit)
 * @param end_block Ending block height (0 for current height)
 * @return cJSON array of txids or NULL on failure
 */
cJSON *get_address_txids_range(const char *address, int32_t start_block, int32_t end_block)
{
	int32_t retval = OK;
	cJSON *params = NULL, *result = NULL, *addresses = NULL, *addr_obj = NULL;

	if (!address) {
		return NULL;
	}

	// Build params: [{"addresses": ["<address>"], "start": N, "end": M}]
	addresses = cJSON_CreateArray();
	cJSON_AddItemToArray(addresses, cJSON_CreateString(address));
	
	addr_obj = cJSON_CreateObject();
	cJSON_AddItemToObject(addr_obj, "addresses", addresses);
	
	if (start_block > 0) {
		cJSON_AddNumberToObject(addr_obj, "start", start_block);
	}
	if (end_block > 0) {
		cJSON_AddNumberToObject(addr_obj, "end", end_block);
	}
	
	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, addr_obj);

	retval = chips_rpc("getaddresstxids", params, &result);
	cJSON_Delete(params);

	if (retval != OK) {
		return NULL;
	}

	return result;
}

/**
 * Decode transaction and extract data field
 * @param txid Transaction ID
 * @return cJSON object with decoded data or NULL
 */
cJSON *decode_tx_data(const char *txid)
{
	int32_t retval = OK;
	cJSON *params = NULL, *result = NULL;

	if (!txid) {
		return NULL;
	}

	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, cJSON_CreateString(txid));
	cJSON_AddItemToArray(params, cJSON_CreateNumber(1)); // verbose = true

	retval = chips_rpc("getrawtransaction", params, &result);
	cJSON_Delete(params);

	if (retval != OK || !result) {
		return NULL;
	}

	// Look for vout with data in OP_RETURN or identity output
	cJSON *vout = cJSON_GetObjectItem(result, "vout");
	if (vout) {
		for (int i = 0; i < cJSON_GetArraySize(vout); i++) {
			cJSON *output = cJSON_GetArrayItem(vout, i);
			cJSON *scriptPubKey = cJSON_GetObjectItem(output, "scriptPubKey");
			if (scriptPubKey) {
				// Check for identity output with data
				cJSON *identityprimary = cJSON_GetObjectItem(scriptPubKey, "identityprimary");
				if (identityprimary) {
					// This is an identity output - check for data
					const char *hex = jstr(scriptPubKey, "hex");
					if (hex) {
						// The data is embedded in the transaction
						// For now, return the whole result for further processing
						return result;
					}
				}
			}
		}
	}

	return result;
}

/**
 * Get sender addresses from a transaction's inputs
 * @param txid Transaction ID
 * @return cJSON array of sender addresses or NULL
 */
cJSON *get_tx_sender_addresses(const char *txid)
{
	int32_t retval = OK;
	cJSON *params = NULL, *result = NULL, *addresses = NULL;

	if (!txid) {
		return NULL;
	}

	params = cJSON_CreateArray();
	cJSON_AddItemToArray(params, cJSON_CreateString(txid));
	cJSON_AddItemToArray(params, cJSON_CreateNumber(1)); // verbose = true

	retval = chips_rpc("getrawtransaction", params, &result);
	cJSON_Delete(params);

	if (retval != OK || !result) {
		return NULL;
	}

	addresses = cJSON_CreateArray();

	// Get addresses from vout of previous transactions (via vin)
	cJSON *vin = cJSON_GetObjectItem(result, "vin");
	if (vin) {
		for (int i = 0; i < cJSON_GetArraySize(vin); i++) {
			cJSON *input = cJSON_GetArrayItem(vin, i);
			const char *prev_txid = jstr(input, "txid");
			int prev_vout = jint(input, "vout");
			
			if (prev_txid) {
				// Get the previous transaction to find the address
				cJSON *prev_params = cJSON_CreateArray();
				cJSON_AddItemToArray(prev_params, cJSON_CreateString(prev_txid));
				cJSON_AddItemToArray(prev_params, cJSON_CreateNumber(1));
				
				cJSON *prev_result = NULL;
				retval = chips_rpc("getrawtransaction", prev_params, &prev_result);
				cJSON_Delete(prev_params);
				
				if (retval == OK && prev_result) {
					cJSON *prev_vout_arr = cJSON_GetObjectItem(prev_result, "vout");
					if (prev_vout_arr && prev_vout < cJSON_GetArraySize(prev_vout_arr)) {
						cJSON *output = cJSON_GetArrayItem(prev_vout_arr, prev_vout);
						cJSON *scriptPubKey = cJSON_GetObjectItem(output, "scriptPubKey");
						if (scriptPubKey) {
							cJSON *addr_arr = cJSON_GetObjectItem(scriptPubKey, "addresses");
							if (addr_arr && cJSON_GetArraySize(addr_arr) > 0) {
								const char *addr = jstri(addr_arr, 0);
								if (addr) {
									cJSON_AddItemToArray(addresses, cJSON_CreateString(addr));
								}
							}
						}
					}
					cJSON_Delete(prev_result);
				}
			}
		}
	}

	cJSON_Delete(result);
	return addresses;
}
