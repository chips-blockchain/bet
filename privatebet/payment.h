bits256 BET_clientrhash();
void BET_chip_recv(char *label,struct privatebet_info *bet);
void BET_hostrhash_update(bits256 rhash);
bits256 BET_hosthash_extract(cJSON *argjson,int32_t chipsize);
struct privatebet_peerln *BET_invoice_complete(char *nextlabel,cJSON *item,struct privatebet_info *bet);
int32_t BET_clientpay(uint64_t chipsize);
void BET_channels_parse();
void BET_p2p_paymentloop(void * _ptr);

