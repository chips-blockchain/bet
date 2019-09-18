bits256 BET_clientrhash();
void BET_chip_recv(char *label,struct privatebet_info *bet);
void BET_hostrhash_update(bits256 rhash);
bits256 BET_hosthash_extract(cJSON *argjson,int32_t chipsize);
struct privatebet_peerln *BET_invoice_complete(char *nextlabel,cJSON *item,struct privatebet_info *bet);
int32_t BET_clientpay(uint64_t chipsize);
void BET_channels_parse();
/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/
int32_t BET_DCV_pay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_create_invoice_request(struct privatebet_info *bet,int32_t playerid, int32_t amount);
int32_t BET_DCV_invoice_pay(struct privatebet_info *bet,struct privatebet_vars *vars,int playerid,int amount);
void BET_DCV_paymentloop(void * _ptr);

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
	int32_t BET_player_create_invoice(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,char* deckid);
int32_t BET_player_create_invoice_request(cJSON *argjson,struct privatebet_info *bet,int32_t amount);
int32_t BET_player_invoice_pay(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int amount);
int32_t BET_player_paymentloop(void * _ptr);

int32_t BET_player_create_betting_invoice_request(cJSON *argjson,cJSON *actionResponse,struct privatebet_info *bet,int32_t amount);
int32_t BET_rest_DCV_create_invoice_request(struct lws *wsi,int32_t amount,int32_t playerID);


