#include "bet.h"
int32_t BET_iswatchonly(char *address);
void BET_spentmultisigaddress(char *address, double amount);
void BET_importaddress(char *address);
char *BET_getnewaddress();
int BET_validateaddress(char *address);
void BET_listaddressgroupings();
cJSON *BET_transferfunds(double amount, char *address);
cJSON *BET_sendrawtransaction(cJSON *signedTransaction);
cJSON *BET_signrawtransactionwithwallet(char *rawtransaction);
int32_t BET_publishmultisigtransaction(char *tx);
cJSON *BET_createrawmultisigtransaction(double amount, char *toaddress,
					char *fromaddress);
cJSON *BET_createrawtransaction(double amount, char *address);
void BET_listunspent();
int32_t BET_get_chips_blockheight();
int32_t BET_get_ln_blockheight();
void BET_check_sync();
double BET_getbalance();
int32_t BET_lock_transaction(int32_t fundAmount);
