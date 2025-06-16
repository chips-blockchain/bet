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

#include "network.h"
#include "bet.h"
#include "cards777.h"
#include "common.h"
#include "gfshare.h"
#include "err.h"

char *bet_get_etho_ip()
{
	struct ifreq ifr;
	int fd;
	unsigned char ip_address[15];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	strcpy((char *)ip_address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	return (inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

char *bet_tcp_sock_address(int32_t bindflag, char *str, char *ipaddr, uint16_t port)
{
	sprintf(str, "tcp://%s:%u", bindflag == 0 ? ipaddr : "*",
		port); // ws is worse
	return (str);
}

int32_t bet_nanosock(int32_t bindflag, char *endpoint, int32_t nntype)
{
	int32_t sock, timeout, err = OK;
	if ((sock = nn_socket(AF_SP, nntype)) >= 0) {
		if (bindflag == 0) {
			if (nn_connect(sock, endpoint) < 0) {
				dlg_error("connect to %s error for %s\n", endpoint, nn_strerror(nn_errno()));
				nn_close(sock);
				return (-1);
			}
		} else {
			if (nn_bind(sock, endpoint) < 0) {
				err = ERR_PORT_BINDING;
				dlg_error("bind to %s error for %s\n", endpoint, nn_strerror(nn_errno()));
				nn_close(sock);
				return (-1);
			} else
				dlg_info("Network endpoints of this node :: (%s)", endpoint);
		}
		timeout = 1;
		nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(timeout));
		timeout = 100;
		nn_setsockopt(sock, NN_SOL_SOCKET, NN_SNDTIMEO, &timeout, sizeof(timeout));
		if (nntype == NN_SUB)
			nn_setsockopt(sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
	}
	return (sock);
}

cJSON *bet_msg_dealer_with_response_id(cJSON *argjson, char *dealer_ip, char *message)
{
	int32_t c_subsock, c_pushsock, bytes, recvlen;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	void *ptr;
	cJSON *response_info = NULL;

	memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
	memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

	if ((dealer_ip == NULL) || (strlen(dealer_ip) == 0)) {
		dlg_info("Invalid dealer IP");
		return NULL;
	}

	bet_tcp_sock_address(0, bind_sub_addr, dealer_ip, dealer_bvv_pub_sub_port);
	c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

	bet_tcp_sock_address(0, bind_push_addr, dealer_ip, dealer_bvv_push_pull_port);
	c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

	dlg_info("c_pushsock::%d, c_subsock ::%d, data::%s", c_pushsock, c_subsock, cJSON_Print(argjson));

	bytes = nn_send(c_pushsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
	if (bytes < 0) {
		return NULL;
	} else {
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
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}

	nn_close(c_pushsock);
	nn_close(c_subsock);

	return response_info;
}

int32_t bet_send_data(int32_t socket, char *data, int32_t length)
{
	int32_t bytes_sent;

	bytes_sent = nn_send(socket, data, length, 0);
	if (bytes_sent < 0)
		return -1;
	else
		return 0;
}
