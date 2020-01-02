#include "bet.h"
#include "cashier.h"
#include "network.h"
#include "common.h"

int32_t no_of_notaries = 4;
char notary_node_addrs[][64] = { "bQepVNtzfjMaBJdaaCq68trQDAPDgKnwrD", "bSa7CrTXykfPZ6yhThjXAoQ8r4H7muiPPC",
				 "bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ", "bR7BXnWT1yVSP9aB57pq22XN2WYNpGgDrD" };
char notary_node_ips[][25] = { "159.69.23.28", "159.69.23.29", "159.69.23.30", "159.69.23.31" };
char msig_addr[64] = { "bQJTo8knsbSoU7k9oGADa6qfWGWyJtxC3o" };

struct cashier *cashier_info = NULL;
int32_t live_notaries = 0;
int32_t notary_status[4] = { 0 };
char *bet_check_notary_status()
{
	int32_t c_subsock, c_pushsock;
	uint16_t cashier_pubsub_port = 7901, cashier_pushpull_port = 7902;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	pthread_t cashier_thrd[no_of_notaries];
	struct cashier *cashier_info = NULL;

	cashier_info = calloc(1, sizeof(struct cashier));

	for (int i = 0; i < no_of_notaries; i++) {
		int temp = live_notaries;
		memset(cashier_info, 0x00, sizeof(struct cashier));
		memset(bind_sub_addr, 0x00, sizeof(bind_sub_addr));
		memset(bind_push_addr, 0x00, sizeof(bind_push_addr));

		bet_tcp_sock_address(0, bind_sub_addr, notary_node_ips[i], cashier_pubsub_port);
		c_subsock = bet_nanosock(0, bind_sub_addr, NN_SUB);

		bet_tcp_sock_address(0, bind_push_addr, notary_node_ips[i], cashier_pushpull_port);
		c_pushsock = bet_nanosock(0, bind_push_addr, NN_PUSH);

		cashier_info = calloc(1, sizeof(struct cashier));

		cashier_info->c_subsock = c_subsock;
		cashier_info->c_pushsock = c_pushsock;

		if (OS_thread_create(&cashier_thrd[i], NULL, (void *)bet_cashier_status_loop, (void *)cashier_info) !=
		    0) {
			printf("\nerror in launching cashier");
			exit(-1);
		}

		if (pthread_join(cashier_thrd[i], NULL)) {
			printf("\nError in joining the main thread for cashier");
		}

		if ((temp + 1) == live_notaries)
			notary_status[i] = 1;
	}
	if (live_notaries > 0) {
		printf("Below notaries are live, you can choose one\n");
		for (int i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1)
				printf("%d. %s\n", i, notary_node_ips[i]);
		}

		int choice;
	top:
		printf("Enter your choice::\n");
		scanf("%d", &choice);
		if (((choice >= 0) && (choice < no_of_notaries)) && (notary_status[choice] == 1)) {
			return notary_node_ips[choice];
		} else
			goto top;
	}
	return NULL;
}

int32_t bet_send_status(struct cashier *cashier_info)
{
	int32_t retval = 1, bytes;
	cJSON *live_info = NULL;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_cashier_backend(cJSON *argjson, struct cashier *cashier_info)
{
	char *method = NULL;
	int retval = 1;

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	if ((method = jstr(argjson, "method")) != 0) {
		if (strcmp(method, "live") == 0) {
			retval = bet_send_status(cashier_info);
		}
	}
	return retval;
}

static int32_t bet_cashier_client_backend(cJSON *argjson, struct cashier *cashier_info)
{
	char *method = NULL;
	int retval = 1;

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	if ((method = jstr(argjson, "method")) != 0) {
		if (strcmp(method, "live") == 0) {
			retval = bet_send_status(cashier_info);
		}
	}
	return retval;
}

void bet_cashier_status_loop(void *_ptr)
{
	int32_t recvlen = 0, bytes;
	void *ptr = NULL;
	cJSON *argjson = NULL;
	struct cashier *cashier_info = _ptr;
	cJSON *live_info = NULL;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	bytes = nn_send(cashier_info->c_pushsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);

	if (bytes < 0)
		printf("%s::%d::Failed to send data\n", __FUNCTION__, __LINE__);
	else {
		while (cashier_info->c_pushsock >= 0 && cashier_info->c_subsock >= 0) {
			ptr = 0;
			if ((recvlen = nn_recv(cashier_info->c_subsock, &ptr, NN_MSG, 0)) > 0) {
				char *tmp = clonestr(ptr);
				if ((argjson = cJSON_Parse(tmp)) != 0) {
					if (strcmp(jstr(argjson, "method"), "live") == 0)
						live_notaries++;

					free_json(argjson);
					break;
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
}

void bet_cashier_client_loop(void *_ptr)
{
	int32_t recvlen = 0;
	void *ptr = NULL;
	cJSON *argjson = NULL;
	struct cashier *cashier_info = _ptr;

	printf("%s::%d::cahsier client started\n", __FUNCTION__, __LINE__);
	while (cashier_info->c_pushsock >= 0 && cashier_info->c_subsock >= 0) {
		ptr = 0;
		if ((recvlen = nn_recv(cashier_info->c_subsock, &ptr, NN_MSG, 0)) > 0) {
			char *tmp = clonestr(ptr);
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				if (bet_cashier_client_backend(argjson, cashier_info) < 0) {
					printf("\nFAILURE\n");
					// do something here, possibly this could be because unknown commnad or because of encountering a special case which state machine fails to handle
				}

				free_json(argjson);
			}
			if (tmp)
				free(tmp);
			if (ptr)
				nn_freemsg(ptr);
		}
	}
end:
	printf("\nThere is a problem in sending data::%s::%d\n", __FUNCTION__, __LINE__);
}

void bet_cashier_server_loop(void *_ptr)
{
	int32_t recvlen = 0;
	void *ptr = NULL;
	cJSON *msgjson = NULL;
	struct cashier *cashier_info = _ptr;
	uint8_t flag = 1;

	printf("cashier server node started\n");
	while (flag) {
		if (cashier_info->c_pubsock >= 0 && cashier_info->c_pullsock >= 0) {
			ptr = 0;
			char *tmp = NULL;
			recvlen = nn_recv(cashier_info->c_pullsock, &ptr, NN_MSG, 0);
			if (recvlen > 0)
				tmp = clonestr(ptr);
			if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
				if (bet_cashier_backend(msgjson, cashier_info) < 0) {
					printf("\nFAILURE\n");
					// do something here, possibly this could be because unknown commnad or because of encountering a special case which state machine fails to handle
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
}
