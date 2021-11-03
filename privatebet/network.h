#include "bet.h"
char *bet_get_etho_ip();
char *bet_tcp_sock_address(int32_t bindflag, char *str, char *ipaddr, uint16_t port);
int32_t bet_nanosock(int32_t bindflag, char *endpoint, int32_t nntype);
cJSON *bet_msg_dealer_with_response_id(cJSON *argjson, char *dealer_ip, char *message);
int32_t bet_send_data(int32_t socket, char *data, int32_t length);
