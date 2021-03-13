/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "error.h"
#include "network.h"
#include "bet.h"
#include "cards777.h"
#include "common.h"
#include "gfshare.h"

const char * bet_get_etho_ip()
{
     struct ifreq ifr;
     int fd;
     unsigned char ip_address[15];

     fd = socket(AF_INET, SOCK_DGRAM, 0);
     ifr.ifr_addr.sa_family = AF_INET;
     memcpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
     ioctl(fd, SIOCGIFADDR, &ifr);
     close(fd);
     strcpy(ip_address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
     return (inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

// Return ERRVAL here i.e convert nanosock errval to beterrval
bstatus_t bet_nanosock(const bsocket_t * socket)
{
     if (socket == NULL)
	  goto err;

     if ((socket->sock = nn_socket(AF_SP, socket->nntype)) >= 0) {
	  if (socket->bindflag == 0) {
	       if (nn_connect(sock->sock, socket->endpoint) < 0) {
		    printf("connect to %s error for %s\n", endpoint, nn_strerror(nn_errno()));
		    nn_close(socket->sock);
		    return B_CONNECT_ERR;
	       } /*else
		   printf("nntype.%d connect to %s connectsock.%d\n", nntype, endpoint, sock);
		 */
	  } else {
	       if (nn_bind(socket->sock, socket->endpoint) < 0) {
		    printf("bind to %s error for %s\n", socket->endpoint, nn_strerror(nn_errno()));
		    nn_close(socket->sock);
		    return B_CONNECT_ERR;
	       } else
		    printf("(%s) bound\n", endpoint);
	  }
	  nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVTIMEO, &socket->snd_timeout, sizeof(timeout));
	  nn_setsockopt(sock, NN_SOL_SOCKET, NN_SNDTIMEO, &socket->rcv_timeout, sizeof(timeout));
	  // maxsize = 2 * 1024 * 1024;
	  // nn_setsockopt(sock,NN_SOL_SOCKET,NN_RCVBUF,&maxsize,sizeof(maxsize));
	  if (nntype == NN_SUB)
	       nn_setsockopt(socket->sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
     }
     return B_SUCCESS;
}

b_status_t bet_msg_dealer_with_response_id(cJSON * argjson,
					   cJSON * response_info,
					   const char * dealer_ip,
					   const char * message)
{
     int32_t c_subsock, c_pushsock, bytes, recvlen;
     uint16_t dealer_pubsub_port = 7797, dealer_pushpull_port = 7797 + 1;
     char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
     void *ptr;

     memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
     memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

     sprintf(bind_sub_addr, "%s%s:%u", BET_TCP_PREFIX,
	     (bindflags == 0 ? dealer_ip : "*"),
	     dealder_pubsub_port);

     c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

     sprintf(bind_push_addr, "%s%s:%u", BET_TCP_PREFIX,
	     (bindflags == 0 ? dealer_ip : "*"),
	     dealer_pushpull_port);

     c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

     if (bytes = nn_send(c_pushsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0) < 0)
	  return B_SEND_ERR;
     else {
	  while (c_pushsock >= 0 && c_subsock >= 0) {
	       ptr = 0;
	       if ((recvlen = nn_recv(c_subsock, &ptr, NN_MSG, 0)) > 0) {
		    char *tmp = clonestr(ptr);
		    if ((response_info = cJSON_Parse(tmp)) != 0) {
			 if ((strcmp(jstr(argjson, "method"), message) == 0) &&
			     (strcmp(jstr(argjson, "id"), unique_id) == 0)) {
			      break;
			 }
		    }
		    if (tmp)
			 free(tmp);
		    if (ptr)
			 nn_freemsg(ptr);
	       }
	  }
     }

     nn_close(c_pushsock);
     nn_close(c_subsock);

     return B_SUCCESS;
}
