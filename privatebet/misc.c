#include "misc.h"
#include "../includes/curl/curl.h"
#include "../includes/curl/easy.h"

int32_t hexstr_to_str(char *input, char *output)
{
	char code[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	int32_t loop, flag;
	int i;

	i = 0;
	loop = 0;
	int num = 0;
	while (input[loop] != '\0') {
		flag = 0;
		for (int32_t j = 0; j < 16; j++) {
			if (code[j] == input[loop]) {
				if ((loop % 2) == 0)
					num = j * 16;
				else {
					num = num + j;
					sprintf((char *)(output + i), "%c", num);
					i++;
					num = 0;
				}
				flag = 1;
				break;
			}
		}
		loop++;
		if (flag == 0)
			break;
	}
	output[i++] = '\0';
	return flag;
}

void str_to_hexstr(char *input, char *output)
{
	int loop;
	int i;

	i = 0;
	loop = 0;

	while (input[loop] != '\0') {
		sprintf((char *)(output + i), "%02x", input[loop]);
		loop++;
		i += 2;
	}
	output[i++] = '\0';
}

void delete_file(char *file_name)
{
	if (remove(file_name) != 0) {
		dlg_warn("Intermediate file %s is not removed", file_name);
	}
}

int check_url(const char *url)
{
	CURL *curl = NULL;
	CURLcode response;

	if ((url == NULL) || (strlen(url) == 0))
		return 0;

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		response = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	return (response == CURLE_OK) ? 1 : 0;
}

void float_to_uint32(uint32_t *s, uint32_t *m, uint32_t *e, float number)
{
	uint32_t *ptr = (uint32_t *)&number;

	*s = *ptr >> 31;
	*e = *ptr & 0x7f800000;
	*e >>= 23;
	*m = *ptr & 0x007fffff;
}

void float_to_uint32_s(struct float_num *t, float number)
{
	uint32_t s, e, m;

	uint32_t *ptr = (uint32_t *)&number;

	s = *ptr >> 31;
	e = *ptr & 0x7f800000;
	e >>= 23;
	m = *ptr & 0x007fffff;

	t->sign = s;
	t->mantisa = m;
	t->exponent = e;
}

float uint32_s_to_float(struct float_num t)
{
	uint32_t s, e, m;
	float number;

	uint32_t *ptr = (uint32_t *)&number;

	s = t.sign;
	m = t.mantisa;
	e = t.exponent;

	s <<= 31;
	e <<= 23;

	*ptr = s | e | m;

	return number;
}

void struct_to_byte_arr(const void *object, size_t size, uint8_t *out)
{
	const uint8_t *byte;
	for (byte = object; size--; ++byte) {
		*out = *byte;
		++out;
	}
}

void cJSON_hex(cJSON *argjson, char **hexstr)
{
	int32_t hex_data_len;

	hex_data_len = 2 * strlen(cJSON_Print(argjson)) + 1;
	*hexstr = calloc(hex_data_len, sizeof(char));
	str_to_hexstr(cJSON_Print(argjson), *hexstr);
}

cJSON *hex_cJSON(char *hex_data)
{
	char *data = NULL;
	cJSON *out = NULL;

	if (hex_data == NULL)
		return NULL;

	data = calloc(1, strlen(hex_data));
	hexstr_to_str(hex_data, data);

	out = cJSON_CreateObject();
	out = cJSON_Parse(data);

	if (data)
		free(data);

	return out;
}

cJSON *struct_table_to_cJSON(struct table *t)
{
	cJSON *table_info = NULL;

	table_info = cJSON_CreateObject();
	if (t) {
		cJSON_AddNumberToObject(table_info, "max_players", t->max_players);
		cJSON_AddNumberToObject(table_info, "big_blind", uint32_s_to_float(t->big_blind));
		cJSON_AddNumberToObject(table_info, "min_stake", uint32_s_to_float(t->min_stake));
		cJSON_AddNumberToObject(table_info, "max_stake", uint32_s_to_float(t->max_stake));
		cJSON_AddStringToObject(table_info, "table_id", t->table_id);
		cJSON_AddStringToObject(table_info, "dealer_id", t->dealer_id);
	}
	return table_info;
}
