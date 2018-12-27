/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/

int32_t BET_p2p_initiate_statemachine(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_do_blinds(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);

int32_t BET_DCV_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_big_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_small_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_round_betting_response(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_next_turn(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);

int32_t BET_p2p_big_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_DCV_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);


/***************************************************************
Here contains the functions which are common to all nodes
****************************************************************/
int32_t BET_p2p_betting_statemachine(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_display_current_state(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);


/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t BET_player_small_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_player_big_blind_bet(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_dealer_info(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_p2p_small_blind(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_player_round_betting(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_player_round_betting_response(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);





