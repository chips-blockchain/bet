#ifndef PRIVATEBET_NET_H_
#define PRIVATEBET_NET_H_

#include "bet.h"

// If system does'nt support 'eth0' query the system to find the
// available interfaces
#define DEFAULT_BET_INTF "eth0"

#define BET_TCP_PREFIX "tcp://"
#define BET_UDP_PREFIX "udp://"

b_status_t bet_get_intf_ip(const char * intf);

// These APIs would be deprecated soon, warn the user

int32_t bet_nanosock(int32_t bindflag, char *endpoint, int32_t nntype)  \
    __attribute__ ((deprecated("PLEASE USE STRUCTURE BASED IMPLEMENTATION OF THIS API")));

b_status_t bet_nanosock(b_socket_t * sock);

// These APIs would be deprecated soon, warn the user
cJSON *bet_msg_dealer_with_response_id(cJSON *argjson, char *dealer_ip, char *message) \
    __attribute__ ((deprecated("PLEASE USE MODIFIED IMPLEMENTATION OF THE API")));

b_status_t *bet_msg_dealer_with_response_id(cJSON *argjson, cJSON * resp_info, char *dealer_ip, char *message);

#endif // PRIVATEBET_NET_H_
