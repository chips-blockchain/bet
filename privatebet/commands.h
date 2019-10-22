
int BET_validateaddress(char* address);
void BET_listaddressgroupings();
cJSON* BET_transferfunds(double amount);
cJSON* BET_sendrawtransaction(cJSON *signedTransaction);
cJSON* BET_signrawtransactionwithwallet(char *rawtransaction);
cJSON* BET_createrawtransaction(double amount);
void BET_listunspent();
int32_t BET_get_chips_blockheight();
int32_t BET_get_ln_blockheight();
void BET_check_sync();
double BET_getbalance();
int32_t BET_lock_transaction(int32_t fundAmount);
