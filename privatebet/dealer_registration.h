#ifndef DEALER_REGISTRATION_H
#define DEALER_REGISTRATION_H

#include "bet.h"

// Constants for dealer registration
#define DEALER_REGISTRATION_FEE 1.0 // 1 CHIPS registration fee
#define DEALER_DEREGISTRATION_COMMISSION 0.05 // 5% commission on deregistration

// Function declarations
int32_t register_dealer(char *dealer_id);
int32_t deregister_dealer(char *dealer_id);
bool is_dealer_registered(char *dealer_id);
double get_dealer_registration_fee(void);
double get_dealer_deregistration_commission(void);
int32_t process_dealer_registration_tx(cJSON *tx_data);
int32_t raise_dealer_registration_dispute(char *dealer_id, char *dispute_action);

#endif // DEALER_REGISTRATION_H