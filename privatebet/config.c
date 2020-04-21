#include "config.h"
#include "common.h"

char *dealer_config_file = "./config/dealer_config.json";
char *notaries_file = "./config/notaries.json";

cJSON *bet_read_json_file(char *file_name)
{
	FILE *fp = NULL;
	cJSON *json_data = NULL;
	char *data = NULL, buf[256];
	unsigned long data_size = 1024, buf_size = 256, temp_size = 0;
	unsigned long new_size = data_size;

	data = calloc(data_size, sizeof(char));
	if (!data) {
		goto end;
	}

	fp = fopen(file_name, "r");
	if (fp == NULL) {
		printf("%s::%d::Failed to open %s\n", __FUNCTION__, __LINE__, file_name);
		goto end;
	} else {
		while (fgets(buf, buf_size, fp) != NULL) {
			temp_size = temp_size + strlen(buf);
			if (temp_size >= new_size) {
				char *temp = calloc(new_size, sizeof(char));
				strncpy(temp, data, strlen(data));
				free(data);
				new_size = new_size * 2;
				data = calloc(new_size, sizeof(char));
				strncpy(data, temp, strlen(temp));
				free(temp);
				printf("MEMORY DOUBLED:: IN READING :: %s :: NEEDING MORE MEMORY\n", file_name);
			}
			strcat(data, buf);
			memset(buf, 0x00, buf_size);
		}
		json_data = cJSON_CreateObject();
		json_data = cJSON_Parse(data);
	}
end:
	if (data)
		free(data);
	return json_data;
}

void bet_parse_config_file()
{
	cJSON *config_info = NULL;

	config_info = bet_read_json_file(dealer_config_file);
	if (config_info) {
		max_players = jint(config_info, "max_players");
		table_stack_in_chips = jdouble(config_info, "table_stack_in_chips");
		chips_tx_fee = jdouble(config_info, "chips_tx_fee");
	}
}

void bet_parse_notary_file()
{
	cJSON *notaries_info = NULL;

	notaries_info = bet_read_json_file(notaries_file);
	if (notaries_info) {
		no_of_notaries = cJSON_GetArraySize(notaries_info);
		notary_node_ips = (char **)malloc(no_of_notaries * sizeof(char *));
		notary_node_pubkeys = (char **)malloc(no_of_notaries * sizeof(char *));

		for (int32_t i = 0; i < no_of_notaries; i++) {
			cJSON *node_info = cJSON_CreateObject();
			node_info = cJSON_GetArrayItem(notaries_info, i);

			notary_node_ips[i] = (char *)malloc(strlen(jstr(node_info, "ip")) + 1);
			memset(notary_node_ips[i], 0x00, strlen(jstr(node_info, "ip")) + 1);

			notary_node_pubkeys[i] = (char *)malloc(strlen(jstr(node_info, "pubkey")) + 1);
			memset(notary_node_pubkeys[i], 0x00, strlen(jstr(node_info, "pubkey")) + 1);

			strncpy(notary_node_ips[i], jstr(node_info, "ip"), strlen(jstr(node_info, "ip")));
			strncpy(notary_node_pubkeys[i], jstr(node_info, "pubkey"), strlen(jstr(node_info, "pubkey")));
		}
	}
}

