#ifndef DEALER_H
#define DEALER_H

#include "bet.h"

int32_t add_dealer(char *dealer_id, const char *aggregator_fqn);
int32_t dealer_table_init(struct table *t);
bool is_players_shuffled_deck(char *table_id);
int32_t dealer_shuffle_deck(char *id);
int32_t handle_game_state(struct table *t);
int32_t register_table(struct table t);
int32_t dealer_init(struct table t);
int32_t dealer_init_with_reset(struct table t);
int32_t dealer_reset_table(struct table *t);
int32_t dealer_initiate_settlement(struct table *t, struct privatebet_vars *vars);

#endif /* DEALER_H */
