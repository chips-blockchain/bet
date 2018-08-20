int32_t BET_pubkeyfind(struct privatebet_info *bet,bits256 pubkey,char *peerid);
int32_t BET_pubkeyadd(struct privatebet_info *bet,bits256 pubkey,char *peerid);
void BET_betinfo_set(struct privatebet_info *bet,char *game,int32_t range,int32_t numrounds,int32_t maxplayers);
cJSON *BET_betinfo_json(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_betvars_parse(struct privatebet_info *bet,struct privatebet_vars *vars,cJSON *argjson);
int32_t BET_betinfo_parse(struct privatebet_info *bet,struct privatebet_vars *vars,cJSON *msgjson);
void BET_tablestatus_send(struct privatebet_info *bet,struct privatebet_vars *vars);

