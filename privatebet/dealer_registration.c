#include "dealer_registration.h"
#include "vdxf.h"
#include "err.h"
#include "misc.h"
#include "commands.h"

double get_dealer_registration_fee(void) {
    return DEALER_REGISTRATION_FEE;
}

double get_dealer_deregistration_commission(void) {
    return DEALER_DEREGISTRATION_COMMISSION;
}

// Internal function to add dealer to dealers list
static int32_t add_dealer_to_list(char *dealer_id) {
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

int32_t register_dealer(char *dealer_id) {
    int32_t retval = OK;
    double balance = 0;
    cJSON *tx = NULL, *tx_data = NULL;

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
        dlg_error("Insufficient balance. Required: %f CHIPS, Available: %f CHIPS", 
                 DEALER_REGISTRATION_FEE, balance);
        return ERR_CHIPS_INSUFFICIENT_FUNDS;
    }

    // Create payin_tx with dealer_id in data
    tx_data = cJSON_CreateObject();
    cJSON_AddStringToObject(tx_data, "dealer_id", dealer_id);
    cJSON_AddStringToObject(tx_data, "type", "dealer_registration");

    // Transfer registration fee with dealer_id in tx data
    tx = chips_transfer_funds_with_data(DEALER_REGISTRATION_FEE, DEALERS_ID, tx_data);
    if (!tx) {
        dlg_error("Failed to transfer registration fee");
        return ERR_TRANSFER_FAILED;
    }

    dlg_info("Successfully initiated dealer registration for %s", dealer_id);
    return OK;
}

int32_t deregister_dealer(char *dealer_id) {
    int32_t retval = OK;
    double refund_amount = 0;
    cJSON *tx = NULL;
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

    // Transfer refund amount back to dealer
    tx = chips_transfer_funds(refund_amount, dealer_id);
    if (!tx) {
        dlg_error("Failed to transfer refund amount");
        return ERR_TRANSFER_FAILED;
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
int32_t process_dealer_registration_tx(cJSON *tx_data) {
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