char *BET_gameresult(cJSON *argjson);
char *BET_oracle_command(void *ctx,char *method,cJSON *argjson);
cJSON *BET_createdeck_request(bits256 *pubkeys,int32_t numplayers,int32_t range);
char *BET_oracle_request(char *method,cJSON *reqjson);
