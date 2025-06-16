#include "dealer_registration.h"
#include "vdxf.h"
#include "err.h"
#include "misc.h"
#include "commands.h"

double get_dealer_registration_fee(void)
{
	return DEALER_REGISTRATION_FEE;
}

double get_dealer_deregistration_commission(void)
{
	return DEALER_DEREGISTRATION_COMMISSION;
}

// Internal function to add dealer to dealers list
static int32_t add_dealer_to_list(char *dealer_id)
{
	int32_t retval = OK;
	cJSON *dealers_info = NULL, *dealers = NULL, *out = NULL;

	if (!dealer_id) {
		return ERR_NULL_ID;
	}
	if (!is_id_exists(dealer_id, 0)) {
		return ERR_ID_NOT_FOUND;
	}

	if (!id_cansignfor(DEALERS_ID, 0, &retval)) {
		return retval;
	}

	dealers_info = cJSON_CreateObject();
	dealers = list_dealers();
	if (!dealers) {
		dealers = cJSON_CreateArray();
	}
	jaddistr(dealers, dealer_id);
	cJSON_AddItemToObject(dealers_info, "dealers", dealers);
	out = update_cmm_from_id_key_data_cJSON(DEALERS_ID, DEALERS_KEY, dealers_info, false);

	if (!out) {
		return ERR_UPDATEIDENTITY;
	}
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

bool is_dealer_registered(char *dealer_id)
{
	if (!dealer_id || !is_id_exists(dealer_id, false)) {
		return false;
	}

	cJSON *dealers = list_dealers();
	if (!dealers) {
		return false;
	}

	bool is_registered = false;
	for (int32_t i = 0; i < cJSON_GetArraySize(dealers); i++) {
		if (0 == strcmp(dealer_id, jstri(dealers, i))) {
			is_registered = true;
			break;
		}
	}

	cJSON_Delete(dealers);
	return is_registered;
}

int32_t register_dealer(char *dealer_id)
{
	int32_t retval = OK;
	double balance = 0;
	cJSON *tx_data = NULL, *op_id = NULL;

	if (!dealer_id) {
		return ERR_NULL_ID;
	}

	if (!is_id_exists(dealer_id, 0)) {
		return ERR_ID_NOT_FOUND;
	}

	// Check if dealer is already registered
	if (is_dealer_registered(dealer_id)) {
		dlg_info("Dealer %s is already registered", dealer_id);
		return OK;
	}

	// Check if user has enough balance for registration fee
	balance = chips_get_balance();
	if (balance < DEALER_REGISTRATION_FEE) {
		dlg_error("Insufficient balance. Required: %f CHIPS, Available: %f CHIPS", DEALER_REGISTRATION_FEE,
			  balance);
		return ERR_CHIPS_INSUFFICIENT_FUNDS;
	}

	// Create payin_tx with dealer_id in data
	// The JSON will look like:
	// {
	//     "dealer_id": "<dealer_id>",
	//     "type": "dealer_registration",
	//     "amount": <DEALER_REGISTRATION_FEE>,
	//     "destination": "dealers"
	// }
	tx_data = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_data, "dealer_id", dealer_id);
	cJSON_AddStringToObject(tx_data, "type", "dealer_registration");
	cJSON_AddNumberToObject(tx_data, "amount", DEALER_REGISTRATION_FEE);
	cJSON_AddStringToObject(tx_data, "destination", DEALERS_ID);

	// Transfer registration fee with dealer_id in tx data
	op_id = verus_sendcurrency_data(DEALERS_ID_FQN, DEALER_REGISTRATION_FEE, tx_data);
	if (!op_id) {
		dlg_error("Failed to transfer registration fee");
		return ERR_SENDCURRENCY;
	}

	// Store transaction data and ID in dealer's ID for dispute resolution
	// registration_info format:
	// {
	//     "tx_data": {
	//         "dealer_id": "<dealer_id>",
	//         "type": "dealer_registration",
	//         "amount": <DEALER_REGISTRATION_FEE>,
	//         "destination": "dealers"
	//     },
	//     "tx_id": "<op_id>"
	// }
	cJSON *registration_info = cJSON_CreateObject();
	cJSON_AddItemToObject(registration_info, "tx_data", tx_data);
	cJSON_AddItemToObject(registration_info, "tx_id", op_id);

	cJSON *out = update_cmm_from_id_key_data_cJSON(dealer_id, "registration_info", registration_info, false);
	if (!out) {
		dlg_error("Failed to store registration info for dealer %s", dealer_id);
		return ERR_UPDATEIDENTITY;
	}
	dlg_info("Stored registration info for dealer %s", dealer_id);

	dlg_info("Successfully initiated dealer registration for %s", dealer_id);
	return OK;
}

int32_t deregister_dealer(char *dealer_id)
{
	int32_t retval = OK;
	double refund_amount = 0;
	cJSON *tx_data = NULL, *op_id = NULL;
	cJSON *dealers = NULL;

	if (!dealer_id) {
		return ERR_NULL_ID;
	}

	if (!is_id_exists(dealer_id, 0)) {
		return ERR_ID_NOT_FOUND;
	}

	// Check if dealer is registered
	if (!is_dealer_registered(dealer_id)) {
		dlg_error("Dealer %s is not registered", dealer_id);
		return ERR_DEALER_UNREGISTERED;
	}

	// Calculate refund amount (registration fee - commission)
	refund_amount = DEALER_REGISTRATION_FEE * (1.0 - DEALER_DEREGISTRATION_COMMISSION);

	// Create tx data for refund
	tx_data = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_data, "dealer_id", dealer_id);
	cJSON_AddStringToObject(tx_data, "type", "dealer_deregistration");
	cJSON_AddNumberToObject(tx_data, "amount", refund_amount);
	cJSON_AddStringToObject(tx_data, "destination", dealer_id);

	// Transfer refund amount back to dealer
	op_id = verus_sendcurrency_data(dealer_id, refund_amount, tx_data);
	if (!op_id) {
		dlg_error("Failed to transfer refund amount");
		return ERR_SENDCURRENCY;
	}

	// Remove dealer from dealers list
	dealers = list_dealers();
	if (dealers) {
		cJSON *new_dealers = cJSON_CreateArray();
		for (int32_t i = 0; i < cJSON_GetArraySize(dealers); i++) {
			if (strcmp(dealer_id, jstri(dealers, i)) != 0) {
				jaddistr(new_dealers, jstri(dealers, i));
			}
		}

		cJSON *dealers_info = cJSON_CreateObject();
		cJSON_AddItemToObject(dealers_info, "dealers", new_dealers);
		cJSON *out = update_cmm_from_id_key_data_cJSON(DEALERS_ID, DEALERS_KEY, dealers_info, false);

		if (!out) {
			dlg_error("Failed to update dealers list");
			return ERR_UPDATEIDENTITY;
		}
	}

	dlg_info("Successfully deregistered dealer %s", dealer_id);
	return OK;
}

// Function to be called by the daemon when processing new blocks
int32_t process_dealer_registration_tx(cJSON *tx_data)
{
	char *dealer_id = NULL;
	char *type = NULL;
	int32_t retval = OK;

	if (!tx_data) {
		return ERR_NULL_KEY;
	}

	type = jstr(tx_data, "type");
	if (!type || strcmp(type, "dealer_registration") != 0) {
		return OK; // Not a dealer registration tx
	}

	dealer_id = jstr(tx_data, "dealer_id");
	if (!dealer_id) {
		return ERR_NULL_ID;
	}

	// Add dealer to the list
	retval = add_dealer_to_list(dealer_id);
	if (retval == OK) {
		dlg_info("Successfully processed dealer registration for %s", dealer_id);
	}

	return retval;
}

// Function to raise a dealer registration dispute
int32_t raise_dealer_registration_dispute(char *dealer_id, char *dispute_action)
{
	int32_t retval = OK;
	cJSON *registration_info = NULL, *tx_data = NULL, *op_id = NULL;

	if (!dealer_id || !dispute_action) {
		return ERR_NULL_ID;
	}

	if (!is_id_exists(dealer_id, 0)) {
		return ERR_ID_NOT_FOUND;
	}

	// If dispute_action is not add_dealer, default to refund
	if (strcmp(dispute_action, "add_dealer") != 0) {
		dispute_action = "refund";
	}

	// Get stored registration info from dealer's ID
	registration_info = get_cJSON_from_id_key(dealer_id, "registration_info", 0);
	if (!registration_info) {
		dlg_error("No registration info found for dealer %s", dealer_id);
		return ERR_ID_NOT_FOUND;
	}

	// Create dispute transaction data
	tx_data = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_data, "type", "dealer_registration_dispute");
	cJSON_AddStringToObject(tx_data, "dealer_id", dealer_id);
	cJSON_AddStringToObject(tx_data, "dispute_action", dispute_action);
	cJSON_AddItemToObject(tx_data, "tx_id", cJSON_DetachItemFromObject(registration_info, "tx_id"));
	cJSON_AddItemToObject(tx_data, "tx_data", cJSON_DetachItemFromObject(registration_info, "tx_data"));

	// Send dispute transaction to dealers ID
	op_id = verus_sendcurrency_data(DEALERS_ID_FQN, 0, tx_data);
	if (!op_id) {
		dlg_error("Failed to send dispute transaction");
		return ERR_SENDCURRENCY;
	}

	dlg_info("Successfully raised dealer registration dispute for %s", dealer_id);
	return OK;
}

// Function to process a dealer registration dispute
static int32_t process_dealer_registration_dispute(cJSON *tx_data)
{
	char *dealer_id = NULL, *dispute_action = NULL, *tx_id = NULL;
	cJSON *original_tx_data = NULL;
	int32_t retval = OK;

	if (!tx_data) {
		return ERR_NULL_KEY;
	}

	dealer_id = jstr(tx_data, "dealer_id");
	dispute_action = jstr(tx_data, "dispute_action");
	tx_id = jstr(tx_data, "tx_id");
	original_tx_data = cJSON_GetObjectItem(tx_data, "tx_data");

	// If any required field is missing, default to refund
	if (!dealer_id || !tx_id || !original_tx_data) {
		dlg_error("Missing required fields in dispute transaction, defaulting to refund");
		dispute_action = "refund";
	}

	// If dispute_action is not add_dealer, default to refund
	if (!dispute_action || strcmp(dispute_action, "add_dealer") != 0) {
		dispute_action = "refund";
	}

	// Verify the original transaction exists and is valid
	// TODO: Implement this

	// Process based on dispute action
	if (strcmp(dispute_action, "refund") == 0) {
		double amount = jdouble(original_tx_data, "amount");

		// Create refund transaction
		cJSON *refund_data = cJSON_CreateObject();
		cJSON_AddStringToObject(refund_data, "dealer_id", dealer_id);
		cJSON_AddStringToObject(refund_data, "type", "dealer_registration_refund");
		cJSON_AddNumberToObject(refund_data, "amount", amount);
		cJSON_AddStringToObject(refund_data, "destination", dealer_id);

		// Send refund
		cJSON *refund_tx = verus_sendcurrency_data(dealer_id, amount, refund_data);
		if (!refund_tx) {
			dlg_error("Failed to process refund for dealer %s", dealer_id);
			return ERR_SENDCURRENCY;
		}
		dlg_info("Processed refund for dealer %s", dealer_id);
	} else if (strcmp(dispute_action, "add_dealer") == 0) {
		// Add dealer to dealers list
		retval = add_dealer_to_list(dealer_id);
		if (retval != OK) {
			dlg_error("Failed to add dealer %s to dealers list", dealer_id);
			return retval;
		}
		dlg_info("Added dealer %s to dealers list", dealer_id);
	}

	return OK;
}

void process_dealer_registration_block(char *blockhash)
{
	int32_t blockcount = 0, retval = OK;
	char verus_addr[1][100] = { DEALERS_ID_FQN };
	cJSON *blockjson = NULL, *tx_data = NULL;

	// Check if node has access to dealers ID
	if (!is_id_exists(DEALERS_ID_FQN, 1)) {
		dlg_error("Dealers ID ::%s doesn't exist", DEALERS_ID_FQN);
		return;
	}

	// Get block information
	blockjson = cJSON_CreateObject();
	blockjson = chips_get_block_from_block_hash(blockhash);
	if (blockjson == NULL) {
		dlg_error("Failed to get block info from blockhash");
		return;
	}

	blockcount = jint(blockjson, "height");
	if (blockcount <= 0) {
		dlg_error("Invalid block height, check if the underlying blockchain is syncing right");
		return;
	}
	dlg_info("Processing block height = %d for dealer registrations", blockcount);

	// Get all UTXOs for the dealers ID in this block
	cJSON *utxos = cJSON_CreateObject();
	utxos = getaddressutxos(verus_addr, 1);

	for (int32_t i = 0; i < cJSON_GetArraySize(utxos); i++) {
		cJSON *utxo = cJSON_GetArrayItem(utxos, i);
		if (jint(utxo, "height") == blockcount) {
			char *txid = jstr(utxo, "txid");
			dlg_info("Processing tx_id::%s", txid);

			// Extract transaction data
			tx_data = chips_extract_tx_data_in_JSON(txid);
			if (!tx_data) {
				dlg_error("Failed to extract transaction data for tx %s", txid);
				continue;
			}

			// Check transaction type
			char *type = jstr(tx_data, "type");
			if (!type) {
				dlg_info("Skipping transaction with no type");
				continue;
			}

			if (strcmp(type, "dealer_registration") == 0) {
				// Process regular dealer registration
				char *dealer_id = jstr(tx_data, "dealer_id");
				if (!dealer_id) {
					dlg_error("No dealer_id found in transaction data");
					continue;
				}

				// Check if amount matches registration fee
				double amount = jdouble(tx_data, "amount");
				if (amount != DEALER_REGISTRATION_FEE) {
					dlg_error("Invalid registration fee amount: %f (expected: %f)", amount,
						  DEALER_REGISTRATION_FEE);

					// Create refund transaction data
					cJSON *refund_data = cJSON_CreateObject();
					cJSON_AddStringToObject(refund_data, "dealer_id", dealer_id);
					cJSON_AddStringToObject(refund_data, "type", "dealer_registration_refund");
					cJSON_AddNumberToObject(refund_data, "amount", amount);
					cJSON_AddStringToObject(refund_data, "destination", dealer_id);

					// Send refund using send_currency_data
					cJSON *refund_tx = verus_sendcurrency_data(dealer_id, amount, refund_data);
					if (!refund_tx) {
						dlg_error("Failed to refund invalid registration fee to %s", dealer_id);
					} else {
						dlg_info("Refunded invalid registration fee to %s, tx: %s", dealer_id,
							 cJSON_Print(refund_tx));
					}
					cJSON_Delete(refund_data);
					continue;
				}

				// Add dealer to dealers list
				retval = add_dealer_to_list(dealer_id);
				if (retval != OK) {
					dlg_error("Failed to add dealer %s to dealers list: %s", dealer_id,
						  bet_err_str(retval));

					// Create refund transaction data
					cJSON *refund_data = cJSON_CreateObject();
					cJSON_AddStringToObject(refund_data, "dealer_id", dealer_id);
					cJSON_AddStringToObject(refund_data, "type", "dealer_registration_refund");
					cJSON_AddNumberToObject(refund_data, "amount", amount);
					cJSON_AddStringToObject(refund_data, "destination", dealer_id);

					// Send refund using send_currency_data
					cJSON *refund_tx = verus_sendcurrency_data(dealer_id, amount, refund_data);
					if (!refund_tx) {
						dlg_error("Failed to refund registration fee to %s", dealer_id);
					} else {
						dlg_info("Refunded registration fee to %s, tx: %s", dealer_id,
							 cJSON_Print(refund_tx));
					}
					cJSON_Delete(refund_data);
				} else {
					dlg_info("Successfully registered dealer %s", dealer_id);
				}
			} else if (strcmp(type, "dealer_registration_dispute") == 0) {
				// Process dealer registration dispute
				retval = process_dealer_registration_dispute(tx_data);
				if (retval != OK) {
					dlg_error("Failed to process dealer registration dispute: %s",
						  bet_err_str(retval));
				}
			}
		}
	}

	dlg_info("Finished processing dealer registrations for block %d", blockcount);
}