#ifndef BET_NETWORK_H_
#define BET_NETWORK_H_

#include "bet.h"

// Define constants here
#define BET_TCP_PREFIX "tcp://"
#define BET_UDP_PREFIX "udp://"

// TODO: convert to packed alignement
typedef struct _bsocket {
     int32_t sock;
     int32_t bindflag;
     char * endpoint;
     int32_t nntype;
     int snd_timeout;
     int rcv_timeout;
} __attribute__((aligned)) bsocket_t;

const char * bet_get_etho_ip();

bstatus_t bet_nanosock(const bsocket_t * socket);

b_status_t * bet_msg_dealer_with_response_id(cJSON * argjson,
					     cJSON * response_info,
					     const char * dealer_ip,
					     const char * message);

cJSON *bet_msg_dealer_with_response_id(cJSON *argjson, char *dealer_ip, char *message);


#endif // BET_NETWORK_H_
