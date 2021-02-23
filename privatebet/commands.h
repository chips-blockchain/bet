#include "bet.h"

#define arg_size 8192
#define tx_data_size 4196

int32_t bet_alloc_args(int argc, char ***argv);
void bet_dealloc_args(int argc, char ***argv);
int32_t chips_iswatchonly(char *address);
void chips_spend_multi_sig_address(char *address, double amount);
cJSON *chips_import_address(char *address);
char *chips_get_new_address();
int chips_validate_address(char *address);
cJSON *chips_list_address_groupings();
cJSON *chips_get_block_hash_from_height(int64_t block_height);
cJSON *chips_get_block_from_block_hash(char *block_hash_info);
cJSON *chips_get_block_from_block_height(int64_t block_height);
int32_t chips_if_tx_vin_of_tx(cJSON *txid, char *vin_tx_id);
cJSON *chips_find_parent_tx(int64_t block_height, char *vin_tx_id);
cJSON *chips_get_vin_from_tx(char *txid);
cJSON *validate_given_tx(int64_t block_height, char *txid);
cJSON *chips_transfer_funds(double amount, char *address);
cJSON *chips_send_raw_tx(cJSON *signed_tx);
cJSON *chips_sign_raw_tx_with_wallet(char *raw_tx);
int32_t chips_publish_multisig_tx(char *tx);
cJSON *chips_create_raw_tx(double amount, char *address);
int32_t chips_get_block_count();
void check_ln_chips_sync();
cJSON *bet_get_chips_ln_bal_info();
double chips_get_balance();
cJSON *bet_get_chips_ln_addr_info();
cJSON *chips_add_multisig_address();
cJSON *chips_add_multisig_address_from_list(int32_t threshold_value, cJSON *addr_list);
int32_t chips_check_if_tx_unspent(char *input_tx);
char *chips_get_block_hash_from_txid(char *txid);
int32_t chips_get_block_height_from_block_hash(char *block_hash);
cJSON *chips_create_tx_from_tx_list(char *to_addr, int32_t no_of_txs, char tx_ids[][100]);
cJSON *chips_sign_msig_tx_of_table_id(char *ip, cJSON *raw_tx, char *table_id);
cJSON *chips_sign_msig_tx(char *ip, cJSON *raw_tx);
cJSON *chips_spend_msig_txs(char *to_addr, int no_of_txs, char tx_ids[][100]);
cJSON *chips_get_raw_tx(char *tx);
cJSON *chips_decode_raw_tx(cJSON *raw_tx);
int32_t chips_validate_tx(char *tx);
int32_t chips_extract_data(char *tx, char **rand_str);
cJSON *chips_create_raw_tx_with_data(double amount, char *address, char *data);
cJSON *chips_transfer_funds_with_data(double amount, char *address, char *data);
cJSON *chips_deposit_to_ln_wallet(double channel_chips);
double chips_get_balance_on_address_from_tx(char *address, char *tx);
char *chips_get_wallet_address();
cJSON *chips_create_payout_tx(cJSON *payout_addr, int32_t no_of_txs, char tx_ids[][100], char *data);

int32_t chips_check_tx_exists_in_unspent(char *tx_id);
int32_t chips_check_tx_exists(char *tx_id);
int32_t run_command(int argc, char **argv);
int32_t make_command(int argc, char **argv, cJSON **argjson);

char *ln_get_new_address();
int32_t ln_dev_block_height();
int32_t ln_listfunds();
int32_t ln_get_uri(char **uri);
int32_t ln_connect_uri(char *uri);
cJSON *ln_fund_channel(char *channel_id, int32_t channel_fund_satoshi);
int32_t ln_pay(char *bolt11);
cJSON *ln_connect(char *id);
void ln_check_peer_and_connect(char *id);
int32_t ln_get_channel_status(char *id);
int32_t ln_wait_for_tx_block_height(int32_t block_height);
int32_t ln_establish_channel(char *uri);
