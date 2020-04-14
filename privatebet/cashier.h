void bet_check_notaries();
char *bet_check_notary_status();
int32_t bet_send_status(struct cashier *cashier_info);
int32_t bet_cashier_backend(cJSON *argjson, struct cashier *cashier_info);
void bet_cashier_server_loop(void *_ptr);
void bet_cashier_client_loop(void *_ptr);
void bet_cashier_status_loop(void *_ptr);
int32_t bet_submit_msig_raw_tx(cJSON *tx);
