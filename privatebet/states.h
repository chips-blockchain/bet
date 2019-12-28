#include "bet.h"
/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/

int32_t BET_p2p_initiate_statemachine(cJSON *argjson,
                                      struct privatebet_info *bet,
                                      struct privatebet_vars *vars);

int32_t BET_DCV_big_blind_bet(cJSON *argjson, struct privatebet_info *bet,
                              struct privatebet_vars *vars);
int32_t BET_DCV_big_blind(cJSON *argjson, struct privatebet_info *bet,
                          struct privatebet_vars *vars);
int32_t BET_DCV_small_blind_bet(cJSON *argjson, struct privatebet_info *bet,
                                struct privatebet_vars *vars);
int32_t BET_DCV_round_betting(cJSON *argjson, struct privatebet_info *bet,
                              struct privatebet_vars *vars);
int32_t BET_DCV_round_betting_response(cJSON *argjson,
                                       struct privatebet_info *bet,
                                       struct privatebet_vars *vars);
int32_t BET_DCV_next_turn(cJSON *argjson, struct privatebet_info *bet,
                          struct privatebet_vars *vars);

int32_t BET_p2p_big_blind(cJSON *argjson, struct privatebet_info *bet,
                          struct privatebet_vars *vars);
int32_t BET_DCV_small_blind(cJSON *argjson, struct privatebet_info *bet,
                            struct privatebet_vars *vars);

/***************************************************************
Here contains the functions which are common to all nodes
****************************************************************/
int32_t BET_p2p_betting_statemachine(cJSON *argjson,
                                     struct privatebet_info *bet,
                                     struct privatebet_vars *vars);
int32_t BET_p2p_display_current_state(cJSON *argjson,
                                      struct privatebet_info *bet,
                                      struct privatebet_vars *vars);

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t BET_player_small_blind_bet(cJSON *argjson, struct privatebet_info *bet,
                                   struct privatebet_vars *vars);
int32_t BET_player_big_blind_bet(cJSON *argjson, struct privatebet_info *bet,
                                 struct privatebet_vars *vars);
int32_t BET_p2p_dealer_info(cJSON *argjson, struct privatebet_info *bet,
                            struct privatebet_vars *vars);
int32_t BET_p2p_small_blind(cJSON *argjson, struct privatebet_info *bet,
                            struct privatebet_vars *vars);
int32_t BET_player_round_betting(cJSON *argjson, struct privatebet_info *bet,
                                 struct privatebet_vars *vars);
int32_t BET_player_round_betting_response(cJSON *argjson,
                                          struct privatebet_info *bet,
                                          struct privatebet_vars *vars);

int32_t BET_player_round_betting_test(cJSON *argjson,
                                      struct privatebet_info *bet,
                                      struct privatebet_vars *vars);
int32_t BET_rest_small_blind_update(struct lws *wsi, cJSON *argjson,
                                    int32_t amount);
int32_t BET_rest_big_blind_update(struct lws *wsi, cJSON *argjson,
                                  int32_t amount);
int32_t BET_rest_player_round_betting_update(struct lws *wsi, cJSON *argjson,
                                             int option, int32_t bet_amount);
int32_t BET_rest_DCV_small_blind(struct lws *wsi);
int32_t BET_rest_DCV_round_betting(struct lws *wsi);
int32_t BET_rest_initiate_statemachine(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_bvv_dealer_info(struct lws *wsi, cJSON *argjson);
int32_t BET_rest_player_dealer_info(struct lws *wsi, cJSON *argjson,
                                    int32_t playerID);
int32_t BET_rest_betting_statemachine(struct lws *wsi, cJSON *argjson);
