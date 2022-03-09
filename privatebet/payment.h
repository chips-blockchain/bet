#include "bet.h"
/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/
int32_t bet_dcv_pay(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_create_invoice_request(struct privatebet_info *bet, int32_t playerid, int32_t amount);
int32_t bet_dcv_invoice_pay(struct privatebet_info *bet, struct privatebet_vars *vars, int playerid, int amount);
void bet_dcv_paymentloop(void *_ptr);

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t bet_player_create_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars,
				  char *deckid);
int32_t bet_player_create_invoice_request(cJSON *argjson, struct privatebet_info *bet, int32_t amount);
int32_t bet_player_invoice_pay(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int amount);
void bet_player_paymentloop(void *_ptr);

int32_t bet_player_invoice_request(cJSON *argjson, cJSON *actionResponse, struct privatebet_info *bet, int32_t amount);
int32_t bet_player_log_bet_info(cJSON *argjson, struct privatebet_info *bet, int32_t amount, int32_t action);
