#include "bet.h"
int32_t chips_iswatchonly(char *address);
void chips_spend_multi_sig_address(char *address, double amount);
void chips_import_address(char *address);
char *chips_get_new_address();
int chips_validate_address(char *address);
void chips_list_address_groupings();
cJSON *chips_transfer_funds(double amount, char *address);
cJSON *chips_send_raw_tx(cJSON *signedTransaction);
cJSON *chips_sign_raw_tx_with_wallet(char *rawtransaction);
int32_t chips_publish_multisig_tx(char *tx);
cJSON *chips_create_raw_multi_sig_tx(double amount, char *toaddress,
				     char *fromaddress);
cJSON *chips_create_raw_tx(double amount, char *address);
void chips_list_unspent();
int32_t chips_get_block_count();
int32_t ln_dev_block_height();
void check_ln_chips_sync();
double chips_get_balance();
int32_t chips_lock_transaction(int32_t fundAmount);
