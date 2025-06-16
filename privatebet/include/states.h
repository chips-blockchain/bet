#include "bet.h"
/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/

int32_t bet_initiate_statemachine(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);

int32_t bet_dcv_big_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_big_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_small_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_round_betting(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_round_betting_response(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_next_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars,
			  int32_t *next_turn);

int32_t bet_player_big_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_dcv_small_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);

/***************************************************************
Here contains the functions which are common to all nodes
****************************************************************/
int32_t bet_player_betting_statemachine(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_display_current_state(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t bet_player_small_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_big_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_dealer_info(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_small_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
int32_t bet_player_round_betting_response(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);

int32_t bet_player_round_betting(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars);
