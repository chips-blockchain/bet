#include "bet.h"
#include "cashier.h"
#include "network.h"
#include "common.h"
#include "commands.h"

int32_t no_of_notaries = 4;
int32_t threshold_value = 2;

char notary_node_addrs[][64] = { "bQepVNtzfjMaBJdaaCq68trQDAPDgKnwrD", "bSa7CrTXykfPZ6yhThjXAoQ8r4H7muiPPC",
				 "bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ", "bR7BXnWT1yVSP9aB57pq22XN2WYNpGgDrD" };
/*

*/
char **notary_node_ips = NULL; //{ "159.69.23.28", "159.69.23.29", "159.69.23.30", "159.69.23.31" };

char **notary_node_pubkeys = NULL; /*{ "034d2b213240cfb4efcc24cc21a237a2313c0c734a4f0efc30087c095fd010385f",
				   "02137b5400ace827c225238765d4661a1b4fe589b9b625b10469c69f0867f7bc53",
				   "03b020866c9efae106e3c086a640e8b50cce7ae91cb30996ecf0f8816ce5ed8f49",
				   "0274ae1ce244bd0f9c52edfb6b9e60dc5d22f001dd74af95d1297edbcc8ae39568" };*/

struct cashier *cashier_info = NULL;
int32_t live_notaries = 0;
int32_t *notary_status = NULL;

double table_stack_in_chips = 0.01;
double chips_tx_fee = 0.0005;

char dev_fund_addr[64] = { "RSdMRYeeouw3hepxNgUzHn34qFhn1tsubb" }; // donation Address

char legacy_2_of_3_msig_addr[64] = { "bQJTo8knsbSoU7k9oGADa6qfWGWyJtxC3o" };
char legacy_2_of_4_msig_addr[64] = { "bRCUpox55j6sFJBuEn9E1fwNLFKFvRvo9W" };
char *legacy_m_of_n_msig_addr = NULL;

void bet_compute_m_of_n_msig_addr()
{
	cJSON *msig_addr = NULL;
	msig_addr = chips_add_multisig_address();
	if (msig_addr) {
		legacy_m_of_n_msig_addr = (char *)malloc(strlen(jstr(msig_addr, "address")) + 1);
		memset(legacy_m_of_n_msig_addr, 0x00, strlen(jstr(msig_addr, "address")) + 1);
		strncpy(legacy_m_of_n_msig_addr, jstr(msig_addr, "address"), strlen(jstr(msig_addr, "address")));
		if (chips_iswatchonly(legacy_m_of_n_msig_addr) == 0)
			chips_import_address(legacy_m_of_n_msig_addr);
	}
}

void bet_check_notaries()
{
	bet_check_notary_status();

	if (live_notaries < 2) {
		printf("Not enough notaries are available, if you continue you lose funds\n");
		exit(0);
	} else {
		printf("Notary node status\n");
		for (int i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1)
				printf("%d. %s active\n", i + 1, notary_node_ips[i]);
			else
				printf("%d. %s not active\n", i + 1, notary_node_ips[i]);
		}
	}
}

char *bet_check_notary_status()
{
	int32_t c_subsock, c_pushsock;
	uint16_t cashier_pubsub_port = 7901, cashier_pushpull_port = 7902;
	char bind_sub_addr[128] = { 0 }, bind_push_addr[128] = { 0 };
	pthread_t cashier_thrd[no_of_notaries];
	struct cashier *cashier_info = NULL;

	cashier_info = calloc(1, sizeof(struct cashier));

	live_notaries = 0;
	notary_status = (int *)malloc(no_of_notaries * sizeof(int));
	for (int i = 0; i < no_of_notaries; i++) {
		notary_status[i] = 0;
	}

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
#if 0
	if (live_notaries > 0) {
		printf("Notary node status\n");
		for (int i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1)
				printf("%d. %s active\n", i+1, notary_node_ips[i]);
			else
				printf("%d. %s not active\n", i+1, notary_node_ips[i]);
		}
#if 0
		int choice;
	top:
		printf("Enter your choice::\n");
		scanf("%d", &choice);
		if (((choice >= 0) && (choice < no_of_notaries)) && (notary_status[choice] == 1)) {
			return notary_node_ips[choice];
		} else
			goto top;
#endif
	}
#endif
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
		} else if (strcmp(method, "raw_msig_tx") == 0) {
			cJSON *signed_tx = NULL;
			signed_tx = cJSON_CreateObject();
			cJSON_AddStringToObject(signed_tx, "method", "signed_tx");
			char *tx = cJSON_Print(cJSON_GetObjectItem(argjson, "tx"));
			cJSON_AddItemToObject(signed_tx, "signed_tx", chips_sign_raw_tx_with_wallet(tx));
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(signed_tx));
			int bytes = nn_send(cashier_info->c_pubsock, cJSON_Print(signed_tx),
					    strlen(cJSON_Print(signed_tx)), 0);
			if (bytes < 0)
				retval = -1;
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
		} else if (strcmp(method, "signed_tx") == 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
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

int32_t bet_submit_msig_raw_tx(cJSON *tx)
{
	cJSON *msig_raw_tx = NULL;
	int32_t bytes, retval = 0;

	msig_raw_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(msig_raw_tx, "method", "raw_msig_tx");
	cJSON_AddItemToObject(msig_raw_tx, "tx", tx);

	if (cashier_info->c_pushsock > 0) {
		bytes = nn_send(cashier_info->c_pushsock, cJSON_Print(msig_raw_tx), strlen(cJSON_Print(msig_raw_tx)),
				0);
		if (bytes < 0)
			retval = -1;
	}
	return retval;
}
